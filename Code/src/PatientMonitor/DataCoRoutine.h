#include <Arduino.h>
#include <HTS221Sensor.h>
#include <LPS25HBSensor.h>

#include "DataTask.h"

#include "MedicalParams.h"
#include "constants.h"

#if USE_FREERTOS == 1
#include <STM32FreeRTOS.h>
#include <croutine.h>
#endif

#define BUZZER_PIN D7

extern LPS25HBSensor PressTemp;
extern HTS221Sensor HumTemp;

void DataCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex ){

    bool dataLegit, isAlerting=false;
    float temperature, humidity, pressure;

    crSTART(xHandle);

    pinMode(BUZZER_PIN,OUTPUT);

    while(1){
        
        /*
        * Sample humidity, temperature and pressure
        */
        HumTemp.GetHumidity(&humidity);
        HumTemp.GetTemperature(&temperature);
        PressTemp.GetPressure(&pressure);

        dataLegit = DataLegit(humidity,temperature,pressure);

        if ( !dataLegit && !isAlerting ){
            
            /* Data is not legit and the device is not alerting yet, 
            *  alert the patient. */
            AlertPatientBegin();
            isAlerting = true;
        }

        if ( dataLegit && isAlerting ){
            
            /* Data is nowlegit and the device was already alerting, 
            *  quit alerting */
            AlertPatientEnd();
            isAlerting = false;
        }

        ShowMeasurements(&humidity,&temperature,&pressure);

        /* Sleep for a while. The sleep duration depends on
            the sampling frequency that is appropriate for
            healthcare.
        */
        crDELAY(xHandle, SAMPLE_FREQUENCY / portTICK_PERIOD_MS)
        
    }

    crEND();
}