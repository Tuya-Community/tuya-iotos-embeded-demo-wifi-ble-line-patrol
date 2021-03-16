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

#ifndef __TUYA_DEVICE_H__
#define __TUYA_DEVICE_H__

/* Includes ------------------------------------------------------------------*/
#include "tuya_cloud_com_defs.h"
    
#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */
    
#ifdef _TUYA_DEVICE_GLOBAL
    #define _TUYA_DEVICE_EXT 
#else
    #define _TUYA_DEVICE_EXT extern
#endif /* _TUYA_DEVICE_GLOBAL */ 
    
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/              
/* Exported macro ------------------------------------------------------------*/
// device information define
#define DEV_SW_VERSION USER_SW_VER
#define PRODECT_ID "frppntblkqpgq9be"

/* Exported functions ------------------------------------------------------- */
/**
 * @Function: app_init
 * @Description: 
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
_TUYA_DEVICE_EXT \
VOID app_init(VOID);

/**
 * @Function: pre_device_init
 * @Description: device information (SDK information, version number, firmware identification, etc.) and reboot reason printing, log level setting 
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
_TUYA_DEVICE_EXT \
VOID pre_device_init(VOID);

/**
 * @Function: device_init
 * @Description: device initialization process 
 * @Input: none
 * @Output: none
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
_TUYA_DEVICE_EXT \
OPERATE_RET device_init(VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TUYA_DEVICE_H__ */
