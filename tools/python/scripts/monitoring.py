import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))

import time
from wkpf.wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()


while 1:
    sys.stdout.write("node 2 :")
    sys.stdout.write(repr(comm.getWuObjectList(2)))

    sys.stdout.write("node 4 :")
    sys.stdout.write(repr(comm.getWuObjectList(4)))

    sys.stdout.write("node 6 :")
    sys.stdout.write(repr(comm.getWuObjectList(6)))

    sys.stdout.flush()
    time.sleep(5)
