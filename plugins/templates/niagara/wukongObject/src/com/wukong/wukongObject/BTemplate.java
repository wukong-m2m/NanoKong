package com.wukong.wukongObject;

import java.io.*;
import javax.baja.*;
import javax.baja.sys.*;
import javax.baja.sys.Property.*;
import javax.baja.sys.Action.*;
import javax.baja.sys.Topic.*;

public final class B{{ component.get("name")|convert_filename_to_java }} extends BComponent {

/*-
class B{{ component.get("name")|convert_filename_to_java }}
{
  properties 
  {
      {% for property in component.xpath("property") %}
      {{ property.get("name")|convert_filename_to_java }}: {{ property.get("datatype") }}
      flags { summary }
      {% if property.get("default") %}default {[ {{ property.get("default") }} ]}
      {% elif property.get("datatype").lower() == 'boolean' %}default {[ false ]}
      {% elif property.get("datatype").lower() == 'short' %}default {[ "" ]}
      {% endif %}

      {% endfor %}
  }
}
-*/

}
