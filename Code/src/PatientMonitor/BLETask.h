#include <Arduino.h>
#include <math.h>

#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>
#include <bluenrg_gap.h>
#include <bluenrg_interface.h>

#include "constants.h"
#include "BLESetup.h"

#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

bool discoverable=true, initialized;
int res;

extern int32_t acc[3];
extern int32_t ang_speed[3];

void initialize(){

    devName[0]=AD_TYPE_COMPLETE_LOCAL_NAME;

    for(int i=1; i<8;++i){
        devName[i]= name[i-1];
    }


    PRINTF("Start btle");

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

    COPY_DATA_SERVICE_UUID(uuid);
    res = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &dataServiceHandle);
    
    if(res)
        goto end;

    COPY_ACC_CHAR_UUID(uuid);
    res=aci_gatt_add_char(dataServiceHandle, UUID_TYPE_128, uuid,8,CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 16, 0, &accCharHandle);

    COPY_ANG_SPEED_CHAR_UUID(uuid);
    res=aci_gatt_add_char(dataServiceHandle, UUID_TYPE_128, uuid,8,CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 16, 0, &angSpeedCharHandle);

    attach_HCI_CB(DataSender_CB);

    initialized=true;

    end:
        return;
}

static void BLETask(void* args){

    int old_val=LOW;

    while(initialized != true)
        initialize();
    
    while (1)
    {   
        vTaskSuspend(NULL);
        
        counters.BeginMonitored();
        HCI_Process();
        counters.EndMonitored();

        if(discoverable){

            counters.BeginMonitored();
            res=aci_gap_set_discoverable(ADV_IND,0,0,PUBLIC_ADDR, NO_WHITE_LIST_USE,8,"SonGoku",0,NULL,0,0);
            counters.EndMonitored();

            if(!res)
                discoverable=false;

        }

        counters.BeginMonitored();
        aci_gatt_update_char_value(service_handle,accCharHandle,0,4,acc);
        counters.EndMonitored();

        counters.BeginMonitored();
        aci_gatt_update_char_value(service_handle,angSpeedCharHandle,0,4,ang_speed);
        counters.EndMonitored();
    }
    

    Serial.println("Failed");
    vTaskDelete(NULL);

}