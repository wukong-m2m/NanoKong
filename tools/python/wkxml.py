#!/usr/bin/python

from xml.dom.minidom import parse
import re

class Convert:
    def __CamelCase_to_underscore(self,name):
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
        return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
    def __underscore_to_CamelCase(self,name):
        return ''.join(x.capitalize() or '_' for x in name.split('_'))
    def to_c(self,raw):
        if re.search('.*_.*', raw) == None:
            return self.__CamelCase_to_underscore(raw)
        else:
            return raw.lower()
    def to_java(self,raw):
        if re.search('.*_.*', raw):
            return self.__underscore_to_CamelCase(raw)
        else:
          return raw
    def to_constant(self,raw):
        return to_c_name(raw).upper()

class WuClass:
    def __init__(self, name, id, properties, virtual, soft):
        self.__name = name
        self.__id = id
        self.__properties = properties
        self.__virtual = virtual
        self.__soft = soft
    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self.__name,self.__id,str(self.__virtual),str(self.__soft),str(self.__properties))
    def getJavaName(self):
        return Convert.to_java(self.__name)
    def getCName(self):
        return Convert.to_c(self.__name)
    def getConstName(self):
        return Convert.to_constant(self.__name)

class WuClassProperty:
    def __init__(self, name, id, datatype, access):
        self.__name = name
        self.__id = id
        self.__datatype = datatype
        self.__access = access
    def __repr__(self):
        return "WuClassProperty %s (id=%s, datatype=%s access:%s)" % (self.__name, self.__id, self.__datatype, self.__access)
    def getConstName(self):
        return Convert.to_constant(self.__name)
    def getConstType(self):
        return Convert.to_constant(self.__datatype)
    def getConstAccess(self):
        return Convert.to_constant(self.__access)
 
class WuType:
    def __init__(self, name, type, values):
        self.__name = name
        self.__type = type
        self.__values = values
    def __repr__(self):
        return "WuType %s (type=%s) val:%s" % (self.__name, self.__type, str(self.__values))
    def getConstType(self):
        return Convert.to_constant(self.__type)
    def getConstName(self):
        return Convert.to_constant(self.__name)
    def getJavaName(self):
        return Convert.to_java(self.__name)
    def getConstValue(self,index):
        return Convert.to_constant(self.__values[index])
       
class Parser:
    def __init__(self, path):
        self.__wuClasses = {}
        self.__wuTypes = {}
        self.__reset(path)
    
    def __repr__(self):
        return "//// wuTypes\n%s\n//// wuClasses\n%s" % (str(self.__wuTypes), str(self.__wuClasses))
    
    def __reset(self, path):
        dom = parse(path)
        trueFalse = {u'true':True, u'false':False, u'soft':True, u'hard':False}
        access = {u'readwrite':'rw', u'readonly':'ro', u'writeonly':'wo'}

        #self.__wuTypes.update({
        #    wt.getAttribute('name'): WuType( wt.getAttribute('name'),wt.getAttribute('type'),tuple([e.getAttribute('value') for e in wt.getElementsByTagName('enum')]) ) for wt in dom.getElementsByTagName('WuTypedef') if wt.getAttribute('type') == u'enum'
        #})

        for wuType in dom.getElementsByTagName('WuTypedef'):
            assert wuType.getAttribute('type') == u'enum'
            self.__wuTypes[wuType.getAttribute('name')] = WuType( wuType.getAttribute('name'), wuType.getAttribute('type'), tuple([e.getAttribute('value') for e in wuType.getElementsByTagName('enum')]) )
        self.__wuTypes.update({i:i for i in [u'short', u'boolean']})

        for wuClass in dom.getElementsByTagName('WuClass'):
            wuClassName = wuClass.getAttribute('name')
            wuClassId = int(wuClass.getAttribute('id'),0)
            wuClassProp = {}
            for i, prop in enumerate(wuClass.getElementsByTagName('property')):
                propType = prop.getAttribute('datatype')
                propName = prop.getAttribute('name')
                assert propType in self.__wuTypes, "Error! undefinded property type %s in xml %s" % (propType, path)
                wuClassProp[propName] = WuClassProperty(propName, i, propType, access[prop.getAttribute('access')])
                wuClassProp[i] = wuClassProp[propName]

            self.__wuClasses[wuClassName] = WuClass(wuClassName, wuClassId, wuClassProp, trueFalse[wuClass.getAttribute('virtual')], trueFalse[wuClass.getAttribute('type')])
            self.__wuClasses[wuClassId] = self.__wuClasses[wuClassName]
        del access, trueFalse, dom


    """ function for wuClass """
    def getWuClass(self, query):
        return self.__wuClasses[query]

    """ function for wuClass Property """
    def getProperty(self, classQuery, propQuery):
        return self.__wuClasses[classQuery].properties[propQuery]

    """ function for wuClass Property Datatype"""    
    def getWuTypeValues(self, typeQuery):
        t = self.__wuTypes[typeQuery]
        if t is str:
            return None
        return t.values

    def getPropertyWuTypeValues(self, classQuery, propQuery, typeQuery):
        return self.getWuTypeValues(self.getProperty(classQuery, propQuery).datatype)

#if __name__ == '__main__':
#    p = Parser("../../ComponentDefinitions/WuKongStandardLibrary.xml")
