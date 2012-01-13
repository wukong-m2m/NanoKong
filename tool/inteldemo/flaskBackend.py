from flask import Flask
from flask import request
from flask import jsonify
app = Flask(__name__)

import getStatus
import setThreshold
import setPeopleInRoom
import reprogram
import pynvc3

@app.route("/")
def hello():
  return "Hello World!"

@app.route("/updateStatus")
def flaskUpdateStatus():
  filename = "/Users/niels/Sites/getStatus"
  reply = getStatus.getStatus(1)
  f = open(filename,"w")
  if not reply == None and len(reply) == 3:
    f.write("""{{"scenario": {0}, "threshold": {1}, "lightsensor": {2}, "lamp_on": {3}}}""".format(1, reply[0], reply[1], reply[2]))
  elif not reply == None and len(reply) == 4:
    f.write("""{{"scenario": {0}, "threshold": {1}, "lightsensor": {2}, "lamp_on": {3}, "occupied": {4}}}""".format(2, reply[0], reply[1], reply[2], reply[3]))
  else:
    print "=====FAIL======"
    print reply  
  f.close()
  f = open(filename,"r")
  print f.readlines()
  f.close()
  return "OK"


@app.route("/getStatus")
def flaskGetStatus():
#  return jsonify(scenario=1,
#                       threshold=21,
#                       lightsensor=22,
#                       lamp_on=0)
  reply = getStatus.getStatus(1)
  if reply == None:
    print "Fail!"
  elif len(reply) == 3:
    return """{{"scenario": {0},
                "threshold": {1},
                "lightsensor": {2},
                "lamp_on": {3}}}""".format(1, reply[0], reply[1], reply[2])
  else:
    return """{{"scenario": {0},
                "threshold": {1},
                "lightsensor": {2},
                "lamp_on": {3}
                "occupied": {4}}}""".format(2, reply[0], reply[1], reply[2], reply[3])

@app.route("/setThreshold")
def flaskSetThreshold():
  threshold = int(request.args.get("threshold","255"))
  setThreshold.setThreshold(1, threshold)
  return "threshold set to {}".format(threshold)

@app.route("/setPeopleInRoom")
def flaskSetPeopleInRoom():
  peopleinroom = int(request.args.get("peopleinroom","1"))
  setPeopleInRoom.setPeopleInRoom(1, peopleinroom)
  return "peopleinroom set to {}".format(peopleinroom)

@app.route("/reprogram")
def flaskReprogram():
  scenario = int(request.args.get("scenario","1"))
  if scenario == 1:
    reprogram.reprogramNvmdefault(1, "bytecodeV1.h")
    return "reprogrammed to scenario 1"
  if scenario == 2:
    reprogram.reprogramNvmdefault(1, "bytecodeV2.h")
    return "reprogrammed to scenario 2"
  else:
    return ""

if __name__ == "__main__":
  pynvc3.init()
  flaskUpdateStatus()
  app.run(host='0.0.0.0')