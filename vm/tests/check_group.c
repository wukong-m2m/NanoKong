#include "../src/group.c"

void fail_unless(bool expr, const char* comment) {
    if (!expr) {
        printf(comment);
    }
}

int
main (void)
{
    // First test
    view v = { 0, 
            (address_t[]){ 0, 1, 2 }, 3, 
            (address_t[]){ 3 }, 1, 
            (address_t[]){ 2 }, 1, 
            (address_t[]){ 5 }, 1, 
            0, NVMCOMM_ADDRESS_NULL};
    group g = { "TEST", 0, &v, 1};
    fail_unless(am_i_leader(&g),
            "Leader is not determined correctly, my rank is %d", 
            get_rank_of(current_view(&g, nvmcomm_get_node_id())));

    return 0;
}
