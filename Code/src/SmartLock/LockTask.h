/**
 * @file LockTask.h
 * @author Davide Li Calsi
 * @brief This task emulates the detection of some requests, that can both be valid or invalid.
 * If the requests are valid, wake up the DoorTask to open or close the door.
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "CryptoLock.h"

#if USE_FREERTOS == 1
#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

extern PerfCountersMonitor counters;
extern TaskHandle_t htask2, htask6;
extern SemaphoreHandle_t critical;
extern bool should_open_door;

#define DOOR_HANDLE htask2
#define BLE_HANDLE htask6

static void LockActivity(void* args){

    int rand_index;
    uint32_t notification_val;

    Serial.println("Lock Task waits");
    //xTaskNotifyWait(0x00,0x01,&notification_val,portMAX_DELAY);
    
    Serial.println("Lock Task resumes");
    
    while(1){

        vTaskSuspend(NULL);

        rand_index = random(0,6);
        char name[USER_NAME_SIZE];
        
        strcpy(name, rand_index < 5 ? users[rand_index] : "evil");

        /*This call is not monitored because it emulates 
          the request generation, which is an operation executed
          remotely by the user's device.
        */
        IssueRequest(name,rand_index % 2 == 0);
        
        counters.disable();
        counters.enable();
        int ret = VerifyRequest((byte*) buffer);
        counters.getPCounters();

        if ( ret <= 0 ){
            
            counters.updateECounter(1,1);
            counters.updateTotal();

            if ( !DATA_ONLY )
                Serial.println("Valid request");

            /*Set the 0th notification bit in order to notify that
            the door should be opened*/
            //xTaskNotify(DOOR_HANDLE,0x1,eSetBits);
            should_open_door = ret == LOCK_SUCCESS_OPEN ? true: false;
            vTaskResume(DOOR_HANDLE);

        }
        else{
            
            counters.updateECounter(1,0);
            counters.updateTotal();
            if ( !DATA_ONLY )
                Serial.println("Invalid request");
            
            /* Do not set any bit since the door must not be opened/closed*/
            //xTaskNotify(BLE_HANDLE,0x0,eNoAction);
            //vTaskResume(BLE_HANDLE);
        }

    }

    vTaskDelete(NULL);

    
}