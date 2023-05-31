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

extern TaskHandle_t htask6;
extern PerfCountersMonitor counters;
extern float humidity;
extern HTS221Sensor HumTemp;

float hum_measurements[200];

float getStdDev(int samples_num){

  float mean = 0, squared_sum =0;
  int i;

  for (i=0; i<samples_num;++i){

    counters.BeginMonitored();
    mean+=hum_measurements[i];
    counters.EndMonitored();
  }

  mean /= samples_num;

  for (i=0; i<samples_num;++i){

    counters.BeginMonitored();
    squared_sum += (hum_measurements[i] - mean)*(hum_measurements[i] - mean);
    counters.EndMonitored();
  }

  squared_sum /= samples_num;

  return squared_sum;
}

/*
##########################################
            FAN ACTIVITY
##########################################
*/
//Flag indicating if the fan is on

static void HumSamplerTask(void* arg){
  
  int array_index=0;

  uint32_t begin,end;

  begin = millis();

  while(1){
    
    PRINTF("Sampling humidity\n");
    
    counters.BeginMonitored();

    HumTemp.GetHumidity(&humidity);

    hum_measurements[array_index++] = humidity;
    
    counters.updateECounter(1,WM_ECOUNTER_HUM_SAMPLES);
    counters.EndMonitored();

    if ( millis() - begin >= 30000 ){
      getStdDev(array_index);
      begin = millis();
    }

    vTaskDelay(2*configMAIN_TASK_DELAY/portTICK_PERIOD_MS);
    
  }

  #if USE_FREERTOS == 1
  vTaskDelete(NULL);
  #endif
}
