#include <debug.h>
#include <wkpf.h>
#include <delay.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_THRESHOLD
int16_t trigger_count = 0;
bool prev_input=false;
nvmtime_t elapsed_time = 0;
nvmtime_t start_time = 0;
bool output = false;
void wuclass_trigger_update(wkpf_local_wuobject *wuobject) {
  bool input;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_TRIGGER_INPUT, &input);
  if (input == true && prev_input == false){
    start_time = nvm_current_time;
    trigger_count = trigger_count +1;
  }else if (input== false && prev_input == true) {
    elapsed_time = nvm_current_time- start_time;
    if (elapsed_time > 3000){
      output = false;
    }else{
      output = !output;
    }
    
    trigger_count = 0;
  }else if (input == true){
    trigger_count = trigger_count +1;
  }
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Native Trigger): input %x prev_input %x\n", input, prev_input);
  prev_input = input;
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_TRIGGER_OUTPUT, output);
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Native Trigger): output %x count %x time %x\n",output, trigger_count, nvm_current_time-start_time);
}

#endif // ENABLE_WUCLASS_THRESHOLD
