# author: Penn Su
import tornado.ioloop
import tornado.web
import os, sys, zipfile
from xml.dom.minidom import parse
from threading import Thread
import time
import StringIO

ALLOWED_EXTENSIONS = set(['bog'])
TARGET = 'HAScenario2'
ROOT_PATH = '..'
APP_PATH = os.path.join(ROOT_PATH, 'Applications')
IP = '127.0.0.1'
if len(sys.argv) >= 2:
  IP = sys.argv[1]

applications = []
worker = None

# Helper functions
def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

def statusString(status):
  if len(status) == 0:
    return 'uploading...'
  elif len(status) == 1:
    return 'extracting...'
  elif len(status) == 2:
    return 'converting...'
  elif len(status) == 3:
    return 'compiling...'
  elif len(status) == 4:
    return 'done'

class Application:
  def __init__(self, name, desc, factory, file):
    self.name = name
    self.desc = desc
    self.status = []
    self.worker = Thread(target=factory, args=(file, self.status))
    self.worker.start()

  def getStatus(self):
    return statusString(self.status)

# List all uploaded applications
class list_applications(tornado.web.RequestHandler):
  def factory(self, file, status):
    status.append(1)
    print status
    #time.sleep(2)
    z = zipfile.ZipFile(file)
    z.extract('file.xml')
    status.append(2)
    print status
    #time.sleep(2)
    os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', TARGET, APP_PATH))
    status.append(3)
    print status
    #time.sleep(2)
    os.chdir('../vm/build/avr_mega2560/')
    os.system('make generate')
    os.system('make FLOWXML=%s DISCOVERY_FLAGS=-H' % (TARGET))
    os.system('make avrdude')
    status.append(4)
    #time.sleep(2)
    print 'done'

  def get(self):
    self.render('templates/index.html', applications=applications)

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

# Returns a form to upload new application
class upload_bog(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/upload.html')

# Display a specific application
class display_application(tornado.web.RequestHandler):
  def get(self, app_id):
    pass

# Edit a specific application
class edit_application(tornado.web.RequestHandler):
  def get(self, app_id):
    pass

# Update a specific application
class update_application(tornado.web.RequestHandler):
  def put(self, app_id):
    pass

# Destroy a specific application
class destroy_application(tornado.web.RequestHandler):
  def delete(self, app_id):
    pass

class return_status(tornado.web.RequestHandler):
  def post(self):
    global applications
    current_status = applications[int(self.get_argument('id'))].status
    print '**[current_status]**', current_status
    self.content_type = 'application/json'
    self.write({'status':0, 'current_status':len(current_status)})

settings = {
  "static_path": os.path.join(os.path.dirname(__file__), "static"),
  "debug": True
}

app = tornado.web.Application([
  (r"/", list_applications),
  (r"/applications", list_applications),
  (r"/applications/new", upload_bog),
  (r"/applications/([0-9]+)", display_application),
  (r"/applications/([0-9]+)/edit", edit_application),
  (r"/applications/([0-9]+)", update_application),
  (r"/applications/([0-9]+)", destroy_application),
  (r"/status", return_status)
], **settings)

if __name__ == "__main__":
  app.listen(5000)
  tornado.ioloop.IOLoop.instance().start()
