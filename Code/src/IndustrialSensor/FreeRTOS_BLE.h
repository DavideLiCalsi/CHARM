#include "constants.h"

#include <SPI.h>
#include <SPBTLE_RF.h>
#include <hci.h>
#include <bluenrg_utils.h>
#include <bluenrg_aci.h>

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
static void ProcessBLE(void* args){

    while (1){

        // Keep the task suspended
        vTaskSuspend(NULL);
        
        // Call the ISR from the BlueNRG library
        counters.BeginMonitored();
        HCI_Isr();
        counters.EndMonitored();

        // Resume the task that processes bluetooth requests
        vTaskResume(htask4);

    }


    vTaskDelete(NULL);

    
}