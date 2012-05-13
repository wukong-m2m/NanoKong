#ifndef WKPF_WUOBJECTSH
#define WKPF_WUOBJECTSH

#include "types.h"
#include "heap.h"
#include "wkpf_wuclasses.h"

// TODONR: only works if heap id 0 isn't used.
#define WKPF_IS_NATIVE_WUOBJECT(x)               (x->virtual_wuclass_instance_heap_id == 0)
#define WKPF_IS_VIRTUAL_WUOBJECT(x)              (x->virtual_wuclass_instance_heap_id != 0)

typedef struct wkpf_local_wuobject_struct {
    wkpf_wuclass_definition *wuclass;
    uint8_t port_number;
    heap_id_t virtual_wuclass_instance_heap_id; // Set for virtual wuclasses, 0 for native wuclasses
    bool need_to_call_update;
} wkpf_local_wuobject;

extern uint8_t wkpf_create_wuobject(uint16_t wuclass_id, uint8_t port_number, heap_id_t virtual_wuclass_instance_heap_id);
extern uint8_t wkpf_remove_wuobject(uint8_t port_number);
extern uint8_t wkpf_get_wuobject_by_port(uint8_t port_number, wkpf_local_wuobject **wuobject);
extern uint8_t wkpf_get_wuobject_by_index(uint8_t index, wkpf_local_wuobject **wuobject);
extern uint8_t wkpf_get_wuobject_by_heap_id(heap_id_t virtual_wuclass_instance_heap_id, wkpf_local_wuobject **wuobject);
extern uint8_t wkpf_get_number_of_wuobjects();
extern void wkpf_set_need_to_call_update_for_wuobject(wkpf_local_wuobject *wuobject);
extern bool wkpf_get_next_wuobject_to_update(wkpf_local_wuobject **wuobject);
extern bool wkpf_heap_id_in_use(heap_id_t heap_id); // To prevent virtual wuclass objects from being garbage collected

#endif // WKPF_WUOBJECTSH
