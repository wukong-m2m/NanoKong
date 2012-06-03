package com.wukong.wukongObject;

import java.io.*;
import javax.baja.*;
import javax.baja.sys.*;
import javax.baja.sys.Property.*;
import javax.baja.sys.Action.*;
import javax.baja.sys.Topic.*;

{% for dependency in component.xpath("property") %}{% if dependency.get("depends") %}import com.wukong.wukongObject.B{{ dependency.get("depends") }};
import com.wukong.wukongObject.B{{ dependency.get("depends") }}.*;{% endif %}{% endfor %}

public final class B{{ component.get("name")|convert_filename_to_java }} extends BComponent {

/*-
class B{{ component.get("name")|convert_filename_to_java }}
{
  properties 
  {
      {% for property in component.xpath("property") %}
      {{ property.get("name")|convert_filename_to_java }}: {% if property.get("depends") %}B{{ property.get('datatype') }}{% else %}{% if property.get("datatype").lower() != 'short' %}{{ property.get("datatype") }}{% else %}int{% endif %}{% endif %}
      flags { summary }
      {% if property.get("default") %}default {[ {% if property.get("depends") %}B{% endif %}{{ property.get("default") }} ]}
      {% elif property.get("datatype").lower() == 'boolean' %}default {[ false ]}
      {% elif property.get("datatype").lower() == 'short' %}default {[ 0 ]}
      {% elif property.get("datatype").lower() == 'int' %}default {[ 0 ]}
      {% elif property.get("datatype").lower() == 'double' %}default {[ 0.0 ]}
      {% elif property.get("datatype").lower() == 'float' %}default {[ 0.0 ]}
      {% elif property.get("datatype").lower() == 'string' %}default {[ "" ]}
      {% endif %}

      {% endfor %}
  }
}
-*/

}
