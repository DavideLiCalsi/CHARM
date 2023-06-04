/*
This header file contains definition of useful constants
*/

#include "FreeRTOSConfig.h"
#include "Wire.h"

#define PIN_BLE_SPI_MOSI   (11)
#define PIN_BLE_SPI_MISO   (12)
#define PIN_BLE_SPI_SCK    (3)

#define PIN_BLE_SPI_nCS    (A1)
#define PIN_BLE_SPI_RESET  (7)
#define PIN_BLE_SPI_IRQ    (A0)

#define PIN_BLE_LED    (0xFF)


#define DEV_I2C Wire
#define SerialPort Serial

//Useful numerical constants
#define NUM_OF_PERFCOUNTERS 20
#define NUM_OF_EVENTCOUNTERS 20
#define FLASH_SIZE_BYTES 524288
#define FLASH_BEGIN 0x08000000
#define MALWARE_SIZE 10240

#define SEED "seed"

/**
 * @brief  Configuration macros that can be set to 
 * personalize the running application.
 * 
 */

//Flag raised if FreeRTOS is to be used
#define USE_FREERTOS 1

//Set to 1 if you are enabling malicious interr.
#define MAL_INT_EN 1

//Set to 1 in order to only log data in csv format
#define DATA_ONLY 1

//Temperature and humidity tresholds to activate fan. Used in old experiments, irrelevant now
#define TEMP_THRESHOLD 22.0

#define HUMIDITY_TREHSOLD 43.0

#define configFREERTOS_COROUTINES configUSE_CO_ROUTINES

#if DATA_ONLY == 0
#define PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**
 * Select which usecase to run
 */
#define USECASE_SMARTLOCK 0
#define USECASE_INDUSTRIALSENSOR 1
#define USECASE_PATIENTMONITOR 2
#define USECASE_WEATHERMONITOR 3
#define USECASE_GENERAL 4
#define USECASE_OVH 5

// Configurable constants start here
