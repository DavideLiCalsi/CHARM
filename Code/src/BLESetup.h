#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>
#include <bluenrg_gap.h>
#include <bluenrg_interface.h>

#include "constants.h"

/**
 * @brief Auxiliar defintions to handle the UUIDs,
 * for both BLE services and characteristics.
 * 
 */

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_DATA_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x42,0x82,0x1a,0x40, 0xe4,0x77, 0x11,0xe2, 0x82,0xd0, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)

#define COPY_ACC_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x43,0xfb,0x6e,0xbf,0x1e,0x64,0x45,0x94,0xa1,0x30,0x6b,0x14,0x76,0x4b,0xd3,0x99)
#define COPY_ANG_SPEED_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x41,0x82,0x1a,0x40, 0xe4,0x77, 0x11,0xe2, 0x82,0xd0, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_TEMPERATURE_CHAR_UUID(uuid_struct)         COPY_UUID_128(uuid_struct,0xa3,0x2e,0x55,0x20, 0xe4,0x77, 0x11,0xe2, 0xa9,0xe3, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_HUMIDITY_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x01,0xc5,0x0b,0x60, 0xe4,0x8c, 0x11,0xe2, 0xa0,0x73, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)


extern PerfCountersMonitor counters;
extern bool discoverable;

// Configure BTLE_SPI
static SPIClass BTLE_SPI(PIN_BLE_SPI_MOSI, PIN_BLE_SPI_MISO, PIN_BLE_SPI_SCK);

// Configure BTLE pins
static SPBTLERFClass BTLE(&BTLE_SPI, PIN_BLE_SPI_nCS, PIN_BLE_SPI_IRQ, PIN_BLE_SPI_RESET, PIN_BLE_LED);

/* 
    Server name, should be at most 7 characters. SonGoku is the main
    character of the epic Chinese book "Journey to West"
*/
const char *name = "SonGoku"; 

// Server address
uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};

char devName[8];
uint8_t uuid[16];

// Handles to services and characteristics
uint16_t service_handle, dev_name_char_handle, appearance_char_handle, 
dataServiceHandle, temperatureCharHandle, humidityCharHandle, accCharHandle, angSpeedCharHandle;

/**
 * @brief Callback used by the BlueNRG library to manage
 * requests and events. It is called by the HCI layer.
 * 
 * @param pckt The received packet
 */
void DataSender_CB(void* pckt){

  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  /* obtain event packet */
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

  if(hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch(event_pckt->evt){

    /*
        Set the device in discoverable mode, so that further connections are available.
        Update the counter that measures BLE disconnections.
    */
    case EVT_DISCONN_COMPLETE:
        discoverable=true;
        PRINTF("Disconnection\n");
        counters.updateECounter(1, ECOUNTER_BLE_DISCONNECTION);
    break;

    case EVT_LE_META_EVENT:
        {
            evt_le_meta_event *evt = (evt_le_meta_event *)event_pckt->data;

            switch(evt->subevent){
                
                /*
                    Update the counter that measures connections.
                */
                case EVT_LE_CONN_COMPLETE:
                {
                    PRINTF("Connection\n");
                    counters.updateECounter(1,ECOUNTER_BLE_CONNECTION);
                }

            break;
            }
        }
    break;

    case EVT_VENDOR:

    evt_blue_aci* blue_evt = (evt_blue_aci*)event_pckt->data;
    evt_gatt_read_permit_req* pr;

    switch (blue_evt->ecode)
    {   
        /*
            Handle the BLE read requests, grant reading.
            Update the counter that measures read requests.
        */
        case EVT_BLUE_GATT_READ_PERMIT_REQ:
            PRINTF("Read request\n");
            pr = (evt_gatt_read_permit_req*) blue_evt->data;

            counters.disable();
            counters.enable();
            aci_gatt_allow_read(pr->conn_handle);

            counters.getPCounters();
            counters.updateECounter(1,ECOUNTER_BLE_READ);
            counters.updateTotal();
            break;
        
    }
    break;
    
    }
}
