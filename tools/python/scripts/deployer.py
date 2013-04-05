#!/usr/bin/env python
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../master'))

from wkpf.wubutler import *

WuButler().load_applications()
print [app.config() for app in WuButler().applications]
WuButler().applications[0]
