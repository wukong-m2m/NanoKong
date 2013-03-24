import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class {{ name }} {

    // =========== Begin: Generated by the translator from application WuML
    /* Component names to indexes:
    {%- for component in changesets.components %}
    {{ component.type }} => {{ component.index }}
    {%- endfor %}
    */

    //link table
    // fromComponentIndex(2 bytes), fromPropertyId(1 byte), toComponentIndex(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)
    //eg. (byte)0,(byte)0, (byte)0, (byte)2,(byte)0, (byte)1, (byte)1,(byte)0
    private final static byte[] linkDefinitions = {
        // Note: Component instance id and wuclass id are little endian
        // Note: using WKPF constants now, but this should be generated as literal bytes by the WuML->Java compiler.
        // Connect input controller to threshold
        {%- for link in changesets.links %}
        {{ link|linkinjava }}{{ ',' if not loop.last else '' }}
        {%- endfor %}
    };

    //component node id and port number table
    // each row corresponds to the component index mapped from component ID above
    // each row has two items: node id, port number
    private final static byte[][] componentInstanceToWuObjectAddrMap = {
      {%- for component in changesets.components %}
      new byte[]{ {%- for wuobject in component.instances %}
        {{ wuobject|wuobjectinjava }}{{ ',' if not loop.last else '' }}
        {%- endfor %}
      },
      {%- endfor %}
    };

    private final static byte[][] heartbeatToNodeAddrMap = {
      {%- for heartbeatgroup in changesets.heartbeatgroups %}
      new byte[]{ {%- for node in heartbeatgroup.nodes %}
        {{ node|nodeinjava }}{{ ',' if not loop.last else '' }}
        {%- endfor %}
      },
      {%- endfor %}
    };

    private final static int[] heartbeatGroupPeriods = {
      {%- for heartbeatgroup in changesets.heartbeatgroups %}
      {{ heartbeatgroup.period }},
      {%- endfor %}
    };
    // =========== End: Generated by the translator from application WuML

    public static void main (String[] args) {
        System.out.println("{{ name }}");
        System.out.println(WKPF.getMyNodeId());
        WKPF.loadHeartbeatToNodeAddrMap(heartbeatToNodeAddrMap);
        WKPF.loadHeartbeatPeriods(heartbeatGroupPeriods);
        WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
        WKPF.loadLinkDefinitions(linkDefinitions);
        initialiseLocalWuObjects();

        while(true){
            VirtualWuObject wuobject = WKPF.select();
            if (wuobject != null) {
                wuobject.update();
            }
        }
    }

    private static void initialiseLocalWuObjects() {
        {%- for component in changesets.components %}
        //all WuClasses from the same group has the same instanceIndex and wuclass
        if (WKPF.isLocalComponent((short){{ component.index }})) {

            {% for wuobject in component.instances %}

            if (WKPF.getMyNodeId() == (short){{ wuobject.node_id }}) {
                {% if not wuobject.hasLocalWuClass %}

                // Virtual WuClasses (Java)
                VirtualWuObject wuclassInstance{{ wuobject.wuclass|wuclassname }} = new {{ wuobject.wuclass|wuclassvirtualclassname }}();
                WKPF.registerWuClass(WKPF.{{ wuobject.wuclass|wuclassconstname }}, {{ wuobject.wuclass|wuclassgenclassname }}.properties);
                WKPF.createWuObject((short)WKPF.{{ wuobject.wuclass|wuclassconstname }}, WKPF.getPortNumberForComponent((short){{ component.index }}), wuclassInstance{{ wuobject.wuclass|wuclassname }});
                {%- for property_tp in wuobject.properties_with_default_values -%}
                {%- set property = property_tp[0]|getPropertyByName -%}
                {%- if property -%}
                {% if property.datatype.lower() == 'boolean' %}
                WKPF.setPropertyBoolean(wuclassInstance{{ wuobject.wuclass|wuclassname }}, WKPF.{{ property|propertyconstname }}, {{ property_tp[1] }});
                {% elif property.datatype.lower() == 'int' or property.datatype.lower() == 'short' %}
                WKPF.setPropertyShort(wuclassInstance{{ wuobject.wuclass|wuclassname }}, WKPF.{{ property|propertyconstname }}, (short){{ property_tp[1] }});
                {% elif property.datatype.lower() == 'refresh_rate' %}
                WKPF.setPropertyRefreshRate(wuclassInstance{{ wuobject.wuclass|wuclassname }}, WKPF.{{ property|propertyconstname }}, (short){{ property_tp[1] }});
                {% else %}
                WKPF.setPropertyShort(wuclassInstance{{ wuobject.wuclass|wuclassname }}, WKPF.{{ property|propertyconstname }}, WKPF.{{ property|propertyconstantvalue }});
                {%- endif -%}
                {%- endif -%}
                {%- endfor -%}

                {% else %}

                // Native WuClasses (C)
                WKPF.createWuObject((short)WKPF.{{ wuobject.wuclass|wuclassconstname }}, WKPF.getPortNumberForComponent((short){{ component.index }}), null);
                {%- for property_tp in wuobject.properties_with_default_values -%}
                {%- set property = property_tp[0]|getPropertyByName -%}
                {%- if property -%}
                {% if property.datatype.lower() == 'boolean' %}
                WKPF.setPropertyBoolean((short){{ component.index }}, WKPF.{{ property|propertyconstname }}, {{ property_tp[1] }});
                {% elif property.datatype.lower() == 'int' or property.datatype.lower() == 'short' %}
                WKPF.setPropertyShort((short){{ component.index }}, WKPF.{{ property|propertyconstname }}, (short){{ property_tp[1] }});
                {% elif property.datatype.lower() == 'refresh_rate' %}
                WKPF.setPropertyRefreshRate((short){{ component.index }}, WKPF.{{ property|propertyconstname }}, (short){{ property_tp[1] }});
                {% else %}
                WKPF.setPropertyShort((short){{ component.index }}, WKPF.{{ property|propertyconstname }}, WKPF.{{ property|propertyconstantvalue }});
                {%- endif -%}
                {%- endif -%}
                {%- endfor -%}

                {% endif %}

            }

            {% endfor %}

        }

        {%- endfor %}
    }
}