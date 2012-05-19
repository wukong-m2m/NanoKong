#!/usr/bin/python

# Translator convert the flow of WuKong components and their definitions into one application Java file
#
# Author: B.L.Tsai
# Date: May 18, 2012

import os
import sys
sys.path.append("../python")
from wkpf import NodeInfo, WuClass, WuObject
import pickle
from xml.dom.minidom import parse
from optparse import OptionParser
from jinja2 import Template
from struct import pack
from operator import itemgetter

print_wuClasses = False
print_Components = True
print_links = True
out_fd = None

def indentor(s, num):
    return "\n".join((num * 4 * " ") + line for line in s.splitlines() if line.strip() != '')

def loadNodeList(filename):
  f = open(filename, "r")
  node_list = pickle.load(f)
  f.close()
  return node_list

def saveNodeList(node_list, filename):
  f = open(filename, "w")
  pickle.dump(node_list, f)
  f.close()

def getNodeList(options):
  if (not options.do_discovery and not options.use_hardcoded_discovery and not options.discovery_file):
    print "Please specify either -D to do network discovery, -d to use a previously stored discovery result, or store the result of the discovery, or -H to use the hardcoded discovery result (DEBUG ONLY)"
    exit()

  if options.do_discovery:
    import wkpfcomm
    node_list = wkpfcomm.getNodeInfos();
  elif options.use_hardcoded_discovery:
    node1 = NodeInfo(nodeId=1,
                     wuClasses=(WuClass(nodeId=1, wuClassId=0, isVirtual=False),
                                WuClass(nodeId=1, wuClassId=1, isVirtual=False),
                                WuClass(nodeId=1, wuClassId=3, isVirtual=False),
                                WuClass(nodeId=1, wuClassId=5, isVirtual=False)), # generic, threshold, numeric_controller, light_sensor
                     wuObjects=(WuObject(nodeId=1, portNumber=0, wuClassId=0),
                                WuObject(nodeId=1, portNumber=1, wuClassId=3),
                                WuObject(nodeId=1, portNumber=2, wuClassId=5))) # numeric_controller at port 1, light sensor at port 2
    node3 = NodeInfo(nodeId=3,
                     wuClasses=(WuClass(nodeId=3, wuClassId=0, isVirtual=False),
                                WuClass(nodeId=3, wuClassId=1, isVirtual=False),
                                WuClass(nodeId=3, wuClassId=4, isVirtual=False)), # generic, threshold, light
                     wuObjects=(WuObject(nodeId=3, portNumber=0, wuClassId=0),
                                WuObject(nodeId=3, portNumber=4, wuClassId=4))) # light at port 4
    node_list = (node1, node3)
  else:
    node_list = loadNodeList(options.discovery_file)

  if (node_list == None):
    print "Unable to load node list"
    exit()

  if (options.do_discovery and options.discovery_file): # Save the discovery result for future use
    saveNodeList(node_list, options.discovery_file)
  return node_list

def parser():
    parser = OptionParser("usage: %prog [options]")
    parser.add_option("-c", "--component", action="store", type="string", dest="pathc", help="WuKong Component XML file path")
    parser.add_option("-f", "--flow", action="store", type="string", dest="pathf", help="WuKong Flow XML file path")
    parser.add_option("-o", "--output-path", action="store", type="string", dest="out", help="the dest. folder path of output java file")
    parser.add_option("-d", "--discovery-result", action="store", type="string", dest="discovery_file", help="The file containing the discovery result. Will be read if -D isn't specified, or created/overwritten if -D is specified. Either -D, -H, -d or both must be used.")
    parser.add_option("-D", "--do-discovery", action="store_true", dest="do_discovery", help="Do a new discovery of the network nodes. If a discovery file is specified using -d, the result will be stored there for future use. Either -D, -H, -d or both must be used.")
    parser.add_option("-H", "--use-hardcoded-discovery", action="store_true", dest="use_hardcoded_discovery", help="Use hardcoded discovery result (DEBUG ONLY).")
    
    (options, args) = parser.parse_args()
    if not options.pathc or not options.pathf: parser.error("invalid component and flow xml, please refer to -h for help")

    node_list = getNodeList(options)

    c_dom = parse(options.pathc)
    f_dom = parse(options.pathf)
    java_class_name = f_dom.getElementsByTagName('application')[0].getAttribute('name')
    out_dir = options.out
    if not out_dir: out_dir = os.getcwd()
    assert os.path.exists(out_dir), "Error! the specified output directory does not exist."
    global out_fd
    out_fd = open(os.path.join(out_dir, java_class_name+".java"), 'w')
   
    ## First, parse out WuClasse definitions with additional customized property types
    path = options.pathc
    wuTypedefs_dict = {}
    for wuTypedef in c_dom.getElementsByTagName('WuTypedef'):
        tmp_typedef_dict = {}
        if wuTypedef.getAttribute('type') == u'enum':
            for enum in wuTypedef.getElementsByTagName('enum'):
                tmp_typedef_dict[enum.getAttribute('value')] = 'WKPF.ENUM_%s_' + enum.getAttribute('value').upper()
        assert len(tmp_typedef_dict) > 0, 'Error! unclear wuTypedef %s in xml %s' % (wuTypedef.getAttribute('name'), path)
        wuTypedefs_dict[wuTypedef.getAttribute('name')] = tmp_typedef_dict
    
    if print_wuClasses: print>>out_fd, "//========== WuClass Definitions =========="
    wuClasses_dict = {}
    truefalse_dict = {u'true':True, u'false':False, u'soft':True, u'hard':False}
    tmp_access_dict = {u'readwrite':'rw', u'readonly':'ro', u'writeonly':'wo'}
    for wuClass in c_dom.getElementsByTagName('WuClass'):
        wuClassName = wuClass.getAttribute('name')
        wuClassId = int(wuClass.getAttribute('id'),0)
        tmp_prop_dict = {}
        for i, prop in enumerate(wuClass.getElementsByTagName('property')):
            prop_type = prop.getAttribute('datatype')
            prop_name = prop.getAttribute('name')
            # if the property is customized
            if prop_type in wuTypedefs_dict:
                prop_type = wuTypedefs_dict[prop_type]
                # since we know the customized property's name, update the const. str.
                for enum_value, enum_str in prop_type.items():
                    prop_type[enum_value] = enum_str % (wuClassName.upper()+"_"+prop_name.upper())
            tmp_prop_dict[prop_name] = {'id':i, 'type':prop_type, 'access':tmp_access_dict[prop.getAttribute('access')]}
        assert len(tmp_prop_dict) > 0, 'Error! no property found in wuClass %s in xml %s' % (wuClassName, path)
        wuClasses_dict[wuClassName] = {'id':wuClassId, 'prop':tmp_prop_dict, 'vrtl':truefalse_dict[wuClass.getAttribute('virtual')],'soft':truefalse_dict[wuClass.getAttribute('type')]}
        assert wuClassId not in wuClasses_dict, "Error! wuClass %s's id % conflict with wuClass %s in xml %s" % (wuClassName, wuClassId, wuClasses_dict[wuClassId], path)
        wuClasses_dict[wuClassId] = wuClassName
        if print_wuClasses: print>>out_fd, "//", wuClassName, wuClasses_dict[wuClassName]
    if print_wuClasses: print>>out_fd, "//"

    wuTypedefs_dict = tmp_prop_dict = tmp_access_dict = tmp_typedef_dict = c_dom = {}

    # wuClasses_dict [ wuClass' name ] = { 
    #   'id' : wuClass ID
    #   'prop' : wuClass' properties
    #            {'id' : property's ID, 'type' : u'short' or u'boolean' or enum's dict[enum's value] , 'access': 'rw' or 'ro' or 'wo'}
    #   'vrtl' : true or false (whether wuClass is virtual or not)
    #   'soft' : true (soft) or false (hard)
    # }

    def findProperty(prop_name, class_dict, class_name, path):
        assert class_name in class_dict, 'Error! illegal component type (wuClass name) %s in xml %s' % (wuClassName, path)
        tmp_prop_dict = class_dict[class_name]['prop']
        assert prop_name in tmp_prop_dict, 'Error! property %s is not in wuClass %s in xml %s' % (prop_name,class_name,path)
        return tmp_prop_dict[prop_name]

    ## Second, parse out components (wuClass instances) and links btwn components
    if print_Components: print>>out_fd, "//========== Components Definitions =========="
    components_dict = {}
    links_list = []
    path = options.pathf
    for i, component in enumerate(f_dom.getElementsByTagName('component')):
        wuClassName = component.getAttribute('type')
        instanceName = component.getAttribute('instanceId')

        tmp_dflt_list = []
        for prop in component.getElementsByTagName('property'):
            prop_name = prop.getAttribute('name')
            prop_type = findProperty(prop_name, wuClasses_dict, wuClassName, path)['type']
            prop_dflt_value = prop.getAttribute('default')
            if type(prop_type) is dict:
                prop_dflt_value = prop_type[prop_dflt_value]    # it becomes type str
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
        if print_Components: print>>out_fd, "//", i, instanceName, components_dict[instanceName]

        for link in component.getElementsByTagName('link'):
            toInstanceName = link.getAttribute('toInstanceId')
            links_list += [ (instanceName, link.getAttribute('fromProperty'), toInstanceName, link.getAttribute('toProperty'), toInstanceName) ]
    if print_Components: print>>out_fd, "//"

    def short2byte(i): return [ord(b) for b in pack("H", i)]

    ## Finally, check the validity of links and output as a string
    if print_links: 
        print>>out_fd, "//========== Links Definitions =========="
        print>>out_fd, "// fromCompInstanceId(2 bytes), fromPropertyId(1 byte), toCompInstanceId(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)"
    link_table_str = ''
    for link in links_list:
        assert link[2] in components_dict, 'Error! cannot find target component %s linked from component %s in xml %s' % (link[2], link[0])
   
        fromInstanceId = short2byte(components_dict[ link[0] ]['cmpid'])
        fromPropertyId = findProperty(link[1], wuClasses_dict, components_dict[ link[0] ]['class'], path)['id']
        link_table_str += "(byte)%d,(byte)%d, " % (fromInstanceId[0], fromInstanceId[1])
        link_table_str += "(byte)%d,  " % fromPropertyId

        toInstanceId = short2byte(components_dict[ link[2] ]['cmpid'])
        toPropertyId = findProperty(link[3], wuClasses_dict, components_dict[ link[2] ]['class'], path)['id']
        link_table_str += "(byte)%d,(byte)%d, " % (toInstanceId[0], toInstanceId[1])
        link_table_str += "(byte)%d,  " % toPropertyId

        wuClassId = short2byte(components_dict[ link[4] ]['classid'])
        link_table_str += "(byte)%d, (byte)%d,\n" % (wuClassId[0], wuClassId[1])
        if print_links: 
            print>>out_fd, "//", link
            print>>out_fd, "//", (fromInstanceId[0],fromInstanceId[1]), fromPropertyId, (toInstanceId[0], toInstanceId[1]), toPropertyId, (wuClassId[0], wuClassId[1])
    if print_links: print>>out_fd, "//"

    return java_class_name, wuClasses_dict, components_dict, indentor(link_table_str,1), out_dir, node_list

    # components_dict[instanceName] = {
    #   'cmpid': component's ID, 
    #   'class': WuClass's Name, 
    #   'classid': WuClasses's ID,
    #   'defaults': a list of tuples [ ( property's name, property's default value(int or bool value) ) ]
    # }

def mapper(wuClasses_dict, components_dict, node_list):
    # find nodes
    hard_dict = {}
    soft_dict = {}
    node_port_dict = {}
    for node in node_list:
        for wuObject in node.nativeWuObjects:
            assert wuObject.wuClassId in node.nativeWuClasses, 'Error! the wuClass of wuObject %s does not exist on node %d' % (wuObject, node.nodeId)
            if node.nodeId in node_port_dict:
                node_port_dict[node.nodeId] = sorted(node_port_dict[node.nodeId] + [wuObject.portNumber])
            else:
                node_port_dict[node.nodeId] = [wuObject.portNumber]

            if wuObject.wuClassId in hard_dict:
                hard_dict[wuObject.wuClassId] += [(node.nodeId, wuObject.portNumber)]
            else:
                hard_dict[wuObject.wuClassId] = [(node.nodeId, wuObject.portNumber)]

        for wuClassId in node.nativeWuClasses:
            wuClass = wuClasses_dict[wuClasses_dict[wuClassId]]
            if wuClass['soft']:
                if wuClassId in soft_dict:
                    soft_dict[wuClassId] += [(node.nodeId, False)] # (nodeId, virtual?) for now, assumed all to be native
                else:
                    soft_dict[wuClassId] = [(node.nodeId, False)] # (nodeId, virtual?) for now, assumed all to be native

    components_list = sorted(components_dict.values(), key=itemgetter('cmpid'))

    ## TODO: define mapping algorithm & generate mapping table
    map_table_str = ""

    ## generate WKPF initialization statements
    reg_func_call = "WKPF.registerWuClass(WKPF.WUCLASS_%s, GENERATEDVirtual%sWuObject.properties);\n"
    create_obj_call = "WKPF.createWuObject((short)WKPF.WUCLASS_%s, WKPF.getPortNumberForComponent((short)%d), %s);\n" 
    set_prop_call = "WKPF.setProperty%s((short)%d, WKPF.PROPERTY_%s_%s, %s);\n" 
    reg_stmts = ''
    init_stmts = ''

    for component in components_list:
        wuClassName = component['class']
        wuClass = wuClasses_dict[wuClassName]
        wuClassName = wuClassName.replace(' ', '_')
        cmpId = component['cmpid']
        if_stmts = ''

        if wuClass['soft']: # it is a soft component
            if wuClass['id'] in soft_dict:
                soft_component = soft_dict[wuClass['id']][0]
                # pick the first
                port_list = node_port_dict[soft_component[0]]
                vrtlflag = soft_component[1]
            else:
                port_list = node_port_dict.itervalues().next()
                vrtlflag = True
            port_num = None
            for i in range(len(port_list)+1):
                if i not in port_list:
                    port_list += [i]
                    port_list.sort()
                    port_num = i
                    break
            assert port_num != None, "Error! cannot find an unique port number"
            map_table_str += "(byte)%d, (byte)%d, \n" % (soft_component[0], port_num)
            if vrtlflag: # if it is virtual?
                reg_stmts += reg_func_call % (wuClassName.upper(), wuClassName.replace('_',''))
                wuClassInstVar = "wuclassInstance%s" % wuClassName
                if_stmts += "VirtualWuObject %s = new Virtual%sWuObject();\n" % (wuClassInstVar, wuClassName.replace('_', ''))
                if_stmts += create_obj_call % (wuClassName.upper(), cmpId, wuClassInstVar)
            else:
                if_stmts += create_obj_call % (wuClassName.upper(), cmpId, "null")
        else: # it is a hard component
            hard_component = hard_dict[wuClass['id']][0]
            map_table_str += "(byte)%d, (byte)%d, \n" % (hard_component[0], hard_component[1])

        # set default values
        for prop_name, prop_value in component['defaults']:
            prop_type = type(prop_value)
            if prop_type is int:
                if_stmts += set_prop_call % ("Short", cmpId, wuClassName.upper(), prop_name.upper(), "(short)"+str(prop_value))
            elif prop_type is bool:
                if_stmts += set_prop_call % ("Boolean", cmpId, wuClassName.upper(), prop_name.upper(), str(prop_value).lower()) 
            elif prop_type is unicode:
                if_stmts += set_prop_call % ("Short", cmpId, wuClassName.upper(),prop_name.upper(), prop_value)
            elif prop_type is tuple and prop_value[0] == 'r':
                if_stmts += set_prop_call % ("RefreshRate", cmpId, wuClassName.upper(),prop_name.upper(), "(short)"+str(prop_value[1]))
            else:
                assert False, 'Error! property %s of unknown type %s' % (prop_name.upper(), prop_type)

        init_stmts += "if (WKPF.isLocalComponent((short)%d)) {\n%s\n}\n" % (cmpId, indentor(if_stmts,1)) if if_stmts != '' else '// no need to init component %d' % cmpId

    return indentor(map_table_str, 1), reg_stmts + init_stmts


def javacodegen(link_table, map_table, comp_init, java_class_name):
    
    import_stmt = indentor("""
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;
    """, 0)

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
    comp_init_stmts = indentor(comp_init, 2)

    main_loop = indentor("""
VirtualWuObject wuclass = WKPF.select();
if (wuclass != null) {
    wuclass.update();
}
    """, 3)

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
{{ COMPONENT_INIT }}
    }
}
    """)

    rendered_tpl = tpl.render(IMPORT_STATEMENTS=import_stmt,
            CLASS_NAME=java_class_name,
            LINK_DEFINITIONS=linkDefinitions,
            MAPPING_DEFINITIONS=componentInstanceToWuObjectAddrMap,
            WKPF_INIT_STATEMENTS=wkpf_init,
            MAIN_LOOP_STATEMENTS=main_loop,
            COMPONENT_INIT_FUNC_NAME=comp_init_func_name,
            COMPONENT_INIT=comp_init_stmts
        )

    print>>out_fd, "//========== Code =========="
    print>>out_fd, rendered_tpl

if __name__ == "__main__":
    java_class_name, wuClasses_dict, components_dict, links_table, out_dir, node_list = parser()
    map_table, comp_init = mapper(wuClasses_dict, components_dict, node_list)
    javacodegen(links_table, map_table, comp_init, java_class_name)
    print "Translator msg: the file %s.java generated is on the path %s" % (java_class_name, out_dir)
