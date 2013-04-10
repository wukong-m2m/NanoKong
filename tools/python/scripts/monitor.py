import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))
import gevent

import time
from wkpf.wkpfcomm import *

comm = getComm()

while 1:
  gevent.sleep(0)
