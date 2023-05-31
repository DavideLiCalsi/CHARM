#include <Arduino.h>

#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>
#include <bluenrg_gap.h>
#include <bluenrg_interface.h>

#include "constants.h"

#if USE_FREERTOS == 1
#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

extern TaskHandle_t htask4, htask7, htask2;
extern SemaphoreHandle_t critical;
extern PerfCountersMonitor counters;

#define LOCK_HANDLE htask4
#define DOOR_HANDLE htask2
#define PROCESSER_HANDLE htask7

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_LOCK_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x42,0x82,0x1a,0x40, 0xe4,0x77, 0x11,0xe2, 0x82,0xd0, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_LOCKED_CHAR_UUID(uuid_struct)         COPY_UUID_128(uuid_struct,0xa3,0x2e,0x55,0x20, 0xe4,0x77, 0x11,0xe2, 0xa9,0xe3, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)

// Configure BTLE_SPI
static SPIClass BTLE_SPI(PIN_BLE_SPI_MOSI, PIN_BLE_SPI_MISO, PIN_BLE_SPI_SCK);

// Configure BTLE pins
static SPBTLERFClass BTLE(&BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

const char *name = "SonGoku"; //Should be at most 7 characters
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};

bool discoverable=true, initialized=false, wait=false;

int res;

uint32_t notification;

uint8_t hwVersion=0;
uint16_t fwVersion=0, service_handle, dev_name_char_handle, appearance_char_handle, lockServiceHandle, lockedCharHandle;
char devName[8];
uint8_t uuid[16];

void SmartLock_CB(void* pckt){

  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  /* obtain event packet */
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

  if(hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch(event_pckt->evt){

      case EVT_CONN_COMPLETE:
      counters.updateECounter(1,SL_ECOUNTER_BLE_CONNECTION);
      break;

      case EVT_DISCONN_COMPLETE:

      counters.updateECounter(1,SL_ECOUNTER_BLE_DISCONNECTION);
    
      discoverable=true;
      break;

      case EVT_VENDOR:
     {
        evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;
        
        switch(blue_evt->ecode){
            
            case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
            evt_gatt_attr_modified_IDB04A1 *evt = (evt_gatt_attr_modified_IDB04A1*)blue_evt->data;
            char command[6] = "\0\0\0\0\0";
            strncpy(command, (char*) evt->att_data, 5);
            Serial.printf("Attr handle: %d - Data len: %d - Data value: %s\n", evt->attr_handle, evt->data_length, command);
            
            if ( !strcmp(command, "open") || !strcmp(command,"close")){
                wait=true;

            }
            aci_gatt_update_char_value(lockServiceHandle,lockedCharHandle,0,5,command);
            break;
      }

    }
  }
}

void initialize(){

    devName[0]=AD_TYPE_COMPLETE_LOCAL_NAME;

    for(int i=1; i<8;++i){
        devName[i]= name[i-1];
    }


    Serial.println("Start btle");

    BTLE.begin();

    res=aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,CONFIG_DATA_PUBADDR_LEN, SERVER_BDADDR);

    if(res)
        goto end;

    res=aci_gatt_init();
    
    if(res)
        goto end;

    res = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1,&service_handle,&dev_name_char_handle,&appearance_char_handle);

    if ( res )
        goto end;
    
    res=aci_gap_set_discoverable(ADV_IND,0,0,PUBLIC_ADDR, NO_WHITE_LIST_USE,8,devName,0,NULL,0,0);
        if ( res )
            Serial.printf("Failed with code %d\n", res);
    
    discoverable=false;
    
    aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, 7, (uint8_t *)&devName[1]);

    COPY_LOCK_SERVICE_UUID(uuid);
    res = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &lockServiceHandle);
    
    if(res)
        goto end;

    COPY_LOCKED_CHAR_UUID(uuid);
    res=aci_gatt_add_char(lockServiceHandle, UUID_TYPE_128, uuid, 10,CHAR_PROP_WRITE, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &lockedCharHandle);

    attach_HCI_CB(SmartLock_CB);

    res=aci_gap_set_discoverable(ADV_IND,0,0,PUBLIC_ADDR, NO_WHITE_LIST_USE,8,devName,0,NULL,0,0);
        if ( res )
            Serial.printf("Failed with code %d\n", res);

    initialized=true;

    end:
        return;
}

static void BLETask(void* args){

    while(initialized != true)
        initialize();
    
    while (1)
    {   
        vTaskSuspend(NULL);

        if (wait){
            
            counters.disable();
            counters.enable();
            aci_gap_set_non_discoverable();
            counters.getPCounters();
            counters.updateTotal();

            vTaskResume(LOCK_HANDLE);
            vTaskSuspend(NULL);

            Serial.println("Ble resume");

            wait=false;
            discoverable=true;

        }
        
        counters.disable();
        counters.enable();
        HCI_Process();
        counters.getPCounters();
        counters.updateTotal();

        if(discoverable){
            
            do {
                
                counters.disable();
                counters.enable();
                res=aci_gap_set_discoverable(ADV_IND,0,0,PUBLIC_ADDR, NO_WHITE_LIST_USE,8,"SonGoku",0,NULL,0,0);

                if(!res)
                    discoverable=false;

                counters.getPCounters();
                counters.updateTotal();

                //Serial.printf("%d\n",res);

            }while(res == BLE_STATUS_TIMEOUT);
            

        }

    }

    Serial.println("Failed");
    vTaskDelete(NULL);

}