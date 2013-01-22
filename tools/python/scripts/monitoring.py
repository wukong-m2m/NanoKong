import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))

import time
from wkpf.wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()

'''
print "node 2 :"
print "[wuclasses]"
for wuclass in comm.getWuClassList(2):
    print wuclass

print "[wuobjects]"
for wuobject in comm.getWuObjectList(2):
    print wuobject

print "node 4 :"
print "[wuclasses]"
for wuclass in comm.getWuClassList(4):
    print wuclass

print "[wuobjects]"
for wuobject in comm.getWuObjectList(4):
    print wuobject

print "node 6 :"
print "[wuclasses]"
for wuclass in comm.getWuClassList(6):
    print wuclass

print "[wuobjects]"
for wuobject in comm.getWuObjectList(6):
    print wuobject

print "\n\n"
'''

sys.stdout.flush()
while 1:
  pass
