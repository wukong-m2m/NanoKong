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
import re
from lxml import etree
from optparse import OptionParser

CWD = os.getcwd()

parser = OptionParser()
parser.add_option('-i', '--input_xml', dest='component_file')
parser.add_option('-p', '--projectdir', dest='project_dir')
(options, args) = parser.parse_args()

print options, args

# Directories
global_vm_dir = os.path.join('vm', 'src')
vm_dir = os.path.join('vm', 'src', 'native_wuclasses')
java_dir = os.path.join('java', 'nanovm', 'wkpf')
plugin_dir = os.path.join('plugins')

# Filenames
global_vm_header_filename = 'GENERATEDwkpf_wuclass_library.h'
global_virtual_constants_filename = 'GENERATEDWKPF.java'

# Paths
component_library_path = options.component_file
global_vm_header_path = os.path.join(options.project_dir, global_vm_dir, global_vm_header_filename)
global_virtual_constants_path = os.path.join(options.project_dir, java_dir, global_virtual_constants_filename)

# IOs
component_library = open(component_library_path)
global_vm = open(global_vm_header_path, 'w')
global_virtual_constants = open(global_virtual_constants_path, 'w')

# Lines
global_vm_header_lines = []
global_virtual_constants_lines = []

def CamelCaseToUnderscore(name):
  s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
  return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def underscore_to_camel_case(name):
  return ''.join(x.capitalize() or '_' for x in name.split('_'))

def convert_filename_to_c(raw):
  if re.search('.*_.*', raw) == None:
    return CamelCaseToUnderscore(raw)
  else:
    return raw.lower()

def convert_filename_to_java(raw):
  return underscore_to_camel_case(raw)

def convert_constant(raw):
  return convert_filename_to_c(raw).upper()

# Parse ComponentLibrary XML
component_tree = etree.parse(component_library)
component_root = component_tree.getroot()
wuclasses = component_root.xpath("WuClass")
wutypedefs = component_root.xpath("WuTypedef")
wutypedefs_hash = []

# Boilerplate for Java global constants file
global_virtual_constants_lines.append('''
    package nanovm.wkpf;

    public class GENERATEDWKPF {
''')

# Parsing to WuKong Profile Framework Component Library header
typeind = len(wuclasses)
for wutypedef in wutypedefs:
  wutypedefs_hash.append(convert_constant(wutypedef.get("name")))

  # Generate global header typedef definition for VM
  for item in wutypedef:
    cline = "#define WKPF_" + convert_constant(wutypedef.get("type")) + "_" + convert_constant(wutypedef.get("name")) + "_" + convert_constant(item.get("value")) + " %d\n" % (typeind)
    jline = "public static final short " + convert_constant(wutypedef.get("type")) + "_" + convert_constant(wutypedef.get("name")) + "_" + convert_constant(item.get("value")) + " = %d;\n" % (typeind)
    global_vm_header_lines.append(cline)
    global_virtual_constants_lines.append(jline)
    typeind += 1
print "==================End of TypeDefs====================="

for wuclass in wuclasses:
  index = wuclass.get("id")
  properties = wuclass.xpath("property")

  # Classname
  wuclass_native_header_classname = 'GENERATED' + 'wuclass_' + convert_filename_to_c(wuclass.get("name"))
  wuclass_native_impl_classname = 'GENERATED' + 'wuclass_' + convert_filename_to_c(wuclass.get("name"))
  wuclass_virtual_super_classname = 'GENERATED' + 'Virtual' + convert_filename_to_java(wuclass.get("name")) + 'WuObject'

  # Filename
  wuclass_native_header_filename = wuclass_native_header_classname + '.h'
  wuclass_native_impl_filename = wuclass_native_impl_classname + '.c'
  wuclass_virtual_super_filename = wuclass_virtual_super_classname + '.java'

  # Path
  wuclass_native_header_path = os.path.join(options.project_dir, vm_dir, wuclass_native_header_filename)
  wuclass_native_impl_path = os.path.join(options.project_dir, vm_dir, wuclass_native_impl_filename)
  wuclass_virtual_super_path = os.path.join(options.project_dir, java_dir, wuclass_virtual_super_filename)

  # IOs
  wuclass_native_header = open(wuclass_native_header_path, 'w')
  wuclass_native_impl = open(wuclass_native_impl_path, 'w')
  if wuclass.get('virtual') == 'true':
    wuclass_virtual_super = open(wuclass_virtual_super_path, 'w') 
  # Lines
  wuclass_native_header_lines = []
  wuclass_native_impl_lines = []
  wuclass_virtual_super_lines = []


  # Generate global header definition for VM
  global_vm_header_lines.append("#define WKPF_WUCLASS_" + convert_constant(wuclass.get("name")) + " %s\n" % (index))

  for indprop, property in enumerate(properties):
    global_vm_header_lines.append("#define WKPF_PROPERTY_" + convert_constant(wuclass.get("name")) + "_" + convert_constant(property.get("name")) + " " + str(indprop) + "\n")

  global_vm_header_lines.append("\n")


  # Generate global constants definition for Java
  global_virtual_constants_lines.append("public static final short WUCLASS_" + convert_constant(wuclass.get("name")) + " = %s;\n" % (index))

  for indprop, property in enumerate(properties):
    global_virtual_constants_lines.append("public static final byte PROPERTY_" + convert_constant(wuclass.get("name")) + "_" + convert_constant(property.get("name")) + " = " + str(indprop) + ";\n")

  global_virtual_constants_lines.append("\n")


  # Parsing to WuKong Profile Framework Component Library header in Java
  if wuclass.get('virtual') == 'true':
    wuclass_virtual_super_lines.append('''
    package nanovm.wkpf;

    public abstract class %s extends VirtualWuObject {
      public static byte[] properties = new byte[] {
    ''' % (wuclass_virtual_super_classname))

    for ind, property in enumerate(properties):
      datatype = convert_constant(property.get("datatype"))
      access = convert_constant(property.get("access"))

      print "datatype is", datatype
      print wutypedefs_hash

      if datatype in wutypedefs_hash: 
        datatype = "SHORT"
      line = "WKPF.PROPERTY_TYPE_" + datatype + "|WKPF.PROPERTY_ACCESS_" + access
      if ind < len(properties)-1:
        line += ","
      line += "\n"
      wuclass_virtual_super_lines.append(line)

    wuclass_virtual_super_lines.append('''
      };
    ''')

    for propind, property in enumerate(properties):
      wuclass_virtual_super_lines.append("protected static final byte %s = %d;\n" % (convert_constant(property.get('name')), propind))

    wuclass_virtual_super_lines.append('''
    }
    ''')

  # Generate C header for each native component implementation
  wuclass_native_header_lines.append('''
  #include <wkpf.h>
  #include "native_wuclasses.h"

  #ifndef WUCLASS_%sH
  #define WUCLASS_%sH

  extern wkpf_wuclass_definition wuclass_%s;

  #endif
  ''' % (
          convert_constant(wuclass.get("name")),
          convert_constant(wuclass.get("name")),
          convert_filename_to_c(wuclass.get("name"))
        ))

  # Generate C implementation for each native component implementation
  wuclass_native_impl_lines.append('''
  #include <wkpf.h>
  #include "native_wuclasses.h"

  #ifdef ENABLE_WUCLASS_%s

  extern void wuclass_%s_update(wkpf_local_wuobject *wuobject);

  uint8_t wuclass_%s_properties[] = {
  ''' % (
          convert_constant(wuclass.get("name")),
          convert_filename_to_c(wuclass.get("name")),
          convert_filename_to_c(wuclass.get("name")),
        ))

  for ind, property in enumerate(properties):
    datatype = convert_constant(property.get("datatype"))
    access = convert_constant(property.get("access"))

    if datatype in wutypedefs_hash:
      datatype = "SHORT"
    line = "WKPF_PROPERTY_TYPE_" + datatype + "+WKPF_PROPERTY_ACCESS_" + access
    if ind < len(properties)-1:
      line += ","
    line += "\n"
    wuclass_native_impl_lines.append(line)

  wuclass_native_impl_lines.append('''
  };
  ''')

  wuclass_native_impl_lines.append('''
  wkpf_wuclass_definition wuclass_%s = {
    %s,
    wuclass_%s_update,
    %d,
    wuclass_%s_properties
  };
  ''' % (convert_filename_to_c(wuclass.get("name")), 
        'WKPF_WUCLASS_'+convert_constant(wuclass.get("name")),
        convert_filename_to_c(wuclass.get("name")),
        len(properties), 
        convert_filename_to_c(wuclass.get("name"))))

  wuclass_native_impl_lines.append('''
  #endif
  ''')

  wuclass_native_header.writelines(wuclass_native_header_lines)
  wuclass_native_header.close()

  wuclass_native_impl.writelines(wuclass_native_impl_lines)
  wuclass_native_impl.close()

  if wuclass.get('virtual') == 'true':
    wuclass_virtual_super.writelines(wuclass_virtual_super_lines)
    wuclass_virtual_super.close()

  print "==================End of Component====================="

global_virtual_constants_lines.append('''
}
''')

global_vm.writelines(global_vm_header_lines)
global_vm.close()

global_virtual_constants.writelines(global_virtual_constants_lines)
global_virtual_constants.close()
