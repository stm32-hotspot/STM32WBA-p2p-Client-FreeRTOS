/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_freertos.c
  * @author  MCD Application Team
  * @brief   FreeRTOS applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* Definition of all FreeRTOS global resources used by the application */

/* BPKA_TASK related resources */
osThreadId_t BPKA_Thread;

const osThreadAttr_t BPKA_Thread_Attr = {
  .name = "BPKA_Thread",
  .priority = BPKA_TASK_PRIO,
  .stack_size = BPKA_TASK_STACK_SIZE
};

osSemaphoreId_t BPKA_Thread_Sem;

const osSemaphoreAttr_t BPKA_Thread_Sem_Attr = {
  .name = "BPKA_Thread_Sem"
};

/* HW_RNG_TASK related resources */
osThreadId_t HW_RNG_Thread;

const osThreadAttr_t HW_RNG_Thread_Attr = {
  .name = "HW_RNG_Thread",
  .priority = HW_RNG_TASK_PRIO,
  .stack_size = HW_RNG_TASK_STACK_SIZE
};

osSemaphoreId_t HW_RNG_Thread_Sem;

const osSemaphoreAttr_t HW_RNG_Thread_Sem_Attr = {
  .name = "HW_RNG_Thread_Sem"
};

/* BLE_STACK_TASK related resources */
osThreadId_t BLE_HOST_Thread;

const osThreadAttr_t BLE_HOST_Thread_Attr = {
  .name = "BLE_HOST_Thread",
  .priority = BLE_HOST_TASK_PRIO,
  .stack_size = BLE_HOST_TASK_STACK_SIZE
};

osSemaphoreId_t BLE_HOST_Thread_Sem;

const osSemaphoreAttr_t BLE_HOST_Thread_Sem_Attr = {
  .name = "BLE_HOST_Thread_Sem"
};

/* HCI_ASYNCH_EVT_TASK related resources */
osThreadId_t HCI_ASYNCH_EVT_Thread;

const osThreadAttr_t HCI_ASYNCH_EVT_Thread_Attr = {
  .name = "HCI_ASYNCH_EVT_Thread",
  .priority = HCI_ASYNCH_EVT_TASK_PRIO,
  .stack_size = HCI_ASYNCH_EVT_TASK_STACK_SIZE
};

osSemaphoreId_t HCI_ASYNCH_EVT_Thread_Sem;

const osSemaphoreAttr_t HCI_ASYNCH_EVT_Thread_Sem_Attr = {
  .name = "HCI_ASYNCH_EVT_Thread_Sem"
};

/* LINK_LAYER_TASK related resources */
osThreadId_t LINK_LAYER_Thread;

const osThreadAttr_t LINK_LAYER_Thread_Attr = {
  .name = "LINK_LAYER_Thread",
  .priority = LINK_LAYER_TASK_PRIO,
  .stack_size = LINK_LAYER_TASK_STACK_SIZE
};

osSemaphoreId_t LINK_LAYER_Thread_Sem;

const osSemaphoreAttr_t LINK_LAYER_Thread_Sem_Attr = {
  .name = "LINK_LAYER_Thread_Sem"
};

osMutexId_t LINK_LAYER_Thread_Mutex;

const osMutexAttr_t LINK_LAYER_Thread_Mutex_Attr = {
  .name = "LINK_LAYER_Thread_Mutex"
};

/* AMM_BCKGND_TASK related resources */
osThreadId_t AMM_BCKGND_Thread;

const osThreadAttr_t AMM_BCKGND_Thread_Attr = {
  .name = "AMM_BCKGND_Thread",
  .priority = AMM_BCKGND_TASK_PRIO,
  .stack_size = AMM_BCKGND_TASK_STACK_SIZE
};

osSemaphoreId_t AMM_BCKGND_Thread_Sem;

const osSemaphoreAttr_t AMM_BCKGND_Thread_Sem_Attr = {
  .name = "AMM_BCKGND_Thread_Sem"
};

/* FLASH_MANAGER_BCKGND_TASK related resources */
osThreadId_t FLASH_MANAGER_BCKGND_Thread;

const osThreadAttr_t FLASH_MANAGER_BCKGND_Thread_Attr = {
  .name = "FLASH_MANAGER_BCKGND_Thread",
  .priority = FLASH_MANAGER_BCKGND_TASK_PRIO,
  .stack_size = FLASH_MANAGER_BCKGND_TASK_STACK_SIZE
};

osSemaphoreId_t FLASH_MANAGER_BCKGND_Thread_Sem;

const osSemaphoreAttr_t FLASH_MANAGER_BCKGND_Thread_Sem_Attr = {
  .name = "FLASH_MANAGER_BCKGND_Thread_Sem"
};

/* PB1_BUTTON_PUSHED_TASK related resources */
osThreadId_t PB1_BUTTON_PUSHED_Thread;

const osThreadAttr_t PB1_BUTTON_PUSHED_Thread_Attr = {
  .name = "PB1_BUTTON_PUSHED_Thread",
  .priority = PB1_BUTTON_PUSHED_TASK_PRIO,
  .stack_size = PB1_BUTTON_PUSHED_TASK_STACK_SIZE
};

osSemaphoreId_t PB1_BUTTON_PUSHED_Thread_Sem;

const osSemaphoreAttr_t PB1_BUTTON_PUSHED_Thread_Sem_Attr = {
  .name = "PB1_BUTTON_PUSHED_Thread_Sem"
};

/* PB2_BUTTON_PUSHED_TASK related resources */
osThreadId_t PB2_BUTTON_PUSHED_Thread;

const osThreadAttr_t PB2_BUTTON_PUSHED_Thread_Attr = {
  .name = "PB2_BUTTON_PUSHED_Thread",
  .priority = PB2_BUTTON_PUSHED_TASK_PRIO,
  .stack_size = PB2_BUTTON_PUSHED_TASK_STACK_SIZE
};

osSemaphoreId_t PB2_BUTTON_PUSHED_Thread_Sem;

const osSemaphoreAttr_t PB2_BUTTON_PUSHED_Thread_Sem_Attr = {
  .name = "PB2_BUTTON_PUSHED_Thread_Sem"
};

/* PB3_BUTTON_PUSHED_TASK related resources */
osThreadId_t PB3_BUTTON_PUSHED_Thread;

const osThreadAttr_t PB3_BUTTON_PUSHED_Thread_Attr = {
  .name = "PB3_BUTTON_PUSHED_Thread",
  .priority = PB3_BUTTON_PUSHED_TASK_PRIO,
  .stack_size = PB3_BUTTON_PUSHED_TASK_STACK_SIZE
};

osSemaphoreId_t PB3_BUTTON_PUSHED_Thread_Sem;

const osSemaphoreAttr_t PB3_BUTTON_PUSHED_Thread_Sem_Attr = {
  .name = "PB3_BUTTON_PUSHED_Thread_Sem"
};

/* GATT Client discover related resources */
osThreadId_t GATTC_Discover_Thread;

const osThreadAttr_t GATTC_Discover_Thread_Attr = {
  .name = "GATTC_Discover_Thread",
  .priority = GATTC_DISCOVER_TASK_PRIO,
  .stack_size = GATTC_DISCOVER_TASK_STACK_SIZE
};

osSemaphoreId_t GATTC_Discover_Thread_Sem;

const osSemaphoreAttr_t GATTC_Discover_Thread_Sem_Attr = {
  .name = "GATTC_Discover_Thread_Sem"
};

/* LINK_LAYER_TEMP_MEAS_TASK related resources */
osThreadId_t LINK_LAYER_TEMP_MEAS_Thread;

const osThreadAttr_t LINK_LAYER_TEMP_MEAS_Thread_Attr = {
  .name = "LINK_LAYER_TEMP_MEAS_Thread",
  .priority = LINK_LAYER_TEMP_MEAS_TASK_PRIO,
  .stack_size = LINK_LAYER_TEMP_MEAS_TASK_STACK_SIZE
};

osSemaphoreId_t LINK_LAYER_TEMP_MEAS_Thread_Sem;

const osSemaphoreAttr_t LINK_LAYER_TEMP_MEAS_Thread_Sem_Attr = {
  .name = "LINK_LAYER_TEMP_MEAS_Thread_Sem"
};

/* CONNECTION_REQUEST related resources */
osThreadId_t CONNECTION_REQ_Thread;

const osThreadAttr_t CONNECTION_REQ_Thread_Attr = {
  .name = "CONNECTION_REQ_Thread",
  .priority = CONNECTION_REQ_TASK_PRIO,
  .stack_size = CONNECTION_REQ_TASK_STACK_SIZE
};

osSemaphoreId_t CONNECTION_REQ_Thread_Sem;

const osSemaphoreAttr_t CONNECTION_REQ_Thread_Sem_Attr = {
  .name = "CONNECTION_REQ_Thread_Sem"
};

/* gap_cmd_resp sync mechanism */
osSemaphoreId_t IDLEEVT_PROC_GAP_COMPLETE_Sem;

const osSemaphoreAttr_t IDLEEVT_PROC_GAP_COMPLETE_Sem_Attr = {
  .name = "IDLEEVT_PROC_GAP_COMPLETE_Sem"
};

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  MX_FreeRTOS_Init
  * @param  None
  * @retval None
  */
void MX_FreeRTOS_Init(void)
{
  /* USER CODE BEGIN  Before_osKernelInitialize */

  /* USER CODE END  Before_osKernelInitialize */

  osKernelInitialize();

  /* USER CODE BEGIN  osKernelInitialize_Error */

  /* USER CODE END  osKernelInitialize_Error */
}

  /**
  * @brief  MX_FreeRTOS_Init
  * @param  None
  * @retval None
  */
void MX_FreeRTOS_Start(void)
{
  /* USER CODE BEGIN  Before_osKernelStart */

  /* USER CODE END  Before_osKernelStart */

  osKernelStart();

  /* USER CODE BEGIN  osKernelStart_Error */

  /* USER CODE END  osKernelStart_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
