#include "config.h"
#include "debug.h"
#include "types.h"
#include "wkpf.h"
#include "native_wuclasses/GENERATED_wuclass_threshold.h"

#ifdef TEST_WKPF

uint16_t passed_count=0;
uint16_t failed_count=0;

void assert_equal_uint(uint16_t a, uint16_t b, char* desc) {
  if (a==b) {
    DEBUGF_TEST("OK: ");
    passed_count++;
  } else {
    DEBUGF_TEST("----------->FAIL: ");
    failed_count++;
  }
  DEBUGF_TEST(desc);
  DEBUGF_TEST("\n");
}

void print_test_summary() {
  DEBUGF_TEST("PASSED: %d, FAILED: %d", passed_count, failed_count);
}

int8_t test_update_dummy = 0;
void update_a(wkpf_local_wuobject *wuobject) {
  DEBUGF_WKPF("UPDATE_A\n");
  test_update_dummy = 1;
}
void update_b(wkpf_local_wuobject *wuobject) {
  DEBUGF_WKPF("UPDATE_B\n");
  test_update_dummy = 2;
}


uint8_t wuclass_a_properties[] = {
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READONLY,
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READWRITE,
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE
};
wkpf_wuclass_definition wuclass_a = {
  0xFF42, // wuclass id
  update_a, // update function pointer
  3, // Number of properties
  wuclass_a_properties
};

uint8_t wuclass_b_properties[] = {
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE
};
wkpf_wuclass_definition wuclass_b = {
  0x43FF, // wuclass id
  update_b, // update function pointer
  1, // Number of properties
  wuclass_b_properties
};

wkpf_wuclass_definition wuclass_virtual = {
  0x1234, // wuclass id
  NULL, // update function pointer
  1, // Number of properties
  wuclass_b_properties
};

void test_wuclasses() {
  int8_t retval;
  wkpf_wuclass_definition *wuclass;
  
  assert_equal_uint(wkpf_get_number_of_wuclasses(), 0, "number of wuclasses 0");

  retval = wkpf_register_wuclass(wuclass_a);
  assert_equal_uint(retval, WKPF_OK, "register wuclass a");
  assert_equal_uint(wkpf_get_number_of_wuclasses(), 1, "number of wuclasses 1");

  retval = wkpf_register_wuclass(wuclass_b);
  assert_equal_uint(retval, WKPF_OK, "register wuclass b");
  assert_equal_uint(wkpf_get_number_of_wuclasses(), 2, "number of wuclasses 2");

  retval = wkpf_register_wuclass(wuclass_a);
  assert_equal_uint(retval, WKPF_ERR_WUCLASS_ID_IN_USE, "register wuclass a again should fail");
  assert_equal_uint(wkpf_get_number_of_wuclasses(), 2, "number of wuclasses still 2");

  retval = wkpf_get_wuclass_by_id(0xFF42, &wuclass);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuclass by id 0xFF42");
  assert_equal_uint(wuclass->wuclass_id, 0xFF42, "retrieved wuclass: id matches");
  assert_equal_uint(wuclass->number_of_properties, 3, "retrieved wuclass: 3 properties");
  test_update_dummy = 0;
  wuclass->update(NULL);
  assert_equal_uint(test_update_dummy, 1, "retrieved wuclass: update function");

  retval = wkpf_get_wuclass_by_id(0x43FF, &wuclass);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuclass by id 0x43FF");
  assert_equal_uint(wuclass->wuclass_id, 0x43FF, "retrieved wuclass: id matches");
  assert_equal_uint(wuclass->number_of_properties, 1, "retrieved wuclass: 1 property");
  test_update_dummy = 0;
  wuclass->update(NULL);
  assert_equal_uint(test_update_dummy, 2, "retrieved wuclass: update function");

  retval = wkpf_get_wuclass_by_index(0, &wuclass);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuclass by index 0");
  assert_equal_uint(wuclass->wuclass_id, 0xFF42, "retrieved wuclass: id matches");
  assert_equal_uint(wuclass->number_of_properties, 3, "retrieved wuclass: 3 properties");
  assert_equal_uint(wuclass->properties[0], WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READONLY, "retrieved wuclass: property 0");  
  assert_equal_uint(wuclass->properties[1], WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READWRITE, "retrieved wuclass: property 1");  
  assert_equal_uint(wuclass->properties[2], WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE, "retrieved wuclass: property 2");  

  retval = wkpf_get_wuclass_by_index(1, &wuclass);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuclass by index 1");
  assert_equal_uint(wuclass->wuclass_id, 0x43FF, "retrieved wuclass: id matches");
  assert_equal_uint(wuclass->number_of_properties, 1, "retrieved wuclass: 1 property");

  retval = wkpf_get_wuclass_by_index(2, &wuclass);
  assert_equal_uint(retval, WKPF_ERR_WUCLASS_NOT_FOUND, "retrieving wuclass by index 2 should fail");

  retval = wkpf_get_wuclass_by_id(0x1234, &wuclass);
  assert_equal_uint(retval, WKPF_ERR_WUCLASS_NOT_FOUND, "retrieving wuclass by id 0x1234 should fail");

  retval = 0;
  for (int i=2; i<MAX_NUMBER_OF_WUCLASSES; i++) {
    wkpf_wuclass_definition wuclass_x = { i, update_b, 1, wuclass_b_properties };
    retval |= wkpf_register_wuclass(wuclass_x);
  }
  assert_equal_uint(retval, WKPF_OK, "registered MAX_NUMBER_OF_WUCLASSES wuclasses");
  wkpf_wuclass_definition wuclass_y = { MAX_NUMBER_OF_WUCLASSES+1, update_b, 1, wuclass_b_properties };
  retval = wkpf_register_wuclass(wuclass_y);
  assert_equal_uint(retval, WKPF_ERR_OUT_OF_MEMORY, "registering wuclass 6 should fail (out of memory)");

  print_test_summary();
}

void test_wuobjects() {
  int8_t retval;
  wkpf_local_wuobject *wuobject;
  
  retval = wkpf_register_wuclass(wuclass_a);
  retval = wkpf_register_wuclass(wuclass_b);
  assert_equal_uint(wkpf_get_number_of_wuclasses(), 2, "registered 2 wuclasses");
  
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 0, "number of wuobjects 0");

  retval = wkpf_create_wuobject(wuclass_a.wuclass_id, 0x40, 0);
  assert_equal_uint(retval, WKPF_OK, "create wuobject for wuclass A at port 40");
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 1, "number of wuobjects 1");

  test_update_dummy = 0;
  retval = wkpf_create_wuobject(wuclass_b.wuclass_id, 0x80, 0);
  assert_equal_uint(retval, WKPF_OK, "create wuobject for wuclass B at port 80");
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 2, "number of wuobjects 2");
  assert_equal_uint(test_update_dummy, 2, "update function was called when creating wuobject");

  retval = wkpf_create_wuobject(wuclass_a.wuclass_id, 0x80, 0);
  assert_equal_uint(retval, WKPF_ERR_PORT_IN_USE, "create another wuobject at port 80 should fail");
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 2, "number of still wuobjects 2");

  retval = wkpf_create_wuobject(wuclass_a.wuclass_id, 0x81, 0);
  assert_equal_uint(retval, WKPF_OK, "but creating another instance of wuclass A at port 81 is allowed");
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 3, "number of wuobjects 3");


  retval = wkpf_get_wuobject_by_index(0, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuobject by index 0");
  assert_equal_uint(wuobject->wuclass->wuclass_id, 0xFF42, "running wuclass 0xFF42");
  assert_equal_uint(wuobject->port_number, 0x40, "running on port 40");

  retval = wkpf_get_wuobject_by_index(2, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuobject by index 2");
  assert_equal_uint(wuobject->wuclass->wuclass_id, 0xFF42, "running wuclass 0xFF42");
  assert_equal_uint(wuobject->port_number, 0x81, "running on port 81");

  retval = wkpf_get_wuobject_by_port(0x81, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuobject by port 0x81");
  assert_equal_uint(wuobject->wuclass->wuclass_id, 0xFF42, "running wuclass 0xFF42");
  assert_equal_uint(wuobject->port_number, 0x81, "running on port 81");

  retval = wkpf_get_wuobject_by_port(0x82, &wuobject);
  assert_equal_uint(retval, WKPF_ERR_WUOBJECT_NOT_FOUND, "retrieving wuobject by port 0x82 should fail");

  assert_equal_uint(wkpf_get_number_of_wuobjects(), 3, "number of wuobjects 3");
  retval = wkpf_remove_wuobject(0x80);
  assert_equal_uint(retval, WKPF_OK, "removing wuobject at port 0x80");
  assert_equal_uint(wkpf_get_number_of_wuobjects(), 2, "number of wuobjects 2");
  retval = wkpf_get_wuobject_by_port(0x80, &wuobject);
  assert_equal_uint(retval, WKPF_ERR_WUOBJECT_NOT_FOUND, "retrieving wuobject by port 0x80 should now fail");
  retval = wkpf_get_wuobject_by_index(2, &wuobject);
  assert_equal_uint(retval, WKPF_ERR_WUOBJECT_NOT_FOUND, "retrieving wuobject at index 2 should now fail");
  retval = wkpf_get_wuobject_by_index(1, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "retrieving wuobject by index 1 (was at index 2 before)");
  assert_equal_uint(wuobject->wuclass->wuclass_id, 0xFF42, "running wuclass 0xFF42");
  assert_equal_uint(wuobject->port_number, 0x81, "running on port 81");

  retval = wkpf_register_wuclass(wuclass_virtual);
  assert_equal_uint(retval, WKPF_OK, "registered fake virtual wuclass with id 0x1234");
  retval = wkpf_create_wuobject(wuclass_virtual.wuclass_id, 0x82, 0);
  assert_equal_uint(retval, WKPF_ERR_NEED_VIRTUAL_WUCLASS_INSTANCE, "Can't create wuobject for virtual wuclass without an instance");
  retval = wkpf_create_wuobject(wuclass_virtual.wuclass_id, 0x82, 1);
  assert_equal_uint(retval, WKPF_OK, "Created wuobject for virtual wuclass with a wuclass instance");

  print_test_summary();
}

void test_properties() {
  int8_t retval;
  int16_t value_int16=42;
  bool value_boolean=42;
  wkpf_local_wuobject *wuobject_a;
  wkpf_local_wuobject *wuobject_b;
  
  retval = wkpf_register_wuclass(wuclass_a);
  retval = wkpf_register_wuclass(wuclass_b);
  retval = wkpf_create_wuobject(wuclass_a.wuclass_id, 0x40, 0);
  retval = wkpf_create_wuobject(wuclass_b.wuclass_id, 0x80, 0);
  retval = wkpf_get_wuobject_by_port(0x40, &wuobject_a);
  retval = wkpf_get_wuobject_by_port(0x80, &wuobject_b);
  
  retval = wkpf_external_read_property_int16(wuobject_a, 2, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 2 of wuobject at port 0x40");
  assert_equal_uint(value_int16, 0, "initially 0");
  retval = wkpf_external_write_property_int16(wuobject_a, 2, 0x1234);
  assert_equal_uint(retval, WKPF_OK, "writing 0x1234 to property 2 of wuobject at port 0x40");
  retval = wkpf_external_read_property_int16(wuobject_a, 2, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 2 of wuobject at port 0x40");
  assert_equal_uint(value_int16, 0x1234, "value is now 0x1234");

  retval = wkpf_external_write_property_int16(wuobject_a, 4, 0x1234);
  assert_equal_uint(retval, WKPF_ERR_PROPERTY_NOT_FOUND, "writing to property 4 of wuobject at port 0x40 fails (only 3 properties)");
  retval = wkpf_external_write_property_int16(wuobject_a, 0, 0x1234);
  assert_equal_uint(retval, WKPF_ERR_READ_ONLY, "writing to property 0 of wuobject at port 0x40 is not allowed (read only)");
  retval = wkpf_external_read_property_int16(wuobject_a, 0,  &value_int16);
  assert_equal_uint(retval, WKPF_OK, "but reading property 0 of wuobject at port 0x40 is allowed");
  assert_equal_uint(value_int16, 0, "initially 0");

  retval = wkpf_internal_write_property_int16(wuobject_a, 0, 0x1234);
  assert_equal_uint(retval, WKPF_OK, "writing to property 0 of wuobject at port 0x40 is allowed internally for wuclass implementation (read only)");
  retval = wkpf_external_read_property_int16(wuobject_a, 0,  &value_int16);
  assert_equal_uint(value_int16, 0x1234, "value changed to 0x1234");

  retval = wkpf_external_read_property_int16(wuobject_a, 1,  &value_int16);
  assert_equal_uint(retval, WKPF_ERR_WRONG_DATATYPE, "property 1 is a boolean, reading as int16 should fail");
  retval = wkpf_external_write_property_int16(wuobject_a, 1,  1);
  assert_equal_uint(retval, WKPF_ERR_WRONG_DATATYPE, "property 1 is a boolean, writing as int16 should fail");
  
  retval = wkpf_external_read_property_boolean(wuobject_a, 1,  &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "property 1 is a boolean, reading as boolean");
  assert_equal_uint(value_boolean, 0, "initially 0");
  retval = wkpf_external_write_property_boolean(wuobject_a, 1,  TRUE);
  assert_equal_uint(retval, WKPF_OK, "property 1 is a boolean, writing as boolean");
  retval = wkpf_external_read_property_boolean(wuobject_a, 1,  &value_boolean);
  assert_equal_uint(value_boolean, TRUE, "value is now 1");

  uint8_t port_number;
  uint8_t property_number;
  while(wkpf_get_next_dirty_property(&port_number, &property_number)) { } // Clear all dirty bits
  retval = wkpf_external_write_property_boolean(wuobject_a, 1, TRUE);
  retval = wkpf_external_write_property_boolean(wuobject_a, 1, FALSE);
  assert_equal_uint(retval, WKPF_OK, "writing FALSE to property 1 of wuobject at port 0x40 using external write");
  retval = wkpf_internal_write_property_int16(wuobject_a, 2, 0x2222);
  assert_equal_uint(retval, WKPF_OK, "writing 0x2222 to property 2 of wuobject at port 0x40 using internal write");
  wkpf_get_next_dirty_property(&port_number, &property_number);
  assert_equal_uint(port_number, 0x40, "port number 0x40 returned by wkpf_get_next_dirty_property");
  assert_equal_uint(property_number, 0x01, "property number 1 returned by wkpf_get_next_dirty_property");
  wkpf_get_next_dirty_property(&port_number, &property_number);
  assert_equal_uint(port_number, 0x40, "port number 0x40 returned by wkpf_get_next_dirty_property");
  assert_equal_uint(property_number, 0x02, "property number 2 returned by wkpf_get_next_dirty_property");

  retval = wkpf_external_write_property_int16(wuobject_b, 0, 0xF1F1);
  assert_equal_uint(retval, WKPF_OK, "writing 0xF1F1 to property 0 of wuobject at port 0x80");
  retval = wkpf_external_read_property_int16(wuobject_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of wuobject at port 0x80");
  assert_equal_uint(value_int16, 0xF1F1, "value is now 0xF1F1");

  retval = wkpf_remove_wuobject(0x40);
  assert_equal_uint(retval, WKPF_OK, "Remove wuobject at port 0x40");
  retval = wkpf_external_read_property_int16(wuobject_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of wuobject at port 0x80");
  assert_equal_uint(value_int16, 0xF1F1, "value is still 0xF1F1");

  retval = wkpf_external_write_property_int16(wuobject_b, 0, 0x1111);
  assert_equal_uint(retval, WKPF_OK, "writing 0x1111 to property 0 of wuobject at port 0x80");
  retval = wkpf_external_read_property_int16(wuobject_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of wuobject at port 0x80");
  assert_equal_uint(value_int16, 0x1111, "value is now 0x1111");

  print_test_summary();
}

void test_native_wuclasses() {
  int8_t retval;
  int16_t value_int16=0;
  wkpf_local_wuobject *wuobject;

  wkpf_init();

  retval = wkpf_get_wuobject_by_port(0x0, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "get generic wuclass wuobject (port 0x0)");
  assert_equal_uint(wuobject->wuclass->wuclass_id, WKPF_WUCLASS_GENERIC, "wuclass id is that of the generic wuclass");
  retval = wkpf_external_read_property_int16(wuobject, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0");
  assert_equal_uint(value_int16, 42, "value is 42");

  print_test_summary();
}

void test_update_for_native_wuclasses() {
  int8_t retval;
  bool value_boolean;
  wkpf_local_wuobject *wuobject;
  
  retval = wkpf_register_wuclass(wuclass_threshold);
  retval &= wkpf_create_wuobject(wuclass_threshold.wuclass_id, 1, 0);
  retval &= wkpf_get_wuobject_by_port(1, &wuobject);
  assert_equal_uint(retval, WKPF_OK, "register threshold wuclass and create wuobject");

  retval = wkpf_external_write_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_OPERATOR, OPERATOR_GT);
  retval &= wkpf_external_write_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_THRESHOLD, 1000);
  retval &= wkpf_external_write_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, 800);
  assert_equal_uint(retval, WKPF_OK, "setup initial properties: operator=>, threshold=1000, value=800");

  retval = wkpf_external_read_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, FALSE, "output is FALSE");

  retval = wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, 1200);
  assert_equal_uint(retval, WKPF_OK, "updating value property using internal function, value=1200");

  retval = wkpf_external_read_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, FALSE, "output still FALSE, internal writes don't trigger update()");

  retval = wkpf_external_write_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, 1100);
  assert_equal_uint(retval, WKPF_OK, "updating value property using external function, value=1100");

  retval = wkpf_external_read_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, TRUE, "output changed to TRUE");

  print_test_summary();
}

void test_wkpf() {
#ifdef TEST_WKPF_WUCLASSES
  test_wuclasses();
#endif
#ifdef TEST_WKPF_WUOBJECTS
  test_wuobjects();
#endif
#ifdef TEST_WKPF_PROPERTIES
  test_properties();
#endif
#ifdef TEST_WKPF_NATIVE_WUCLASSES
  test_native_wuclasses();
#endif
#ifdef TEST_WKPF_UPDATE_FOR_NATIVE_WUCLASSES
  test_update_for_native_wuclasses();
#endif

  while(1) { }
}
#endif // TEST_WKPF
