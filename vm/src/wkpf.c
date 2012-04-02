#include "wkpf.h"
int32_t dummy=0xcc;

int32_t wkpf_get_profile_list() {
	return dummy*2;
}

int32_t wkpf_read_property(uint16_t profile_id, u08_t role_id, u08_t property_id) {
	return dummy;
}

void wkpf_write_property(uint16_t profile_id, u08_t role_id, u08_t property_id, int32_t value) {
 	dummy = value;
	return ;
}
/*

void *getProperty(profile_id, role_id, property_id) {
	dummy = profile_id+property_id;
	return &dummy;
}

setproperty() {
	
}*/
