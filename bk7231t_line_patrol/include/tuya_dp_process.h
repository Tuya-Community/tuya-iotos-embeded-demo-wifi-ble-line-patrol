#ifndef __TUYA_DP_PROCESS_H__
#define __TUYA_DP_PROCESS_H__

#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_types.h"

/* DP define */
typedef UINT8_T SENSOR_DP_T;
#define DP_ID_ADC_VALUE 101

typedef struct  {
    SENSOR_DP_T dp_id_adc_value;

    UINT16_T adc_value;
}SENSOR_STATE_T;

extern SENSOR_STATE_T sensor_state;

VOID_T hw_report_all_dp_status(VOID_T);
VOID_T deal_dp_proc(IN CONST TY_OBJ_DP_S *root);

#endif /* __TUYA_DP_PROCESS */