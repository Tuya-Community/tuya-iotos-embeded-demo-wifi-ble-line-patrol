/*
 * @Author: yj 
 * @email: shiliu.yang@tuya.com
 * @LastEditors: yj 
 * @file name: tuya_device.c
 * @Description: template demo for SDK WiFi & BLE for BK7231T
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-01-23 16:33:00
 * @LastEditTime: 2021-02-25 15:00:00
 */

#define _TUYA_DEVICE_GLOBAL

/* Includes ------------------------------------------------------------------*/
#include "uni_log.h"
#include "tuya_iot_wifi_api.h"
#include "tuya_hal_system.h"
#include "tuya_iot_com_api.h"
#include "tuya_cloud_error_code.h"
#include "gw_intf.h"
#include "gpio_test.h"
#include "tuya_gpio.h"
#include "tuya_key.h"
#include "tuya_led.h"

/* Private includes ----------------------------------------------------------*/
#include "tuya_device.h"
#include "tuya_line_patrol_sensor.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* WiFi related configurations */
#define WIFI_WORK_MODE_SEL          GWCM_OLD_PROD	//select WiFi working mode
#define WIFI_CONNECT_OVERTIME_S     180             //WiFi distribution network timeout, uint：s 

/* Connect network button related settings, long press to enter the connection network mode */
#define WIFI_KEY_PIN                TY_GPIOA_14 //Connecting network key PIN 
#define WIFI_KEY_TIMER_MS           100         //Set WiFi-Key polling scan time, default 100ms 
#define WIFI_KEY_LONG_PRESS_MS      3000        //Key long press time 
#define WIFI_KEY_SEQ_PRESS_MS       400         //Key press time
#define WIFI_KEY_LOW_LEVEL_ENABLE   TRUE        //TRUE: Key press is low level, FALSE: Key press is high level

/* Connection network WiFi-LED settings*/
#define WIFI_LED_PIN                TY_GPIOA_26 //Connection network LED PIN  
#define WIFI_LED_LOW_LEVEL_ENABLE   FALSE       //TRUE：low level turn on LED, FALSE：High level turn on LED
#define WIFI_LED_FAST_FLASH_MS      300         //Set fast blink time is 300ms 
#define WIFI_LED_LOW_FLASH_MS       500         //Set slow blink time is 500ms 

/* Private variables ---------------------------------------------------------*/
LED_HANDLE wifi_led_handle; //Define the WiFi-LED handle, for WiFi-LED registration use 

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
 * @Function: wifi_state_led_reminder
 * @Description: WiFi-LED prompt according to the current WiFi status 
 * @Input: cur_stat：Current WiFi Status 
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_state_led_reminder(IN CONST GW_WIFI_NW_STAT_E cur_stat)
{
    switch (cur_stat)
    {
        case STAT_LOW_POWER:    //WiFi connection network timeout, enter low power mode
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //Turn off WiFi-LED 
        break;

        case STAT_UNPROVISION: //SamrtConfig connected network mode, waiting for connection 
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_FAST_FLASH_MS, 0xFFFF); //WiFi-LED fast blink
        break;

        case STAT_AP_STA_UNCFG: //ap connected network mode, waiting for connection 
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_LOW_FLASH_MS, 0xFFFF); //WiFi-LED slow blink 
        break;

        case STAT_AP_STA_DISC:
        case STAT_STA_DISC:     //SamrtConfig/ap connecting...
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //Turn off WiFi-LED 
        break;

        case STAT_CLOUD_CONN:
        case STAT_AP_CLOUD_CONN: //Already connected to Tuya Cloud 
            tuya_set_led_light_type(wifi_led_handle, OL_HIGH, 0, 0); //Turn on WiFi-LED 
        break;

        default:
        break;
    }
}

/**
 * @Function: wifi_key_process
 * @Description: Key press callback function
 * @Input: port：Pin where the key is pressed 
 * @Input: type：short press, long press or sequence press 
 * @Input: cnt: sequence press count 
 * @Output: none
 * @Return: none
 * @Others: Long press to enter the connection network mode, other non-responsive 
 */
STATIC VOID wifi_key_process(TY_GPIO_PORT_E port, PUSH_KEY_TYPE_E type, INT_T cnt)
{
    PR_DEBUG("port:%d, type:%d, cnt:%d", port, type, cnt);
    OPERATE_RET op_ret = OPRT_OK;
    UCHAR_T ucConnectMode = 0;

    if (port = WIFI_KEY_PIN) {
        if (LONG_KEY == type) { //long press to enter the connection network mode
            PR_DEBUG("key long press, remove device");
            /* remove device manually */
            tuya_iot_wf_gw_unactive();
        } else if (NORMAL_KEY == type) {
            PR_DEBUG("key normal press, non-responsive");
        } else {
            PR_DEBUG("key sequence press, non-responsive");
        }
    }
}

/**
 * @Function: wifi_config_init
 * @Description: Initialize WiFi-Key, WiFi-LED 
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_config_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* WiFi-LED registration */ 
    op_ret = tuya_create_led_handle(WIFI_LED_PIN, TRUE, &wifi_led_handle);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }
    tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //Turn off WiFi-LED

    /* WiFi-Key initialization */
    KEY_USER_DEF_S key_def;

    /* Set WiFi-Key poll time */
    op_ret = key_init(NULL, 0, WIFI_KEY_TIMER_MS);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }

    /* Initialization WiFi parameters */
    memset(&key_def, 0, SIZEOF(key_def));
    key_def.port = WIFI_KEY_PIN;                            //WiFi-Key Pin 
    key_def.long_key_time = WIFI_KEY_LONG_PRESS_MS;         //Set long press time 
    key_def.low_level_detect = WIFI_KEY_LOW_LEVEL_ENABLE;   //TRUE: low level when WiFi-Key is pressed, FALSE：high level when WiFi-Key is pressed 
    key_def.lp_tp = LP_ONCE_TRIG;   //
    key_def.call_back = wifi_key_process;                   //WiFi-Key callback function
    key_def.seq_key_detect_time = WIFI_KEY_SEQ_PRESS_MS;    //WiFi-Key sequence press time configuration

    /* WiFi-Key registration */
    op_ret = reg_proc_key(&key_def);
    if (op_ret != OPRT_OK) {
        PR_ERR("reg_proc_key err:%d", op_ret);
    }
}

/**
 * @Function:gpio_test 
 * @Description: gpio test
 * @Input: in: gpio input pins 
 * @Input: out: gpio output pins 
 * @Output: none
 * @Return: none
 * @Others: none
 */
BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    return gpio_test_all(in, out);
}

/**
 * @Function: mf_user_callback
 * @Description: authorization callback function
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: Empty the data in flash 
 */
VOID mf_user_callback(VOID)
{
    hw_reset_flash_data();
}

/**
 * @Function: prod_test
 * @Description: When the product test hotspot is scanned, enter the product test callback function
 * @Input: flag: authorized flag 
 * @Input: rssi: signal strength 
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID prod_test(BOOL_T flag, SCHAR_T rssi)
{
    if (flag == FALSE || rssi < -60) 
    {
        PR_ERR("Prod test failed... flag:%d, rssi:%d", flag, rssi);
        return;
    }
    PR_NOTICE("flag:%d rssi:%d", flag, rssi);
}

/**
 * @Function: app_init
 * @Description: 
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID app_init(VOID)
{
    /* set WiFi working mode, product test callback function */
    app_cfg_set(WIFI_WORK_MODE_SEL, prod_test);

    /* set the connection network timeout */
    tuya_iot_wf_timeout_set(WIFI_CONNECT_OVERTIME_S);

    /* WiFi-Key, Wifi-LED initialization */
    wifi_config_init();
}

/**
 * @Function: pre_device_init
 * @Description: device information (SDK information, version number, firmware identification, etc.) and reboot reason printing, log level setting 
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID pre_device_init(VOID)
{
    PR_DEBUG("%s",tuya_iot_get_sdk_info());
    PR_DEBUG("%s:%s",APP_BIN_NAME,DEV_SW_VERSION);
    PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);
    PR_NOTICE("system reset reason:[%s]",tuya_hal_system_get_rst_info());
    /* Log printing level setting */
    SetLogManageAttr(TY_LOG_LEVEL_ERR);
}

/**
 * @Function: status_changed_cb
 * @Description: network status changed callback
 * @Input: status: current status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID status_changed_cb(IN CONST GW_STATUS_E status)
{
    PR_NOTICE("status_changed_cb is status:%d",status);

    if(GW_NORMAL == status) {
        hw_report_all_dp_status();
    }else if(GW_RESET == status) {
        PR_NOTICE("status is GW_RESET");
    }
}

/**
 * @Function: upgrade_notify_cb
 * @Description: firmware download finish result callback
 * @Input: fw: firmware info
 * @Input: download_result: 0 means download succes. other means fail
 * @Input: pri_data: private data
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    PR_DEBUG("download  Finish");
    PR_DEBUG("download_result:%d", download_result);
}

/**
 * @Function: get_file_data_cb
 * @Description: firmware download content process callback
 * @Input: fw: firmware info
 * @Input: total_len: firmware total size
 * @Input: offset: offset of this download package
 * @Input: data && len: this download package
 * @Input: pri_data: private data
 * @Output: remain_len: the size left to process in next cb
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset, \
                                     IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d", offset, len);

    return OPRT_OK;
}

/**
 * @Function: gw_ug_inform_cb
 * @Description: gateway ota firmware available nofity callback
 * @Input: fw: firmware info
 * @Output: none
 * @Return: int:
 * @Others: 
 */
INT_T gw_ug_inform_cb(IN CONST FW_UG_S *fw)
{
    PR_DEBUG("Rev GW Upgrade Info");
    PR_DEBUG("fw->fw_url:%s", fw->fw_url);
    PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    PR_DEBUG("fw->file_size:%d", fw->file_size);

    return tuya_iot_upgrade_gw(fw, get_file_data_cb, upgrade_notify_cb, NULL);
}

/**
 * @Function: hw_reset_flash_data
 * @Description: hardware reset, erase user data from flash
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
VOID hw_reset_flash_data(VOID)
{

}

/**
 * @Function: gw_reset_cb
 * @Description: gateway restart callback (callback when the device is removed from the app) 
 * @Input: type:gateway reset type
 * @Output: none
 * @Return: none
 * @Others: reset factory clear flash data
 */
VOID gw_reset_cb(IN CONST GW_RESET_TYPE_E type)
{
    PR_DEBUG("gw_reset_cb type:%d",type);
    if(GW_REMOTE_RESET_FACTORY != type) {
        PR_DEBUG("type is GW_REMOTE_RESET_FACTORY");
        return;
    }

    hw_reset_flash_data();
}

/**
 * @Function: dev_obj_dp_cb
 * @Description: obj dp info cmd callback 
 * @Input: dp:obj dp info
 * @Output: none
 * @Return: none
 * @Others: app send data by dpid  control device stat
 */
VOID dev_obj_dp_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    PR_DEBUG("dp->cid:%s dp->dps_cnt:%d",dp->cid,dp->dps_cnt);
    UCHAR_T i = 0;

    for(i = 0;i < dp->dps_cnt;i++) {
        deal_dp_proc(&(dp->dps[i]));

        /* reply to what you receive from Tuya Cloud. In the actual project, the received data should be processed. */
        //dev_report_dp_json_async(get_gw_cntl()->gw_if.id, dp->dps, dp->dps_cnt);
    }
}

/**
 * @Function: dev_raw_dp_cb
 * @Description: raw dp info cmd callback 
 * @Input: dp: raw dp info
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID dev_raw_dp_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    PR_DEBUG("raw data dpid:%d",dp->dpid);
    PR_DEBUG("recv len:%d",dp->len);
#if 1 
    INT_T i = 0;
    for(i = 0;i < dp->len;i++) {
        PR_DEBUG_RAW("%02X ",dp->data[i]);
    }
#endif
    PR_DEBUG_RAW("\n");
    PR_DEBUG("end");
}

/**
 * @Function: dev_dp_query_cb
 * @Description: dp info query callback, cloud or app actively query device info
 * @Input: dp_qry: query info
 * @Output: none
 * @Return: none
 * @Others: none
 */
STATIC VOID dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry) 
{
    PR_NOTICE("Recv DP Query Cmd");

    hw_report_all_dp_status();
}

/**
 * @Function: wf_nw_status_cb
 * @Description: tuya-sdk network state check callback
 * @Input: stat: curr network status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID wf_nw_status_cb(IN CONST GW_WIFI_NW_STAT_E stat)
{
    PR_NOTICE("wf_nw_status_cb,wifi_status:%d", stat);
    wifi_state_led_reminder(stat);

    if(stat == STAT_AP_STA_CONN || stat >= STAT_STA_CONN) {
        hw_report_all_dp_status();
    }
}

/**
 * @Function: device_init
 * @Description: device initialization process 
 * @Input: none
 * @Output: none
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
OPERATE_RET device_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    TY_IOT_CBS_S wf_cbs = {
        status_changed_cb,\ 
        gw_ug_inform_cb,\   
        gw_reset_cb,\
        dev_obj_dp_cb,\
        dev_raw_dp_cb,\
        dev_dp_query_cb,\
        NULL,
    };

    op_ret = tuya_iot_wf_soc_dev_init_param(WIFI_WORK_MODE_SEL, WF_START_SMART_FIRST, &wf_cbs, NULL, PRODECT_ID, DEV_SW_VERSION);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_wf_soc_dev_init_param error,err_num:%d",op_ret);
        return op_ret;
    }

    op_ret = tuya_iot_reg_get_wf_nw_stat_cb(wf_nw_status_cb);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb is error,err_num:%d",op_ret);
        return op_ret;
    }

    line_patrol_init();

    return op_ret;
}
