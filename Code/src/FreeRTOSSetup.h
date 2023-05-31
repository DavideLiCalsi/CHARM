#include "constants.h"

#include <STM32FreeRTOS.h>

#include <task.h>
#include <semphr.h>

#include "AttestationRoutine.h"
#include "MaliciousInterrupt.h"

#if configUSECASE == USECASE_INDUSTRIALSENSOR
#include "IndustrialSensor/FreeRTOS_BLE.h"
#include "IndustrialSensor/ActuatorsTask.h"
#include "IndustrialSensor/LegitActivity.h"
#include "IndustrialSensor/SamplerTask.h"
#endif

#if configUSECASE == USECASE_SMARTLOCK
#include "SmartLock/LockTask.h"
#include "SmartLock/DoorTask.h"
#include "SmartLock/BLETask.h"
#include "SmartLock/BLEProcesserTask.h"
#endif

#if configUSECASE == USECASE_PATIENTMONITOR
#include "PatientMonitor/DataTask.h"
#include "PatientMonitor/BLETask.h"
#include "PatientMonitor/BLEProcesser.h"
#endif

#if configUSECASE == USECASE_WEATHERMONITOR
#include "WeatherMonitor/BLEProcesser.h"
#include "WeatherMonitor/HumiditySamplerTask.h"
#include "WeatherMonitor/TemperatureSamplerTask.h"
#include "WeatherMonitor/LegitActivity.h"
#endif

#if configUSECASE == USECASE_GENERAL
#include "StimulatorTasks.h"
#endif

#if configUSECASE == USECASE_OVH
//#include "OverheadMeasurements/ArchOVHMeas.h"
#include "OverheadMeasurements/AttestationRoutinePure.h"
#include "OverheadMeasurements/AttestationRoutineMethod.h"
#endif

extern TaskHandle_t htask1, htask2, htask2, htask3, htask4, htask5, htask6, htask7;
extern SemaphoreHandle_t temp_hum_sem, critical;

/**
 * @brief A custom SPI interrupt handler to
 * correctly handle bluetooth requests without
 * messing with FreeRTOS interrupts.
 * 
 */
void SPI_Interrupt_Handler(void){

  BaseType_t checkIfYieldRequired;
  checkIfYieldRequired = xTaskResumeFromISR(htask7);
  portYIELD_FROM_ISR(checkIfYieldRequired);
}

/**
 * @brief Create the FreeRTOS tasks, according to the use-case
 * that we are implementing.
 * 
 * @return int 0 on success, -1 on failure
 */
int setupRTOS(){

  portBASE_TYPE attestation_task,t2,t3,t4,malicious_task,t6,t7;

  /**
   * Application-independent tasks.
   * They are the Malicious task, the Remote Attestation task.
   */
  malicious_task = xTaskCreate(malicious_interrupt, "Roving",configMINIMAL_STACK_SIZE + 6800, NULL, 3,&htask5);
  t3 =pdPASS;// xTaskCreate(idle,"Idle", configMINIMAL_STACK_SIZE,NULL,0,&htask3);
  attestation_task = xTaskCreate(Attest,"Remote_Attestation",configMINIMAL_STACK_SIZE + 3000,NULL,1,&htask1);

  #if configUSECASE == USECASE_INDUSTRIALSENSOR

  #define IS_TASK_SAMPLER_HANDLE htask2
  #define IS_TASK_BLE_HANDLE htask4
  #define IS_TASK_ACTUATORS_HANDLE htask6
  #define IS_TASK_PROCESSBLE_HANDLE htask7

  t2 = xTaskCreate(SamplerTask, "Sampler",configMINIMAL_STACK_SIZE + 5000, NULL, 2,&htask2);
  t4 = xTaskCreate(BLETask, "BLE",configMINIMAL_STACK_SIZE + 15000, NULL, 2,&htask4);
  t6 = xTaskCreate(ActuatorsTask,"Actuators",configMINIMAL_STACK_SIZE + 3000,NULL,1,&htask6);
  t7 = xTaskCreate(ProcessBLE,"BLEProcesser", configMINIMAL_STACK_SIZE + 3000,NULL,3,&htask7);

  #endif

  #if configUSECASE == USECASE_SMARTLOCK

  #define SL_TASK_LOCK_HANDLE htask4
  #define SL_TASK_DOOR_HANDLE htask2
  #define SL_TASK_BLE_HANDLE htask6
  #define SL_TASK_BLEPROCESSER_HANDLE htask7

  t7 = xTaskCreate(BLEProcesserTask, "BLEProcesser", configMINIMAL_STACK_SIZE +5000, NULL, 2, &htask7);
  t4 = xTaskCreate(LockActivity, "Lock", configMINIMAL_STACK_SIZE + 5000, NULL, 1, &htask4);
  t2=xTaskCreate(DoorTask, "Door", configMINIMAL_STACK_SIZE + 2500, NULL, 1, &htask2);
  t6=xTaskCreate(BLETask, "BLE", configMINIMAL_STACK_SIZE +5000, NULL, 2, &htask6);
  #endif

  #if configUSECASE == USECASE_PATIENTMONITOR

  #define PM_TASK_DATA_HANDLE htask2
  #define PM_TASK_BLE_HANDLE htask4
  #define PM_TASK_BLEPROCESSER_HANDLE htask4

  t2 = xTaskCreate(DataTask, "P.Monitor", configMINIMAL_STACK_SIZE + 15000, NULL, 1,&htask2);
  t4 = xTaskCreate(BLETask, "BLE", configMINIMAL_STACK_SIZE + 5000, NULL, 2,&htask4);
  t6 = pdPASS;
  t7 = xTaskCreate(BLEProcesserTask, "BLEProcesser", configMINIMAL_STACK_SIZE + 4000, NULL, 3,&htask7);
  #endif

  #if configUSECASE == USECASE_WEATHERMONITOR

  #define PM_TASK_DATA_HANDLE htask2
  #define PM_TASK_BLE_HANDLE htask4
  #define PM_TASK_BLEPROCESSER_HANDLE htask4

  t2 = xTaskCreate(TempSamplerTask, "TempSampler", configMINIMAL_STACK_SIZE + 10000, NULL, 1,&htask2);
  t4 = xTaskCreate(BLETask, "BLE", configMINIMAL_STACK_SIZE + 5000, NULL, 2,&htask4);
  t6 = xTaskCreate(HumSamplerTask, "HumSampler", configMINIMAL_STACK_SIZE + 10000, NULL, 1,&htask6);
  t7 = xTaskCreate(BLEProcesserTask, "BLEProcesser", configMINIMAL_STACK_SIZE + 5000, NULL, 3,&htask7);
  #endif

  #if configUSECASE == USECASE_GENERAL
  t2 = xTaskCreate(LSU_Stimulator, "LSU", configMINIMAL_STACK_SIZE + 8000, NULL, 2,&htask2);
  t6=xTaskCreate(CPI_Stimulator, "CPI", configMINIMAL_STACK_SIZE + 8000, NULL, 2,&htask6);
  t4=xTaskCreate(EXC_Stimulator, "EXC", configMINIMAL_STACK_SIZE + 8000, NULL, 2,&htask4);
  t7=xTaskCreate(FLD_Stimulator, "FLD", configMINIMAL_STACK_SIZE + 8000, NULL, 2,&htask7);
  #endif

  #if configUSECASE == USECASE_OVH
  t2=pdPASS;//xTaskCreate(ArchOVHTask,"ArchOVH",configMINIMAL_STACK_SIZE+10000,NULL,2,&htask2);
  t3=xTaskCreate(PureAttest,"PureAttestation",configMINIMAL_STACK_SIZE+8000,NULL,3,&htask3);
  t4=xTaskCreate(MethodAttest,"MethodAttestation",configMINIMAL_STACK_SIZE+8000,NULL,2,&htask4);
  t6=pdPASS;
  t7=pdPASS;
  #endif

  if ( attestation_task != pdPASS || t2 != pdPASS || t3 != pdPASS || t4 != pdPASS || malicious_task != pdPASS || t6 != pdPASS || t7 != pdPASS){
    Serial.println("Error during task creation, aborting");
    vTaskDelete(htask1);
    vTaskDelete(htask2);
    vTaskDelete(htask3);
    vTaskDelete(htask4);
    vTaskDelete(htask5);
    vTaskDelete(htask6);
    vTaskDelete(htask7);
    return -1;
  }
  else{
    PRINTF("Tasks succesfully created");
  }

  temp_hum_sem = xSemaphoreCreateBinary();
  critical = xSemaphoreCreateBinary();

  if ( temp_hum_sem == NULL)
    return -1;

  else
    return 0;
}

void vApplicationTickHook(void){

}

void vApplicationIdleHook(void){
  
}