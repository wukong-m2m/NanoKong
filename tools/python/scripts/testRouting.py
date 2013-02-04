import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))

from wkpf.pynvc import *
from wkpf.wkpfcomm import *

comm = getComm()

print "Routing information"
print comm.getRoutingInformation()
