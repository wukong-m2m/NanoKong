import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))

import time
from wkpf.wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()

while 1:
  continue
