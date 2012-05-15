from xml.dom.minidom import parse
from optparse import OptionParser
from jinja2 import Template
from struct import pack

def indentor(s, num):
    return "\n".join((num * 4 * " ") + line for line in s.splitlines() if line.strip() != '')


def parser():
    parser = OptionParser("usage: %prog [options] arg")
    parser.add_option("-c", "--component", action="store", type="string", dest="pathc", help="WuKong Component XML file path")
    parser.add_option("-f", "--flow", action="store", type="string", dest="pathf", help="WuKong Flow XML file path")
    (options, args) = parser.parse_args()
    if not options.pathc or not options.pathf:
        parser.error("invalid component and flow xml, please refer to -h for help")
    comp_dom = parse(options.pathc)
    flow_dom = parse(options.pathf)
    return comp_dom, flow_dom


def mapper(comp_dom, flow_dom):
    # mapping table initialization
    flow_elem_list = flow_dom.getElementsByTagName('component')
    class_elem_list = comp_dom.getElementsByTagName('WuClass')

    map_table = []
    name2id = {}
    componentInstanceId = 0
    for elem_f in flow_elem_list:
        componentInstanceName = elem_f.getAttribute('instanceId')
        wuClassName = elem_f.getAttribute('type')
        for elem_c in class_elem_list:
            if elem_c.getAttribute('name') == wuClassName:
                native = True if elem_c.getAttribute('native') == u'true' else False
                soft = True if elem_c.getAttribute('type') == u'soft' else False
                map_table += [[componentInstanceId, componentInstanceName, int(elem_c.getAttribute('id')), wuClassName, 99, 99, elem_f, elem_c, native, soft]]
                break
        else:
            print "Cannot find wuClass %s " % wuClassName
            map_table += [[componentInstanceId, componentInstanceName, 99, '? wuClassName', 99, 99, elem_f, None, True, False]]
    
        name2id[componentInstanceName] = componentInstanceId
        componentInstanceId += 1
    
    for i in map_table:
        print i
    print ''

    # link table generation
    def findProperty(where, target):
        if where != None:
            where = where.getElementsByTagName('property')
            for i in where:
                if i.getAttribute('name') == target:
                    return (where.index(i), i.getAttribute('datatype'), i)
        return (None, None, None)

    link_table = []
    for elem in flow_dom.getElementsByTagName('link'):
        fromInstanceId = name2id[elem.parentNode.getAttribute('instanceId')]
        fromPropertyId = findProperty(map_table[fromInstanceId][7], elem.getAttribute('fromProperty'))[0]
        toInstanceId = name2id[elem.getAttribute('toInstanceId')]
        toPropertyId = findProperty(map_table[toInstanceId][7], elem.getAttribute('toProperty'))[0]
        profileId = map_table[toInstanceId][2]

        if fromPropertyId == None:
            fromPropertyId = 99
        if toPropertyId == None:
            toPropertyId = 99
        if type(profileId) is str:
            profileId = 99

        link_table += [[fromInstanceId, fromPropertyId, toInstanceId, toPropertyId, profileId]]

    link_table_str = ''
    def short2byte(i):
        return [ord(b) for b in pack("H", i)]

    for row in link_table:
        print row
        fii = short2byte(row[0])
        link_table_str += "(byte)%d, (byte)%d, " % (fii[0], fii[1])
        link_table_str += "(byte)%d,\n" % row[1]
        tii = short2byte(row[2])
        link_table_str += "(byte)%d, (byte)%d, " % (tii[0], tii[1])
        link_table_str += "(byte)%d,\n" % row[3]
        pid = short2byte(row[4])
        link_table_str += "(byte)%d, (byte)%d,\n" % (pid[0], pid[1])
    print ''

    # WKPF initialization generation
    comp_init = ''
    for row in map_table:
        stmts = ''
        if not row[8]: # not native
            stmts += "WKPF.registerProfile(Virtual%sProfile.PROFILE_%s, Virtual%sProfile.properties);\n" % (row[3],row[3].upper(),row[3])
            stmts += "VirtualProfile profileInstance%s = new Virtual%sProfile();\n" % (row[3], row[3])
            stmts += "WKPF.createEndpoint((short)Virtual%sProfile.PROFILE_%s, getPortNumberForComponent(%d), profileInstance%s);\n" % (row[3],row[3].upper(),row[0],row[3])
        elif row[9]: # is soft
            stmts += "WKPF.createEndpoint((short)%d, getPortNumberForComponent(%d), null);\n" % (row[2],row[0])

        prop = row[6].getElementsByTagName('property')
        if prop.length > 0:
            for elem in prop: 
                prop_id, prop_type, prop_def = findProperty(row[7], elem.getAttribute('name'))
                if prop_type == 'enum':
                    enum_list = prop_def.getElementsByTagName('enum')
                    for enum_elem in enum_list:
                        if enum_elem.getAttribute('value') == elem.getAttribute('default'):
                            dflt = enum_list.index(enum_elem)+1
                            break
                    else:
                        dflt = -1
                    stmts += "WKPF.setPropertyShort(%d, %d, (short)%d);\n" % (row[0],prop_id,dflt)
                elif prop_type == 'short':
                    dflt = int(elem.getAttribute('default'))
                    stmts += "WKPF.setPropertyShort(%d, %d,  (short)%d);\n" % (row[0],prop_id,dflt)
                elif prop_type == 'boolean':
                    dflt = elem.getAttribute('default')
                    stmts += "WKPF.setPropertyBoolean(%d, %d, %s);\n" % (row[0],prop_id,dflt)
        comp_init += "if (isLocalComponent(%d)) {\n%s\n}\n" % (row[0], indentor(stmts,1))

    # mapping table generation
    map_table_str = "T.B.D"

    return indentor(link_table_str, 1), indentor(map_table_str, 1), comp_init


def codegen(link_table, map_table, comp_init):
    tpl = Template("""
{{ IMPORT_STATEMENTS }}

public class {{ CLASS_NAME }} {

{{ CLASS_FIELD_ALLOCATION_STATEMENTS }}
{{ CLASS_FUNCTION_ALLOCTION_STATEMENTS }}

{{ LINK_DEFINITIONS }}
{{ MAPPING_DEFINITIONS }}

    public static void main (String[] args) {
{{ WKPF_INIT_STATEMENTS }}

{{ COMPONENT_INIT }}
        
        while(true){
{{ MAIN_LOOP_STATEMENTS }}
{{ SCHEDULING_STATEMENTS }}
        }
    }
}
    """)


    import_stmt = indentor("""
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;
    """, 0)

    class_name = "HAScenario2"

    class_field = indentor("""
private static int myNodeId;
private static short tmpDummy = 0;
    """, 1)
    
    class_func = indentor("""
private static byte getPortNumberForComponent(short componentId) {
    return componentInstanceToEndpointMap[componentId*2 + 1];
}
private static boolean isLocalComponent(short componentId) {    
    int nodeId=componentInstanceToEndpointMap[componentId*2];
    return nodeId == myNodeId;
}
    """, 1)

    link_def = indentor("""
private final static byte[] linkDefinitions = {
%s
}
    """ % link_table, 1)

    map_def = indentor("""
private final static byte[] componentInstanceToEndpointMap{
%s
}
    """ % map_table, 1)
    
    wkpf_init = indentor("""
System.out.println("%s");

myNodeId = WKPF.getMyNodeId();
System.out.println("MY NODE ID:" + myNodeId);

WKPF.loadLinkDefinitions(linkDefinitions);
if (WKPF.getErrorCode() == WKPF.OK)
    System.out.println("Registered link definitions.");
else
    System.out.println("Error while Registering link definitions: " + WKPF.getErrorCode());

WKPF.loadComponentToEndpointMap(componentInstanceToEndpointMap);
if (WKPF.getErrorCode() == WKPF.OK)
    System.out.println("Registered component to endpoint map.");
else
    System.out.println("Error while registering component to endpoint map: " + WKPF.getErrorCode());""" % class_name
    , 2)
    
    comp_init = indentor(comp_init, 2)

    main_loop = indentor("""
VirtualProfile profile = WKPF.select();
if (profile != null) {
    profile.update();
}
    """, 3)

    scheduling = indentor("""
if (isLocalComponent(COMPONENT_INSTANCE_ID_LIGHTSENSOR1)) { 
    tmpDummy += 1;
    System.out.println("HAScenario - updating dummy variable to trigger lightsensor update: " + tmpDummy);
    WKPF.setPropertyShort(COMPONENT_INSTANCE_ID_LIGHTSENSOR1, (byte)(WKPF.PROPERTY_LIGHT_SENSOR_CURRENT_VALUE+1), tmpDummy);
    System.out.println("HAScenario - updating dummy variable to trigger lightsensor update, result: " + WKPF.getErrorCode());
    if (WKPF.getErrorCode() != WKPF.OK)
        System.out.println("Error: " + WKPF.getErrorCode());
}
Timer.wait(1000);
    """, 3)

    rendered_tpl = tpl.render(IMPORT_STATEMENTS=import_stmt,
            CLASS_NAME=class_name,
            CLASS_FIELD_ALLOCATION_STATEMENTS=class_field,
            CLASS_FUNCTION_ALLOCTION_STATEMENTS=class_func,
            LINK_DEFINITIONS=link_def,
            MAPPING_DEFINITIONS=map_def,
            WKPF_INIT_STATEMENTS=wkpf_init,
            MAIN_LOOP_STATEMENTS=main_loop,
            COMPONENT_INIT=comp_init,
            SCHEDULING_STATEMENTS=scheduling)

    print rendered_tpl


if __name__ == "__main__":
    comp_dom, flow_dom = parser()
    link_table, map_table, comp_init = mapper(comp_dom, flow_dom)
    codegen(link_table, map_table, comp_init)
