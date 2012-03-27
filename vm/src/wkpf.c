#include "wkpf.h"

int32_t dummy;

getprofilelist() {
	
}

void *getProperty(profile_id, role_id, property_id) {
	dummy = profile_id+property_id;
	return &dummy;
}

setproperty() {
	
}