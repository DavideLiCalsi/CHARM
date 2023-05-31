#include "constants.h"

/*
* ECounters associated to the IndustrialSensor use-case
*
*/

#define IS_ECOUNTER_TEMP_HUM_SAMPLES 0
#define IS_ECOUNTER_BLE_READ_REQUESTS 1
#define IS_ECOUNTER_BLE_CONNECTION_REQUESTS 2
#define IS_ECOUNTER_BLE_DISCONNECTION_REQUESTS 3
#define IS_ECOUNTER_FAN_ON 4
#define IS_ECOUNTER_FAN_OFF 5
#define IS_ECOUNTER_VALVE_OPEN 6
#define IS_ECOUNTER_VALVE_CLOSED 7


/*
* ECounters associated to the SmartLock use-case
*
*/

#define SL_ECOUNTER_REQUEST_VALID 0
#define SL_ECOUNTER_REQUEST_INVALID 1
#define SL_ECOUNTER_OPEN 2
#define SL_ECOUNTER_CLOSE 3
#define SL_ECOUNTER_BLE_CONNECTION 4
#define SL_ECOUNTER_BLE_DISCONNECTION 5


/*
* ECounters associated to the PatientMonitor use-case
*
*/

#define PM_ECOUNTER_SAMPLES 0
#define PM_ECOUNTER_BLE_READ 1
#define PM_ECOUNTER_BLE_CONNECTIONS 2
#define PM_ECOUNTER_BLE_DISCONNECTIONS 3


/*
* ECounters associated to the WeatherMonitor use-case
*
*/

#define WM_ECOUNTER_TEMP_SAMPLES 0
#define WM_ECOUNTER_HUM_SAMPLES 1
#define WM_ECOUNTER_BLE_CONNECTIONS 2
#define WM_ECOUNTER_BLE_DISCONNECTIONS 3
#define WM_ECOUNTER_BLE_READ 4

/*
* ECounters associated to the General use-case
*
*/
#define GN_ECOUNTER_LSU_STIM 0
#define GN_ECOUNTER_CPI_STIM 1
#define GN_ECOUNTER_FLD_STIM 2
#define GN_ECOUNTER_EXC_STIM 3


/**
 * Global ECounters
 *  
 */
#if configUSECASE == USECASE_INDUSTRIALSENSOR
#define ECOUNTER_BLE_CONNECTION IS_ECOUNTER_BLE_CONNECTION_REQUESTS
#define ECOUNTER_BLE_DISCONNECTION IS_ECOUNTER_BLE_DISCONNECTION_REQUESTS
#define ECOUNTER_BLE_READ IS_ECOUNTER_BLE_READ_REQUESTS
#endif

#if configUSECASE == USECASE_WEATHERMONITOR
#define ECOUNTER_BLE_CONNECTION WM_ECOUNTER_BLE_CONNECTIONS 
#define ECOUNTER_BLE_DISCONNECTION WM_ECOUNTER_BLE_DISCONNECTIONS
#define ECOUNTER_BLE_READ WM_ECOUNTER_BLE_READ
#endif

#if configUSECASE == USECASE_PATIENTMONITOR
#define ECOUNTER_BLE_CONNECTION PM_ECOUNTER_BLE_CONNECTIONS 
#define ECOUNTER_BLE_DISCONNECTION PM_ECOUNTER_BLE_DISCONNECTIONS
#define ECOUNTER_BLE_READ PM_ECOUNTER_BLE_READ
#endif


static char* getEcounterType_WeatherMonitor(int index){

    switch (index)
    {
    case WM_ECOUNTER_TEMP_SAMPLES:
        return "Temp samples";
        break;
    
    case WM_ECOUNTER_HUM_SAMPLES:
        return "Hum samples";
        break;
    
    case WM_ECOUNTER_BLE_CONNECTIONS:
        return "Ble connections";
        break;
    
    case WM_ECOUNTER_BLE_DISCONNECTIONS:
        return "Ble disconnections";
        break;
    
    case WM_ECOUNTER_BLE_READ:
        return "Ble reads";
        break;
    
    default:
        return NULL;
        break;
    }
}

static char* getECounterType_IndustrialSensor(int index){

    switch(index){

            case IS_ECOUNTER_TEMP_HUM_SAMPLES:
            return "Temp/Hum samplings";
            break;

            case IS_ECOUNTER_BLE_READ_REQUESTS:
            return "BLE read requests";
            break;

            case IS_ECOUNTER_BLE_CONNECTION_REQUESTS:
            return "BLE connection requests";
            break;

            case IS_ECOUNTER_BLE_DISCONNECTION_REQUESTS:
            return "BLE disconnection requests";
            break;

            case IS_ECOUNTER_FAN_ON:
            return "Fan turn on";
            break;

            case IS_ECOUNTER_FAN_OFF:
            return "Fan turn off";
            break;

            case IS_ECOUNTER_VALVE_OPEN:
            return "Valve open";
            break;

            case IS_ECOUNTER_VALVE_CLOSED:
            return "Valve close";
            break;

            default:
            return NULL;
        }
}

static char* getECounterType_SmartLock(int index){
    
    switch (index)
    {
    case 0:
        return "R_Invalid";
        break;
    
    case 1:
        return "R_Valid";
        break;

    case 2:
        return "Open";
        break;

    case 3:
        return "Close";
        break;
    
    default:
        return NULL;
        break;
    }
}

static char* getECounterType_PatientMonitor(int index){

    switch(index){

        case PM_ECOUNTER_SAMPLES:
            return "Samples";
        break;

        case PM_ECOUNTER_BLE_CONNECTIONS:
            return "BLE connections";
        break;

        case PM_ECOUNTER_BLE_DISCONNECTIONS:
            return "BLE disconnections";
        break;

        case PM_ECOUNTER_BLE_READ:
            return "BLE read requests";
        break;

        default:
            return NULL;
        break;
    }
}

static char* getECounterType_None(int index){

    switch (index)
    {

    case GN_ECOUNTER_LSU_STIM:
        return "LSU_stim";
        break;
    
    case GN_ECOUNTER_CPI_STIM:
        return "CPI_stim";
        break;
    
    case GN_ECOUNTER_FLD_STIM:
        return "FLD_stim";
        break;

    case GN_ECOUNTER_EXC_STIM:
        return "EXC_stim";
        break;
    
    default:
        return NULL;
        break;
    }
}