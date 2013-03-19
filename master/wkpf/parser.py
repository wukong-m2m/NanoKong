# vim: ts=4 sw=4
#!/usr/bin/python

import os, sys, traceback
import xml.dom.minidom
from models import *

class Parser:
    @staticmethod
    def parseLibraryXMLString(xml_string):
        dom = xml.dom.minidom.parseString(xml_string)

        print 'scanning types'
        for wuType in dom.getElementsByTagName('WuTypedef'):
            if wuType.getAttribute('type').lower() == 'enum':
                values = [element.getAttribute('value') for element in wuType.getElementsByTagName('enum')]

                wutype = WuType(wuType.getAttribute('name'),
                        wuType.getAttribute('type'), 
                        values)
                print wutype
                wutype.save()

        for wuClass in dom.getElementsByTagName('WuClass'):
            wuclass_name = wuClass.getAttribute('name')
            wuclass_id = int(wuClass.getAttribute('id'),0)
            wuclass_type = wuClass.getAttribute('type')
            wuclass_virtual = True if wuClass.getAttribute('virtual').lower() == 'true' else False
            print 'scanning wuclasses', wuclass_name
            properties = []
            for property_id, prop_tag in enumerate(wuClass.getElementsByTagName('property')):
                property_datatype = prop_tag.getAttribute('datatype')
                property_name = prop_tag.getAttribute('name')
                property_value = prop_tag.getAttribute('default')
                property_access = prop_tag.getAttribute('access')
                print 'scanning properties', property_name

                properties.append(WuProperty(property_id, property_name,
                            property_datatype, property_access, property_value))

            wuclass = WuClass(wuclass_id, wuclass_name, wuclass_virtual, wuclass_type, properties)
            wuclass.save()

    @staticmethod
    def parseLibrary(xml_path):
        print 'start scanning', xml_path
        try:
            xml = open(xml_path)
            return Parser.parseLibraryXMLString(xml.read())
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                          limit=2, file=sys.stdout)
