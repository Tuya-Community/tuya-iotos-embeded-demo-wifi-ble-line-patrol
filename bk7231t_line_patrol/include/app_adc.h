#ifndef __APP_ADC_H__
#define __APP_ADC_H__

#include "tuya_cloud_types.h"

//#define ENABLE_RS2255

VOID_T adc_init(VOID_T);

VOID_T get_adc_value(OUT USHORT_T* adc_value);

#ifdef ENABLE_RS2255
VOID_T get_A1_adc_value(OUT USHORT_T* adc_value);
VOID_T get_A2_adc_value(OUT USHORT_T* adc_value);
#endif /* ENABLE_RS2255 */

#endif /* __APP_ADC_H__ */