# author: Penn Su
import tornado.ioloop
import tornado.web
import tornado.template as template
import os, sys, zipfile
import simplejson as json
import hashlib
from xml.dom.minidom import parse
from threading import Thread
import time
import re
import StringIO
import shutil, errno
from subprocess import Popen, PIPE, STDOUT

sys.path.append(os.path.abspath("../tools/python"))
import wkpfcomm

if len(sys.argv) == 2:
  IP = sys.argv[1]
else:
  IP = ''
#ALLOWED_EXTENSIONS = set(['bog'])
#TARGET = 'HAScenario1'
#XML_PATH = os.path.join(ROOT_PATH, 'Applications')
ROOT_PATH = os.path.abspath('..')
APP_DIR = os.path.join(ROOT_PATH, 'vm', 'apps')
BASE_DIR = os.path.join(APP_DIR, 'base')
IP = '127.0.0.1'
if len(sys.argv) >= 2:
  IP = sys.argv[1]

NORMAL = 'NORMAL'
URGENT = 'URGENT'
CRITICAL = 'CRITICAL'

OK = 0
NOTOK = 1

applications = []
worker = None

# Helper functions
def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

def copyAnything(src, dst):
  try:
    shutil.copytree(src, dst)
  except OSError as exc: # python >2.5
    if exc.errno == errno.ENOTDIR:
      shutil.copy(src, dst)
    else: raise

def getAppIndex(app_id):
  global applications
  # make sure it is not unicode
  app_id = app_id.encode('ascii','ignore')
  for index, app in enumerate(applications):
    if app.id == app_id:
      return index
  return None

def delete_application(i):
  global applications
  try:
    shutil.rmtree(applications[i].dir)
    #os.system('rm -rf ' + applications[i].dir)
    applications.pop(i)
    return True
  except Exception as e:
    print e
    print False

def load_app_from_dir(dir):
  app = Application(dir=dir)
  app.loadConfig()
  return app

def update_applications():
  global applications
  print 'update applications'

  application_basenames = [os.path.basename(app.dir) for app in applications]

  for dirname in os.listdir(APP_DIR):
    if dirname.lower() == 'base': continue
    app_dir = os.path.join(APP_DIR, dirname)
    if dirname not in application_basenames:
      print 'not found'
      print repr(dirname)
      print repr(application_basenames)
      applications.append(load_app_from_dir(app_dir))
      application_basenames = [os.path.basename(app.dir) for app in applications]

class Worker:
  # Deprecated?
  def bog_compiler(self, app_path, bog, target, platforms):
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)
      os.chdir(platform_dir)
      z = zipfile.ZipFile(bog)
      z.extract('file.xml')
      # compile to wukong xml and put it in app_path
      os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', target, platform_dir))
      os.system('rm file.xml')
      # TODO: return error for comamnds
      os.system('make application FLOWXML=%s DISCOVERY_FLAGS=-H' % (target))

  # xml is the wukong xml string
  # target is the target application name (e.g. HAScenario1)
  # platforms is a list of platforms to compile on (e.g. ['avr_mega2560'])
  def compiler(self, app, platforms):
    app.clearLog()
    app_path = app.dir
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)
      os.chdir(platform_dir)
      print 'changing to path: %s...' % platform_dir
      pp = Popen('make application FLOWXML=%s DISCOVERY_FLAGS=-H' % (app.id), shell=True, stdout=PIPE, stderr=STDOUT)
      app.status = -1
      while pp.poll() == None:
        print 'polling from popen...'
        line = pp.stdout.readline()
        if line != '':
          app.appendCompileLog(line, NORMAL)
        app.version += 1
      app.status = pp.returncode
      print 'compiler done'

  # target is the target application name (e.g. HAScenario1)
  # platforms is a list of platforms to compile on (e.g. ['avr_mega2560'])
  # XML_PATH is the output of the compiled wukong xml
  def deployer(self, app, ports, platforms):
    app.clearLog()
    app_path = app.dir
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)
      os.chdir(platform_dir)
      print 'changing to path: %s...' % platform_dir
      for port in ports:
        print 'deploying to node: %d' % (port)
        pp = Popen('make nvmcomm_reprogram NODE_ID=%d' % (port), shell=True, stdout=PIPE, stderr=STDOUT)
        app.status = -1
        while pp.poll() == None:
          print 'polling from popen...'
          line = pp.stdout.readline()
          if line != '':
            app.appendCompileLog(line, NORMAL)
          app.version += 1
        app.status = pp.returncode
      print 'deployer done'

  # Deprecated
  def factory(self, file, status):
    status.append(1)
    print status
    z = zipfile.ZipFile(file)
    z.extract('file.xml')
    status.append(2)
    print status
    os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', TARGET, XML_PATH))
    status.append(3)
    print status
    os.chdir('../vm/build/avr_mega2560/')
    os.system('make generate')
    os.system('make FLOWXML=%s DISCOVERY_FLAGS=-H' % (TARGET))
    os.system('make avrdude')
    status.append(4)
    print 'done'


class Application:
  def __init__(self, id='', name='', desc='', file='', dir=''):
    self.id = id
    self.name = name
    self.desc = desc
    self.file = file
    self.xml = ''
    self.dir = dir
    self.compiler = None
    self.deployer = None
    self.version = 0
    self.status = NOTOK

  def appendCompileLog(self, line, tag):
    print 'appendCompileLog'
    if not os.path.exists(os.path.join(self.dir, 'compile.log')):
      curpath = os.path.abspath(os.curdir)
      f = open(os.path.join(self.dir, 'compile.log'), 'w')
    else:
      f = open(os.path.join(self.dir, 'compile.log'), 'a')
    f.write("[%s] %s" % (tag, line))
    f.close

  def clearLog(self):
    print 'clearLog'
    if os.path.exists(os.path.join(self.dir, 'compile.log')):
      try:
        os.remove(os.path.join(self.dir, 'compile.log'))
      except Exception as e:
        print e

  # the format for the log should be [TAG] Detailed messages for the bug.
  # filter matches the corresponding TAG
  # TAG = NORMAL, URGENT, CRITICAL
  def compileLog(self, filter):
    print 'compileLog'
    lines = []
    if os.path.exists(os.path.join(self.dir, 'compile.log')):
      f = open(os.path.join(self.dir, 'compile.log'))
      for line in f.readlines():
        line = line.strip()
        m = re.search('^\[(.*)\]', line)
        if m and m.group(1) == filter:
          lines.append(line)
    return lines

  # the format for the log should be [TAG] Detailed messages for the bug.
  # filter matches the corresponding TAG
  # TAG = NORMAL, URGENT, CRITICAL
  def deployLog(self, filter):
    print 'deployLog'
    lines = []
    if os.path.exists(os.path.join(self.dir, 'deploy.log')):
      f = open(os.path.join(self.dir, 'deploy.log'))
      for line in f.readlines():
        line = line.strip()
        m = re.search('^\[(.*)\]', line)
        if m.group(1) == filter:
          lines.append(line)
    return lines

  def updateXML(self, xml):
    print 'updateConfig'
    self.xml = xml
    self.saveConfig()
    self.clearLog()
    f = open(os.path.join(self.dir, self.id + '.xml'), 'w')
    f.write(xml)
    f.close()

  def loadConfig(self):
    print 'loadConfig'
    config = json.load(open(os.path.join(self.dir, 'config.json')))
    self.id = config['id']
    self.name = config['name']
    self.desc = config['desc']
    self.dir = config['dir']
    self.xml = config['xml']

  def saveConfig(self):
    print 'saveConfig'
    json.dump(self.config(), open(os.path.join(self.dir, 'config.json'), 'w'))

  def getStatus(self):
    return statusString(self.status)

  def config(self):
    return {'id': self.id, 'name': self.name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version}

  def __repr__(self):
    return json.dumps(self.config())

# List all uploaded applications
class main(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/application.html')

class list_applications(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/index.html', applications=applications)

  def post(self):
    global applications
    update_applications()
    apps = [application.config() for application in applications]
    #apps = [{'id': application.id, 'name': application.name, 'desc': application.desc, 'status': application.status} for application in applications]
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

  '''
  def post(self):
    global applications

    if not self.get_argument('name') or not self.request.files['bog_file']:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'name or bog file is missing, please fill in the information'})
    else:
      received = self.request.files['bog_file'][0]
      file = StringIO.StringIO()
      file.write(received['body'])
      filename = received['filename']

      if file and allowed_file(filename):
        applications.append(Application(self.get_argument('name'), self.get_argument('desc'), self.factory, file))
        self.content_type = 'application/json'
        self.write({'status':0, 'id':len(applications)-1})
      else:
        self.content_type = 'application/json'
        self.write({'status':1})
  '''

# Returns a form to upload new application
class new_application(tornado.web.RequestHandler):
  def post(self):
    global applications
    #self.redirect('/applications/'+str(applications[-1].id), permanent=True)
    #self.render('templates/upload.html')
    try:
      app_name = 'application' + str(len(applications))
      app_id = hashlib.md5(app_name).hexdigest()

      app = Application(id=app_id, name=app_name, dir=os.path.join(APP_DIR, app_id))
      applications.append(app)

      # copy base for the new application
      print 'setting up app directory from base...'
      copyAnything(BASE_DIR, app.dir)

      print app

      # dump config file to app
      app.saveConfig()

      self.content_type = 'application/json'
      self.write({'status':0, 'app': app.config()})
    except Exception as e:
      print e
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg':'Cannot create application'})

class application(tornado.web.RequestHandler):
  # Deprecated
  '''
  def get(self, app_id):
    global applications
    self.render('templates/display.html', app_id=app_id, application=applications[int(app_id)])
  '''

  # Display a specific application
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      app = applications[app_ind].config()
      #app = {'name': applications[app_ind].name, 'desc': applications[app_ind].desc, 'id': applications[app_ind].id}
      topbar = template.Loader('.').load('templates/topbar.html').generate(application=applications[app_ind])
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Update a specific application
  def put(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      print 'save_application'
      try:
        applications[app_ind].name = self.get_argument('name', 'application name')
        applications[app_ind].desc = self.get_argument('desc', '')
        applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0})
      except Exception as e:
        print e
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot save application'})

  # Destroy a specific application
  def delete(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      print 'delete_application'
      if delete_application(app_ind):
        self.content_type = 'application/json'
        self.write({'status':0})
      else:
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot delete application'})

class deploy_application(tornado.web.RequestHandler):
  def get(self, app_id):
    wkpfcomm.init(0, debug=True)
    node_ids = wkpfcomm.getNodeIds()

    # debug purpose
    #node_ids = [3, 4, 1, 2]

    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      deployment = template.Loader('.').load('templates/deployment.html').generate(app=applications[app_ind], node_ids=node_ids)
      self.content_type = 'application/json'
      self.write({'status':0, 'page': deployment})

  def post(self, app_id):
    global applications
    print self.request.arguments
    selected_node_ids = [int(id) for id in self.request.arguments.get('selected_node_ids[]')]
    print selected_node_ids
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp

      if len(selected_node_ids) > 0:
        applications[app_ind].deployer = Thread(target=Worker().deployer, args=(applications[app_ind], selected_node_ids, platforms))
        applications[app_ind].deployer.start()

      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class poll_deployer(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      while applications[app_ind].version == self.get_argument('version'):
        continue
      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version, 'deploy_status': applications[app_ind].status, 'normal': applications[app_ind].compileLog(NORMAL), 'error': {'critical': applications[app_ind].compileLog(CRITICAL), 'urgent': applications[app_ind].compileLog(URGENT)}})

class poll_fbp(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      while applications[app_ind].version == self.get_argument('version'):
        continue
      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version, 'compile_status': applications[app_ind].status, 'normal': applications[app_ind].compileLog(NORMAL), 'error': {'critical': applications[app_ind].compileLog(CRITICAL), 'urgent': applications[app_ind].compileLog(URGENT)}})

class save_fbp(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      applications[app_ind].updateXML(self.get_argument('xml'))
      # TODO: need platforms from fbp
      #platforms = self.get_argument('platforms')
      platforms = ['avr_mega2560']

      applications[app_ind].compiler = Thread(target=Worker().compiler, args=(applications[app_ind], platforms,))
      applications[app_ind].compiler.start()

      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class load_fbp(tornado.web.RequestHandler):
  def get(self, app_id):
    self.render('templates/fbp.html')
  
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'xml': applications[app_ind].xml})

class ex_testrtt(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    nodes = [int(id) for id in self.request.arguments.get('nodes[]')]
    # TODO:call testrtt functions, could be exposed by wkpfcomm
    if status:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot exclude this device.'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0})

class in_testrtt(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    nodes = [int(id) for id in self.request.arguments.get('nodes[]')]
    # TODO:call testrtt functions, could be exposed by wkpfcomm
    if status:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot include this device.'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0})

settings = dict(
  static_path=os.path.join(os.path.dirname(__file__), "static"),
  debug=True
)

app = tornado.web.Application([
  (r"/", main),
  (r"/main", main),
  (r"/testrtt/exclude", ex_testrtt),
  (r"/testrtt/include", in_testrtt),
  (r"/application/json", list_applications),
  (r"/application/new", new_application),
  (r"/application/([a-fA-F\d]{32})", application),
  (r"/application/([a-fA-F\d]{32})/deploy", deploy_application),
  (r"/application/([a-fA-F\d]{32})/deploy/poll", poll_deployer),
  (r"/application/([a-fA-F\d]{32})/fbp/save", save_fbp),
  (r"/application/([a-fA-F\d]{32})/fbp/load", load_fbp),
  (r"/application/([a-fA-F\d]{32})/fbp/poll", poll_fbp)
], IP, **settings)

if __name__ == "__main__":
  app.listen(5000)
  tornado.ioloop.IOLoop.instance().start()
