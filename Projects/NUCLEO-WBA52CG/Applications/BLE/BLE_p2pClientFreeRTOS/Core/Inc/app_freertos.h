/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  MCD Application Team
  * @brief   FreeRTOS applicative header file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os2.h"
#include "app_entry.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* Declaration of all FreeRTOS global resources used by the application */

/* BPKA_TASK related resources */
extern osThreadId_t BPKA_Thread;
extern const osThreadAttr_t BPKA_Thread_Attr;
extern osSemaphoreId_t  BPKA_Thread_Sem;
extern const osSemaphoreAttr_t BPKA_Thread_Sem_Attr;

/* HW_RNG_TASK related resources  */
extern osThreadId_t HW_RNG_Thread;
extern const osThreadAttr_t HW_RNG_Thread_Attr;
extern osSemaphoreId_t  HW_RNG_Thread_Sem;
extern const osSemaphoreAttr_t HW_RNG_Thread_Sem_Attr;

/* BLE_STACK_TASK related resources */
extern osThreadId_t BLE_HOST_Thread;
extern const osThreadAttr_t BLE_HOST_Thread_Attr;
extern osSemaphoreId_t BLE_HOST_Thread_Sem;
extern const osSemaphoreAttr_t BLE_HOST_Thread_Sem_Attr;

/* HCI_ASYNCH_EVT_TASK related resources */
extern osThreadId_t HCI_ASYNCH_EVT_Thread;
extern const osThreadAttr_t HCI_ASYNCH_EVT_Thread_Attr;
extern osSemaphoreId_t HCI_ASYNCH_EVT_Thread_Sem;
extern const osSemaphoreAttr_t HCI_ASYNCH_EVT_Thread_Sem_Attr;

/* LINK_LAYER_TASK related resources */
extern osThreadId_t LINK_LAYER_Thread;
extern const osThreadAttr_t LINK_LAYER_Thread_Attr;
extern osSemaphoreId_t LINK_LAYER_Thread_Sem;
extern const osSemaphoreAttr_t LINK_LAYER_Thread_Sem_Attr;
extern osMutexId_t LINK_LAYER_Thread_Mutex;
extern const osMutexAttr_t LINK_LAYER_Thread_Mutex_Attr;

/* AMM_BCKGND_TASK related resources */
extern osThreadId_t AMM_BCKGND_Thread;
extern const osThreadAttr_t AMM_BCKGND_Thread_Attr;
extern osSemaphoreId_t AMM_BCKGND_Thread_Sem;
extern const osSemaphoreAttr_t AMM_BCKGND_Thread_Sem_Attr;

/* FLASH_MANAGER_BCKGND_TASK related resources */
extern osThreadId_t FLASH_MANAGER_BCKGND_Thread;
extern const osThreadAttr_t FLASH_MANAGER_BCKGND_Thread_Attr;
extern osSemaphoreId_t FLASH_MANAGER_BCKGND_Thread_Sem;
extern const osSemaphoreAttr_t FLASH_MANAGER_BCKGND_Thread_Sem_Attr;

/* PB1_BUTTON_PUSHED_TASK related resources */
extern osThreadId_t PB1_BUTTON_PUSHED_Thread;
extern const osThreadAttr_t PB1_BUTTON_PUSHED_Thread_Attr;
extern osSemaphoreId_t PB1_BUTTON_PUSHED_Thread_Sem;
extern const osSemaphoreAttr_t PB1_BUTTON_PUSHED_Thread_Sem_Attr;

/* PB2_BUTTON_PUSHED_TASK related resources */
extern osThreadId_t PB2_BUTTON_PUSHED_Thread;
extern const osThreadAttr_t PB2_BUTTON_PUSHED_Thread_Attr;
extern osSemaphoreId_t PB2_BUTTON_PUSHED_Thread_Sem;
extern const osSemaphoreAttr_t PB2_BUTTON_PUSHED_Thread_Sem_Attr;

/* PB3_BUTTON_PUSHED_TASK related resources */
extern osThreadId_t PB3_BUTTON_PUSHED_Thread;
extern const osThreadAttr_t PB3_BUTTON_PUSHED_Thread_Attr;
extern osSemaphoreId_t PB3_BUTTON_PUSHED_Thread_Sem;
extern const osSemaphoreAttr_t PB3_BUTTON_PUSHED_Thread_Sem_Attr;

/* GATT Client discover related resources */
extern osThreadId_t GATTC_Discover_Thread;
extern const osThreadAttr_t GATTC_Discover_Thread_Attr;
extern osSemaphoreId_t GATTC_Discover_Thread_Sem;
extern const osSemaphoreAttr_t GATTC_Discover_Thread_Sem_Attr;

/* LINK_LAYER_TEMP_MEAS_TASK related resources */
extern osThreadId_t LINK_LAYER_TEMP_MEAS_Thread;
extern const osThreadAttr_t LINK_LAYER_TEMP_MEAS_Thread_Attr;
extern osSemaphoreId_t LINK_LAYER_TEMP_MEAS_Thread_Sem;
extern const osSemaphoreAttr_t LINK_LAYER_TEMP_MEAS_Thread_Sem_Attr;

/* CONNECTION_REQUEST related resources */
extern osThreadId_t CONNECTION_REQ_Thread;
extern const osThreadAttr_t CONNECTION_REQ_Thread_Attr;
extern osSemaphoreId_t CONNECTION_REQ_Thread_Sem;
extern const osSemaphoreAttr_t CONNECTION_REQ_Thread_Sem_Attr;

/* gap_cmd_resp sync mechanism */
extern osSemaphoreId_t IDLEEVT_PROC_GAP_COMPLETE_Sem;
extern const osSemaphoreAttr_t IDLEEVT_PROC_GAP_COMPLETE_Sem_Attr;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define BPKA_TASK_STACK_SIZE    (256*7)
#define BPKA_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define HW_RNG_TASK_STACK_SIZE    (256*7)
#define HW_RNG_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define BLE_HOST_TASK_STACK_SIZE    (256*7)
#define BLE_HOST_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define HCI_ASYNCH_EVT_TASK_STACK_SIZE    (256*7)
#define HCI_ASYNCH_EVT_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define ADV_CANCEL_TASK_STACK_SIZE    (256*7)
#define ADV_CANCEL_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define MEAS_REQ_TASK_STACK_SIZE    (256*7)
#define MEAS_REQ_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define LINK_LAYER_TASK_STACK_SIZE    (256*7)
#define LINK_LAYER_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define AMM_BCKGND_TASK_STACK_SIZE    (256*7)
#define AMM_BCKGND_TASK_PRIO          ((osPriority_t)osPriorityBelowNormal)

#define FLASH_MANAGER_BCKGND_TASK_STACK_SIZE    (256*7)
#define FLASH_MANAGER_BCKGND_TASK_PRIO          ((osPriority_t)osPriorityBelowNormal)

#define PB1_BUTTON_PUSHED_TASK_STACK_SIZE    (256*7)
#define PB1_BUTTON_PUSHED_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define PB2_BUTTON_PUSHED_TASK_STACK_SIZE    (256*7)
#define PB2_BUTTON_PUSHED_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define PB3_BUTTON_PUSHED_TASK_STACK_SIZE    (256*7)
#define PB3_BUTTON_PUSHED_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define GATTC_DISCOVER_TASK_STACK_SIZE    (256*7)
#define GATTC_DISCOVER_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define LINK_LAYER_TEMP_MEAS_TASK_STACK_SIZE    (256*7)
#define LINK_LAYER_TEMP_MEAS_TASK_PRIO          ((osPriority_t) osPriorityNormal)

#define CONNECTION_REQ_TASK_STACK_SIZE    (256*7)
#define CONNECTION_REQ_TASK_PRIO          ((osPriority_t) osPriorityNormal)

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void MX_FreeRTOS_Init(void);
void MX_FreeRTOS_Start(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H__ */
