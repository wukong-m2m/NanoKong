#include "config.h"
#include "debug.h"
#include "types.h"
#include "wkpf.h"
#include "wkpf_profiles.h"
#include "wkpf_endpoints.h"

#ifdef TEST_WKPF

uint16_t passed_count=0;
uint16_t failed_count=0;

void assert_equal_uint(uint32_t a, uint32_t b, char* desc) {
  if (a==b) {
    DEBUGF_TEST("OK: ");
    passed_count++;
  } else {
    DEBUGF_TEST("FAIL: ");
    failed_count++;
  }
  DEBUGF_TEST(desc);
  DEBUGF_TEST("\n");
}

void print_test_summary() {
  DEBUGF_TEST("PASSED: %d, FAILED: %d", passed_count, failed_count);
}

int8_t test_update_dummy = 0;
void print_ok_a() {
  test_update_dummy = 1;
}
void print_ok_b() {
  test_update_dummy = 2;
}


uint8_t profile_a_properties[] = {
  WKPF_PROPERTY_TYPE_INT+WKPF_PROPERTY_ACCESS_RO,
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW,
  WKPF_PROPERTY_TYPE_INT+WKPF_PROPERTY_ACCESS_RW
};
wkpf_profile_definition profile_a = {
  0xFF42, // profile id
  print_ok_a, // update function pointer
  NULL, // Java object
  3, // Number of properties
  profile_a_properties
};

uint8_t profile_b_properties[] = {
  WKPF_PROPERTY_TYPE_INT+WKPF_PROPERTY_ACCESS_RW
};
wkpf_profile_definition profile_b = {
  0x43FF, // profile id
  print_ok_b, // update function pointer
  NULL, // Java object
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
  profile->update();
  assert_equal_uint(test_update_dummy, 1, "retrieved profile: update function");

  retval = wkpf_get_profile_by_id(0x43FF, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by id 0x43FF");
  assert_equal_uint(profile->profile_id, 0x43FF, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 1, "retrieved profile: 1 property");
  test_update_dummy = 0;
  profile->update();
  assert_equal_uint(test_update_dummy, 2, "retrieved profile: update function");

  retval = wkpf_get_profile_by_index(0, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by index 0");
  assert_equal_uint(profile->profile_id, 0xFF42, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 3, "retrieved profile: 3 properties");
  assert_equal_uint(profile->properties[0], WKPF_PROPERTY_TYPE_INT+WKPF_PROPERTY_ACCESS_RO, "retrieved profile: property 0");  
  assert_equal_uint(profile->properties[1], WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW, "retrieved profile: property 1");  
  assert_equal_uint(profile->properties[2], WKPF_PROPERTY_TYPE_INT+WKPF_PROPERTY_ACCESS_RW, "retrieved profile: property 2");  

  retval = wkpf_get_profile_by_index(1, &profile);
  assert_equal_uint(retval, WKPF_OK, "retrieving profile by index 1");
  assert_equal_uint(profile->profile_id, 0x43FF, "retrieved profile: id matches");
  assert_equal_uint(profile->number_of_properties, 1, "retrieved profile: 1 property");

  retval = wkpf_get_profile_by_index(2, &profile);
  assert_equal_uint(retval, WKPF_ERR_PROFILE_NOT_FOUND, "retrieving profile by index 2 should fail");

  retval = wkpf_get_profile_by_id(0x1234, &profile);
  assert_equal_uint(retval, WKPF_ERR_PROFILE_NOT_FOUND, "retrieving profile by id 0x1234 should fail");

  wkpf_profile_definition profile_3 = { 0x3, print_ok_b, NULL, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_3);
  wkpf_profile_definition profile_4 = { 0x4, print_ok_b, NULL, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_4);
  wkpf_profile_definition profile_5 = { 0x5, print_ok_b, NULL, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_5);
  assert_equal_uint(retval, WKPF_OK, "registered 5 profiles");
  wkpf_profile_definition profile_6 = { 0x6, print_ok_b, NULL, 1, profile_b_properties };
  retval = wkpf_register_profile(profile_6);
  assert_equal_uint(retval, WKPF_ERR_OUT_OF_MEMORY, "registering profile 6 should fail (out of memory)");

  print_test_summary();
  while(1) {}
}

void test_endpoints() {
  int8_t retval;
  wkpf_local_endpoint *endpoint;
  
  retval = wkpf_register_profile(profile_a);
  retval = wkpf_register_profile(profile_b);
  assert_equal_uint(wkpf_get_number_of_profiles(), 2, "registered 2 profiles");
  
  assert_equal_uint(wkpf_get_number_of_endpoints(), 0, "number of endpoints 0");

  retval = wkpf_create_endpoint(&profile_a, 0x40);
  assert_equal_uint(retval, WKPF_OK, "create endpoint for profile A at port 40");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 1, "number of endpoints 1");

  retval = wkpf_create_endpoint(&profile_b, 0x80);
  assert_equal_uint(retval, WKPF_OK, "create endpoint for profile B at port 80");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 2, "number of endpoints 2");

  retval = wkpf_create_endpoint(&profile_a, 0x80);
  assert_equal_uint(retval, WKPF_ERR_PORT_IN_USE, "create another endpoint at port 80 should fail");
  assert_equal_uint(wkpf_get_number_of_endpoints(), 2, "number of still endpoints 2");

  retval = wkpf_create_endpoint(&profile_a, 0x81);
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

  print_test_summary();
  while(1) {}
}

#endif // TEST_WKPF
