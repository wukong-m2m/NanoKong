#include "config.h"
#include "debug.h"
#include "types.h"
#include "wkpf.h"
#include "native_profiles/profile_threshold.h"

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
void update_a(wkpf_local_endpoint *endpoint) {
  DEBUGF_WKPF("UPDATE_A\n");
  test_update_dummy = 1;
}
void update_b(wkpf_local_endpoint *endpoint) {
  DEBUGF_WKPF("UPDATE_B\n");
  test_update_dummy = 2;
}


uint8_t profile_a_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ,
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW,
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW
};
wkpf_profile_definition profile_a = {
  0xFF42, // profile id
  update_a, // update function pointer
  3, // Number of properties
  profile_a_properties
};

uint8_t profile_b_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW
};
wkpf_profile_definition profile_b = {
  0x43FF, // profile id
  update_b, // update function pointer
  1, // Number of properties
  profile_b_properties
};

wkpf_profile_definition profile_virtual = {
  0x1234, // profile id
  NULL, // update function pointer
  1, // Number of properties
  profile_b_properties
};

void test_profiles() {
  int8_t retval;
  wkpf_profile_definition *profile;
  
  assert_equal_uint(wkpf_get_number_of_profiles(), 0, "number of profiles 0");

  retval = wkpf_register_profile(profile_a);
  assert_equal_uint(retval, WKPF_OK, "register profile a");
  assert_equal_uint(wkpf_get_number_of_profiles(), 1, "number of profiles 1");

  retval = wkpf_register_profile(profile_b);
  assert_equal_uint(retval, WKPF_OK, "register profile b");
  assert_equal_uint(wkpf_get_number_of_profiles(), 2, "number of profiles 2");

  retval = wkpf_register_profile(profile_a);
  assert_equal_uint(retval, WKPF_ERR_PROFILE_ID_IN_USE, "register profile a again should fail");
  assert_equal_uint(wkpf_get_number_of_profiles(), 2, "number of profiles still 2");

  retval = wkpf_get_profile_by_id(0xFF42, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by id 0xFF42");
  assert_equal_uint(profile->profile_id, 0xFF42, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 3, "retrieved profile: 3 properties");
  test_update_dummy = 0;
  profile->update(NULL);
  assert_equal_uint(test_update_dummy, 1, "retrieved profile: update function");

  retval = wkpf_get_profile_by_id(0x43FF, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by id 0x43FF");
  assert_equal_uint(profile->profile_id, 0x43FF, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 1, "retrieved profile: 1 property");
  test_update_dummy = 0;
  profile->update(NULL);
  assert_equal_uint(test_update_dummy, 2, "retrieved profile: update function");

  retval = wkpf_get_profile_by_index(0, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by index 0");
  assert_equal_uint(profile->profile_id, 0xFF42, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 3, "retrieved profile: 3 properties");
  assert_equal_uint(profile->properties[0], WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ, "retrieved profile: property 0");  
  assert_equal_uint(profile->properties[1], WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW, "retrieved profile: property 1");  
  assert_equal_uint(profile->properties[2], WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, "retrieved profile: property 2");  

  retval = wkpf_get_profile_by_index(1, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by index 1");
  assert_equal_uint(profile->profile_id, 0x43FF, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 1, "retrieved profile: 1 property");

  retval = wkpf_get_profile_by_index(2, &profile);
  assert_equal_uint(retval, WKPF_ERR_PROFILE_NOT_FOUND, "retrieving profile by index 2 should fail");

  retval = wkpf_get_profile_by_id(0x1234, &profile);
  assert_equal_uint(retval, WKPF_ERR_PROFILE_NOT_FOUND, "retrieving profile by id 0x1234 should fail");

  wkpf_profile_definition profile_3 = { 0x3, update_b, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_3);
  wkpf_profile_definition profile_4 = { 0x4, update_b, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_4);
  wkpf_profile_definition profile_5 = { 0x5, update_b, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_5);
  assert_equal_uint(retval, WKPF_OK, "registered 5 profiles");
  wkpf_profile_definition profile_6 = { 0x6, update_b, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_6);
  assert_equal_uint(retval, WKPF_ERR_OUT_OF_MEMORY, "registering profile 6 should fail (out of memory)");

  print_test_summary();
}

void test_endpoints() {
  int8_t retval;
  wkpf_local_endpoint *endpoint;
  
  retval = wkpf_register_profile(profile_a);
  retval = wkpf_register_profile(profile_b);
  assert_equal_uint(wkpf_get_number_of_profiles(), 2, "registered 2 profiles");
  
  assert_equal_uint(wkpf_get_number_of_endpoints(), 0, "number of endpoints 0");

  retval = wkpf_create_endpoint(profile_a.profile_id, 0x40, 0);
  assert_equal_uint(retval, WKPF_OK, "create endpoint for profile A at port 40");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 1, "number of endpoints 1");

  test_update_dummy = 0;
  retval = wkpf_create_endpoint(profile_b.profile_id, 0x80, 0);
  assert_equal_uint(retval, WKPF_OK, "create endpoint for profile B at port 80");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 2, "number of endpoints 2");
  assert_equal_uint(test_update_dummy, 2, "update function was called when creating endpoint");

  retval = wkpf_create_endpoint(profile_a.profile_id, 0x80, 0);
  assert_equal_uint(retval, WKPF_ERR_PORT_IN_USE, "create another endpoint at port 80 should fail");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 2, "number of still endpoints 2");

  retval = wkpf_create_endpoint(profile_a.profile_id, 0x81, 0);
  assert_equal_uint(retval, WKPF_OK, "but creating another instance of profile A at port 81 is allowed");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 3, "number of endpoints 3");


  retval = wkpf_get_endpoint_by_index(0, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "retrieving endpoint by index 0");
  assert_equal_uint(endpoint->profile->profile_id, 0xFF42, "running profile 0xFF42");
  assert_equal_uint(endpoint->port_number, 0x40, "running on port 40");

  retval = wkpf_get_endpoint_by_index(2, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "retrieving endpoint by index 2");
  assert_equal_uint(endpoint->profile->profile_id, 0xFF42, "running profile 0xFF42");
  assert_equal_uint(endpoint->port_number, 0x81, "running on port 81");

  retval = wkpf_get_endpoint_by_port(0x81, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "retrieving endpoint by port 0x81");
  assert_equal_uint(endpoint->profile->profile_id, 0xFF42, "running profile 0xFF42");
  assert_equal_uint(endpoint->port_number, 0x81, "running on port 81");

  retval = wkpf_get_endpoint_by_port(0x82, &endpoint);
  assert_equal_uint(retval, WKPF_ERR_ENDPOINT_NOT_FOUND, "retrieving endpoint by port 0x82 should fail");

  assert_equal_uint(wkpf_get_number_of_endpoints(), 3, "number of endpoints 3");
  retval = wkpf_remove_endpoint(0x80);
  assert_equal_uint(retval, WKPF_OK, "removing endpoint at port 0x80");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 2, "number of endpoints 2");
  retval = wkpf_get_endpoint_by_port(0x80, &endpoint);
  assert_equal_uint(retval, WKPF_ERR_ENDPOINT_NOT_FOUND, "retrieving endpoint by port 0x80 should now fail");
  retval = wkpf_get_endpoint_by_index(2, &endpoint);
  assert_equal_uint(retval, WKPF_ERR_ENDPOINT_NOT_FOUND, "retrieving endpoint at index 2 should now fail");
  retval = wkpf_get_endpoint_by_index(1, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "retrieving endpoint by index 1 (was at index 2 before)");
  assert_equal_uint(endpoint->profile->profile_id, 0xFF42, "running profile 0xFF42");
  assert_equal_uint(endpoint->port_number, 0x81, "running on port 81");

  retval = wkpf_register_profile(profile_virtual);
  assert_equal_uint(retval, WKPF_OK, "registered fake virtual profile with id 0x1234");
  retval = wkpf_create_endpoint(profile_virtual.profile_id, 0x82, 0);
  assert_equal_uint(retval, WKPF_ERR_NEED_VIRTUAL_PROFILE_INSTANCE, "Can't create endpoint for virtual profile without an instance");
  retval = wkpf_create_endpoint(profile_virtual.profile_id, 0x82, 1);
  assert_equal_uint(retval, WKPF_OK, "Created endpoint for virtual profile with a profile instance");

  print_test_summary();
}

void test_properties() {
  int8_t retval;
  int16_t value_int16=42;
  bool value_boolean=42;
  wkpf_local_endpoint *endpoint_a;
  wkpf_local_endpoint *endpoint_b;
  
  retval = wkpf_register_profile(profile_a);
  retval = wkpf_register_profile(profile_b);
  retval = wkpf_create_endpoint(profile_a.profile_id, 0x40, 0);
  retval = wkpf_create_endpoint(profile_b.profile_id, 0x80, 0);
  retval = wkpf_get_endpoint_by_port(0x40, &endpoint_a);
  retval = wkpf_get_endpoint_by_port(0x80, &endpoint_b);
  
  retval = wkpf_external_read_property_int16(endpoint_a, 2, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 2 of endpoint at port 0x40");
  assert_equal_uint(value_int16, 0, "initially 0");
  retval = wkpf_external_write_property_int16(endpoint_a, 2, 0x1234);
  assert_equal_uint(retval, WKPF_OK, "writing 0x1234 to property 2 of endpoint at port 0x40");
  retval = wkpf_external_read_property_int16(endpoint_a, 2, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 2 of endpoint at port 0x40");
  assert_equal_uint(value_int16, 0x1234, "value is now 0x1234");

  retval = wkpf_external_write_property_int16(endpoint_a, 4, 0x1234);
  assert_equal_uint(retval, WKPF_ERR_PROPERTY_NOT_FOUND, "writing to property 4 of endpoint at port 0x40 fails (only 3 properties)");
  retval = wkpf_external_write_property_int16(endpoint_a, 0, 0x1234);
  assert_equal_uint(retval, WKPF_ERR_READ_ONLY, "writing to property 0 of endpoint at port 0x40 is not allowed (read only)");
  retval = wkpf_external_read_property_int16(endpoint_a, 0,  &value_int16);
  assert_equal_uint(retval, WKPF_OK, "but reading property 0 of endpoint at port 0x40 is allowed");
  assert_equal_uint(value_int16, 0, "initially 0");

  retval = wkpf_internal_write_property_int16(endpoint_a, 0, 0x1234);
  assert_equal_uint(retval, WKPF_OK, "writing to property 0 of endpoint at port 0x40 is allowed internally for profile implementation (read only)");
  retval = wkpf_external_read_property_int16(endpoint_a, 0,  &value_int16);
  assert_equal_uint(value_int16, 0x1234, "value changed to 0x1234");

  retval = wkpf_external_read_property_int16(endpoint_a, 1,  &value_int16);
  assert_equal_uint(retval, WKPF_ERR_WRONG_DATATYPE, "property 1 is a boolean, reading as int16 should fail");
  retval = wkpf_external_write_property_int16(endpoint_a, 1,  1);
  assert_equal_uint(retval, WKPF_ERR_WRONG_DATATYPE, "property 1 is a boolean, writing as int16 should fail");
  
  retval = wkpf_external_read_property_boolean(endpoint_a, 1,  &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "property 1 is a boolean, reading as boolean");
  assert_equal_uint(value_boolean, 0, "initially 0");
  retval = wkpf_external_write_property_boolean(endpoint_a, 1,  TRUE);
  assert_equal_uint(retval, WKPF_OK, "property 1 is a boolean, writing as boolean");
  retval = wkpf_external_read_property_boolean(endpoint_a, 1,  &value_boolean);
  assert_equal_uint(value_boolean, TRUE, "value is now 1");

  while(wkpf_get_next_dirty_property()) { } // Clear all dirty bits
  retval = wkpf_external_write_property_boolean(endpoint_a, 1, TRUE);
  retval = wkpf_external_write_property_boolean(endpoint_a, 1, FALSE);
  assert_equal_uint(retval, WKPF_OK, "writing FALSE to property 1 of endpoint at port 0x40 using external write");
  retval = wkpf_internal_write_property_int16(endpoint_a, 2, 0x2222);
  assert_equal_uint(retval, WKPF_OK, "writing 0x2222 to property 2 of endpoint at port 0x40 using internal write");
  value_int16 = wkpf_get_next_dirty_property();
  assert_equal_uint(value_int16, 0x4001, "property 1 returned by wkpf_get_next_dirty_property");
  value_int16 = wkpf_get_next_dirty_property();
  assert_equal_uint(value_int16, 0x4002, "property 2 returned by wkpf_get_next_dirty_property");

  retval = wkpf_external_write_property_int16(endpoint_b, 0, 0xF1F1);
  assert_equal_uint(retval, WKPF_OK, "writing 0xF1F1 to property 0 of endpoint at port 0x80");
  retval = wkpf_external_read_property_int16(endpoint_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of endpoint at port 0x80");
  assert_equal_uint(value_int16, 0xF1F1, "value is now 0xF1F1");

  retval = wkpf_remove_endpoint(0x40);
  assert_equal_uint(retval, WKPF_OK, "Remove endpoint at port 0x40");
  retval = wkpf_external_read_property_int16(endpoint_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of endpoint at port 0x80");
  assert_equal_uint(value_int16, 0xF1F1, "value is still 0xF1F1");

  retval = wkpf_external_write_property_int16(endpoint_b, 0, 0x1111);
  assert_equal_uint(retval, WKPF_OK, "writing 0x1111 to property 0 of endpoint at port 0x80");
  retval = wkpf_external_read_property_int16(endpoint_b, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0 of endpoint at port 0x80");
  assert_equal_uint(value_int16, 0x1111, "value is now 0x1111");

  print_test_summary();
}

void test_native_profiles() {
  int8_t retval;
  int16_t value_int16=0;
  wkpf_local_endpoint *endpoint;

  wkpf_init();

  retval = wkpf_get_endpoint_by_port(0x0, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "get generic profile endpoint (port 0x0)");
  assert_equal_uint(endpoint->profile->profile_id, WKPF_PROFILE_ID_GENERIC, "profile id is that of the generic profile");
  retval = wkpf_external_read_property_int16(endpoint, 0, &value_int16);
  assert_equal_uint(retval, WKPF_OK, "reading property 0");
  assert_equal_uint(value_int16, 42, "value is 42");

  print_test_summary();
}

void test_update_for_native_profiles() {
  int8_t retval;
  bool value_boolean;
  wkpf_local_endpoint *endpoint;
  
  retval = wkpf_register_profile(profile_threshold);
  retval &= wkpf_create_endpoint(profile_threshold.profile_id, 1, 0);
  retval &= wkpf_get_endpoint_by_port(1, &endpoint);
  assert_equal_uint(retval, WKPF_OK, "register threshold profile and create endpoint");

  retval = wkpf_external_write_property_int16(endpoint, WKPF_PROPERTY_ID_THRESHOLD_OPERATOR, OPERATOR_GT);
  retval &= wkpf_external_write_property_int16(endpoint, WKPF_PROPERTY_ID_THRESHOLD_THRESHOLD, 1000);
  retval &= wkpf_external_write_property_int16(endpoint, WKPF_PROPERTY_ID_THRESHOLD_VALUE, 800);
  assert_equal_uint(retval, WKPF_OK, "setup initial properties: operator=>, threshold=1000, value=800");

  retval = wkpf_external_read_property_boolean(endpoint, WKPF_PROPERTY_ID_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, FALSE, "output is FALSE");

  retval = wkpf_internal_write_property_int16(endpoint, WKPF_PROPERTY_ID_THRESHOLD_VALUE, 1200);
  assert_equal_uint(retval, WKPF_OK, "updating value property using internal function, value=1200");

  retval = wkpf_external_read_property_boolean(endpoint, WKPF_PROPERTY_ID_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, FALSE, "output still FALSE, internal writes don't trigger update()");

  retval = wkpf_external_write_property_int16(endpoint, WKPF_PROPERTY_ID_THRESHOLD_VALUE, 1100);
  assert_equal_uint(retval, WKPF_OK, "updating value property using external function, value=1100");

  retval = wkpf_external_read_property_boolean(endpoint, WKPF_PROPERTY_ID_THRESHOLD_OUTPUT, &value_boolean);
  assert_equal_uint(retval, WKPF_OK, "reading output");
  assert_equal_uint(value_boolean, TRUE, "output changed to TRUE");

  print_test_summary();
}


void test_wkpf() {
#ifdef TEST_WKPF_PROFILES
  test_profiles();
#endif
#ifdef TEST_WKPF_ENDPOINTS
  test_endpoints();
#endif
#ifdef TEST_WKPF_PROPERTIES
  test_properties();
#endif
#ifdef TEST_WKPF_NATIVE_PROFILES
  test_native_profiles();
#endif
#ifdef TEST_WKPF_UPDATE_FOR_NATIVE_PROFILES
  test_update_for_native_profiles();
#endif

  while(1) { }
}
#endif // TEST_WKPF
