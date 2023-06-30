/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash_driver.h
  * @author  MCD Application Team
  * @brief   Header for flash_driver.c module
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
#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

/* Exported types ------------------------------------------------------------*/

/* Bit mask to modify Flash Control status */
typedef uint32_t  FD_Flash_ctrl_bm_t;

/* Flash operation status */
typedef enum
{
  FD_FLASHOP_SUCCESS,
  FD_FLASHOP_FAILURE
} FD_FlashOp_Status_t;

/* Flash Driver commands to enable or disable flash access */
typedef enum
{
  LL_FLASH_ENABLE,
  LL_FLASH_DISABLE,
} FD_FLASH_Status_t;

/* Exported constants --------------------------------------------------------*/

#define FD_SYSTEM              0x00000001  /* Bit identifying flash access forbidden by system */
#define FD_LL_EVENT_SCHEDULER  0x00000002  /* Bit identifying flash access forbidden by LL Event Scheduler */

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FD_SetStatus(FD_Flash_ctrl_bm_t Flags_bm, FD_FLASH_Status_t Status);
FD_FlashOp_Status_t FD_WriteData(uint32_t Dest, uint32_t Payload);
FD_FlashOp_Status_t FD_EraseSectors(uint32_t Sect);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_DRIVER_H */
