/**
 * @file LegitActivity.h
 * @author Davide Li Calsi
 * @brief 
 * @version 0.1
 * @date 2022-04-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>

#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>
#include <bluenrg_gap.h>
#include <bluenrg_interface.h>

#include "constants.h"
#include "BLESetup.h"
#if USE_FREERTOS == 1
#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#endif

extern SemaphoreHandle_t critical;
extern PerfCountersMonitor counters;
extern float temperature;

bool discoverable=true, initialized=false;

int res;

void initialize(){

    devName[0]=AD_TYPE_COMPLETE_LOCAL_NAME;

    for(int i=1; i<8;++i){
        devName[i]= name[i-1];
    }


    PRINTF("Starting BTLE\n");

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

    /*
    * Add the data service
    */
    COPY_DATA_SERVICE_UUID(uuid);
    res = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &dataServiceHandle);
    
    if(res)
        goto end;

    /*
    * Add the humidity and temperature charachteristics
    */
    COPY_TEMPERATURE_CHAR_UUID(uuid);
    res=aci_gatt_add_char(dataServiceHandle, UUID_TYPE_128, uuid, 5,CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &temperatureCharHandle);

    COPY_HUMIDITY_CHAR_UUID(uuid);
    res=aci_gatt_add_char(dataServiceHandle, UUID_TYPE_128, uuid, 5,CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &humidityCharHandle);
    
    /*
    * Add the callback
    */
    attach_HCI_CB(DataSender_CB);

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

        uint8_t int_temperature = (uint8_t) temperature;
        uint8_t int_humidity = (uint8_t) humidity;

        counters.BeginMonitored();
        aci_gatt_update_char_value(dataServiceHandle,temperatureCharHandle,0,2, &int_temperature);
        counters.EndMonitored();

        counters.BeginMonitored();
        aci_gatt_update_char_value(dataServiceHandle,humidityCharHandle, 0, 2, &int_humidity);
        counters.EndMonitored();
        
    }
    
    Serial.println("Failed");
    vTaskDelete(NULL);

}