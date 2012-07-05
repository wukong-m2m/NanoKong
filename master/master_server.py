# author: Penn Su
# reference url: http://flask.pocoo.org/docs/patterns/fileuploads/
import os, sys, zipfile
from flask import Flask, request, redirect, url_for, render_template, jsonify
from werkzeug import secure_filename
from xml.dom.minidom import parse

ALLOWED_EXTENSIONS = set(['bog'])
TARGET = 'HAScenario2'
ROOT_PATH = '..'
APP_PATH = os.path.join(ROOT_PATH, 'Applications')
IP = '127.0.0.1'
if len(sys.argv) >= 2:
  IP = sys.argv[1]

app = Flask(__name__)

def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

@app.route("/", methods=['GET', 'POST'])
def upload_bog():
  print request
  if request.method == 'POST':
    print request.files
    file = request.files['bog_file']
    if file and allowed_file(file.filename):
      filename = secure_filename(file.filename)
      #file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
      z = zipfile.ZipFile(file)
      z.extract('file.xml')
      os.system('python ../tools/xml2java/ni2wk.py -i %s -n %s -o %s' % ('file.xml', TARGET, APP_PATH))
      os.chdir('../vm/build/avr_mega2560/')
      os.system('make generate')
      os.system('make FLOWXML=%s DISCOVERY_FLAGS=-H' % (TARGET))
      os.system('make avrdude')
      return jsonify(status=0)
    else:
      return jsonify(status=1)
  else:
    return render_template('upload.html')

if __name__ == "__main__":
  app.debug = True
  app.run(host=IP)
