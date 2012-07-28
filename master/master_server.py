# author: Penn Su
# reference url: http://flask.pocoo.org/docs/patterns/fileuploads/
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

current_status = 0
applications = []
worker = None

# Helper functions
def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

# List all uploaded applications
class list_applications(tornado.web.RequestHandler):
  def factory(self, file):
    global current_status
    current_status = 1
    #time.sleep(2)
    z = zipfile.ZipFile(file)
    z.extract('file.xml')
    current_status = 2
    #time.sleep(2)
    os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', TARGET, APP_PATH))
    current_status = 3
    #time.sleep(2)
    os.chdir('../vm/build/avr_mega2560/')
    os.system('make generate')
    os.system('make FLOWXML=%s DISCOVERY_FLAGS=-H' % (TARGET))
    os.system('make avrdude')
    current_status = 4
    #time.sleep(2)
    print 'done'


  def get(self):
    self.render('templates/index.html', applications=applications)

  def post(self):
    received = self.request.files['bog_file'][0]
    file = StringIO.StringIO()
    file.write(received['body'])
    filename = received['filename']
    if file and allowed_file(filename):
      self.thread = Thread(target=self.factory, args=(file,))
      self.thread.start()
      self.content_type = 'application/json'
      self.write({'status':0})
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
    global current_status
    self.content_type = 'application/json'
    self.write({'status':0, 'current_status':current_status})

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
