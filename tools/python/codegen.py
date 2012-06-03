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
import sys
import re
import distutils.dir_util
sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))
from lxml import etree
from jinja2 import Template
from jinja2 import Environment, FileSystemLoader
from optparse import OptionParser

CWD = os.getcwd()

parser = OptionParser()
parser.add_option('-i', '--input_xml', dest='component_file')
parser.add_option('-p', '--projectdir', dest='project_dir')
parser.add_option('-u', '--plugin', dest='plugin_name')
(options, args) = parser.parse_args()

print options, args

# ------------ Utility functions ------------ #
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
  if re.search('.*_.*', raw):
    return underscore_to_camel_case(raw)
  else:
    return raw

def convert_constant(raw):
  return convert_filename_to_c(raw).upper()

def get_immediate_subdirectories(dir):
  return [name for name in os.listdir(dir)
          if os.path.isdir(os.path.join(dir, name))]

def findInSubdirectory(filename, subdirectory=''):
  if subdirectory:
    path = subdirectory
  else:
    path = os.getcwd()

  for root, dirs, names in os.walk(path):
    if filename in names:
      return os.path.join(root, filename)

    for dirname in dirs:
      result = findInSubdirectory(filename, os.path.join(path, dirname))
      if result != None:
        return result

  return None

def get_all_subdirectories(dir):
  directories = []
  directories += map(lambda x: os.path.join(dir, x), get_immediate_subdirectories(dir))
  for directory in map(lambda x: os.path.join(dir, x), get_immediate_subdirectories(dir)):
    directories += get_all_subdirectories(directory)
  return directories

# Directories
global_vm_dir = os.path.join('vm', 'src')
vm_dir = os.path.join('vm', 'src', 'native_wuclasses')
java_dir = os.path.join('java', 'nanovm', 'wkpf')

plugin_dir = os.path.join(options.project_dir, 'plugins')
template_dir = os.path.join(plugin_dir, 'templates')

if options.plugin_name:
  plugin_template_dir = os.path.join(template_dir, options.plugin_name, 'wukongObject')
  plugin_root_dir = os.path.join(plugin_dir, options.plugin_name, 'wukongObject')

  jinja2_env = Environment(loader=FileSystemLoader(get_all_subdirectories(plugin_template_dir) + [plugin_template_dir]))
  jinja2_env.filters["convert_filename_to_java"] = convert_filename_to_java

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


# Parse ComponentLibrary XML
component_tree = etree.parse(component_library)
component_root = component_tree.getroot()
wuclasses = component_root.xpath("WuClass")
wutypedefs = component_root.xpath("WuTypedef")
wucomponents = component_root.xpath("WuClass | WuTypedef")
wutypedefs_hash = []

# Boilerplate for Java global constants file
global_virtual_constants_lines.append('''
    package nanovm.wkpf;

    public class GENERATEDWKPF {
''')

# Parsing to WuKong Profile Framework Component Library header
for wutypedef in wutypedefs:
  wutypedefs_hash.append(convert_constant(wutypedef.get("name")))

  # Generate global header typedef definition for VM
  enumvalue = 0
  for item in wutypedef:
    cline = "#define WKPF_" + convert_constant(wutypedef.get("type")) + "_" + convert_constant(wutypedef.get("name")) + "_" + convert_constant(item.get("value")) + " %d\n" % (enumvalue)
    jline = "public static final short " + convert_constant(wutypedef.get("type")) + "_" + convert_constant(wutypedef.get("name")) + "_" + convert_constant(item.get("value")) + " = %d;\n" % (enumvalue)
    global_vm_header_lines.append(cline)
    global_virtual_constants_lines.append(jline)
    enumvalue += 1
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


# Create plugin folder structure
if options.plugin_name:
  distutils.dir_util.copy_tree(plugin_template_dir, plugin_root_dir)

  if options.plugin_name == "niagara":
    module_include_path = os.path.join(plugin_root_dir, 'module-include.xml')
    module_include = open(module_include_path, 'w')
    module_include_template = jinja2_env.get_template('module-include.xml')
    module_include.write(module_include_template.render(components=wucomponents))
    module_include.close()


    module_palette_path = os.path.join(plugin_root_dir, 'module.palette')
    module_palette = open(module_palette_path, 'w')
    module_palette_template = jinja2_env.get_template('module.palette')
    module_palette.write(module_palette_template.render(virtuals=component_root.xpath("//*[@virtual='true']"), 
      sensors=component_root.xpath("//*[contains(@name, 'Sensor')]"), 
      controller=component_root.xpath("//*[contains(@name, 'Controller')]"),
      actuators=component_root.xpath("//*[contains(@name, 'Actuator')]")))
    module_palette.close()


    class_implementation_dir = os.path.join(plugin_root_dir, 'src', 'com', 'wukong', 'wukongObject')
    distutils.dir_util.mkpath(class_implementation_dir)
    #class_implementation_template = Template(open(os.path.join(class_implementation_dir, 'BTemplate.java')).read())
    class_implementation_template = jinja2_env.get_template('BTemplate.java')
    enum_implementation_template = jinja2_env.get_template('BTemplateEnum.java')

    for wuclass in wuclasses:
      wuclass_implementation_path = os.path.join(class_implementation_dir, 'B%s.java' % (convert_filename_to_java(wuclass.get("name"))))
      wuclass_implementation = open(wuclass_implementation_path, 'w')
      wuclass_implementation.write(class_implementation_template.render(component=wuclass))

    for wutypedef in wutypedefs:
      if wutypedef.get("type").lower() == 'enum':
        wutypedef_implementation_path = os.path.join(class_implementation_dir, 'B%s.java' % (convert_filename_to_java(wutypedef.get("name"))))
        wutypedef_implementation = open(wutypedef_implementation_path, 'w')
        wutypedef_implementation.write(enum_implementation_template.render(component=wutypedef))

  # delete template files
  if options.plugin_name:
    try:
      os.remove(findInSubdirectory('BTemplate.java', plugin_root_dir))
    except IOError:
      print "Attempting to remove non-existing file %s" % ('BTemplate.java')

    try:
      os.remove(findInSubdirectory('BTemplateEnum.java', plugin_root_dir))
    except IOError:
      print "Attempting to remove non-existing file %s" % ('BTemplateEnum.java')


  print "==================End of Plugin====================="
