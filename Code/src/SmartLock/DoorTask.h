/**
 * @file DoorTask.h
 * @author Davide Li Calsi
 * @brief Task that handles the phyisical door lock
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Servo.h>
#include "constants.h"

#if USE_FREERTOS == 1
#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#include <Arduino.h>

extern TaskHandle_t htask4, htask6, htask7;
extern SemaphoreHandle_t critical;
extern Actuators actuators;
extern PerfCountersMonitor counters;
extern bool should_open_door;

#define LOCK_HANDLE htask4
#define BLE_HANDLE htask6
#define PROCESSER_HANDLE htask7

static void DoorTask(void* args){

    actuators.attachServo();

    Serial.println("Door Task waits");
    //xTaskNotifyWait(0,0x1, &notification_bits, portMAX_DELAY);

    while (1)
    {
        vTaskSuspend(NULL);

        counters.disable();
        counters.enable();
        if ( should_open_door  ){

            //Serial.println("Open door");
            actuators.handleDoor(true);

            counters.getPCounters();
            counters.updateECounter(1,2);
        }
        else{
            //Serial.println("Close door");
            actuators.handleDoor(false);

            counters.getPCounters();
            counters.updateECounter(1,3);
        }
        
        counters.updateTotal();
        //xTaskNotify(BLE_HANDLE,0x0,eNoAction);
        Serial.println("Resume ble");

    }

    vTaskDelete(NULL);
    
}