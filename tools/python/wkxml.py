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
        return to_c(raw).upper()

class WuClassDef:
    def __init__(self, name, id, properties, virtual, soft):
        self.__name = name  # an unicode for class' name
        self.__id = id      # an integer for class' id
        self.__properties = properties  # a dict of WuClassProperty objects accessed thru the prop's name
        self.__virtual = virtual    # a boolean for virtual or native
        self.__soft = soft  # a boolean for soft or hard
    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self.__name,self.__id,str(self.__virtual),str(self.__soft),str(self.__properties))
    def getProperties(self):
        return tuple(self.__properties.values())
    def getId(self):
        return self.__id
    def isVirutal(self):
        return self.__virtual
    def isSoft(self):
        return self.__soft
"""    def getJavaName(self):
        return Convert.to_java(self.__name)
    def getCName(self):
        return Convert.to_c(self.__name)
    def getConstName(self):
        return Convert.to_constant(self.__name)"""

class WuClassProperty:
    def __init__(self, name, id, datatype, access):
        self.__name = name  # an unicode for property's name
        self.__id = id      # an integer for property's id
        self.__datatype = datatype # a WuType object
        self.__access = access  # an unicode for property's accessability
    def __repr__(self):
        return "WuClassProperty %s (id=%s, datatype=%s access:%s)" % (self.__name, self.__id, self.__datatype, self.__access)
    def getId(self):
        return self.__id
    def getAccess(self):
        return self.__access
    def getType(self):
        return self.__datatype
"""    def getConstName(self):
        return Convert.to_constant(self.__name)
    def getConstType(self):
        return self.__datatype.getConstType()
    def getConstAccess(self):
        return Convert.to_constant(self.__access)
    def getJavaConst(self):
        return self.getConstType() + "_" + self.getConstName()"""
 
class WuType:
    def __init__(self, name, type, values):
        self.__name = name  # an unicode for WuType's name
        self.__type = type  # an unicode for WuType's type (for now, there is only one enum type)
        self.__values = values # a tuple of sequential unicode values of the specified type
    def __repr__(self):
        return "WuType %s (type=%s) val:%s" % (self.__name, self.__type, str(self.__values))
"""    def getConstType(self):
        return Convert.to_constant(self.__type)
    def getConstName(self):
        return Convert.to_constant(self.__name)
    def getJavaName(self):
        return Convert.to_java(self.__name)
    def getConstValueByIndex(self,index):
        return Convert.to_constant(self.__values[index])"""
       
class WuClassXMLParser:
    def __init__(self, path):
        self.__wuClasses = {}
        self.__wuTypes = {}
        self.__parse(path)
        self.__path = path
    
    def __repr__(self):
        return "//// wuTypes\n%s\n//// wuClasses\n%s" % (str(self.__wuTypes), str(self.__wuClasses))
    
    def __parse(self, path):
        dom = parse(path)
        trueFalse = {u'true':True, u'false':False, u'soft':True, u'hard':False}
        access = {u'readwrite':'rw', u'readonly':'ro', u'writeonly':'wo'}

        """self.__wuTypes.update({
            wt.getAttribute('name'): WuType( wt.getAttribute('name'),wt.getAttribute('type'),tuple([e.getAttribute('value') for e in wt.getElementsByTagName('enum')]) ) for wt in dom.getElementsByTagName('WuTypedef') if wt.getAttribute('type') == u'enum'
        })"""

        for wuType in dom.getElementsByTagName('WuTypedef'):
            assert wuType.getAttribute('type') == u'enum'
            self.__wuTypes[wuType.getAttribute('name')] = WuType( wuType.getAttribute('name'), wuType.getAttribute('type'), tuple([e.getAttribute('value') for e in wuType.getElementsByTagName('enum')]) )
        self.__wuTypes.update({i:WuType(i,i,None) for i in [u'short', u'boolean', u'refresh_rate']})

        for wuClass in dom.getElementsByTagName('WuClass'):
            wuClassName = wuClass.getAttribute('name')
            wuClassId = int(wuClass.getAttribute('id'),0)
            wuClassProp = {}
            for i, prop in enumerate(wuClass.getElementsByTagName('property')):
                propType = prop.getAttribute('datatype')
                propName = prop.getAttribute('name')
                assert propType in self.__wuTypes, "Error! undefinded property type %s in xml %s" % (propType, path)
                wuClassProp[propName] = WuClassProperty(propName, i, self.__wuTypes[propType], access[prop.getAttribute('access')])

            self.__wuClasses[wuClassName] = WuClassDef(wuClassName, wuClassId, wuClassProp, trueFalse[wuClass.getAttribute('virtual')], trueFalse[wuClass.getAttribute('type')])
        del access, trueFalse, dom


    """ function for wuClass """
    def getWuClass(self, query):
        assert query in self.__wuClasses, "Error! the class %s is not defined in the xml %s" % (classQuery, self.__path)
        return self.__wuClasses[query]

    def getAllWuClasses(self):
        return self.__wuClasses.values()

    def isValidWuClass(self, query):
        return query in self.__wuClasses

    """ function for wuClass Property """
    def getProperty(self, classQuery, propQuery):
        p = self.getWuClass(classQuery).getProperties()
        assert propQuery in p, "Error! the property %s is not belonged to the class %s in the xml %s" % (classQuery, propQuery, self.__path)
        return p[propQuery]

    def isValidProperty(self, classQuery, propQuery):
        if not self.isValidWuClass(classQuery): return False
        return propQuery in self.getWuClass(classQuery).getProperties()

    """ function for wuClass Property Datatype"""    
    def getWuTypeValues(self, typeQuery):
        assert typeQuery in self.__wuTypes, "Error! the type %s is not defined in the xml %s" % (typeQuery, self.__path)
        t = self.__wuTypes[typeQuery]
        if type(t) is unicode: return None
        return t.getValues()

    def isValidWuType(self, query):
        return query in self.__wuTypes

    def getPropertyWuTypeValues(self, classQuery, propQuery, typeQuery):
        return self.getWuTypeValues(self.getProperty(classQuery, propQuery).getDatatype())

#if __name__ == '__main__':
#    p = Parser("../../ComponentDefinitions/WuKongStandardLibrary.xml")
