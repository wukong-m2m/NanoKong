#include "config.h"
#include "debug.h"
#include "types.h"
#include "wkpf.h"
#include "group.h"

#ifdef TEST_GROUP

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

void test_prepare_update()
{
    DEBUGF_TEST("test_prepare_update()");
    if (prepare_update((u08_t[]){ 0, 1, 2, 3 }, 4) < 0) {
        DEBUGF_TEST("FAILED\n");
        failed_count++;
    }

    load_prepared_proposal();
    if (memcmp(prepared_proposal, (u08_t[]){ 0, 1, 2, 3 }, 4) == 0) {
        DEBUGF_TEST("PASSED\n")
        passed_count++;
    }
}

void test_group() {
#ifdef TEST_GROUP_PREPARE_UPDATE
  test_prepare_update();
#endif

#ifdef TEST_GROUP
  print_test_summary();
#endif

  while(1) { }
}
#endif // TEST_GROUP

