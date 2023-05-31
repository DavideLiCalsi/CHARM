#include "constants.h"

extern PerfCountersMonitor counters;
extern Actuators actuators;
extern float temperature, humidity;

#if USE_FREERTOS == 1
extern SemaphoreHandle_t temp_hum_sem;
#endif

/*
##########################################
            SERVO/VALVE
##########################################
*/



static void ActuatorsTask(void* args){

  // Setup the valve, a.k.a. the Servo
  actuators.attachServo();

  while(1){
    
    // Suspend the task
    vTaskSuspend(NULL);

    // Take the semaphore to access the sensor
    xSemaphoreTake(temp_hum_sem, (TickType_t) 10);

    counters.BeginMonitored();

    // If the temperature exceeds the threshold, start the fan
    if (temperature > TEMP_THRESHOLD)
      actuators.coolDown(true);

    // If the temperature is below the threshold, turn off the fan
    if (temperature <= TEMP_THRESHOLD)
      actuators.coolDown(false);

    // If the humidity is above the threshold, open the valve
    if ( humidity > HUMIDITY_TREHSOLD)
      actuators.openValve(true);
    
    // If the humidity is below the threshold, close the valve
    if ( humidity <= HUMIDITY_TREHSOLD)
      actuators.openValve(false);

    counters.EndMonitored();

    // Release the semaphore
    xSemaphoreGive(temp_hum_sem);

  }

  vTaskDelete(NULL);

}
