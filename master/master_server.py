# vim: ts=2 sw=2
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
import datetime
from subprocess import Popen, PIPE, STDOUT

sys.path.append(os.path.abspath("../tools/python"))
from wkapplication import WuApplication
from codegen import *
from wkpf import *
from wkpfcomm import Communication
from inspector import Inspector
import fakedata
sys.path.append(os.path.abspath("../tools/xml2java"))
from translator import Mapper

#ALLOWED_EXTENSIONS = set(['bog'])
#TARGET = 'HAScenario1'
#XML_PATH = os.path.join(ROOT_PATH, 'Applications')
ROOT_PATH = os.path.abspath('..')
TESTRTT_PATH = os.path.join(ROOT_PATH, 'tools', 'python', 'pyzwave')
APP_DIR = os.path.join(ROOT_PATH, 'vm', 'apps')
BASE_DIR = os.path.join(APP_DIR, 'base')
MASTER_IP = '10.3.36.231'
IP = sys.argv[1] if len(sys.argv) >= 2 else '127.0.0.1'


communication = None
def getComm():
  global communication
  if not communication:
    communication = Communication(0)
  return communication

applications = []

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
  app = WuApplication(dir=dir)
  app.loadConfig()
  return app

def update_applications():
  global applications
  print 'update applications'

  application_basenames = [os.path.basename(app.dir) for app in applications]

  for dirname in os.listdir(APP_DIR):
    app_dir = os.path.join(APP_DIR, dirname)
    print app_dir
    if dirname.lower() == 'base': continue
    if not os.path.isdir(app_dir): continue

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
  def compiler(self, app, node_ids, platforms):
    app_path = app.dir
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      # CodeGen
      app.info('Generating necessary files for wukong')
      try:
        codegen = CodeGen(app.xml, ROOT_PATH)
        codegen.generate()
      except Exception as e:
        app.error(e)

      # Mapper results, already did in map_application
      # Generate java code
      app.info('Generating application code in target language (Java)')
      try:
        app.mapper.generateJava()
      except Exception as e:
        app.error(e)

      # Generate nvmdefault.h
      app.info('Compressing application code to bytecode format')
      print 'changing to path: %s...' % platform_dir
      pp = Popen('cd %s; make application FLOWXML=%s' % (platform_dir, app.id), shell=True, stdout=PIPE, stderr=PIPE)
      app.returnCode = None
      while pp.poll() == None:
        print 'polling from popen...'
        line = pp.stdout.readline()
        if line != '':
          app.info(line)

        line = pp.stderr.readline()
        if line != '':
          app.error(line)
        app.version += 1
      app.returnCode = pp.returncode
      app.info('Finishing compression')

      # Deploy nvmdefault.h to nodes
      print 'changing to path: %s...' % platform_dir
      app.info('Deploying to nodes')
      for node_id in node_ids:
        app.info('Deploying to node id: %d' % (node_id))
        print 'deploying to node: %d' % (node_id)
        pp = Popen('cd %s; make nvmcomm_reprogram NODE_ID=%d' % (platform_dir, node_id), shell=True, stdout=PIPE, stderr=PIPE)
        app.returnCode = None
        while pp.poll() == None:
          print 'polling from popen...'
          line = pp.stdout.readline()
          if line != '':
            app.info(line)

          line = pp.stderr.readline()
          if line != '':
            app.error(line)
          app.version += 1
        app.returnCode = pp.returncode
    app.info('Deploying to nodes completed')
    print 'compiler done'

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
        applications.append(WuApplication(self.get_argument('name'), self.get_argument('desc'), self.factory, file))
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

      # copy base for the new application
      print 'setting up app directory from base...'
      copyAnything(BASE_DIR, os.path.join(APP_DIR, app_id))

      app = WuApplication(id=app_id, name=app_name, dir=os.path.join(APP_DIR, app_id))
      applications.append(app)

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
  # topbar info
  def get(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      app = applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=applications[app_ind])
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Display a specific application
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      app = applications[app_ind].config()
      #app = {'name': applications[app_ind].name, 'desc': applications[app_ind].desc, 'id': applications[app_ind].id}
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=applications[app_ind])
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
    try:
      # Discovery results
      # TODO: persistent store
      comm = getComm()
      node_infos = comm.getNodeInfos()

      # debug purpose
      #node_infos = fakedata.node_infos

      app_ind = getAppIndex(app_id)
      if app_ind == None:
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot find the application'})
      else:
        deployment = template.Loader(os.getcwd()).load('templates/deployment.html').generate(app=applications[app_ind], node_infos=node_infos, logs=applications[app_ind].logs(), mapping_results=applications[app_ind].mapping_results)
        self.content_type = 'application/json'
        self.write({'status':0, 'page': deployment})

    except Exception as e:
      print e
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot initiate connection with the baseStation'})

  def post(self, app_id):
    global applications
    # Discovery results
    # TODO: persistent store
    comm = getComm()
    node_infos = comm.getNodeInfos()

    # debug purpose
    #node_infos = fakedata.node_infos

    node_ids = [info.nodeId for info in node_infos]

    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp

      if len(node_ids) > 0:
        applications[app_ind].compiler = Thread(target=Worker().compiler, args=(applications[app_ind], node_ids, platforms,))
        applications[app_ind].compiler.start()

      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class map_application(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    # Discovery results
    # TODO: persistent store
    comm = getComm()
    node_infos = comm.getNodeInfos()

    # debug purpose
    #node_infos = fakedata.node_infos

    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp

      # TODO: rewrite translator.py to have a class that produces mapping results with node infos and Application xml to replace compiler (should be part of deploy)
      applications[app_ind].mapper = Mapper(applications[app_ind], node_infos, applications[app_ind].xml)
      applications[app_ind].mapping_results = applications[app_ind].mapper.map_with_location_tree(fakedata.locTree, fakedata.queries)

      ret = []
      for key, value in applications[app_ind].mapping_results.items():
        ret.append({'instanceId': value.getInstanceId(), 'name': value.getWuClassName(), 'nodeId': value.getNodeId(), 'portNumber': value.getPortNumber()})


      self.content_type = 'application/json'
      self.write({'status':0, 'mapping_results': ret, 'version': applications[app_ind].version})

class monitor_application(tornado.web.RequestHandler):
  def get(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      #applications[app_ind].inspector = Inspector(applications[app_ind].mapping_results)
      monitor = template.Loader(os.getcwd()).load('templates/monitor.html').generate(app=applications[app_ind], mapping_results={})
      self.content_type = 'application/json'
      self.write({'status':0, 'page': monitor})


# Never let go
class poll(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      #print applications[app_ind].version, self.get_argument('version')
      #if int(applications[app_ind].version) <= int(self.get_argument('version')):
        #self.content_type = 'application/json'
        #self.write({'status':0, 'version': applications[app_ind].version, 'returnCode': applications[app_ind].returnCode, 'logs': applications[app_ind].retrieve()})
      #else:
      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version, 'returnCode': applications[app_ind].returnCode, 'logs': applications[app_ind].retrieve()})

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

class poll_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications

    comm = getComm()
    status = comm.currentStatus()
    if status != None:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': status.split('\n')})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': []})

class stop_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications

    comm = getComm()
    if comm.onStopMode() == 0:
      self.content_type = 'application/json'
      self.write({'status':0})
    else:
      self.content_type = 'application/json'
      self.write({'status':1})

class exclude_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications

    comm = getComm()
    if comm.onDeleteMode() == 0:
      self.content_type = 'application/json'
      self.write({'status':0, 'log': 'Going into exclude mode'})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'log': 'There is an error going into exclude mode'})

class include_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications

    comm = getComm()
    print 'onAddMode'
    if comm.onAddMode() == 0:
      self.content_type = 'application/json'
      self.write({'status':0, 'log': 'Going into include mode'})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'log': 'There is an error going into include mode'})

class testrtt(tornado.web.RequestHandler):
  def get(self):
    testrtt = template.Loader(os.getcwd()).load('templates/testrtt.html').generate(log=['Please press the include or exclude button on the nodes.'])
    self.content_type = 'application/json'
    self.write({'status':0, 'testrtt':testrtt})

settings = dict(
  static_path=os.path.join(os.path.dirname(__file__), "static"),
  debug=True
)

app = tornado.web.Application([
  (r"/", main),
  (r"/main", main),
  (r"/testrtt/exclude", exclude_testrtt),
  (r"/testrtt/include", include_testrtt),
  (r"/testrtt/stop", stop_testrtt),
  (r"/testrtt/poll", poll_testrtt),
  (r"/testrtt", testrtt),
  (r"/applications", list_applications),
  (r"/applications/new", new_application),
  (r"/applications/([a-fA-F\d]{32})", application),
  (r"/applications/([a-fA-F\d]{32})/poll", poll),
  (r"/applications/([a-fA-F\d]{32})/deploy", deploy_application),
  (r"/applications/([a-fA-F\d]{32})/deploy/map", map_application),
  (r"/applications/([a-fA-F\d]{32})/monitor", monitor_application),
  (r"/applications/([a-fA-F\d]{32})/fbp/save", save_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/load", load_fbp),
], IP, **settings)

if __name__ == "__main__":
  update_applications()
  app.listen(5000)
  tornado.ioloop.IOLoop.instance().start()
