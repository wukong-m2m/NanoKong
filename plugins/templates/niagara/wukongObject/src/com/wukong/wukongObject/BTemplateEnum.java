package com.wukong.wukongObject;

import java.io.*;
import javax.baja.*;
import javax.baja.sys.*;
import javax.baja.sys.Property.*;
import javax.baja.sys.Action.*;
import javax.baja.sys.Topic.*;

public final class B{{ component.get("name")|convert_filename_to_java }} extends BFrozenEnum {

/*-
enum B{{ component.get("name")|convert_filename_to_java }}
{
  range 
  {
    {% for property in component.xpath("enum") %}{{ property.get("value").lower() }},
    {% endfor %}
  }
}
-*/

}
