#include "PerfCounters.hpp"

#include "STM32FreeRTOS.h"
#include "task.h"

#define CLOCKS_PER_SEC SystemCoreClock

extern PerfCountersMonitor counters;

clock_t clock(){
    return micros();
}

static void ArchOVHTask(void* args){

    clock_t begin,end;
    
    begin=clock();
    counters.enable();
    end=clock();
    Serial.printf("Enabling DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %f milliseconds\n\n", 1000*(end-begin)/CLOCKS_PER_SEC );

    begin=clock();
    counters.disable();
    end=clock();
    Serial.printf("Disabling DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %f milliseconds\n\n", 1000*(end-begin)/CLOCKS_PER_SEC );

    begin=clock();
    counters.getPCounters();
    end=clock();
    Serial.printf("Reading DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %d milliseconds\n\n", 1000*(end-begin)/CLOCKS_PER_SEC );

    begin=clock();
    counters.updateTotal();
    end=clock();
    Serial.printf("Summing DWT counters to total required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %d milliseconds\n\n", 1000*(end-begin)/CLOCKS_PER_SEC );

    vTaskDelete(NULL);

}