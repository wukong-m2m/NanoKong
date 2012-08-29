# vim: ts=4 sw=4
#!/usr/bin/python

import xml.dom.minidom
import re

classTypes = {'true':True, 'false':False, 'soft':True, 'hard':False}
accessTypes = {'readwrite':'rw', 'readonly':'ro', 'writeonly':'wo'}
basicDataTypes = ['short', 'boolean', 'refresh_rate']

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

class WuClassDefList:
    def __init__(self):
        self.wuclasses = []

    def __repr__(self):
        ret_val = ''
        for wuclass in self.wuclasses:
            ret_val += str(wuclass)+'\n' 
        return ret_val

    def __contains__(self, typeName):
        return typeName in [wuclass.getXmlName() for wuclass in self.wuclasses]

    def getByTypeName(self, typeName):
        for wuclass in self.wuclasses:
            if typeName == wuclass.getXmlName():
                return wuclass
        return None

    def append(self, wuclassdef):
        self.wuclasses.append(wuclassdef)

class WuClassDef:
    def __init__(self, name, id, properties, virtual, soft):
        self.name = name  # an unicode for class' name
        self.java_class_name = "Virtual" + Convert.to_java(self.name) + "WuObject"
        self.java_generated_baseclass_name = "GENERATED" + self.java_class_name
        self.c_update_function_name = Convert.to_c(self.name) + "_update"
        self.c_class_definition_struct_name = Convert.to_c(self.name)
        self.java_wkpf_constant_name = "WUCLASS_" + Convert.to_constant(self.name.upper())

        self.id = id      # an integer for class' id
        self.properties = properties  # a dict of WuClassProperty objects accessed thru the prop's name
        for prop in self.properties.items():
            prop[1].setJavaConstName(self.name.upper())
            prop[1].setWuClassDefName(self.name)
        self.virtual = virtual    # a boolean for virtual or native
        self.soft = soft  # a boolean for soft or hard

    def __contains__(self, propertyName):
        return propertyName in [property[1].getXmlName() for property in self.properties.items()]

    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self.name,self.id,str(self.virtual),str(self.soft),str(self.properties))

    def getPropertyByName(self, name):
        print self.properties  
        return self.properties[name]

    def getXmlName(self):
        return self.name 
    def getJavaClassName(self):
        return self.java_class_name
    def getJavaGenClassName(self):
        return self.java_generated_baseclass_name
    def getCStructName(self):
        return self.c_class_definition_struct_name
    def getJavaConstName(self):
        return self.java_wkpf_constant_name
    def getWuClassName(self):
        return self.name
    def getProperties(self):
        return tuple(self.properties)
    def getId(self):
        return self.id
    def isVirtual(self):
        return self.isVirtual
    def isSoft(self):
        return self.isSoft

class WuClassProperty:
    def __init__(self, name, id, datatype, value_to_javanames, access):
        self.__wuclassdefName = ''
        self.__name = name  # an unicode for property's name
        self.__java_wkpf_constant_name = "PROPERTY_%s_" + self.__name.upper()

        self.__id = id      # an integer for property's id
        self.__datatype = datatype # a WuType object
        self.__value_to_javanames = value_to_javanames # values for datatype and a dict to look up value to corresponding JavaName
        self.__access = access  # an unicode for property's accessability
        self.default_value = ''
    def __repr__(self):
        return "WuClassProperty %s (id=%s, datatype=%s access:%s)" % (self.__name, self.__id, self.__datatype, self.__access)
    def getXmlName(self):
        return self.__name 
    def getJavaConstName(self):
        return self.__java_wkpf_constant_name
    def getNiagaraName(self):
        return self.__name
    def setWuClassDefName(self, name):
        self.__wuclassdefName = name
    def setJavaConstName(self, class_name):
        self.__java_wkpf_constant_name = self.__java_wkpf_constant_name % class_name    
    def getJavaNameForValue(self, value):
        return self.__value_to_javanames[value]
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
    def getValueInJavaConstByValue(self, value):
        if self.__type.lower() == 'enum':
            return 'GENERATEDWKPF.' + self.__java_wkpf_constant_name + "_" + value.upper()
        else:
            # TODO: not sure what to do here
            return 'GENERATEDWKPF.' + self.__type
    def getWuTypedef(self):
        defs = {}
        if self.__values == None:     #Sen 8.23 add handler in case __values == None
            return defs
        for value in self.__values:
            defs[value] = self.getValueInJavaConstByValue(value)
        return defs

class WuClassXMLParser:
    def __init__(self, path):
        self.__wuClasses = WuClassDefList()
        self.__wuTypes = [WuType(i,i,None) for i in basicDataTypes]
        self.__path = path

    def __repr__(self):
        return "//// wuTypes\n%s\n//// wuClasses\n%s" % (str(self.__wuTypes), str(self.__wuClasses))
    
    def parse(self):
        dom = xml.dom.minidom.parse(self.__path)

        """self.__wuTypes.update({
            wt.getAttribute('name'): WuType( wt.getAttribute('name'),wt.getAttribute('type'),tuple([e.getAttribute('value') for e in wt.getElementsByTagName('enum')]) ) for wt in dom.getElementsByTagName('WuTypedef') if wt.getAttribute('type') == u'enum'
        })"""

        wuTypedefs_dict = {}

        # wuType parsing
        for wuType in dom.getElementsByTagName('WuTypedef'):
            # for enum type
            if wuType.getAttribute('type') == u'enum':
                self.__wuTypes.append( WuType( wuType.getAttribute('name'), wuType.getAttribute('type'), tuple([element.getAttribute('value') for element in wuType.getElementsByTagName('enum')]) ) )
        # create a dict of WuType index by xmlname corresponding to a dict of values to JavaNames
        for wuType in self.getAllWuTypes():
            wuTypedefs_dict[wuType.getXmlName()] = wuType.getWuTypedef()

        # wuClass parsing
        for wuClass in dom.getElementsByTagName('WuClass'):
            wuClassName = wuClass.getAttribute('name')
            wuClassId = int(wuClass.getAttribute('id'),0)
            wuClassProperties = {}
            for i, prop in enumerate(wuClass.getElementsByTagName('property')):
                propType = prop.getAttribute('datatype')
                propName = prop.getAttribute('name')
                # is it necessary?
                def comparison(s): return lambda x: x.getXmlName() == s
                assert self.isValidWuType(comparison(propType), propType), "Error! undefinded property type %s in xml %s" % (propType, path)

                wuClassProperties[propName]= WuClassProperty(propName, i, propType, wuTypedefs_dict[propType], accessTypes[prop.getAttribute('access')]) 
            self.__wuClasses.append( WuClassDef(wuClassName, wuClassId, wuClassProperties, classTypes[wuClass.getAttribute('virtual')], classTypes[wuClass.getAttribute('type')]) )
        return self.__wuClasses

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
