from xml.dom.minidom import parse
from optparse import OptionParser
from jinja2 import Template
from struct import pack
from operator import itemgetter

def indentor(s, num):
    return "\n".join((num * 4 * " ") + line for line in s.splitlines() if line.strip() != '')


def parser():
    parser = OptionParser("usage: %prog [options] arg")
    parser.add_option("-c", "--component", action="store", type="string", dest="pathc", help="WuKong Component XML file path")
    parser.add_option("-f", "--flow", action="store", type="string", dest="pathf", help="WuKong Flow XML file path")
    (options, args) = parser.parse_args()
    if not options.pathc or not options.pathf:
        parser.error("invalid component and flow xml, please refer to -h for help")
    c_dom = parse(options.pathc)
    f_dom = parse(options.pathf)
   
    ## First, parse out WuClasse definitions with additional self-defined property types
    path = options.pathc
    wuTypedefs_dict = {}
    for wuTypedef in c_dom.getElementsByTagName('WuTypedef'):
        tmp_typedef_dict = {}
        if wuTypedef.getAttribute('type') == u'enum':
            for enum in wuTypedef.getElementsByTagName('enum'):
                tmp_typedef_dict[enum.getAttribute('value')] = 'ENUM_!_' + enum.getAttribute('value').upper()
        assert len(tmp_typedef_dict) > 0, 'Error! unclear wuTypedef %s in xml %s' % (wuTypedef.getAttribute('name'), path)
        wuTypedefs_dict[wuTypedef.getAttribute('name')] = tmp_typedef_dict
    
    print "//========== WuClass Definitions =========="
    wuClasses_dict = {}
    truefalse_dict = {u'true':True, u'false':False, u'soft':True, u'hard':False}
    for wuClass in c_dom.getElementsByTagName('WuClass'):
        tmp_prop_dict = {}
        tmp_access_dict = {u'readwrite':'rw', u'readonly':'ro', u'writeonly':'wo'}
        i = 0
        for prop in wuClass.getElementsByTagName('property'):
            prop_type = prop.getAttribute('datatype')
            if prop_type in wuTypedefs_dict:
                prop_type = wuTypedefs_dict[prop_type]
            tmp_prop_dict[prop.getAttribute('name')] = {'id':i, 'type':prop_type, 'access':tmp_access_dict[prop.getAttribute('access')]}
            i += 1
        assert len(tmp_prop_dict) > 0, 'Error! no property found in wuClass %s in xml %s' % (wuClass.getAttribute('name'), path)
        wuClasses_dict[wuClass.getAttribute('name')] = {'id':int(wuClass.getAttribute('id'),0), 'prop':tmp_prop_dict, 'vrtl':truefalse_dict[wuClass.getAttribute('virtual')],'soft':truefalse_dict[wuClass.getAttribute('type')]}
        print "//", wuClass.getAttribute('name'), wuClasses_dict[wuClass.getAttribute('name')]
    print "//"

    wuTypedefs_dict = tmp_prop_dict = tmp_access_dict = tmp_typedef_dict = {}

    # wuClasses_dict [ wuClass' name ] = { 
    #   'id' : wuClass ID
    #   'prop' : wuClass' properties
    #            {'id' : property's ID, 'type' : u'short' or u'boolean' or enum's dict[enum's value] , 'access': 'rw' or 'ro' or 'wo'}
    #   'vrtu' : true or false (whether wuClass is virtual or not)
    #   'soft' : true (soft) or false (hard)
    # }

    def findProperty(prop_name, class_dict, class_name, path):
        assert class_name in class_dict, 'Error! illegal component type (wuClass name) %s in xml %s' % (wuClassName, path)
        tmp_prop_dict = class_dict[class_name]['prop']
        assert prop_name in tmp_prop_dict, 'Error! property %s is not in wuClass %s in xml %s' % (prop_name,class_name,path)
        return tmp_prop_dict[prop_name]

    ## Second, parse out components (wuClass instances) and links btwn components
    print "//========== Components Definitions =========="
    components_dict = {}
    links_list = []
    i = 0
    path = options.pathf
    for component in f_dom.getElementsByTagName('component'):
        wuClassName = component.getAttribute('type')
        instanceName = component.getAttribute('instanceId')

        tmp_dflt_list = []
        for prop in component.getElementsByTagName('property'):
            prop_name = prop.getAttribute('name')
            prop_type = findProperty(prop_name, wuClasses_dict, wuClassName, path)['type']
            prop_dflt_value = prop.getAttribute('default')
            if type(prop_type) is dict:
                prop_dflt_value = prop_type[prop_dflt_value].replace('!',wuClassName.upper()+"_"+prop_name.upper())    # it becomes type str
            elif prop_type == u'short':
                prop_dflt_value = int(prop_dflt_value,0)
            elif prop_type == u'boolean':
                prop_dflt_value = truefalse_dict[prop_dflt_value]
            elif prop_type == u'refresh_rate':
                prop_dflt_value = ('r', int(prop_dflt_value,0)) # it becomes type tuple
            else:
                assert False, 'Error! property %s of unknown type %s in xml %s' % (prop_name, prop_type, path)
            tmp_dflt_list += [(prop_name, prop_dflt_value)]

        components_dict[instanceName] = {'cmpid':i, 'class':wuClassName, 'classid':wuClasses_dict[wuClassName]['id'], 'defaults':tmp_dflt_list }
        i += 1
        print "//", instanceName, components_dict[instanceName]

        for link in component.getElementsByTagName('link'):
            toInstanceName = link.getAttribute('toInstanceId')
            links_list += [ (instanceName, link.getAttribute('fromProperty'), toInstanceName, link.getAttribute('toProperty'), toInstanceName) ]
    print "//"

    def short2byte(i): return [ord(b) for b in pack("H", i)]

    ## Finally, check the validity of links and output as a string
    print "//========== Links Definitions =========="
    link_table_str = ''
    for link in links_list:
        assert link[2] in components_dict, 'Error! cannot find target component %s linked from component %s in xml %s' % (link[2], link[0])
   
        fromInstanceId = short2byte(components_dict[ link[0] ]['cmpid'])
        fromPropertyId = findProperty(link[1], wuClasses_dict, components_dict[ link[0] ]['class'], path)['id']
        link_table_str += "(byte)%d, (byte)%d, " % (fromInstanceId[0], fromInstanceId[1])
        link_table_str += "(byte)%d,\n" % fromPropertyId

        toInstanceId = short2byte(components_dict[ link[2] ]['cmpid'])
        toPropertyId = findProperty(link[3], wuClasses_dict, components_dict[ link[2] ]['class'], path)['id']
        link_table_str += "(byte)%d, (byte)%d, " % (toInstanceId[0], toInstanceId[1])
        link_table_str += "(byte)%d,\n" % toPropertyId

        wuClassId = short2byte(components_dict[ link[4] ]['classid'])
        link_table_str += "(byte)%d, (byte)%d,\n" % (wuClassId[0], wuClassId[1])
        print "//", link
        print "//", (fromInstanceId[0],fromInstanceId[1]), fromPropertyId, (toInstanceId[0], toInstanceId[1]), toPropertyId, (wuClassId[0], wuClassId[1])
    print "//"

    return wuClasses_dict, components_dict, indentor(link_table_str,1)

    # components_dict[instanceName] = {
    #   'cmpid': component's ID, 
    #   'class': WuClass's Name, 
    #   'classid': WuClasses's ID,
    #   'defaults': a list of tuples [ ( property's name, property's default value(int or bool value) ) ]
    # }

def mapper(wuClasses_dict, components_dict):

    ## TODO: find nodes
    # ?

    ## TODO: find wuClassList
    # wkpf.getWuClassList(nodeId)

    ## TODO: find wuObjectList
    # wkpf.getWuObjectList(nodeId)

    ## TODO: define mapping algorithm & generate mapping table
    # def get():
    map_table_str = """
(byte)1, (byte)0x1, // Component 0: input controller    @ node 1, port 1
(byte)1, (byte)0x2, // Component 1: light sensor        @ node 1, port 2
(byte)3, (byte)0x3, // Component 2: threshold           @ node 3, port 3
(byte)1, (byte)0x5, // Component 3: occupancy           @ node 1, port 5
(byte)3, (byte)0x5, // Component 4: and gate            @ node 3, port 5
(byte)3, (byte)0x4, // Component 5: light               @ node 3, port 4
    """

    ## generate WKPF initialization statements
    reg_stmts = ''
    init_stmts = ''
    for component in sorted(components_dict.values(), key=itemgetter('cmpid')):
        wuClassName = component['class']
        wuClass = wuClasses_dict[wuClassName]
        wuClassName = wuClassName.replace(' ', '_')
        cmpId = component['cmpid']
        if_stmts = ''
        if wuClass['vrtl']: # is virtual?   # TODO: and not found in network
            reg_stmts += "WKPF.registerWuClass(WKPF.WUCLASS_%s, GENERATEDVirtual%sWuObject.properties);\n" % (wuClassName.upper(),wuClassName.replace('_',''))
            if_stmts += "VirtualWuObject wuclassInstance%s = new Virtual%sWuObject();\n" % (wuClassName, wuClassName.replace('_', ''))
            if_stmts += "WKPF.createWuObject((short)WKPF.WUCLASS_%s, WKPF.getPortNumberForComponent((short)%d), wuclassInstance%s);\n" % (wuClassName.upper(),cmpId,wuClassName)
        elif wuClass['soft']: # is soft?
            if_stmts += "WKPF.createWuObject((short)WKPF.WUCLASS_%s, WKPF.getPortNumberForComponent((short)%d), null);\n" % (wuClassName.upper(),cmpId)

        for item in component['defaults']:
            prop_type = type(item[1])
            if prop_type is int:
                if_stmts += "WKPF.setPropertyShort((short)%d, WKPF.PROPERTY_%s_%s, (short)%d);\n" % (cmpId, wuClassName.upper(),item[0].upper(), item[1])
            elif prop_type is bool:
                if_stmts += "WKPF.setPropertyBoolean((short)%d, WKPF.PROPERTY_%s_%s, %s);\n" % (cmpId, wuClassName.upper(),item[0].upper(), str(item[1]).lower()) 
            elif prop_type is unicode:
                if_stmts += "WKPF.setPropertyShort((short)%d, WKPF.PROPERTY_%s_%s, WKPF.%s);\n" % (cmpId, wuClassName.upper(),item[0].upper(), item[1])
            elif prop_type is tuple and item[1][0] == 'r':
                if_stmts += "WKPF.setPropertyRefreshRate((short)%d, WKPF.PROPERTY_%s_%s, (short)%d);\n" % (cmpId, wuClassName.upper(),item[0].upper(), item[1][1])
            else:
                assert False, 'Error! property %s of unknown type %s' % (item[0].upper(), prop_type)

        init_stmts += "if (WKPF.isLocalComponent((short)%d)) {\n%s\n}\n" % (cmpId, indentor(if_stmts,1))

    return indentor(map_table_str, 1), (reg_stmts, init_stmts)


def javacodegen(link_table, map_table, comp_init):
    tpl = Template("""{{ IMPORT_STATEMENTS }}

public class {{ CLASS_NAME }} {
    public static void main (String[] args) {
{{ WKPF_INIT_STATEMENTS }}
        {{ COMPONENT_INIT_FUNC_NAME }}();
        
        while(true){
{{ MAIN_LOOP_STATEMENTS }}
        }
    }

{{ LINK_DEFINITIONS }}

{{ MAPPING_DEFINITIONS }}

    private static void {{ COMPONENT_INIT_FUNC_NAME }}() {
{{ WUCLASS_REGISTRATION }}

{{ COMPONENT_INIT }}
    }
}
    """)

    import_stmt = indentor("""
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;
    """, 0)

    java_class_name = "app"

    linkDefinitions = indentor("""
private final static byte[] linkDefinitions = {
%s
};
    """ % link_table, 1)

    componentInstanceToWuObjectAddrMap = indentor("""
private final static byte[] componentInstanceToWuObjectAddrMap = {
%s
};
    """ % map_table, 1)
    
    wkpf_init = indentor("""
System.out.println("%s");
WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
WKPF.loadLinkDefinitions(linkDefinitions);
    """ % java_class_name
    , 2)
    
    comp_init_func_name = "initialiseLocalWuObjects"
    registration = indentor(comp_init[0], 2)
    comp_init_stmts = indentor(comp_init[1], 2)

    main_loop = indentor("""
VirtualWuObject wuclass = WKPF.select();
if (wuclass != null) {
    wuclass.update();
}
    """, 3)

    rendered_tpl = tpl.render(IMPORT_STATEMENTS=import_stmt,
            CLASS_NAME=java_class_name,
            LINK_DEFINITIONS=linkDefinitions,
            MAPPING_DEFINITIONS=componentInstanceToWuObjectAddrMap,
            WKPF_INIT_STATEMENTS=wkpf_init,
            MAIN_LOOP_STATEMENTS=main_loop,
            COMPONENT_INIT_FUNC_NAME=comp_init_func_name,
            WUCLASS_REGISTRATION=registration,
            COMPONENT_INIT=comp_init_stmts
        )

    print "//========== Code =========="
    print rendered_tpl

if __name__ == "__main__":
    wuClasses_dict, components_dict, links_table = parser()
    map_table, comp_init = mapper(wuClasses_dict, components_dict)
    javacodegen(links_table, map_table, comp_init)
