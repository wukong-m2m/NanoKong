#!/usr/bin/python
import sys
sys.path.append("/Users/niels/git/nanokong/tools/python")

from flask import Flask
from flask import request
from flask import jsonify
app = Flask(__name__)

import reprogram
import pynvc
import wkpf
from wkpf import Endpoint

numericInputEndpoint = Endpoint(nodeId=3, portNumber=1, profileId=3)
lightSensorEndpoint = Endpoint(nodeId=3, portNumber=2, profileId=5)
lightEndpoint = Endpoint(nodeId=3, portNumber=4, profileId=4)

@app.route("/")
def hello():
  return "Hello World!"

@app.route("/updateStatus")
def flaskUpdateStatus():
  filename = "/Users/niels/Sites/getStatus"
  scenario=1 # TODO
  threshold = wkpf.getProperty(numericInputEndpoint, propertyNumber=0)
  lightSensorValue = wkpf.getProperty(lightSensorEndpoint, propertyNumber=0)
  lightOnOff = wkpf.getProperty(lightEndpoint, propertyNumber=0)
  f = open(filename,"w")
  if scenario == 1:
    f.write("""{{"scenario": {0}, "threshold": {1}, "lightsensor": {2}, "lamp_on": {3}}}""".format(1, threshold, lightSensorValue, 1 if lightOnOff else 0))
  else:
    f.write("""{{"scenario": {0}, "threshold": {1}, "lightsensor": {2}, "lamp_on": {3}, "occupied": {4}}}""".format(2, threshold, lightSensorValue, lightOnOff, False))
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
  wkpf.setProperty(numericInputEndpoint, propertyNumber=0, datatype=wkpf.DATATYPE_INT16, value=threshold)
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
    reprogram.reprogramNvmdefault(1, "bytecodeLightSensorV1.h")
    return "reprogrammed to scenario 1"
  if scenario == 2:
    reprogram.reprogramNvmdefault(1, "bytecodeLightSensorV2.h")
    return "reprogrammed to scenario 2"
  else:
    return ""

if __name__ == "__main__":
  flaskUpdateStatus()
  app.run(host='0.0.0.0')