/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ll_sys_sequencer_if.c
  * @author  MCD Application Team
  * @brief   Source file for initiating the system sequencer
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

#include "app_common.h"
#include "ll_intf.h"
#include "ll_sys.h"
/* FREERTOS_MARK */
#include "app_freertos.h"
#include "main.h"
#include "adc_ctrl.h"
#include "linklayer_plat.h"

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
void ll_sys_bg_temperature_measurement(void);
static void ll_sys_bg_temperature_measurement_init(void);
static void request_temperature_measurement(void);
/* FREERTOS_MARK */
static void request_temperature_measurement_Entry(void* thread_input);
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

/* FREERTOS_MARK */
static void ll_sys_bg_process_Entry(void* thread_input);

/**
  * @brief  Link Layer background process initialization
  * @param  None
  * @retval None
  */
void ll_sys_bg_process_init(void)
{
  /* Register tasks */
  /* FREERTOS_MARK */
  LINK_LAYER_Thread_Sem = osSemaphoreNew(1, 0, &LINK_LAYER_Thread_Sem_Attr);
  LINK_LAYER_Thread_Mutex = osMutexNew(&LINK_LAYER_Thread_Mutex_Attr);
  LINK_LAYER_Thread = osThreadNew(ll_sys_bg_process_Entry, NULL, &LINK_LAYER_Thread_Attr);
}

/**
  * @brief  Link Layer background process next iteration scheduling
  * @param  None
  * @retval None
  */
void ll_sys_schedule_bg_process(void)
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(LINK_LAYER_Thread_Sem);
}

void ll_sys_config_params(void)
{
  ll_intf_config_ll_ctx_params(USE_RADIO_LOW_ISR, NEXT_EVENT_SCHEDULING_FROM_ISR);
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  ll_sys_bg_temperature_measurement_init();
  ll_intf_set_temperature_sensor_state();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
}

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/**
  * @brief  Link Layer temperature request background process initialization
  * @param  None
  * @retval None
  */
void ll_sys_bg_temperature_measurement_init(void)
{
  /* Register tasks */
  /* FREERTOS_MARK */
  LINK_LAYER_TEMP_MEAS_Thread_Sem = osSemaphoreNew(1, 0, &LINK_LAYER_TEMP_MEAS_Thread_Sem_Attr);
  LINK_LAYER_TEMP_MEAS_Thread = osThreadNew(request_temperature_measurement_Entry, NULL, &LINK_LAYER_TEMP_MEAS_Thread_Attr);
}

void ll_sys_bg_temperature_measurement(void)
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(LINK_LAYER_TEMP_MEAS_Thread_Sem);
}

void request_temperature_measurement(void)
{
  int16_t temperature_value = 0;

  UTILS_ENTER_LIMITED_CRITICAL_SECTION(RCC_INTR_PRIO<<4);
  adc_ctrl_req(SYS_ADC_LL_EVT, ADC_ON);
  temperature_value = adc_ctrl_request_temperature();
  adc_ctrl_req(SYS_ADC_LL_EVT, ADC_OFF);
  ll_intf_set_temperature_value(temperature_value);
  UTILS_EXIT_LIMITED_CRITICAL_SECTION();
}

/* FREERTOS_MARK */
static void request_temperature_measurement_Entry(void* thread_input)
{
  (void)(thread_input);

  while(1)
  {
    osSemaphoreAcquire(LINK_LAYER_TEMP_MEAS_Thread_Sem, osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    request_temperature_measurement();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

/* FREERTOS_MARK */
static void ll_sys_bg_process_Entry(void* thread_input)
{
  (void)(thread_input);

  while(1)
  {
    osSemaphoreAcquire(LINK_LAYER_Thread_Sem, osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    ll_sys_bg_process();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

void LINKLAYER_PLAT_EnableOSContextSwitch(void)
{
  /* Not used with ST sequencer */
}

void LINKLAYER_PLAT_DisableOSContextSwitch(void)
{
  /* Not used with ST sequencer */
}
