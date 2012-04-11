#include "wkpf.h"

uint8_t is_first=0;
uint8_t profile_pointer=0;
uint8_t profile_number=3;
uint8_t profile_array_size=0;
uint8_t profile_array[20];

struct profile_instance 
{
	uint16_t profile_id;
	uint8_t role_id;
	uint8_t property_num;
	uint8_t property_rw;
	struct def_property 
	{
		uint8_t property_id;
		int32_t value;
	} property[2];

} profile[3];

void profile_init()
{
	is_first=1;
	for(size8_t i=0;i<profile_number;i++)
	{
		profile[i].profile_id=(uint16_t)i;
		profile[i].role_id=i+10;
		profile[i].property_num=2;
		for(size8_t j=0;j<2;j++)
		{
			profile[i].property[j].property_id=1;
			profile[i].property[j].value=i+j;
		}
	}

}

uint8_t wkpf_get_profile_list() {
	if(!is_first)	profile_init();

	int8_t value;	
	if(profile_pointer==0)
	{
		profile_array[0]=profile_number;
		for(size8_t i=0;i<profile_number*3;i=i+3)//generate profile array
		{
			profile_array[i+1]=(uint8_t)(profile[i/3].profile_id>>8);
			profile_array[i+2]=(uint8_t)(profile[i/3].profile_id);
			profile_array[i+3]=profile[i/3].role_id;
		}
		value=profile_array[0];
		profile_array_size=value*3;
		profile_pointer++;
	}
	else
	{
		value=(uint8_t)(profile_array[profile_pointer]);
		if(profile_pointer>=profile_array_size)
			profile_pointer=0;
		else
			profile_pointer++;
	}
	return value;
}

/*

int32_t wkpf_read_property(uint16_t profile_id, uint8_t role_id, uint8_t property_id) {
	if(!is_first)	profile_init();
	if(profile_id>=0 && profile_id<3 && property_id>=0 && property_id<2)
	{
		int32_t rd_val=profile[profile_id].property[property_id].value;
		return rd_val;
	}
	return -1;
}

void wkpf_write_property(uint16_t profile_id, uint8_t role_id, uint8_t property_id, int32_t value) {
	if(!is_first)	profile_init();
	if(profile_id>=0 && profile_id<3 && property_id>=0 && property_id<2)
	{
		profile[profile_id].property[property_id].value=value;
	}
	return ;
}

void *getProperty(profile_id, role_id, property_id) {
	dummy = profile_id+property_id;
	return &dummy;
}

setproperty() {
	
}*/
