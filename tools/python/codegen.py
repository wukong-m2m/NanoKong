#!/usr/bin/python
# vim: ts=2 sw=2

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
from jinja2 import Template
from jinja2 import Environment, FileSystemLoader
from optparse import OptionParser
#from lxml import etree
from wkpf import *
from xml.dom.minidom import parse, parseString

CWD = os.getcwd()


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

class CodeGen:
  def __init__(self, *args, **kwargs):
    # Plugin Name
    if 'plugin_name' in kwargs:

      self.plugin_name = kwargs['plugin_name']
      self.plugin_template_dir = os.path.join(template_dir, kwargs['plugin_name'], 'wukongObject')
      self.plugin_root_dir = os.path.join(plugin_dir, kwargs['plugin_name'], 'wukongObject')

      self.jinja2_env = Environment(loader=FileSystemLoader(get_all_subdirectories(plugin_template_dir) + [plugin_template_dir]))
      self.jinja2_env.filters["convert_filename_to_java"] = convert_filename_to_java

    # component xml dom
    if len(args) == 2:
      # Directories
      self.global_vm_dir = os.path.join('vm', 'src')
      self.vm_dir = os.path.join('vm', 'src', 'native_wuclasses')
      self.java_dir = os.path.join('java', 'nanovm', 'wkpf')

      self.component_string = args[0]
      self.project_dir = args[1]

      self.plugin_dir = os.path.join(self.project_dir, 'plugins')
      self.template_dir = os.path.join(self.plugin_dir, 'templates')

      # Filenames
      self.global_vm_header_filename = 'GENERATEDwkpf_wuclass_library.h'
      self.global_virtual_constants_filename = 'GENERATEDWKPF.java'

      # Paths
      self.global_vm_header_path = os.path.join(self.project_dir, self.global_vm_dir, self.global_vm_header_filename)
      self.global_virtual_constants_path = os.path.join(self.project_dir, self.java_dir, self.global_virtual_constants_filename)

      # IOs
      self.global_vm = open(self.global_vm_header_path, 'w')
      self.global_virtual_constants = open(self.global_virtual_constants_path, 'w')

      # Lines
      self.global_vm_header_lines = []
      self.global_virtual_constants_lines = []
    else:
      raise Exception('Argument required not present')

  def generate(self):
    # Parse ComponentLibrary XML
    dom = parseString(self.component_string)

    wuclasses = dom.getElementsByTagName("WuClass")
    wutypedefs = dom.getElementsByTagName("WuTypedef")
    wutypedefs_hash = []

    print "==================Begin TypeDefs====================="
    # Boilerplate for Java global constants file
    self.global_virtual_constants_lines.append('''
        package nanovm.wkpf;

        public class GENERATEDWKPF {
    ''')

    # Parsing to WuKong Profile Framework Component Library header
    wuTypes = {'short': WuType('short', 'short'), 'boolean': WuType('boolean', 'boolean'), 'refresh_rate': WuType('refresh_rate', 'refresh_rate')}

    for wutypedef in wutypedefs:
      if wutypedef.getAttribute('type').lower() == 'enum':
        wuTypes[wutypedef.getAttribute('name')] = WuType(wutypedef.getAttribute('name'), wutypedef.getAttribute('type'), tuple([element.getAttribute('value') for element in wutypedef.getElementsByTagName('enum')]))
      else:
        wuTypes[wutypedef.getAttribute('name')] = WuType(wutypedef.getAttribute('name'), wutypedef.getAttribute('type'))

      wutype = wuTypes[wutypedef.getAttribute('name')]

      wutypedefs_hash.append(convert_constant(wutypedef.getAttribute("name")))

      # Generate global header typedef definition for VM
      for enumvalue, value in enumerate(wutype.getAllowedValues()):
        cline = "#define " + wutype.getValueInCConstant(value) + " %d\n" % (enumvalue)
        jline = "public static final short " + wutype.getValueInCConstant(value) + " = %d;\n" % (enumvalue)

        self.global_vm_header_lines.append(cline)
        self.global_virtual_constants_lines.append(jline)
    print "==================End of TypeDefs====================="

    print "==================Begin WuClasses====================="
    for wuclass in wuclasses:
      wuclassName = wuclass.getAttribute('name')
      wuclassId = int(wuclass.getAttribute('id'),0)
      wuclassProperties = {}
      for i, prop in enumerate(wuclass.getElementsByTagName('property')):
          propType = prop.getAttribute('datatype')
          propName = prop.getAttribute('name')

          wuclassProperties[propName] = WuProperty(wuclassName, propName, i, wuTypes[propType], prop.getAttribute('access')) 

      wuClass = WuClass(wuclassName, wuclassId, wuclassProperties, True if wuclass.getAttribute('virtual').lower() == 'true' else False, True if wuclass.getAttribute('type').lower() == 'soft' else False)


      # Native header
      wuclass_native_header_path = os.path.join(self.project_dir, self.vm_dir, wuClass.getCFileName() + '.h')
      wuclass_native_header = open(wuclass_native_header_path, 'w')

      # Native impl
      wuclass_native_impl_path = os.path.join(self.project_dir, self.vm_dir, wuClass.getCFileName() + '.c')
      wuclass_native_impl = open(wuclass_native_impl_path, 'w')

      # Virtual (Java)
      if wuclass.getAttribute('virtual') == 'true':
        wuclass_virtual_super_path = os.path.join(self.project_dir, self.java_dir, wuClass.getJavaGenClassName() + '.java')
        wuclass_virtual_super = open(wuclass_virtual_super_path, 'w') 

      # Lines
      wuclass_native_header_lines = []
      wuclass_native_impl_lines = []
      wuclass_virtual_super_lines = []

      # Generate global header definition for VM
      self.global_vm_header_lines.append("#define " + wuClass.getCConstName() + " %s\n" % (wuClass.getId()))

      # Generate global constants definition for Java
      self.global_virtual_constants_lines.append("public static final short " + wuClass.getJavaConstName() + " = %s;\n" % (wuClass.getId()))

      for indprop, property in enumerate(wuClass.getProperties().values()):
        self.global_vm_header_lines.append("#define " + property.getCConstName() + " " + str(indprop) + "\n")

        self.global_virtual_constants_lines.append("public static final byte " + property.getJavaConstName() + " = " + str(indprop) + ";\n")

      self.global_vm_header_lines.append("\n")

      self.global_virtual_constants_lines.append("\n")


      # Parsing to WuKong Profile Framework Component Library header in Java
      if wuclass.getAttribute('virtual') == 'true':
        wuclass_virtual_super_lines.append('''
        package nanovm.wkpf;

        public abstract class %s extends VirtualWuObject {
          public static byte[] properties = new byte[] {
        ''' % (wuClass.getJavaGenClassName()))

        for ind, property in enumerate(wuClass.getProperties().values()):
          datatype = property.getDataType()
          access = property.getAccess()

          if datatype in wutypedefs_hash:
            datatype = "SHORT"

          line = "WKPF.PROPERTY_TYPE_" + datatype + "|WKPF.PROPERTY_ACCESS_" + access
          if ind < len(wuClass.getProperties())-1:
            line += ","

          line += "\n"
          wuclass_virtual_super_lines.append(line)

        wuclass_virtual_super_lines.append('''
          };
        ''')

        for propind, property in enumerate(wuClass.getProperties().values()):
          wuclass_virtual_super_lines.append("protected static final byte %s = %d;\n" % (property.getName(), propind))

        wuclass_virtual_super_lines.append('''
        }
        ''')

      # Generate C header for each native component implementation
      wuclass_native_header_lines.append('''
      #include <wkpf.h>
      #include "native_wuclasses.h"

      #ifndef %sH
      #define %sH

      extern wkpf_wuclass_definition %s;

      #endif
      ''' % (
              wuClass.getCDefineName(),
              wuClass.getCDefineName(),
              wuClass.getCName()
            ))

      # Generate C implementation for each native component implementation
      wuclass_native_impl_lines.append('''
      #include <wkpf.h>
      #include "native_wuclasses.h"

      #ifdef ENABLE_%s

      extern void %s(wkpf_local_wuobject *wuobject);

      uint8_t %s[] = {
      ''' % (
              wuClass.getCDefineName(),
              wuClass.getCUpdateName(),
              wuClass.getCPropertyName(),
            ))

      for ind, property in enumerate(wuClass.getProperties().values()):
        datatype = property.getDataType()
        access = property.getAccess()

        if datatype in wutypedefs_hash:
          datatype = "SHORT"

        line = "WKPF_PROPERTY_TYPE_" + datatype + "+WKPF_PROPERTY_ACCESS_" + access
        if ind < len(wuClass.getProperties())-1:
          line += ","

        line += "\n"
        wuclass_native_impl_lines.append(line)

      wuclass_native_impl_lines.append('''
      };
      ''')

      wuclass_native_impl_lines.append('''
      wkpf_wuclass_definition %s = {
        %s,
        %s,
        %d,
        %s
      };
      ''' % (wuClass.getCName(), 
            wuClass.getCConstName(),
            wuClass.getCUpdateName(),
            len(wuClass.getProperties()), 
            wuClass.getCPropertyName()))

      wuclass_native_impl_lines.append('''
      #endif
      ''')

      wuclass_native_header.writelines(wuclass_native_header_lines)
      wuclass_native_header.close()

      wuclass_native_impl.writelines(wuclass_native_impl_lines)
      wuclass_native_impl.close()

      if wuclass.getAttribute('virtual') == 'true':
        wuclass_virtual_super.writelines(wuclass_virtual_super_lines)
        wuclass_virtual_super.close()

      print "==================End of WuClasses====================="

    self.global_virtual_constants_lines.append('''
    }
    ''')

    self.global_vm.writelines(self.global_vm_header_lines)
    self.global_vm.close()

    self.global_virtual_constants.writelines(self.global_virtual_constants_lines)
    self.global_virtual_constants.close()


  def generate_plugin(self):
    dom = parseString(self.component_string)

    wuclasses = dom.getElementsByTagName("WuClass")
    wutypedefs = dom.getElementsByTagName("WuTypedef")
    wucomponents = component_root.xpath("WuClass | WuTypedef")

    # Create plugin folder structure
    if self.plugin_name:
      distutils.dir_util.copy_tree(self.plugin_template_dir, self.plugin_root_dir)

      if self.plugin_name == "niagara":
        module_include_path = os.path.join(self.plugin_root_dir, 'module-include.xml')
        module_include = open(module_include_path, 'w')
        module_include_template = self.jinja2_env.get_template('module-include.xml')
        module_include.write(module_include_template.render(components=wucomponents))
        module_include.close()

        module_palette_path = os.path.join(self.plugin_root_dir, 'module.palette')
        module_palette = open(module_palette_path, 'w')
        module_palette_template = self.jinja2_env.get_template('module.palette')
        module_palette.write(module_palette_template.render(
          virtuals=component_root.xpath("//*[@virtual='true']"), 
          sensors=component_root.xpath("//*[contains(@name, 'Sensor')]"), 
          controllers=component_root.xpath("//*[contains(@name, 'Controller')]"),
          actuators=component_root.xpath("//*[contains(@name, 'Actuator')]")))
        module_palette.close()


        class_implementation_dir = os.path.join(self.plugin_root_dir, 'src', 'com', 'wukong', 'wukongObject')
        distutils.dir_util.mkpath(class_implementation_dir)
        #class_implementation_template = Template(open(os.path.join(class_implementation_dir, 'BTemplate.java')).read())
        class_implementation_template = self.jinja2_env.get_template('BTemplate.java')
        enum_implementation_template = self.jinja2_env.get_template('BTemplateEnum.java')

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
      try:
        os.remove(findInSubdirectory('BTemplate.java', self.plugin_root_dir))
      except IOError:
        print "Attempting to remove non-existing file %s" % ('BTemplate.java')

      try:
        os.remove(findInSubdirectory('BTemplateEnum.java', self.plugin_root_dir))
      except IOError:
        print "Attempting to remove non-existing file %s" % ('BTemplateEnum.java')


      print "==================End of Plugin====================="



if __name__ == "__main__":
  parser = OptionParser()
  parser.add_option('-i', '--input_xml', dest='component_file')
  parser.add_option('-p', '--projectdir', dest='project_dir')
  parser.add_option('-u', '--plugin', dest='plugin_name')
  (options, args) = parser.parse_args()

  print options, args

  if os.path.exists(options.component_file) and options.project_dir:
    codegen = CodeGen(open(options.component_file).read(), options.project_dir)
    codegen.generate()
