import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import time
from wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()


while 1:
    sys.stdout.write("node 2 :", comm.getWuObjectList(2))
    sys.stdout.write("node 4 :", comm.getWuObjectList(4))
    sys.stdout.write("node 6 :", comm.getWuObjectList(6))
    sys.stdout.flush()
    time.sleep(5)
