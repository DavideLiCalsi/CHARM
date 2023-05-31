#include "constants.h"

/**
 * @brief Both the IndustrialSensor and the WeatherMonitor usecases need
 * the HTS221 sensor only, to measure temperature and humidity.
 * 
 */
#if configUSECASE == USECASE_INDUSTRIALSENSOR || configUSECASE == USECASE_WEATHERMONITOR
#include <HTS221Sensor.h>
HTS221Sensor HumTemp(&DEV_I2C);

bool HTS221SensorSetup(){

    return  HumTemp.begin() == HTS221_STATUS_OK && HumTemp.Enable() == HTS221_STATUS_OK;
}
#endif

/**
 * @brief The PatientMonitor usecase only requires the LSM6DS0 sensors,
 * to sample acceleration and angular velocity.
 * 
 */
#if configUSECASE == USECASE_PATIENTMONITOR
#include <LSM6DS0Sensor.h>
LSM6DS0Sensor AccGyr(&DEV_I2C,LSM6DS0_ACC_GYRO_I2C_ADDRESS_LOW);

bool LSM6DS0SensorSetup(){

    return  AccGyr.begin() == LSM6DS0_STATUS_OK &&
            AccGyr.Enable_X() == LSM6DS0_STATUS_OK &&
            AccGyr.Enable_X() == LSM6DS0_STATUS_OK
            ;
}
#endif

/**
 * @brief Setup the sensor required by the usecase that is running
 * 
 * @return true Success
 * @return false Failure
 */
bool SensorSetup(){

    #if configUSECASE == USECASE_PATIENTMONITOR

    return LSM6DS0SensorSetup();

    #endif

    #if configUSECASE == USECASE_INDUSTRIALSENSOR || configUSECASE == USECASE_WEATHERMONITOR
    return HTS221SensorSetup();
    #endif

    return true;
}

