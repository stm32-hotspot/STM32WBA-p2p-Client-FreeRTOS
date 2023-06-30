/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    host_stack_sequencer.c
  * @author  MCD Application Team
  * @brief : Source file for the stack tasks
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
#include "host_stack_sequencer.h"
#include "app_conf.h"
#include "ll_sys.h"

/* External function prototypes -----------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

void HostStack_Process(void)
{
  /* USER CODE BEGIN HostStack_Process 0 */

  /* USER CODE END HostStack_Process 0 */

  BleStackCB_Process();

  /* USER CODE BEGIN HostStack_Process 1 */

  /* USER CODE END HostStack_Process 1 */
}

void BleStackCB_Process(void)
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(BLE_HOST_Thread_Sem);
}
