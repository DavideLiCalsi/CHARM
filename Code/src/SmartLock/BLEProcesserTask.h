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

extern TaskHandle_t htask6;
extern PerfCountersMonitor counters;

static void BLEProcesserTask(void* args){

    while(1){
        
        vTaskSuspend(NULL);

        counters.disable();
        counters.enable();
        HCI_Isr();
        counters.getPCounters();
        counters.updateTotal();
        vTaskResume(htask6);
        
    }

    vTaskDelete(NULL);
}