#include "soc_adc.h"
#include "uni_log.h"
#include "tuya_gpio.h"

#include "app_adc.h"

#ifdef ENABLE_RS2255
/* RS2255 IC 引脚 */
#define RS2255_A    TY_GPIOA_6
#define RS2255_B    TY_GPIOA_7
#endif /* ENABLE_RS2255 */

/* ADC  */
#define ADC_DATA_LEN    4 //ADC 单次采集次数
static  tuya_adc_dev_t  tuya_adc;

VOID_T adc_init(VOID_T)
{
    /* ADC initialization*/
    tuya_adc.priv.pData = Malloc(ADC_DATA_LEN * sizeof(USHORT_T));
    memset(tuya_adc.priv.pData, 0, ADC_DATA_LEN*sizeof(USHORT_T));
    tuya_adc.priv.data_buff_size = ADC_DATA_LEN; //设置数据缓存个数

#ifdef ENABLE_RS2255
    /* RS2255 initialization */
    tuya_gpio_inout_set(RS2255_A, FALSE);
    tuya_gpio_inout_set(RS2255_B, FALSE);
#endif /* ENABLE_RS2255 */
}

VOID_T get_adc_value(OUT USHORT_T* adc_value) 
{
    INT_T ret;

    if (adc_value == NULL) {
        PR_ERR("pm25_adc_value is NULL");
        return;
    }
    memset(tuya_adc.priv.pData, 0, ADC_DATA_LEN*sizeof(USHORT_T));
    ret = tuya_hal_adc_init(&tuya_adc);
    if (ret != OPRT_OK) {
        PR_ERR("ADC init error : %d ", ret);
        return;
    }

    ret = ret = tuya_hal_adc_value_get(ADC_DATA_LEN, adc_value); 
    if (ret != OPRT_OK) {
        PR_ERR("ADC get value error : %d ", ret);
    }

    tuya_hal_adc_finalize(&tuya_adc);
    return;
}

#ifdef ENABLE_RS2255
VOID_T get_A1_adc_value(OUT USHORT_T* adc_value) 
{
    tuya_gpio_write(RS2255_A, FALSE);
    tuya_gpio_write(RS2255_B, TRUE);
    tuya_hal_system_sleep(300);
    get_adc_value(adc_value);
}

VOID_T get_A2_adc_value(OUT USHORT_T* adc_value) 
{
    tuya_gpio_write(RS2255_A, TRUE);
    tuya_gpio_write(RS2255_B, FALSE);
    tuya_hal_system_sleep(300);
    get_adc_value(adc_value);
}
#endif /* ENABLE_RS2255 */
