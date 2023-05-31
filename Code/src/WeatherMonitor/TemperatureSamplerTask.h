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

#include <HTS221Sensor.h>

#if USE_FREERTOS == 1
#include <STM32FreeRTOS.h>
#include <task.h>
#include <queue.h>
#endif

extern TaskHandle_t htask6, htask4;
extern PerfCountersMonitor counters;
extern float temperature;
extern HTS221Sensor HumTemp;

float temp_measurements[200];

/*
##########################################
            FAN ACTIVITY
##########################################
*/
//Flag indicating if the fan is on

static void TempSamplerTask(void* arg){
  
  int array_index=0;
  uint32_t begin,end;

  begin = millis();

  while(1){
    
    PRINTF("Sampling temperature\n");

    counters.BeginMonitored();

    HumTemp.GetTemperature(&temperature);
    counters.updateECounter(1,WM_ECOUNTER_TEMP_SAMPLES);

    counters.EndMonitored();

    temp_measurements[array_index++] = temperature;

    if ( millis() - begin >= 2*60*1000 ){
      
      float average = 0;

      // Compute the 2 minutes average
      for ( int i=0; i<array_index; ++i){

        counters.BeginMonitored();

        average += temp_measurements[i];

        counters.EndMonitored();

      }

      counters.BeginMonitored();
      average /= array_index;
      array_index=0;

      temperature = average;

      begin = millis();
      counters.EndMonitored();

      vTaskResume(htask4);
    }

    vTaskDelay(configMAIN_TASK_DELAY/portTICK_PERIOD_MS);
    
  }

  #if USE_FREERTOS == 1
  vTaskDelete(NULL);
  #endif
}
