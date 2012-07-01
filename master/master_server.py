import os
import sys
from flask import Flask, request, redirect, url_for
from werkzeug import secure_filename

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

@app.route("/", methods=['POST'])
def upload_bog():
  file = request.files['file']
  if file and allowed_file(file.filename):
    filename = secure_filename(file.filename)
    file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
    return 'yes'
  else:
    return 'no'

if __name__ == "__main__":
  app.run(host=IP)
