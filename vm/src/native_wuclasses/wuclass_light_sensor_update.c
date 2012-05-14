#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include "native_wuclasses.h"
#include <avr/io.h>

#ifdef ENABLE_WUCLASS_LIGHT_SENSOR

void wuclass_light_sensor_update(wkpf_local_wuobject *wuobject) {
  // Pieced together from IntelDemoLightSensorV1.java, Adc.java and native_avr.c

  // Adc.setPrescaler(Adc.DIV64);
  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

  // Adc.setReference(Adc.INTERNAL);
  ADMUX = (3 << 6) & 0xc0;              // set reference value

  // light_sensor_reading = Adc.getByte(Adc.CHANNEL15);
  // ADLAR = 1
  u08_t channel  = 39;
  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
  ADCSRB |= (channel & 0x20)>>2;

  // do conversion
  ADCSRA |= _BV(ADSC);                  // Start conversion
  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
  ADCSRA |= _BV(ADIF);                  // clear ADCIF
  DEBUGF_WKPFUPDATE("WKPFUPDATE(LightSensor): Sensed light value: %x\n", ADCH);
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, ADCH);
}

#endif // ENABLE_WUCLASS_LIGHT_SENSOR
