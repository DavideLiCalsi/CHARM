/**
 * @file SamplerTask.h
 * @author Davide Li Calsi
 * @brief This task samples temperature and humidity and wakes up the
 * actuators task.
 * @version 0.1
 * @date 2022-04-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "constants.h"
#include "EndoEvents/EndoEvent.h"

#include <HTS221Sensor.h>

#include <STM32FreeRTOS.h>
#include <task.h>
#include <queue.h>


extern TaskHandle_t htask6;
extern PerfCountersMonitor counters;
extern float temperature, humidity;
extern HTS221Sensor HumTemp;

void ArtificialFault(float* x){

}

/*
##########################################
            SAMPLER TASK
##########################################
*/

/**
 * @brief Sample Temperature and Humidity
 * 
 * @param arg 
 */
static void SamplerTask(void* arg){

  uint32_t begin_hum, begin_temp, next_fault_hum, next_fault_temp;
  int iterations_counter_hum=0, iterations_counter_temp=0;

  next_fault_hum = 1000 * GetEndoEvent(1.2);
  next_fault_temp = 1000 * GetEndoEvent(1.2);
  begin_hum = millis();
  begin_temp = millis();

  while(1){
    
    counters.BeginMonitored();
    HumTemp.GetHumidity(&humidity);
    counters.EndMonitored();

    counters.BeginMonitored();
    HumTemp.GetTemperature(&temperature);
    counters.EndMonitored();

    counters.updateECounter(1,IS_ECOUNTER_TEMP_HUM_SAMPLES);

    if (iterations_counter_hum > 0){
      humidity=0;
      iterations_counter_hum--;
    }

    // Artificially emulate a humidity "fault"
    if (millis() - begin_hum >= next_fault_hum && iterations_counter_hum == 0){
      PRINTF("Hum fault!");
      humidity = 0;
      next_fault_hum = 1000*GetEndoEvent(1/configENDO_FREQUENCY_VALVE);
      iterations_counter_hum = configENDO_KEEP_INTERVAL;
      begin_hum=millis();
    }

    if (iterations_counter_temp > 0){
      temperature=100000;
      iterations_counter_temp--;
    }

    // Artificially emulate a temperature "fault"
    if (millis() - begin_temp >= next_fault_temp){
      PRINTF("Temperature fault!");
      temperature = 100000;
      next_fault_temp = 1000*GetEndoEvent(1/configENDO_FREQUENCY_FAN);
      iterations_counter_temp = configENDO_KEEP_INTERVAL;
      begin_temp=millis();
    }

    vTaskResume(htask6);

    vTaskDelay(configMAIN_TASK_DELAY/portTICK_PERIOD_MS);
    
  }

  vTaskDelete(NULL);

}
