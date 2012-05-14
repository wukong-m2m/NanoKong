#!/usr/bin/python

# Wukong middleware framework
# Codegen component frontend for component.xml
# plus backend for parsing and generating code for
# java component implementation
# and plugin for NiagaraAX tool
#
#
# Author: Penn Su
# Date: May 13, 2012

import os
from lxml import etree
from optparse import OptionParser

CWD = os.getcwd()

parser = OptionParser()
parser.add_option('-n', '--nanovmtool', action='store_true', dest='nanovmtool')
parser.add_option('-v', '--vm', action='store_true', dest='vm')
parser.add_option('-u', '--upload', action='store_true', dest='upload')
parser.add_option('-s', '--screen', action='store_true', dest='screen')
(options, args) = parser.parse_args()

print options, args

component_file = 'WuKongComponents.xml'

# Front End
component_tree = etree.parse(open(component_file))
component_root = component_tree.getroot()
wuclasses = component_root.xpath("WuClass")

# Parsing to WuKong Profile Framework Component Library header
for wuclass in wuclasses:
  index = wuclass.get("id")
  properties = wuclass.xpath("property")

  # TODO:Generate global header definition for VM
  print "#define", "WKPF_PROFILE_" + wuclass.get("name").upper().replace(' ', '_') + " %s" % (index)

  for property in properties:
    print "WKPF.PROPERTY_TYPE_" + property.get("datatype").upper() + "|WKPF.PROPERTY_ACCESS_" + property.get("access").upper() + ","

  print '''
    };
  '''

  enumerations = wuclass.xpath("//enumvalue")
  for ind, enumeration in enumerate(enumerations):
    name = enumeration.getparent().get("name")
    print "public static final short " + name.upper().replace(' ', '_') + "_" + enumeration.get("value").upper().replace(' ', '_') + " = %d" % (ind) + ";"

  print '''
  }
  '''

  # TODO:Parsing to WuKong Profile Framework Component Library header in Java

  # TODO:Generate C constants for each component (TODO:discuss where to put them)
  print '''
  uint8_t wuclass_%s_properties = {
  '''

  for property in properties:
    print "WKPF_PROPERTY_TYPE_" + property.get("datatype").upper() + "+WKPF_PROPERTY_ACCESS_" + property.get("access").upper() + ","

  print '''
  };
  ''' % (wuclass.get("name").lower().replace(' ', '_'), index, len(properties), wuclass.get("name").lower().replace(' ', '_'))

  print '''
  wkpf_wuclass_definition wuclass_%s = {
    %d,
    %d,
    wuclass_%s_properties
  };
  ''' % (wuclass.get("name").lower().replace(' ', '_'), index, len(properties), wuclass.get("name").lower().replace(' ', '_'))

  # TODO:Generate GEN_Java code for each component
