from xml.dom.minidom import parse
from optparse import OptionParser
from jinja2 import Template

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
    componentInstanceId = 0
    for elem in flow_elem_list:
        componentInstanceName = elem.getAttribute('instanceId')
        wuClassName = elem.getAttribute('type')
        for elem_t in class_elem_list:
            if elem_t.getAttribute('name') == wuClassName:
                map_table += [[componentInstanceId, componentInstanceName, int(elem_t.getAttribute('id')), wuClassName, '? endpnt ID', '? port #']]
                break
        else:
            print "Cannot find wuClass %s " % wuClassName
            map_table += [[componentInstanceId, componentInstanceName, '? wuClassId', '? wuClassName', '? endpnt ID', '? port #']]

        componentInstanceId += 1
    
    #for i in map_table:
    #    print i

    


def codegen():
    tpl = Template("""
{{ IMPORT_STATEMENTS }}
public class {{ CLASS_NAME }}
{
{{ CLASS_FIELD_ALLOCATION_STATEMENTS }}
{{ CLASS_FUNCTION_ALLOCTION_STATEMENTS }}

{{ LINK_DEFINITIONS }}
{{ MAPPING_TABLE }}

    public static void main (String[] args) {
{{ WKPF_INIT_STATEMENTS }}

{{ COMPONENT_INIT }}
        
        while(true){
{{ MAIN_LOOP_STATEMENTS }}
{{ SCHEDULING_ALGORITHM_STATEMENTS }}
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
}
    """, 1)

    table = 0
    
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
    
    comp_init = 0

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
            MAPPING_TABLE=table,
            WKPF_INIT_STATEMENTS=wkpf_init,
            MAIN_LOOP_STATEMENTS=main_loop,
            COMPONENT_INIT=comp_init)

    #print rendered_tpl


if __name__ == "__main__":
    cdom, fdom = parser()
    mapper(cdom, fdom)
    #codegen()
