/**
 * @file DataTask.h
 * @author Davide Li Calsi
 * @brief 
 * This task emulates the activity of a healthcare device that constantly monitors the
 * life parameters of a patient. In this case the application uses the HTS221 sensor and
 * the LPS25HB sensor to measure temperature, humidity and pressure.
 * If one of these parameters is above or below some known watermarks (defined in the MedicalParams.h
 * file), a warning is triggered by emitting sound through and active buzzer.
 * The warning is stopped when the parameters go back to normal values.
 * 
 * The application was developed taking inspiration from 
 * https://ww1.microchip.com/downloads/en/DeviceDoc/00001062C.pdf, page 15.
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>

#include "MedicalParams.h"
#include "constants.h"

#if USE_FREERTOS == 1
#include <STM32FreeRTOS.h>
#include <task.h>
#include <queue.h>
#endif

#define BUZZER_PIN D1

extern LSM6DS0Sensor AccGyr;
extern int32_t acc[3];
extern int32_t ang_speed[3];

/**
 * @brief Check if the measured data is within a 
 * valid interval
 * 
 * @param hum Humidity value to check
 * @param temp Temperature value to check
 * @return true The measurements are legit
 * @return false Otherwise
 */
bool DataLegit(float hum, float temp){

    return HUMIDITY_LW < hum && HUMIDITY_HW > hum &&
            TEMPERATURE_LW < temp && TEMPERATURE_HW > temp;
}

/**
 * @brief Emit sound through the active buzzer
 * 
 */
void AlertPatientBegin(){

    digitalWrite(BUZZER_PIN,HIGH);
}

/**
 * @brief Stop emitting sound through the active buzzer
 * 
 */
void AlertPatientEnd(){

    digitalWrite(BUZZER_PIN,LOW);
}

/**
 * @brief Pretty prints the measurements
 * 
 * @param acc Acceleration value
 * @param ang_speed angular speed value
 */
void ShowMeasurements(float* acc, float* ang_speed){

    if ( !DATA_ONLY){
        Serial.println("****** COLLECTED MEASUREMENTS BEG ******\n");
        Serial.print("Acceleration: ");
        Serial.println(*acc);
        Serial.print("Angular speed: ");
        Serial.println(*ang_speed);
        Serial.println("\n****** COLLECTED MEASUREMENTS END ******");
        Serial.println("\n");
    }
}

/**
 * @brief The task that emulates the activity of a healthcare device.
 * It monitors the sympthoms of a patient affected by Parkinson's disease.
 * 
 * @param args 
 */
static void DataTask(void* args){

    bool dataLegit, isAlerting=false;

    pinMode(BUZZER_PIN,OUTPUT);

    while(1){
        
        /*
        * Sample acceleration and angular velocity
        */
        counters.resetECounters_Partial();
        counters.resetPCounters_Partial();

        counters.BeginMonitored();
        
        AccGyr.Get_X_Axes(acc);
        AccGyr.Get_G_Axes(ang_speed);

        counters.updateECounter(1,PM_ECOUNTER_SAMPLES);

        counters.EndMonitored();

        /*
            Sleep for 20 ms
        */
        vTaskDelay(configMAIN_TASK_DELAY/ portTICK_PERIOD_MS);
        
    }

    vTaskDelete(NULL);
}