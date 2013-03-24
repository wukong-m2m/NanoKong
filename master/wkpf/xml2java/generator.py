# vim: ts=4 sw=4
#!/usr/bin/python

import os, sys
sys.path.append(os.path.join(os.path.abspath(os.path.dirname(__file__)), "../.."))
from jinja2 import Template, Environment, FileSystemLoader
from struct import pack

from wkpf.models import *

from configuration import *
from wkpf.util import *


class Generator:
    @staticmethod
    def generate(name, changesets):
        # i is the number to be transform into byte array, n is the number of bytes to use (little endian)
        def bytestring(i, n): 
            return ['(byte)' + str(ord(b)) for b in pack("H", i)][:n]

        def nodeinjava(node):
            return str(node.id)

        def wuobjectinjava(wuobject):
            return ', '.join([str(wuobject.node_id),
                            str(wuobject.port_number)])
            
        def linkinjava(link):
            return ', '.join(bytestring(link.from_component_index, 2)
                    + bytestring(link.from_property_id, 1)
                    + bytestring(link.to_component_index, 2)
                    + bytestring(link.to_property_id, 1)
                    + bytestring(link.to_wuclass_id, 2))

        def wuclassname(wuclass):
            return wuclass.name

        def wuclassvirtualclassname(wuclass):
            return "Virtual" + Convert.to_java(wuclass.name) + "WuObject"

        def wuclassconstname(wuclass):
            return "WUCLASS_" + Convert.to_constant(wuclass.name)

        def wuclassgenclassname(wuclass):
            return "GENERATEDVirtual" + Convert.to_java(wuclass.name) + "WuObject"

        def propertyconstname(property):
            print 'propertyconstname'
            return "PROPERTY_" + Convert.to_constant(property.wuclass.name) + "_" + Convert.to_constant(property.name)

        # doesn't really matter to check since basic types are being take care of in application.java
        def propertyconstantvalue(property):
            print 'propertyconstantvalue'
            wutype = WuType.where(name=property.datatype)
            if wutype:
                return wutype[0].type.upper() + '_' + Convert.to_constant(property.datatype) + "_" + Convert.to_constant(property.value)
            else:
                return 'ENUM' + '_' + Convert.to_constant(property.datatype) + "_" + Convert.to_constant(property.value)

        def getPropertyByName(name):
            property_query = WuProperty.where(name=name)
            print 'getPropertyByName'
            # need to query by wuclass id as well
            if property_query:
                for property in property_query:
                    if property.wuclass:
                        print property
                        return property
            else:
                return False


        print 'generating', os.path.join(JAVA_OUTPUT_DIR, name+".java")
        jinja2_env = Environment(loader=FileSystemLoader([os.path.join(os.path.dirname(__file__), 'jinja_templates')]))
        jinja2_env.filters['nodeinjava'] = nodeinjava
        jinja2_env.filters['wuobjectinjava'] = wuobjectinjava
        jinja2_env.filters['linkinjava'] = linkinjava
        jinja2_env.filters['wuclassname'] = wuclassname
        jinja2_env.filters['wuclassvirtualclassname'] = wuclassvirtualclassname
        jinja2_env.filters['wuclassconstname'] = wuclassconstname
        jinja2_env.filters['wuclassgenclassname'] = wuclassgenclassname
        jinja2_env.filters['propertyconstname'] = propertyconstname
        jinja2_env.filters['propertyconstantvalue'] = propertyconstantvalue
        jinja2_env.filters['getPropertyByName'] = getPropertyByName
        output = open(os.path.join(JAVA_OUTPUT_DIR, name + ".java"), 'w')
        output.write(jinja2_env.get_template('application2.java').render(name=name, changesets=changesets))
        output.close()
