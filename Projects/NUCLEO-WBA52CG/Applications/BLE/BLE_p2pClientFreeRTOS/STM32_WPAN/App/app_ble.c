/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_ble.c
  * @author  MCD Application Team
  * @brief   BLE Application
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
#include "main.h"
#include "app_common.h"
#include "ble.h"
#include "app_ble.h"
#include "host_stack_sequencer.h"
/* FREERTOS_MARK */

#include "otp.h"
#include "stm32_timer.h"
#include "stm_list.h"
#include "host_ble.h"
#include "nvm.h"
#include "advanced_memory_manager.h"
#include "blestack.h"
#include "simple_nvm_arbiter.h"
#include "gatt_client_app.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wbaxx_nucleo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/**
 * Security parameters structure
 */
typedef struct
{
  /**
   * IO capability of the device
   */
  uint8_t ioCapability;

  /**
   * Authentication requirement of the device
   * Man In the Middle protection required?
   */
  uint8_t mitm_mode;

  /**
   * bonding mode of the device
   */
  uint8_t bonding_mode;

  /**
   * this variable indicates whether to use a fixed pin
   * during the pairing process or a passkey has to be
   * requested to the application during the pairing process
   * 0 implies use fixed pin and 1 implies request for passkey
   */
  uint8_t Use_Fixed_Pin;

  /**
   * minimum encryption key size requirement
   */
  uint8_t encryptionKeySizeMin;

  /**
   * maximum encryption key size requirement
   */
  uint8_t encryptionKeySizeMax;

  /**
   * fixed pin to be used in the pairing process if
   * Use_Fixed_Pin is set to 1
   */
  uint32_t Fixed_Pin;

  /**
   * this flag indicates whether the host has to initiate
   * the security, wait for pairing or does not have any security
   * requirements.
   * 0x00 : no security required
   * 0x01 : host should initiate security by sending the slave security
   *        request command
   * 0x02 : host need not send the clave security request but it
   * has to wait for paiirng to complete before doing any other
   * processing
   */
  uint8_t initiateSecurity;
  /* USER CODE BEGIN tSecurityParams*/

  /* USER CODE END tSecurityParams */
}SecurityParams_t;

/**
 * Global context contains all BLE common variables.
 */
typedef struct
{
  /**
   * security requirements of the host
   */
  SecurityParams_t bleSecurityParam;

  /**
   * gap service handle
   */
  uint16_t gapServiceHandle;

  /**
   * device name characteristic handle
   */
  uint16_t devNameCharHandle;

  /**
   * appearance characteristic handle
   */
  uint16_t appearanceCharHandle;

  /**
   * connection handle of the current active connection
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandle;
  /* USER CODE BEGIN BleGlobalContext_t*/

  /* USER CODE END BleGlobalContext_t */
}BleGlobalContext_t;

typedef struct
{
  BleGlobalContext_t BleApplicationContext_legacy;
  APP_BLE_ConnStatus_t Device_Connection_Status;
  /* USER CODE BEGIN PTD_1*/
  uint8_t deviceServerFound;
  uint8_t a_deviceServerBdAddr[BD_ADDR_SIZE];
  /* USER CODE END PTD_1 */
}BleApplicationContext_t;
/* USER CODE BEGIN PTD */
typedef struct
{
  uint8_t  Identifier;
  uint16_t L2CAP_Length;
  uint16_t Interval_Min;
  uint16_t Interval_Max;
  uint16_t Slave_Latency;
  uint16_t Timeout_Multiplier;
} APP_BLE_p2p_Conn_Update_req_t;
/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
/**
 * GATT buffer size (in bytes)
 */
#define BLE_GATT_BUF_SIZE \
          BLE_TOTAL_BUFFER_SIZE_GATT(CFG_BLE_NUM_GATT_ATTRIBUTES, \
                                     CFG_BLE_NUM_GATT_SERVICES, \
                                     CFG_BLE_ATT_VALUE_ARRAY_SIZE)

#define MBLOCK_COUNT              (BLE_MBLOCKS_CALC(PREP_WRITE_LIST_SIZE, \
                                                    CFG_BLE_MAX_ATT_MTU, \
                                                    CFG_BLE_NUM_LINK) \
                                   + CFG_BLE_MBLOCK_COUNT_MARGIN)

#define BLE_DYN_ALLOC_SIZE \
        (BLE_TOTAL_BUFFER_SIZE(CFG_BLE_NUM_LINK, MBLOCK_COUNT))

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static tListNode BleAsynchEventQueue;
static const uint8_t a_MBdAddr[BD_ADDR_SIZE] =
{
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF)),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40)
};

static uint8_t a_BdAddrUdn[BD_ADDR_SIZE];

/**
 *   Identity root key used to derive LTK and CSRK
 */
static const uint8_t a_BLE_CfgIrValue[16] = CFG_BLE_IRK;

/**
 * Encryption root key used to derive LTK and CSRK
 */
static const uint8_t a_BLE_CfgErValue[16] = CFG_BLE_ERK;
static BleApplicationContext_t bleAppContext;

GATT_CLIENT_APP_ConnHandle_Notif_evt_t clientHandleNotification;

static const char a_GapDeviceName[] = {  'T', 'E', 'M', 'P', 'L', 'A', 'T', 'E' }; /* Gap Device Name */

uint64_t buffer_nvm[CFG_BLEPLAT_NVM_MAX_SIZE] = {0};

/* USER CODE BEGIN PV */
APP_BLE_p2p_Conn_Update_req_t APP_BLE_p2p_Conn_Update_req;
static uint16_t new_ConnHdl;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private function prototypes -----------------------------------------------*/
static void BleStack_Process_BG(void);
static void Ble_UserEvtRx(void);
static void Ble_Hci_Gap_Gatt_Init(void);
static const uint8_t* BleGetBdAddress(void);
static void gap_cmd_resp_wait(void);
static void gap_cmd_resp_release(void);
static void BLE_NvmCallback (SNVMA_Callback_Status_t);
/* FREERTOS_MARK */
static void BleStack_Process_BG_Entry(void* thread_input);
static void Ble_UserEvtRx_Entry(void* thread_input);
static void connection_request_entry(void* thread_input);
/* USER CODE BEGIN PFP */
static uint8_t analyse_adv_report(hci_le_advertising_report_event_rp0 *p_adv_report);
static void Connect_Request(void);
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/
void APP_BLE_Init(void)
{
  /* USER CODE BEGIN APP_BLE_Init_1 */
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  /* USER CODE END APP_BLE_Init_1 */

  LST_init_head(&BleAsynchEventQueue);
  /* FREERTOS_MARK */
  BLE_HOST_Thread_Sem = osSemaphoreNew(1, 0, &BLE_HOST_Thread_Sem_Attr);
  BLE_HOST_Thread = osThreadNew(BleStack_Process_BG_Entry, NULL, &BLE_HOST_Thread_Attr);
  
  HCI_ASYNCH_EVT_Thread_Sem = osSemaphoreNew(1, 0, &HCI_ASYNCH_EVT_Thread_Sem_Attr);
  HCI_ASYNCH_EVT_Thread = osThreadNew(Ble_UserEvtRx_Entry, NULL, &HCI_ASYNCH_EVT_Thread_Attr);

  /* NVM emulation in RAM initialization */
  NVM_Init(buffer_nvm, 0, CFG_BLEPLAT_NVM_MAX_SIZE);

  /* First register the APP BLE buffer */
  SNVMA_Register (APP_BLE_NvmBuffer,
                  (uint32_t *)buffer_nvm,
                  (CFG_BLEPLAT_NVM_MAX_SIZE * 2));

  /* Realize a restore */
  SNVMA_Restore (APP_BLE_NvmBuffer);

  /* USER CODE BEGIN APP_BLE_Init_Buffers */

  /* USER CODE END APP_BLE_Init_Buffers */

  /* Check consistency */
  if (NVM_Get (NVM_FIRST, 0xFF, 0, 0, 0) != NVM_EOF)
  {
    NVM_Discard (NVM_ALL);
  }

  /* Initialize the BLE Host */
  if (HOST_BLE_Init() == 0u)
  {
    /**
     * Initialization of HCI & GATT & GAP layer
     */
    Ble_Hci_Gap_Gatt_Init();

    /**
     * Initialization of the BLE Services
     */
    SVCCTL_Init();

    /**
     * Initialize GATT Client Application
     */
    GATT_CLIENT_APP_Init();

    /* USER CODE BEGIN APP_BLE_Init_3 */
    ret = aci_hal_set_radio_activity_mask(0x0020);
    if (ret != BLE_STATUS_SUCCESS)
    {
      APP_DBG_MSG("  Fail   : aci_hal_set_radio_activity_mask command, result: 0x%2X\n", ret);
    }
    else
    {
      APP_DBG_MSG("  Success: aci_hal_set_radio_activity_mask command\n\r");
    }
    
    CONNECTION_REQ_Thread_Sem = osSemaphoreNew(1, 0, &CONNECTION_REQ_Thread_Sem_Attr);
    CONNECTION_REQ_Thread = osThreadNew(connection_request_entry, NULL, &CONNECTION_REQ_Thread_Attr);
  }
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification(void *p_Pckt)
{
  tBleStatus ret = BLE_STATUS_ERROR;
  hci_event_pckt    *p_event_pckt;
  evt_le_meta_event *p_meta_evt;
  evt_blecore_aci   *p_blecore_evt;

  p_event_pckt = (hci_event_pckt*) ((hci_uart_pckt *) p_Pckt)->data;
  UNUSED(ret);
  /* USER CODE BEGIN SVCCTL_App_Notification */

  /* USER CODE END SVCCTL_App_Notification */

  switch (p_event_pckt->evt)
  {
    case HCI_DISCONNECTION_COMPLETE_EVT_CODE:
    {
      hci_disconnection_complete_event_rp0 *p_disconnection_complete_event;
      p_disconnection_complete_event = (hci_disconnection_complete_event_rp0 *) p_event_pckt->data;

      if (p_disconnection_complete_event->Connection_Handle == bleAppContext.BleApplicationContext_legacy.connectionHandle)
      {
        bleAppContext.BleApplicationContext_legacy.connectionHandle = 0;
        bleAppContext.Device_Connection_Status = APP_BLE_IDLE;
        APP_DBG_MSG(">>== HCI_DISCONNECTION_COMPLETE_EVT_CODE\n");
        APP_DBG_MSG("     - Connection Handle:   0x%02X\n     - Reason:    0x%02X\n",
                    p_disconnection_complete_event->Connection_Handle,
                    p_disconnection_complete_event->Reason);

        /* USER CODE BEGIN EVT_DISCONN_COMPLETE_2 */

        /* USER CODE END EVT_DISCONN_COMPLETE_2 */
      }

      gap_cmd_resp_release();

      /* USER CODE BEGIN EVT_DISCONN_COMPLETE_1 */

      /* USER CODE END EVT_DISCONN_COMPLETE_1 */
      break; /* HCI_DISCONNECTION_COMPLETE_EVT_CODE */
    }

    case HCI_LE_META_EVT_CODE:
    {
      p_meta_evt = (evt_le_meta_event*) p_event_pckt->data;
      /* USER CODE BEGIN EVT_LE_META_EVENT */

      /* USER CODE END EVT_LE_META_EVENT */
      switch (p_meta_evt->subevent)
      {
        case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE:
        {
          hci_le_connection_update_complete_event_rp0 *p_conn_update_complete;
          p_conn_update_complete = (hci_le_connection_update_complete_event_rp0 *) p_meta_evt->data;
          APP_DBG_MSG(">>== HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE\n");
          APP_DBG_MSG("     - Connection Interval:   %.2f ms\n     - Connection latency:    %d\n     - Supervision Timeout:   %d ms\n",
                       p_conn_update_complete->Conn_Interval*1.25,
                       p_conn_update_complete->Conn_Latency,
                       p_conn_update_complete->Supervision_Timeout*10);
          UNUSED(p_conn_update_complete);
          /* USER CODE BEGIN EVT_LE_CONN_UPDATE_COMPLETE */

          /* USER CODE END EVT_LE_CONN_UPDATE_COMPLETE */
          break;
        }
        case HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE:
        {
          hci_le_phy_update_complete_event_rp0 *p_le_phy_update_complete;
          p_le_phy_update_complete = (hci_le_phy_update_complete_event_rp0*)p_meta_evt->data;
          UNUSED(p_le_phy_update_complete);

          gap_cmd_resp_release();

          /* USER CODE BEGIN EVT_LE_PHY_UPDATE_COMPLETE */

          /* USER CODE END EVT_LE_PHY_UPDATE_COMPLETE */
          break;
        }
        case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE:
        {
          hci_le_enhanced_connection_complete_event_rp0 *p_enhanced_conn_complete;
          p_enhanced_conn_complete = (hci_le_enhanced_connection_complete_event_rp0 *) p_meta_evt->data;

          APP_DBG_MSG(">>== HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE - Connection handle: 0x%04X\n", p_enhanced_conn_complete->Connection_Handle);
          APP_DBG_MSG("     - Connection established with @:%02x:%02x:%02x:%02x:%02x:%02x\n",
                      p_enhanced_conn_complete->Peer_Address[5],
                      p_enhanced_conn_complete->Peer_Address[4],
                      p_enhanced_conn_complete->Peer_Address[3],
                      p_enhanced_conn_complete->Peer_Address[2],
                      p_enhanced_conn_complete->Peer_Address[1],
                      p_enhanced_conn_complete->Peer_Address[0]);
          APP_DBG_MSG("     - Connection Interval:   %.2f ms\n     - Connection latency:    %d\n     - Supervision Timeout: %d ms\n",
                      p_enhanced_conn_complete->Conn_Interval * 1.25,
                      p_enhanced_conn_complete->Conn_Latency,
                      p_enhanced_conn_complete->Supervision_Timeout * 10
                     );

          if (bleAppContext.Device_Connection_Status == APP_BLE_LP_CONNECTING)
          {
            /* Connection as client */
            bleAppContext.Device_Connection_Status = APP_BLE_CONNECTED_CLIENT;
          }
          else
          {
            /* Connection as server */
            bleAppContext.Device_Connection_Status = APP_BLE_CONNECTED_SERVER;
          }
          bleAppContext.BleApplicationContext_legacy.connectionHandle = p_enhanced_conn_complete->Connection_Handle;

          /* USER CODE BEGIN HCI_EVT_LE_ENHANCED_CONN_COMPLETE */
          if (p_enhanced_conn_complete->Role == HCI_ROLE_CENTRAL)
          {
            new_ConnHdl = p_enhanced_conn_complete->Connection_Handle;
            osSemaphoreRelease(CONNECTION_REQ_Thread_Sem);
          }
          /* USER CODE END HCI_EVT_LE_ENHANCED_CONN_COMPLETE */
          break; /* HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE */
        }
        case HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE:
        {
          hci_le_connection_complete_event_rp0 *p_conn_complete;
          p_conn_complete = (hci_le_connection_complete_event_rp0 *) p_meta_evt->data;

          APP_DBG_MSG(">>== HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE - Connection handle: 0x%04X\n", p_conn_complete->Connection_Handle);
          APP_DBG_MSG("     - Connection established with @:%02x:%02x:%02x:%02x:%02x:%02x\n",
                      p_conn_complete->Peer_Address[5],
                      p_conn_complete->Peer_Address[4],
                      p_conn_complete->Peer_Address[3],
                      p_conn_complete->Peer_Address[2],
                      p_conn_complete->Peer_Address[1],
                      p_conn_complete->Peer_Address[0]);
          APP_DBG_MSG("     - Connection Interval:   %.2f ms\n     - Connection latency:    %d\n     - Supervision Timeout: %d ms\n",
                      p_conn_complete->Conn_Interval * 1.25,
                      p_conn_complete->Conn_Latency,
                      p_conn_complete->Supervision_Timeout * 10
                     );

          if (bleAppContext.Device_Connection_Status == APP_BLE_LP_CONNECTING)
          {
            /* Connection as client */
            bleAppContext.Device_Connection_Status = APP_BLE_CONNECTED_CLIENT;
          }
          else
          {
            /* Connection as server */
            bleAppContext.Device_Connection_Status = APP_BLE_CONNECTED_SERVER;
          }
          bleAppContext.BleApplicationContext_legacy.connectionHandle = p_conn_complete->Connection_Handle;

          GATT_CLIENT_APP_Set_Conn_Handle(0, p_conn_complete->Connection_Handle);

          if (bleAppContext.Device_Connection_Status == APP_BLE_CONNECTED_CLIENT)
          {
            osSemaphoreRelease(GATTC_Discover_Thread_Sem);
          }

          /* USER CODE BEGIN HCI_EVT_LE_CONN_COMPLETE */

          if (p_conn_complete->Role == HCI_ROLE_CENTRAL)
          {
            new_ConnHdl = p_conn_complete->Connection_Handle;
            osSemaphoreRelease(CONNECTION_REQ_Thread_Sem);
          }

          /* USER CODE END HCI_EVT_LE_CONN_COMPLETE */
          break; /* HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE */
        }
        case HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE:
        {
          hci_le_advertising_report_event_rp0 *p_adv_report;
          p_adv_report = (hci_le_advertising_report_event_rp0 *) p_meta_evt->data;
          UNUSED(p_adv_report);
          /* USER CODE BEGIN HCI_EVT_LE_ADVERTISING_REPORT */

          uint8_t found_status;
          found_status = analyse_adv_report(p_adv_report);
          if (found_status != 0)
          {
            APP_BLE_Procedure_Gap_Central(PROC_GAP_CENTRAL_SCAN_TERMINATE);
          }
          /* USER CODE END HCI_EVT_LE_ADVERTISING_REPORT */
          break; /* HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE */
        }
        default:
          /* USER CODE BEGIN SUBEVENT_DEFAULT */

          /* USER CODE END SUBEVENT_DEFAULT */
          break;
      }

      /* USER CODE BEGIN META_EVT */

      /* USER CODE END META_EVT */
    }
      break; /* HCI_LE_META_EVT_CODE */

    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
    {
      p_blecore_evt = (evt_blecore_aci*) p_event_pckt->data;
      /* USER CODE BEGIN EVT_VENDOR */

      /* USER CODE END EVT_VENDOR */
      switch (p_blecore_evt->ecode)
      {
        /* USER CODE BEGIN ecode */

        /* USER CODE END ecode */
        case ACI_L2CAP_CONNECTION_UPDATE_REQ_VSEVT_CODE:
        {
          aci_l2cap_connection_update_req_event_rp0 *p_l2cap_conn_update_req;
          uint8_t req_resp = 0x01;
          p_l2cap_conn_update_req = (aci_l2cap_connection_update_req_event_rp0 *) p_blecore_evt->data;
          UNUSED(p_l2cap_conn_update_req);

          /* USER CODE BEGIN EVT_L2CAP_CONNECTION_UPDATE_REQ */

          /* USER CODE END EVT_L2CAP_CONNECTION_UPDATE_REQ */

          ret = aci_l2cap_connection_parameter_update_resp(p_l2cap_conn_update_req->Connection_Handle,
                                                           p_l2cap_conn_update_req->Interval_Min,
                                                           p_l2cap_conn_update_req->Interval_Max,
                                                           p_l2cap_conn_update_req->Slave_Latency,
                                                           p_l2cap_conn_update_req->Timeout_Multiplier,
                                                           CONN_CE_LENGTH_MS(10),
                                                           CONN_CE_LENGTH_MS(10),
                                                           p_l2cap_conn_update_req->Identifier,
                                                           req_resp);
          if(ret != BLE_STATUS_SUCCESS)
          {
            APP_DBG_MSG("  Fail   : aci_l2cap_connection_parameter_update_resp command\n");
          }
          else
          {
            APP_DBG_MSG("  Success: aci_l2cap_connection_parameter_update_resp command\n");
          }

          break;
        }
        case ACI_GAP_PROC_COMPLETE_VSEVT_CODE:
        {
          APP_DBG_MSG(">>== ACI_GAP_PROC_COMPLETE_VSEVT_CODE\n");
          aci_gap_proc_complete_event_rp0 *p_gap_proc_complete;
          p_gap_proc_complete = (aci_gap_proc_complete_event_rp0*) p_blecore_evt->data;
          UNUSED(p_gap_proc_complete);
          /* USER CODE BEGIN EVT_GAP_PROCEDURE_COMPLETE */
          if ((p_gap_proc_complete->Procedure_Code == GAP_GENERAL_DISCOVERY_PROC) &&
              (p_gap_proc_complete->Status == 0x00))
          {
            APP_DBG_MSG("-- GAP_GENERAL_DISCOVERY_PROC completed\n");
            osThreadFlagsSet( CONNECTION_REQ_Thread, 1 );
          }
          /* USER CODE END EVT_GAP_PROCEDURE_COMPLETE */
          break; /* ACI_GAP_PROC_COMPLETE_VSEVT_CODE */
        }
        case ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE:
          /* USER CODE BEGIN RADIO_ACTIVITY_EVENT*/
          BSP_LED_Toggle(LED_GREEN);
          /* USER CODE END RADIO_ACTIVITY_EVENT*/
          break; /* ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE */
        /* USER CODE BEGIN EVT_VENDOR_1 */

        /* USER CODE END EVT_VENDOR_1 */
      break; /* HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE */
      }
    }
      /* USER CODE BEGIN EVENT_PCKT */

      /* USER CODE END EVENT_PCKT */

    default:
      /* USER CODE BEGIN ECODE_DEFAULT*/

      /* USER CODE END ECODE_DEFAULT*/
      break;
  }

  return (SVCCTL_UserEvtFlowEnable);
}

APP_BLE_ConnStatus_t APP_BLE_Get_Client_Connection_Status(uint16_t Connection_Handle)
{
  APP_BLE_ConnStatus_t conn_status;

  if (bleAppContext.BleApplicationContext_legacy.connectionHandle == Connection_Handle)
  {
    conn_status = bleAppContext.Device_Connection_Status;
  }
  else
  {
    conn_status = APP_BLE_IDLE;
  }

  return conn_status;
}

void APP_BLE_Procedure_Gap_General(ProcGapGeneralId_t ProcGapGeneralId)
{
  tBleStatus status;
  uint8_t phy_tx, phy_rx;

  switch(ProcGapGeneralId)
  {
    case PROC_GAP_GEN_PHY_TOGGLE:
    {
      /* FREERTOS_MARK */
      osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
      status = hci_le_read_phy(bleAppContext.BleApplicationContext_legacy.connectionHandle, &phy_tx, &phy_rx);
      /* FREERTOS_MARK */
      osMutexRelease(LINK_LAYER_Thread_Mutex);
      if (status != BLE_STATUS_SUCCESS)
      {
        APP_DBG_MSG("hci_le_read_phy failure: reason=0x%02X\n",status);
      }
      else
      {
        APP_DBG_MSG("==>> hci_le_read_phy - Success\n");
        APP_DBG_MSG("==>> PHY Param  TX= %d, RX= %d\n", phy_tx, phy_rx);
        if ((phy_tx == HCI_TX_PHY_LE_2M) && (phy_rx == HCI_RX_PHY_LE_2M))
        {
          APP_DBG_MSG("==>> hci_le_set_phy PHY Param  TX= %d, RX= %d - ", HCI_TX_PHY_LE_1M, HCI_RX_PHY_LE_1M);
          status = hci_le_set_phy(bleAppContext.BleApplicationContext_legacy.connectionHandle, 0, HCI_TX_PHYS_LE_1M_PREF, HCI_RX_PHYS_LE_1M_PREF, 0);
          if (status != BLE_STATUS_SUCCESS)
          {
            APP_DBG_MSG("Fail\n");
          }
          else
          {
            APP_DBG_MSG("Success\n");
            gap_cmd_resp_wait();/* waiting for HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE */
          }
        }
        else
        {
          APP_DBG_MSG("==>> hci_le_set_phy PHY Param  TX= %d, RX= %d - ", HCI_TX_PHYS_LE_2M_PREF, HCI_RX_PHYS_LE_2M_PREF);
          status = hci_le_set_phy(bleAppContext.BleApplicationContext_legacy.connectionHandle, 0, HCI_TX_PHYS_LE_2M_PREF, HCI_RX_PHYS_LE_2M_PREF, 0);
          if (status != BLE_STATUS_SUCCESS)
          {
            APP_DBG_MSG("Fail\n");
          }
          else
          {
            APP_DBG_MSG("Success\n");
            gap_cmd_resp_wait();/* waiting for HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE */
          }
        }
      }
      break;
    }/* PROC_GAP_GEN_PHY_TOGGLE */
    case PROC_GAP_GEN_CONN_TERMINATE:
    {
      status = aci_gap_terminate(bleAppContext.BleApplicationContext_legacy.connectionHandle, 0x13);
      if (status != BLE_STATUS_SUCCESS)
      {
         APP_DBG_MSG("aci_gap_terminate failure: reason=0x%02X\n", status);
      }
      else
      {
        APP_DBG_MSG("==>> aci_gap_terminate : Success\n");
      }
      gap_cmd_resp_wait();/* waiting for HCI_DISCONNECTION_COMPLETE_EVT_CODE */
      break;
    }/* PROC_GAP_GEN_CONN_TERMINATE */
    case PROC_GATT_EXCHANGE_CONFIG:
    {
      status = aci_gatt_exchange_config(bleAppContext.BleApplicationContext_legacy.connectionHandle);
      if (status != BLE_STATUS_SUCCESS)
      {
        APP_DBG_MSG("aci_gatt_exchange_config failure: reason=0x%02X\n", status);
      }
      else
      {
        APP_DBG_MSG("==>> aci_gatt_exchange_config : Success\n");
      }
      break;
    }
    /* USER CODE BEGIN GAP_GENERAL */

    /* USER CODE END GAP_GENERAL */
    default:
      break;
  }
  return;
}

void APP_BLE_Procedure_Gap_Central(ProcGapCentralId_t ProcGapCentralId)
{
  tBleStatus status;
  uint32_t paramA, paramB, paramC, paramD;

  UNUSED(paramA);
  UNUSED(paramB);
  UNUSED(paramC);
  UNUSED(paramD);

  /* First set parameters before calling ACI APIs, only if needed */
  switch(ProcGapCentralId)
  {
    case PROC_GAP_CENTRAL_SCAN_START:
    {
      paramA = SCAN_INT_MS(500);
      paramB = SCAN_WIN_MS(500);
      paramC = APP_BLE_SCANNING;

      break;
    }/* PROC_GAP_CENTRAL_SCAN_START */
    case PROC_GAP_CENTRAL_SCAN_TERMINATE:
    {
      paramA = 1;
      paramB = 1;
      paramC = APP_BLE_IDLE;

      break;
    }/* PROC_GAP_CENTRAL_SCAN_TERMINATE */
    default:
      break;
  }

  /* Call ACI APIs */
  switch(ProcGapCentralId)
  {
    case PROC_GAP_CENTRAL_SCAN_START:
    {
      status = aci_gap_start_general_discovery_proc(paramA, paramB, GAP_PUBLIC_ADDR, 0);

      if (status != BLE_STATUS_SUCCESS)
      {
        APP_DBG_MSG("aci_gap_start_general_discovery_proc - fail, result: 0x%02X\n", status);
      }
      else
      {
        bleAppContext.Device_Connection_Status = (APP_BLE_ConnStatus_t)paramC;
        APP_DBG_MSG("==>> aci_gap_start_general_discovery_proc - Success\n");
      }

      break;
    }/* PROC_GAP_CENTRAL_SCAN_START */
    case PROC_GAP_CENTRAL_SCAN_TERMINATE:
    {
      status = aci_gap_terminate_gap_proc(GAP_GENERAL_DISCOVERY_PROC);
      if (status != BLE_STATUS_SUCCESS)
      {
        APP_DBG_MSG("aci_gap_terminate_gap_proc - fail, result: 0x%02X\n",status);
      }
      else
      {
        bleAppContext.Device_Connection_Status = (APP_BLE_ConnStatus_t)paramC;
        APP_DBG_MSG("==>> aci_gap_terminate_gap_proc - Success\n");
      }
      break;
    }/* PROC_GAP_CENTRAL_SCAN_TERMINATE */

    default:
      break;
  }
  return;
}

/* USER CODE BEGIN FD*/
void APP_BLE_Key_Button1_Action(void)
{
  if (bleAppContext.Device_Connection_Status != APP_BLE_CONNECTED_CLIENT)
  {
    APP_BLE_Procedure_Gap_Central(PROC_GAP_CENTRAL_SCAN_START);
  }
  else
  {
    P2Pclient_write_char();
  }

  return;
}

void APP_BLE_Key_Button2_Action(void)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
  
  if (bleAppContext.Device_Connection_Status != APP_BLE_CONNECTED_CLIENT)
  {
    ret = aci_gap_clear_security_db();
    if (ret != BLE_STATUS_SUCCESS)
    {
      APP_DBG_MSG("==>> aci_gap_clear_security_db - Fail, result: 0x%02X \n", ret);
    }
    else
    {
      APP_DBG_MSG("==>> aci_gap_clear_security_db - Success\n");
    }
  }
  else
  {

  }
  
  return;
}

void APP_BLE_Key_Button3_Action(void)
{
  return;
}
/* FREERTOS_MARK */
void APP_BLE_Key_Button1_Action_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(PB1_BUTTON_PUSHED_Thread_Sem, osWaitForever);
    APP_BLE_Key_Button1_Action();
  }
}

void APP_BLE_Key_Button2_Action_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(PB2_BUTTON_PUSHED_Thread_Sem, osWaitForever);
    APP_BLE_Key_Button2_Action();
  }
}

void APP_BLE_Key_Button3_Action_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(PB3_BUTTON_PUSHED_Thread_Sem, osWaitForever);
    APP_BLE_Key_Button3_Action();
  }
}

/* USER CODE END FD*/

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static void Ble_Hci_Gap_Gatt_Init(void)
{
  uint8_t role;
  uint16_t gap_service_handle, gap_dev_name_char_handle, gap_appearance_char_handle;
  const uint8_t *p_bd_addr;

  uint16_t a_appearance[1] = {BLE_CFG_GAP_APPEARANCE};
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  /* USER CODE BEGIN Ble_Hci_Gap_Gatt_Init*/

  /* USER CODE END Ble_Hci_Gap_Gatt_Init*/

  APP_DBG_MSG("==>> Start Ble_Hci_Gap_Gatt_Init function\n");

  /**
   * Initialize HCI layer
   */
  /*HCI Reset to synchronise BLE Stack*/
  ret = hci_reset();
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : hci_reset command, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: hci_reset command\n");
  }
  /**
   * Write the BD Address
   */
  p_bd_addr = BleGetBdAddress();
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  CONFIG_DATA_PUBADDR_LEN,
                                  (uint8_t*) p_bd_addr);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_PUBADDR_OFFSET, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_hal_write_config_data command - CONFIG_DATA_PUBADDR_OFFSET\n");
    APP_DBG_MSG("  Public Bluetooth Address: %02x:%02x:%02x:%02x:%02x:%02x\n",p_bd_addr[5],p_bd_addr[4],p_bd_addr[3],p_bd_addr[2],p_bd_addr[1],p_bd_addr[0]);
  }

#if (CFG_BLE_ADDRESS_TYPE == GAP_STATIC_RANDOM_ADDR)

  ret = aci_hal_write_config_data(CONFIG_DATA_RANDOM_ADDRESS_OFFSET, CONFIG_DATA_RANDOM_ADDRESS_LEN, (uint8_t*)a_srd_bd_addr);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_RANDOM_ADDRESS_OFFSET, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_hal_write_config_data command - CONFIG_DATA_RANDOM_ADDRESS_OFFSET\n");
    APP_DBG_MSG("  Random Bluetooth Address: %02x:%02x:%02x:%02x:%02x:%02x\n", (uint8_t)(a_srd_bd_addr[1] >> 8),
                                                                               (uint8_t)(a_srd_bd_addr[1]),
                                                                               (uint8_t)(a_srd_bd_addr[0] >> 24),
                                                                               (uint8_t)(a_srd_bd_addr[0] >> 16),
                                                                               (uint8_t)(a_srd_bd_addr[0] >> 8),
                                                                               (uint8_t)(a_srd_bd_addr[0]));
  }
#endif /* CFG_BLE_ADDRESS_TYPE == GAP_STATIC_RANDOM_ADDR */

  /**
   * Write Identity root key used to derive LTK and CSRK
   */
  ret = aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN, (uint8_t*)a_BLE_CfgIrValue);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET\n");
  }

  /**
   * Write Encryption root key used to derive LTK and CSRK
   */
  ret = aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN, (uint8_t*)a_BLE_CfgErValue);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET\n");
  }

  /**
   * Set TX Power.
   */
  ret = aci_hal_set_tx_power_level(1, CFG_TX_POWER);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_hal_set_tx_power_level command, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_hal_set_tx_power_level command\n");
  }

  /**
   * Initialize GATT interface
   */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_gatt_init command, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_gatt_init command\n");
  }

  /**
   * Initialize GAP interface
   */
  role = 0U;
  role |= GAP_CENTRAL_ROLE;

/* USER CODE BEGIN Role_Mngt*/

/* USER CODE END Role_Mngt */

  if (role > 0)
  {
    ret = aci_gap_init(role,
                       PRIVACY_DISABLED,
                       sizeof(a_GapDeviceName),
                       &gap_service_handle,
                       &gap_dev_name_char_handle,
                       &gap_appearance_char_handle);

    if (ret != BLE_STATUS_SUCCESS)
    {
      APP_DBG_MSG("  Fail   : aci_gap_init command, result: 0x%02X\n", ret);
    }
    else
    {
      APP_DBG_MSG("  Success: aci_gap_init command\n");
    }

  }

  ret = aci_gatt_update_char_value(gap_service_handle,
                                   gap_appearance_char_handle,
                                   0,
                                   sizeof(a_appearance),
                                   (uint8_t *)&a_appearance);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_gatt_update_char_value - Appearance, result: 0x%02X\n", ret)
  }
  else
  {
    APP_DBG_MSG("  Success: aci_gatt_update_char_value - Appearance\n");
  }

  /**
   * Initialize IO capability
   */
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability = CFG_IO_CAPABILITY;
  ret = aci_gap_set_io_capability(bleAppContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_gap_set_io_capability command, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_gap_set_io_capability command\n");
  }

  /**
   * Initialize authentication
   */
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode             = CFG_MITM_PROTECTION;
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin  = CFG_ENCRYPTION_KEY_SIZE_MIN;
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax  = CFG_ENCRYPTION_KEY_SIZE_MAX;
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin         = CFG_USED_FIXED_PIN;
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin             = CFG_FIXED_PIN;
  bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode          = CFG_BONDING_MODE;
  /* USER CODE BEGIN Ble_Hci_Gap_Gatt_Init_1*/

  /* USER CODE END Ble_Hci_Gap_Gatt_Init_1*/

  ret = aci_gap_set_authentication_requirement(bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
                                               bleAppContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
                                               CFG_SC_SUPPORT,
                                               CFG_KEYPRESS_NOTIFICATION_SUPPORT,
                                               bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
                                               bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
                                               bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
                                               bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
                                               CFG_BLE_ADDRESS_TYPE);
  if (ret != BLE_STATUS_SUCCESS)
  {
    APP_DBG_MSG("  Fail   : aci_gap_set_authentication_requirement command, result: 0x%02X\n", ret);
  }
  else
  {
    APP_DBG_MSG("  Success: aci_gap_set_authentication_requirement command\n");
  }

  /**
   * Initialize whitelist
   */
  if (bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode)
  {
    ret = aci_gap_configure_whitelist();
    if (ret != BLE_STATUS_SUCCESS)
    {
      APP_DBG_MSG("  Fail   : aci_gap_configure_whitelist command, result: 0x%02X\n", ret);
    }
    else
    {
      APP_DBG_MSG("  Success: aci_gap_configure_whitelist command\n");
    }
  }
  APP_DBG_MSG("==>> End Ble_Hci_Gap_Gatt_Init function\n");

  return;
}

static void Ble_UserEvtRx( void)
{
  SVCCTL_UserEvtFlowStatus_t svctl_return_status;
  BleEvtPacket_t *phcievt;

  LST_remove_head ( &BleAsynchEventQueue, (tListNode **)&phcievt );

  svctl_return_status = SVCCTL_UserEvtRx((void *)&(phcievt->evtserial));

  if (svctl_return_status != SVCCTL_UserEvtFlowDisable)
  {
    AMM_Free((uint32_t *)phcievt);
  }
  else
  {
    LST_insert_head ( &BleAsynchEventQueue, (tListNode *)phcievt );
  }

  if ((LST_is_empty(&BleAsynchEventQueue) == FALSE) && (svctl_return_status != SVCCTL_UserEvtFlowDisable) )
  {
    /* FREERTOS_MARK */
    osSemaphoreRelease(HCI_ASYNCH_EVT_Thread_Sem);
  }

  /* set the BG_BleStack_Process task for scheduling */
  /* FREERTOS_MARK */
  osSemaphoreRelease(BLE_HOST_Thread_Sem);

}

const uint8_t* BleGetBdAddress(void)
{
  OTP_Data_s *p_otp_addr = NULL;
  const uint8_t *p_bd_addr;
  uint32_t udn;
  uint32_t company_id;
  uint32_t device_id;

  udn = LL_FLASH_GetUDN();

  if (udn != 0xFFFFFFFF)
  {
    company_id = LL_FLASH_GetSTCompanyID();
    device_id = LL_FLASH_GetDeviceID();

    /**
     * Public Address with the ST company ID
     * bit[47:24] : 24bits (OUI) equal to the company ID
     * bit[23:16] : Device ID.
     * bit[15:0] : The last 16bits from the UDN
     * Note: In order to use the Public Address in a final product, a dedicated
     * 24bits company ID (OUI) shall be bought.
     */
    a_BdAddrUdn[0] = (uint8_t)(udn & 0x000000FF);
    a_BdAddrUdn[1] = (uint8_t)((udn & 0x0000FF00) >> 8);
    a_BdAddrUdn[2] = (uint8_t)device_id;
    a_BdAddrUdn[3] = (uint8_t)(company_id & 0x000000FF);
    a_BdAddrUdn[4] = (uint8_t)((company_id & 0x0000FF00) >> 8);
    a_BdAddrUdn[5] = (uint8_t)((company_id & 0x00FF0000) >> 16);

    p_bd_addr = (const uint8_t *)a_BdAddrUdn;
  }
  else
  {
    OTP_Read(0, &p_otp_addr);
    if (p_otp_addr)
    {
      p_bd_addr = (uint8_t*)(p_otp_addr->bd_address);
    }
    else
    {
      p_bd_addr = a_MBdAddr;
    }
  }

  return p_bd_addr;
}

static void BleStack_Process_BG(void)
{
  /* FREERTOS_MARK */
  osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
  uint8_t Running = 0x0;
  Running = BleStack_Process( );
  /* FREERTOS_MARK */
  osMutexRelease(LINK_LAYER_Thread_Mutex);
  if (Running == 0x0)
  {
    BleStackCB_Process( );
  }
}

static void gap_cmd_resp_release(void)
{
  /* FREERTOS_MARK */
  osSemaphoreRelease(IDLEEVT_PROC_GAP_COMPLETE_Sem);
  return;
}

static void gap_cmd_resp_wait(void)
{
  /* FREERTOS_MARK */
  osSemaphoreAcquire(IDLEEVT_PROC_GAP_COMPLETE_Sem, osWaitForever);
  return;
}
static void BLE_NvmCallback (SNVMA_Callback_Status_t CbkStatus)
{
  if (CbkStatus != SNVMA_OPERATION_COMPLETE)
  {
    /* Retry the write operation */
    SNVMA_Write (APP_BLE_NvmBuffer,
                 BLE_NvmCallback);
  }
}

/* USER CODE BEGIN FD_LOCAL_FUNCTION */
static uint8_t analyse_adv_report(hci_le_advertising_report_event_rp0 *p_adv_report)
{
  uint8_t adv_type = 0, adv_data_size, found_status, manufBlueST;
  uint8_t *p_adv_data;
  uint16_t i;

  found_status = 0;
  adv_type = p_adv_report->Advertising_Report[0].Event_Type;
  adv_data_size = p_adv_report->Advertising_Report[0].Length_Data;

  if (adv_type == HCI_ADV_EVT_TYPE_ADV_IND)
  {
    uint8_t ad_length, ad_type;
    p_adv_data = (uint8_t*)(&p_adv_report->Advertising_Report[0].Length_Data) + 1;

    i = 0;
    while(i < adv_data_size)
    {
      ad_length = p_adv_data[i];
      ad_type = p_adv_data[i + 1];

      switch (ad_type)
      {
        case AD_TYPE_FLAGS:
          break;
        case AD_TYPE_TX_POWER_LEVEL:
          break;
        case AD_TYPE_MANUFACTURER_SPECIFIC_DATA:
        {
          manufBlueST = UINT8_MAX;
          if ((ad_length >= 7) && 
              (p_adv_data[i + 4] == 0x02) && /* blueST v2*/
              (p_adv_data[i + 6] == 0x83))
          {
            /* p2pServer BlueST v2 detected */
            manufBlueST = 0x02;
          }
          else if ((ad_length >= 7) && 
                   (p_adv_data[i + 2] == 0x01))
          {
            /* p2pServer BlueST v1 detected */
            manufBlueST = 0x01;
          }
          if(manufBlueST != UINT8_MAX)
          {
            bleAppContext.a_deviceServerBdAddr[0] = p_adv_report->Advertising_Report[0].Address[0];
            bleAppContext.a_deviceServerBdAddr[1] = p_adv_report->Advertising_Report[0].Address[1];
            bleAppContext.a_deviceServerBdAddr[2] = p_adv_report->Advertising_Report[0].Address[2];
            bleAppContext.a_deviceServerBdAddr[3] = p_adv_report->Advertising_Report[0].Address[3];
            bleAppContext.a_deviceServerBdAddr[4] = p_adv_report->Advertising_Report[0].Address[4];
            bleAppContext.a_deviceServerBdAddr[5] = p_adv_report->Advertising_Report[0].Address[5];

            bleAppContext.deviceServerFound = 0x01;
            
            APP_DBG_MSG("  P2P server, db addr 0x%02X:%02X:%02X:%02X:%02X:%02X\n",
                            bleAppContext.a_deviceServerBdAddr[5],
                            bleAppContext.a_deviceServerBdAddr[4],
                            bleAppContext.a_deviceServerBdAddr[3],
                            bleAppContext.a_deviceServerBdAddr[2],
                            bleAppContext.a_deviceServerBdAddr[1],
                            bleAppContext.a_deviceServerBdAddr[0]);
            found_status = 1;
          }
          break;
        }
        default:
          break;
        }/* end of switch*/

      i += ad_length + 1; /* increment the iterator to go on next element*/
      /*
      if (found_status != 0)
      {
        break;
      }
      */
    }/* end of while*/
  }

  return found_status;
}

static void Connect_Request(void)
{
  tBleStatus result;

  if (bleAppContext.deviceServerFound != 0)
  {
    APP_DBG_MSG("Create connection to P2Pserver\n");

    result = aci_gap_create_connection(SCAN_INT_MS(500u), SCAN_WIN_MS(500u),
                                       GAP_PUBLIC_ADDR,
                                       &bleAppContext.a_deviceServerBdAddr[0],
                                       GAP_PUBLIC_ADDR,
                                       CONN_INT_MS(50u), CONN_INT_MS(100u),
                                       0u,
                                       CONN_SUP_TIMEOUT_MS(5000u),
                                       CONN_CE_LENGTH_MS(10u), CONN_CE_LENGTH_MS(10u));
    if (result == BLE_STATUS_SUCCESS)
    {
      bleAppContext.Device_Connection_Status = APP_BLE_LP_CONNECTING;
      APP_DBG_MSG("  wait for event HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE\n");
      osSemaphoreAcquire(CONNECTION_REQ_Thread_Sem, osWaitForever);
      if(new_ConnHdl != 0u)
      {
        GATT_CLIENT_APP_Set_Conn_Handle(0, new_ConnHdl);
        new_ConnHdl = 0u;

        APP_DBG_MSG("Discover services, characteristics and descriptors for P2Pserver\n");
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); 
      }
    }
    else
    {
      APP_DBG_MSG("==>> GAP Create connection Failed , result: 0x%02x\n", result);
    }
  }

  return;
}
/* USER CODE END FD_LOCAL_FUNCTION */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

uint8_t BLECB_Indication( const uint8_t* data,
                          uint16_t length,
                          const uint8_t* ext_data,
                          uint16_t ext_length )
{
  BleEvtPacket_t *phcievt;
  uint16_t total_length = (length+ext_length);
  /* Unused parameter */
  (void)ext_data;

  if (data[0] == HCI_EVENT_PKT_TYPE)
  {
    if (AMM_Alloc (CFG_AMM_VIRTUAL_APP_BLE,
                   DIVC((sizeof(BleEvtPacketHeader_t) + total_length), sizeof (uint32_t)),
                   (uint32_t **)&phcievt,
                   NULL) != AMM_ERROR_OK)
    {
      APP_DBG_MSG("Memory pool Empty\n");
      Error_Handler();
    }
    else if (phcievt != (BleEvtPacket_t *)0 )
    {
      phcievt->evtserial.type = HCI_EVENT_PKT_TYPE;
      phcievt->evtserial.evt.evtcode = data[1];
      phcievt->evtserial.evt.plen  = data[2];
      memcpy( (void*)&phcievt->evtserial.evt.payload, &data[3], data[2]);
      LST_insert_tail(&BleAsynchEventQueue, (tListNode *)phcievt);
      /* FREERTOS_MARK */
      osSemaphoreRelease(HCI_ASYNCH_EVT_Thread_Sem);
    }
  }
  else if (data[0] == HCI_ACLDATA_PKT_TYPE)
  {
  }
  return 0u;
}
/* FREERTOS_MARK */
void BleStack_Process_BG_Entry(void* thread_input)
{
  (void)(thread_input);
  
  while(1)
  {
    osSemaphoreAcquire(BLE_HOST_Thread_Sem, osWaitForever);
    BleStack_Process_BG();
  }
}

void Ble_UserEvtRx_Entry(void* thread_input)
{
  (void)(thread_input);

  while(1)
  {
    osSemaphoreAcquire(HCI_ASYNCH_EVT_Thread_Sem, osWaitForever);
    /* FREERTOS_MARK */
    osMutexAcquire(LINK_LAYER_Thread_Mutex, osWaitForever);
    Ble_UserEvtRx();
    /* FREERTOS_MARK */
    osMutexRelease(LINK_LAYER_Thread_Mutex);
  }
}
static void connection_request_entry(void* thread_input)
{
  (void)(thread_input);

  while(1)
  {
    osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
    /* FREERTOS_MARK */
    Connect_Request();
  }
}

void NVMCB_Store( const uint32_t* ptr, uint32_t size )
{
  (void)ptr;
  (void)size;

  /* Call SNVMA for storing - Without callback */
  SNVMA_Write (APP_BLE_NvmBuffer,
               BLE_NvmCallback);
}
/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */