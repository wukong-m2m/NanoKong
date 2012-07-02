# author: Penn Su
# reference url: http://flask.pocoo.org/docs/patterns/fileuploads/
import os, sys, zipfile
from flask import Flask, request, redirect, url_for, render_template
from werkzeug import secure_filename
from xml.dom.minidom import parse

UPLOAD_FOLDER = 'bog'
ALLOWED_EXTENSIONS = set(['bog'])
IP = '127.0.0.1'
if len(sys.argv) >= 2:
  IP = sys.argv[1]

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

@app.route("/", methods=['GET'])
def display_master():
  return render_template('upload.html')

@app.route("/", methods=['POST'])
def upload_bog():
  file = request.files['file']
  if file and allowed_file(file.filename):
    filename = secure_filename(file.filename)
    #file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
    z = zipfile.ZipFile(file)
    print z
    dom = parse(z.extract('file.xml'))
    print dom
    return jsonify(status=0)
  else:
    return jsonify(status=1)

if __name__ == "__main__":
  app.debug = True
  app.run(host=IP)
