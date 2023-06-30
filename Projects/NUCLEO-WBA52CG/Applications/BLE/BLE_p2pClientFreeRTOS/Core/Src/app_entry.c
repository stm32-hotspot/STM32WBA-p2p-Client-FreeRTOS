/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  MCD Application Team
  * @brief   Entry point of the application
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
#include "app_common.h"
#include "app_conf.h"
#include "main.h"
#include "app_entry.h"
/* FREERTOS_MARK */
#include "app_freertos.h"
#include "stm32_lpm.h"
#include "stm32_timer.h"
#include "stm32_mm.h"
#include "stm32_adv_trace.h"
#include "app_ble.h"
#include "app_sys.h"
#include "otp.h"
#include "scm.h"
#include "bpka.h"
#include "ll_sys.h"
#include "advanced_memory_manager.h"
#include "flash_driver.h"
#include "flash_manager.h"
#include "simple_nvm_arbiter.h"
#include "app_debug.h"
#include "adc_ctrl.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wbaxx_nucleo.h"
#include "usart_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
EXTI_HandleTypeDef exti_handle;

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
/* Section specific to button management using UART */
#define C_SIZE_CMD_STRING       256U

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* FREERTOS_MARK */
/* static bool system_startup_done = FALSE; */

static uint32_t AMM_Pool[CFG_AMM_POOL_SIZE];
static AMM_InitParameters_t ammInitConfig =
{
  .p_PoolAddr = AMM_Pool,
  .PoolSize = CFG_AMM_POOL_SIZE,
  .VirtualMemoryNumber = CFG_AMM_VIRTUAL_MEMORY_NUMBER,
  .a_VirtualMemoryConfigList =
  {
    /* Virtual Memory #1 */
    {
      .Id = CFG_AMM_VIRTUAL_STACK_BLE,
      .BufferSize = CFG_AMM_VIRTUAL_STACK_BLE_BUFFER_SIZE
    },
    /* Virtual Memory #2 */
    {
      .Id = CFG_AMM_VIRTUAL_APP_BLE,
      .BufferSize = CFG_AMM_VIRTUAL_APP_BLE_BUFFER_SIZE
    },
  }
};
/* USER CODE BEGIN PV */
/* Section specific to button management using UART */
static uint8_t CommandString[C_SIZE_CMD_STRING];
static uint16_t indexReceiveChar = 0;

/* USER CODE END PV */

/* Private functions prototypes-----------------------------------------------*/
static void Config_HSE(void);
static void System_Init( void );
static void SystemPower_Config( void );

/**
 * @brief Wrapper for init function of the MM for the AMM
 *
 * @param p_PoolAddr: Address of the pool to use - Not use -
 * @param PoolSize: Size of the pool - Not use -
 *
 * @return None
 */
static void AMM_WrapperInit (uint32_t * const p_PoolAddr, const uint32_t PoolSize);

/**
 * @brief Wrapper for allocate function of the MM for the AMM
 *
 * @param BufferSize
 *
 * @return Allocated buffer
 */
static uint32_t * AMM_WrapperAllocate (const uint32_t BufferSize);

/**
 * @brief Wrapper for free function of the MM for the AMM
 *
 * @param p_BufferAddr
 *
 * @return None
 */
static void AMM_WrapperFree (uint32_t * const p_BufferAddr);
/* FREERTOS_MARK */
static void BPKA_BG_Process_Entry(void* thread_input);
static void HW_RNG_Process_Entry(void* thread_input);
static void AMM_BackgroundProcess_Entry(void* thread_input);
static void FM_BackgroundProcess_Entry(void* thread_input);

/* USER CODE BEGIN PFP */
#if (CFG_LED_SUPPORTED == 1)
static void Led_Init(void);
#endif
#if (CFG_BUTTON_SUPPORTED == 1)
static void Button_Init(void);
#endif
/* Section specific to button management using UART */
static void RxUART_Init(void);
static void RxCpltCallback(uint8_t *pdata, uint16_t size, uint8_t error);
static void UartCmdExecute(void);

/* External variables --------------------------------------------------------*/
extern uint8_t charRx;
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void MX_APPE_Config(void)
{
  /* Configure HSE Tuning */
  Config_HSE();
}

void MX_APPE_Init(void)
{
  APP_DEBUG_SIGNAL_SET(APP_APPE_INIT);

  /* System initialization */
  System_Init();

  /* Configure the system Power Mode */
  SystemPower_Config();

  /* Initialize the Advance Memory Manager */
  AMM_Init (&ammInitConfig);

  /* Register the AMM background task */
  /* FREERTOS_MARK */
  AMM_BCKGND_Thread_Sem = osSemaphoreNew(1, 0, &AMM_BCKGND_Thread_Sem_Attr);
  AMM_BCKGND_Thread = osThreadNew(AMM_BackgroundProcess_Entry, NULL, &AMM_BCKGND_Thread_Attr);

  /* Initialize the Simple NVM Arbiter */
  SNVMA_Init ((uint32_t *)CFG_SNVMA_START_ADDRESS);

  /* Register the flash manager task */
  FLASH_MANAGER_BCKGND_Thread_Sem = osSemaphoreNew(1, 0, &AMM_BCKGND_Thread_Sem_Attr);
  FLASH_MANAGER_BCKGND_Thread = osThreadNew(FM_BackgroundProcess_Entry, NULL, &FLASH_MANAGER_BCKGND_Thread_Attr);
/* USER CODE BEGIN APPE_Init_1 */
#if (CFG_LED_SUPPORTED == 1)
  Led_Init();
#endif
#if (CFG_BUTTON_SUPPORTED == 1)
  Button_Init();
#endif

  RxUART_Init();
/* USER CODE END APPE_Init_1 */

  /* FREERTOS_MARK */
  BPKA_Thread_Sem = osSemaphoreNew(1, 0, &BPKA_Thread_Sem_Attr);
  BPKA_Thread = osThreadNew(BPKA_BG_Process_Entry, NULL, &BPKA_Thread_Attr);

  BPKA_Reset( );

  /* FREERTOS_MARK */
  HW_RNG_Thread_Sem = osSemaphoreNew(1, 0, &HW_RNG_Thread_Sem_Attr);
  HW_RNG_Thread = osThreadNew(HW_RNG_Process_Entry, NULL, &HW_RNG_Thread_Attr);

  HW_RNG_Start( );

  /* Disable flash before any use */
  FD_SetStatus (FD_LL_EVENT_SCHEDULER, LL_FLASH_DISABLE);
  
  /* FREERTOS_MARK */
  IDLEEVT_PROC_GAP_COMPLETE_Sem = osSemaphoreNew(1, 0, &IDLEEVT_PROC_GAP_COMPLETE_Sem_Attr);

  APP_BLE_Init();
  ll_sys_config_params();
  /* FREERTOS_MARK */
  /* system_startup_done = TRUE; */

/* USER CODE BEGIN APPE_Init_2 */

#if (CFG_BUTTON_SUPPORTED == 1)
  /* Register Button Tasks */
  /* FREERTOS_MARK */
  PB1_BUTTON_PUSHED_Thread_Sem = osSemaphoreNew(1, 0, &PB1_BUTTON_PUSHED_Thread_Sem_Attr);
  PB1_BUTTON_PUSHED_Thread = osThreadNew(APP_BLE_Key_Button1_Action_Entry, NULL, &PB1_BUTTON_PUSHED_Thread_Attr);

  PB2_BUTTON_PUSHED_Thread_Sem = osSemaphoreNew(1, 0, &PB2_BUTTON_PUSHED_Thread_Sem_Attr);
  PB2_BUTTON_PUSHED_Thread = osThreadNew(APP_BLE_Key_Button2_Action_Entry, NULL, &PB2_BUTTON_PUSHED_Thread_Attr);

  PB3_BUTTON_PUSHED_Thread_Sem = osSemaphoreNew(1, 0, &PB3_BUTTON_PUSHED_Thread_Sem_Attr);
  PB3_BUTTON_PUSHED_Thread = osThreadNew(APP_BLE_Key_Button3_Action_Entry, NULL, &PB3_BUTTON_PUSHED_Thread_Attr);
#endif
/* USER CODE END APPE_Init_2 */
   APP_DEBUG_SIGNAL_RESET(APP_APPE_INIT);
   return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static void Config_HSE(void)
{
  OTP_Data_s* otp_ptr = NULL;

  /**
   * Read HSE_Tuning from OTP
   */
  if (OTP_Read(DEFAULT_OTP_IDX, &otp_ptr) != HAL_OK) {
    /* OTP no present in flash, apply default gain */
    HAL_RCCEx_HSESetTrimming(0x0C);
  }
  else
  {
    HAL_RCCEx_HSESetTrimming(otp_ptr->hsetune);
  }
}

static void System_Init( void )
{
  /* Clear RCC RESET flag */
  LL_RCC_ClearResetFlags();

  UTIL_TIMER_Init();
  /* Enable wakeup out of standby from RTC ( UTIL_TIMER )*/
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);

#if (CFG_DEBUG_APP_TRACE != 0)
  /*Initialize the terminal using the USART2 */
  UTIL_ADV_TRACE_Init();
  UTIL_ADV_TRACE_SetVerboseLevel(VLEVEL_L); /*!< functional traces*/
  UTIL_ADV_TRACE_SetRegion(~0x0);
#endif

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  adc_ctrl_init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

  return;
}

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config(void)
{
  scm_init();

 /* Initialize low power manager */
  UTIL_LPM_Init();

#if (CFG_LPM_STDBY_SUPPORTED == 1)
  /* Enable SRAM1, SRAM2 and RADIO retention*/
  LL_PWR_SetSRAM1SBRetention(LL_PWR_SRAM1_SB_FULL_RETENTION);
  LL_PWR_SetSRAM2SBRetention(LL_PWR_SRAM2_SB_FULL_RETENTION);
  LL_PWR_SetRadioSBRetention(LL_PWR_RADIO_SB_FULL_RETENTION); /* Retain sleep timer configuration */
#else
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
#endif
}

static void AMM_WrapperInit (uint32_t * const p_PoolAddr, const uint32_t PoolSize)
{
  UTIL_MM_Init ((uint8_t *)p_PoolAddr, ((size_t)PoolSize * sizeof(uint32_t)));
}

static uint32_t * AMM_WrapperAllocate (const uint32_t BufferSize)
{
  return (uint32_t *)UTIL_MM_GetBuffer (((size_t)BufferSize * sizeof(uint32_t)));
}

static void AMM_WrapperFree (uint32_t * const p_BufferAddr)
{
  UTIL_MM_ReleaseBuffer ((void *)p_BufferAddr);
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */
#if (CFG_LED_SUPPORTED == 1)
static void Led_Init( void )
{
  /* Leds Initialization */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  BSP_LED_On(LED_GREEN);

  return;
}
#endif

#if (CFG_BUTTON_SUPPORTED == 1)
static void Button_Init( void )
{
  /* Button Initialization */
  BSP_PB_Init(B1, BUTTON_MODE_EXTI);
  BSP_PB_Init(B2, BUTTON_MODE_EXTI);
  BSP_PB_Init(B3, BUTTON_MODE_EXTI);

  return;
}
#endif
/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

/* FREERTOS_MARK */
void FreeRTOSLowPowerUserEnter( void )
{

  LL_PWR_ClearFlag_STOP();
  /* FREERTOS_MARK */
  /*if(system_startup_done)
  {
    APP_SYS_BLE_EnterDeepSleep();
  }
  */
  LL_RCC_ClearResetFlags();

  /* Wait until System clock is not on HSI */
  while (LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
  
  HAL_SuspendTick();
  
  UTIL_LPM_EnterLowPower();
  
  return;
}

void FreeRTOSLowPowerUserExit( void )
{
  HAL_ResumeTick();
  LL_AHB5_GRP1_EnableClock(LL_AHB5_GRP1_PERIPH_RADIO);
  ll_sys_dp_slp_exit();

  return;
}

void vApplicationIdleHook( void )
{
#if (CFG_LPM_SUPPORTED == 1)
  FreeRTOSLowPowerUserEnter();
  FreeRTOSLowPowerUserExit();
#endif // CFG_LPM_SUPPORTED
}

void BPKACB_Process( void )
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(BPKA_Thread_Sem);
}

void HWCB_RNG_Process( void )
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(HW_RNG_Thread_Sem);
}

  /* FREERTOS_MARK */
void BPKA_BG_Process_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(BPKA_Thread_Sem , osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    BPKA_BG_Process();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

void HW_RNG_Process_Entry(void* thread_input)
{
  (void)(thread_input);

  while(1)
  {
    osSemaphoreAcquire(HW_RNG_Thread_Sem , osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    HW_RNG_Process();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

void AMM_BackgroundProcess_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(AMM_BCKGND_Thread_Sem , osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    AMM_BackgroundProcess();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

void FM_BackgroundProcess_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(FLASH_MANAGER_BCKGND_Thread_Sem, osWaitForever);
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    FM_BackgroundProcess();
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}

void AMM_RegisterBasicMemoryManager (AMM_BasicMemoryManagerFunctions_t * const p_BasicMemoryManagerFunctions)
{
  /* Fulfill the function handle */
  p_BasicMemoryManagerFunctions->Init = AMM_WrapperInit;
  p_BasicMemoryManagerFunctions->Allocate = AMM_WrapperAllocate;
  p_BasicMemoryManagerFunctions->Free = AMM_WrapperFree;
}

void AMM_ProcessRequest (void)
{
  /* Ask for AMM background task scheduling */
  /* FREERTOS_MARK */
  osSemaphoreRelease(AMM_BCKGND_Thread_Sem);
}

void FM_ProcessRequest (void)
{
  /* Schedule the background process */
  osSemaphoreRelease(FLASH_MANAGER_BCKGND_Thread_Sem);
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */
void BSP_PB_Callback(Button_TypeDef Button)
{
  switch (Button)
  {
    case B1:
      /* FREERTOS_MARK */
      osSemaphoreRelease(PB1_BUTTON_PUSHED_Thread_Sem);
      break;

    case B2:
      /* FREERTOS_MARK */
      osSemaphoreRelease(PB2_BUTTON_PUSHED_Thread_Sem);
      break;

    case B3:
      /* FREERTOS_MARK */
      osSemaphoreRelease(PB3_BUTTON_PUSHED_Thread_Sem);
      break;
    default:
      break;
  }

  return;
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == B1_PIN)
  {
    BSP_PB_Callback(B1);
  }
  if (GPIO_Pin == B2_PIN)
  {
    BSP_PB_Callback(B2);
  }
  if (GPIO_Pin == B3_PIN)
  {
    BSP_PB_Callback(B3);
  }

  return;
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  HAL_GPIO_EXTI_Rising_Callback(GPIO_Pin);
}

static void RxUART_Init(void)
{
  UART_StartRx(RxCpltCallback);
}

static void RxCpltCallback(uint8_t *pdata, uint16_t size, uint8_t error)
{
  /* Filling buffer and wait for '\r' char */
  if (indexReceiveChar < C_SIZE_CMD_STRING)
  {
    if (charRx == '\r')
    {
      APP_DBG_MSG("received %s\n", CommandString);

      UartCmdExecute();

      /* Clear receive buffer and character counter*/
      indexReceiveChar = 0;
      memset(CommandString, 0, C_SIZE_CMD_STRING);
    }
    else
    {
      CommandString[indexReceiveChar++] = charRx;
    }
  }

  /* Once a character has been sent, put back the device in reception mode */
  UART_StartRx(RxCpltCallback);
}

static void UartCmdExecute(void)
{
  /* Parse received CommandString */
  if(strcmp((char const*)CommandString, "SW1") == 0)
  {
    APP_DBG_MSG("SW1 OK\n");
    exti_handle.Line = B1_EXTI_LINE;
    HAL_EXTI_GenerateSWI(&exti_handle);
  }
  else if (strcmp((char const*)CommandString, "SW2") == 0)
  {
    APP_DBG_MSG("SW2 OK\n");
    exti_handle.Line = B2_EXTI_LINE;
    HAL_EXTI_GenerateSWI(&exti_handle);
  }
  else if (strcmp((char const*)CommandString, "SW3") == 0)
  {
    APP_DBG_MSG("SW3 OK\n");
    exti_handle.Line = B3_EXTI_LINE;
    HAL_EXTI_GenerateSWI(&exti_handle);
  }
  else
  {
    APP_DBG_MSG("NOT RECOGNIZED COMMAND : %s\n", CommandString);
  }
}

#if (CFG_DEBUG_APP_TRACE != 0)
void RNG_KERNEL_CLK_OFF(void)
{
  /* Do not switch off HSI clock as it is used for traces */
}
#endif
/* USER CODE END FD_WRAP_FUNCTIONS */
