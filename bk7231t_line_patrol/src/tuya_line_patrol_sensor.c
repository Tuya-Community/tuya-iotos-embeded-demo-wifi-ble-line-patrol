#include "uni_log.h"
#include "tuya_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uni_thread.h"

#include "app_adc.h"
#include "tuya_line_patrol_sensor.h"
#include "tuya_dp_process.h"

USHORT_T adc_value;

VOID_T get_adc_value_task(VOID_T)
{
    while(1) {
        get_adc_value(&adc_value);
        PR_NOTICE("alcohol_adc_value:%d", adc_value);
        sensor_state.adc_value = adc_value;
        hw_report_all_dp_status();

        tuya_hal_system_sleep(1000);
    }
}

VOID_T line_patrol_init(VOID_T)
{
    adc_init();

    tuya_hal_thread_create(NULL, "get value", 512 * 4, TRD_PRIO_2, get_adc_value_task, NULL);

}
