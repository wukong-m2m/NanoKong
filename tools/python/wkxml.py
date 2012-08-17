#!/usr/bin/python

from xml.dom.minidom import parse
import re

class Convert:
    @staticmethod
    def CamelCase_to_underscore(name):
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
        return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

    @staticmethod
    def underscore_to_CamelCase(name):
        return ''.join(x.capitalize() or '_' for x in name.split('_'))

    @staticmethod
    def to_c(raw):
        if re.search('.*_.*', raw) == None:
            return Convert.CamelCase_to_underscore(raw)
        else:
            return raw.lower()

    @staticmethod
    def to_java(raw):
        if re.search('.*_.*', raw):
            return Convert.underscore_to_CamelCase(raw)
        else:
          return raw

    @staticmethod
    def to_constant(raw):
        return Convert.to_c(raw).upper()

class WuClassDef:
    def __init__(self, name, id, properties, virtual, soft):
        self.__name = name  # an unicode for class' name
        self.__java_class_name = "Virtual" + Convert.to_java(self.__name) + "WuObject"
        self.__java_generated_baseclass_name = "GENERATED" + self.__java_class_name
        self.__c_update_function_name = Convert.to_c(self.__name) + "_update"
        self.__c_class_definition_struct_name = Convert.to_c(self.__name)
        self.__java_wkpf_constant_name = "WUCLASS_" + Convert.to_constant(self.__name.upper())

        self.__id = id      # an integer for class' id
        self.__properties = properties  # a dict of WuClassProperty objects accessed thru the prop's name
        for prop in self.__properties:
            prop.setJavaConstName(self.__name.upper())
        self.__virtual = virtual    # a boolean for virtual or native
        self.__soft = soft  # a boolean for soft or hard
    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self.__name,self.__id,str(self.__virtual),str(self.__soft),str(self.__properties))
    def getXmlName(self):
        return self.__name 
    def getJavaClassName(self):
        return self.__java_class_name
    def getJavaGenClassName(self):
        return self.__java_generated_baseclass_name
    def getCStructName(self):
        return self.__c_class_definition_struct_name
    def getJavaConstName(self):
        return self.__java_wkpf_constant_name
    def getNiagaraName(self):
        return self.__name

    def getProperties(self):
        return tuple(self.__properties)
    def getId(self):
        return self.__id
    def isVirtual(self):
        return self.__virtual
    def isSoft(self):
        return self.__soft

class WuClassProperty:
    def __init__(self, name, id, datatype, access):
        self.__name = name  # an unicode for property's name
        self.__java_wkpf_constant_name = "PROPERTY_%s_" + self.__name.upper()

        self.__id = id      # an integer for property's id
        self.__datatype = datatype # a WuType object
        self.__access = access  # an unicode for property's accessability
    def __repr__(self):
        return "WuClassProperty %s (id=%s, datatype=%s access:%s)" % (self.__name, self.__id, self.__datatype, self.__access)
    def getXmlName(self):
        return self.__name 
    def getJavaConstName(self):
        return self.__java_wkpf_constant_name
    def getNiagaraName(self):
        return self.__name
    def setJavaConstName(self, class_name):
        self.__java_wkpf_constant_name = self.__java_wkpf_constant_name % class_name    
    def getId(self):
        return self.__id
    def getType(self):
        return self.__datatype
    def getAccess(self):
        return self.__access
    def getTypeObject(self):
        return self.__datatype

class WuType:
    def __init__(self, name, type, values):
        self.__name = name  # an unicode for WuType's name
        self.__type = type  # an unicode for WuType's type (for now, there is only one enum type)
        self.__java_wkpf_constant_name = Convert.to_constant(self.__type) + "_" + Convert.to_constant(self.__name)

        self.__values = values # a tuple of sequential unicode values of the specified type
    def __repr__(self):
        return "WuType %s (type=%s) val:%s" % (self.__name, self.__type, str(self.__values))
    def getXmlName(self):
        return self.__name 
    def getJavaConstName(self):
        return self.__java_wkpf_constant_name
    
    def getType(self):
        return self.__type
    def getValues(self):
        return self.__values
    def getIndexOfValue(self, value):
        return self.__values.index(value)
    def getValueByIndex(self, index):
        return self.__values[index]
    def getJavaConstValueByIndex(self, index):
        return self.__java_wkpf_constant_name + "_" + self.__values[index].upper()
    def getJavaConstValueByValue(self, value):
        return self.getJavaConstValueByIndex(self.getIndexOfValue(value))

class WuClassXMLParser:
    def __init__(self, path):
        self.__wuClasses = []
        self.__wuTypes = [WuType(i,i,None) for i in [u'short', u'boolean', u'refresh_rate']]
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
            self.__wuTypes += [ WuType( wuType.getAttribute('name'), wuType.getAttribute('type'), tuple([e.getAttribute('value') for e in wuType.getElementsByTagName('enum')]) ) ]

        def comparison(s): return lambda x: x.getXmlName() == s
        for wuClass in dom.getElementsByTagName('WuClass'):
            wuClassName = wuClass.getAttribute('name')
            wuClassId = int(wuClass.getAttribute('id'),0)
            wuClassProp = []
            for i, prop in enumerate(wuClass.getElementsByTagName('property')):
                propType = prop.getAttribute('datatype')
                propName = prop.getAttribute('name')
                assert self.isValidWuType(comparison(propType), propType), "Error! undefinded property type %s in xml %s" % (propType, path)
                wuClassProp += [ WuClassProperty(propName, i, propType, access[prop.getAttribute('access')]) ]

            self.__wuClasses += [ WuClassDef(wuClassName, wuClassId, wuClassProp, trueFalse[wuClass.getAttribute('virtual')], trueFalse[wuClass.getAttribute('type')]) ]
        del access, trueFalse, dom

    def isValidWuType(self, lambda_func, typeQuery):
        return len(filter(lambda_func, self.__wuTypes)) == 1

    def isValidWuClass(self, lambda_func, classQuery):
        return len(filter(lambda_func, self.__wuClasses)) == 1

    def getAllWuClasses(self):
        return tuple(self.__wuClasses)

    def getAllWuTypes(self):
        return tuple(self.__wuTypes)

    def getFilePath(self):
        return self.__path

#if __name__ == '__main__':
#    p = Parser("../../ComponentDefinitions/WuKongStandardLibrary.xml")
