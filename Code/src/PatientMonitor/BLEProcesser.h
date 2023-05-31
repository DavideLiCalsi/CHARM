#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>

#include "constants.h"

#include "STM32FreeRTOS.h"
#include "task.h"

extern TaskHandle_t htask4;

/**
 * @brief This task acts a sort of "interrupt handler" for
 * SPI interrupts. Whenever such IRQ is received, the real
 * interrupt handler simply wakes up this task.
 * 
 * @param args 
 */
static void BLEProcesserTask(void* args){

    while(1){
        
        // The task stays suspended
        vTaskSuspend(NULL);

        // When resumed, it calls the BlueNRG ISR
        counters.BeginMonitored();
        HCI_Isr();
        counters.EndMonitored();

        // Finally, it resumes the requests handler
        vTaskResume(htask4);
        
    }

    vTaskDelete(NULL);
}