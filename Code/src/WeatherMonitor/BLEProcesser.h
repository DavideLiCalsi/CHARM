#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>

#include "constants.h"

#if USE_FREERTOS == 1
#include "STM32FreeRTOS.h"
#include "task.h"
#endif

extern TaskHandle_t htask4;

static void BLEProcesserTask(void* args){

    while(1){
        
        vTaskSuspend(NULL);
        counters.BeginMonitored();
        HCI_Isr();
        counters.EndMonitored();
        vTaskResume(htask4);
        
    }

    vTaskDelete(NULL);
}