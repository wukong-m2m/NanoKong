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
wutypedefs = component_root.xpath("WuTypedef")
wutypedefs_hash = {}

# Parsing to WuKong Profile Framework Component Library header
typeind = len(wuclasses)
for wutypedef in wutypedefs:
  wutypedefs_hash[wutypedef.get("name")] = typeind

  # TODO:Generate global header typedef definition for VM
  for item in wutypedef:
    if wutypedef.get("type").lower() == "enum":
      print "#define", "WKPF_" + wutypedef.get("type").upper().replace(' ', '_') + '_' + wutypedef.get("name").upper().replace(' ', '_') + '_' + item.get("value").upper().replace(' ', '_'), "%d" % (typeind)
    else:
      pass
    typeind += 1
print "==================End of TypeDefs====================="

for wuclass in wuclasses:
  index = wuclass.get("id")
  properties = wuclass.xpath("property")

  # TODO:Generate global header definition for VM
  print "#define", "WKPF_WUCLASS_" + wuclass.get("name").upper().replace(' ', '_') + " %s" % (index)

  for indprop, property in enumerate(properties):
    print "#define WKPF_WUCLASS_PROPERTY_" + property.get("name").upper().replace(' ', '_'), str(indprop)


  # TODO:Parsing to WuKong Profile Framework Component Library header in Java
  if wuclass.get('virtual') == 'true':
    print '''
    package nanovm.wkpf;

    public abstract class GENERATED%sWuClass extends VirtualWuClass {
      public static byte[] properties = new byte[] {
    ''' % (wuclass.get('name').lower().capitalize())

    used_enumeration_inds = []

    for property in properties:
      datatype = property.get("datatype").upper().replace(' ', '_')
      access = property.get("access").upper().replace(' ', '_')
      if datatype in wutypedefs_hash: 
        used_enumeration_inds.append(datatype)
        datatype = "SHORT"
      print "WKPF.PROPERTY_TYPE_" + datatype + "|WKPF.PROPERTY_ACCESS_" + access + ","

    print '''
      };
    '''

    for propind, property in enumerate(properties):
      print "protected static final type %s = %d;" % (property.get('name').upper().replace(' ', '_'), propind)
    
    for enum in used_enumeration_inds:
      print "protected static final type %s = %d;" % (enum.lower().replace(' ', '_'), wutypedefs_hash[enum])

    print '''
    }
    '''

  # TODO:Generate C header for each native component implementation
  print '''
  #ifndef WUCLASS_%sH
  #define WUCLASS_%sH

  extern wkpf_wuclass_definition wuclass_%s;

  #endif
  ''' % (
          wuclass.get("name").upper().replace(' ', '_'),
          wuclass.get("name").upper().replace(' ', '_'),
          wuclass.get("name").lower().replace(' ', '_'),
        )

  # TODO:Generate C implementation for each native component implementation
  print '''
  #include <wkpf.h>
  #include "native_wuclasses.h"

  #ifdef ENABLE_WUCLASS_%s

  extern void wuclass_light_update(wkpf_local_wuobject *wuobject);

  uint8_t wuclass_%s_properties[] = {
  ''' % (
          wuclass.get("name").upper().replace(' ', '_'),
          wuclass.get("name").lower().replace(' ', '_'),
        )

  for property in properties:
    datatype = property.get("datatype").upper().replace(' ', '_')
    access = property.get("access").upper().replace(' ', '_')
    if datatype in wutypedefs_hash: datatype = "SHORT"
    print "WKPF_PROPERTY_TYPE_" + datatype + "+WKPF_PROPERTY_ACCESS_" + access + ","

  print '''
  };
  '''

  print '''
  wkpf_wuclass_definition wuclass_%s = {
    %s,
    wuclass_%s_update,
    %d,
    wuclass_%s_properties
  };
  ''' % (wuclass.get("name").lower().replace(' ', '_'), 
        'WKPF_WUCLASS_'+wuclass.get("name").upper().replace(' ', '_'),
        wuclass.get("name").lower().replace(' ', '_'),
        len(properties), 
        wuclass.get("name").lower().replace(' ', '_'))

  print '''
  #endif
  '''
  print "==================End of Component====================="

