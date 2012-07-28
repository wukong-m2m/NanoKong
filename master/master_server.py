# author: Penn Su
# reference url: http://flask.pocoo.org/docs/patterns/fileuploads/
from gevent import monkey
monkey.patch_all()
import gevent
from gevent.wsgi import WSGIServer
from gevent import Greenlet
import os, sys, zipfile
from flask import Flask, request, redirect, url_for, render_template, jsonify
from werkzeug import secure_filename
from xml.dom.minidom import parse
from threading import Thread
import time

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

app = Flask(__name__)

# Helper functions
def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

# List all uploaded applications
@app.route("/", methods=['GET'])
@app.route("/applications", methods=['GET'])
def list_applications():
  return render_template('index.html', applications=applications)

# Create a new application
@app.route("/applications", methods=['POST'])
def create_application():
  global current_status
  file = request.files['bog_file']
  if file and allowed_file(file.filename):
    filename = secure_filename(file.filename)
    #file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
    current_status = 1
    print current_status
    z = zipfile.ZipFile(file)
    z.extract('file.xml')
    current_status = 2
    print current_status
    os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', TARGET, APP_PATH))
    current_status = 3
    print current_status
    os.chdir('../vm/build/avr_mega2560/')
    os.system('make generate')
    os.system('make FLOWXML=%s DISCOVERY_FLAGS=-H' % (TARGET))
    os.system('make avrdude')
    current_status = 4
    print 'done'
    return jsonify(status=0)
  else:
    return jsonify(status=1)

# Returns a form to upload new application
@app.route("/applications/new", methods=['GET'])
def upload_bog():
  return render_template('upload.html')

# Display a specific application
@app.route("/applications/<int:app_id>", methods=['GET'])
def display_application(app_id):
  pass

# Edit a specific application
@app.route("/applications/<int:app_id>/edit", methods=['GET'])
def edit_application(app_id):
  pass

# Update a specific application
@app.route("/applications/<int:app_id>", methods=['PUT'])
def update_application(app_id):
  pass

# Destroy a specific application
@app.route("/applications/<int:app_id>", methods=['DELETE'])
def destroy_application(app_id):
  pass

@app.route("/status", methods=['POST'])
def return_status():
  global current_status
  return jsonify(status=0, current_status=current_status)

if __name__ == "__main__":
  app.debug = True
  #app.run(host=IP)
  http_server = WSGIServer(('', 5000), app)
  http_server.serve_forever()
