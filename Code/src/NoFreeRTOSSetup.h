#include "constants.h"

#if configUSECASE == USECASE_SMARTLOCK
#include "SmartLock/SmartLockNoTask.h"
#define runApplicationTaskless SmartLockNoTask
#endif

#if configUSECASE == USECASE_INDUSTRIALSENSOR
#include "IndustrialSensor/IndustrialSensorNoTask.h"
#define runApplicationTaskless IndustrialSensorNoTask
#endif

#if configUSECASE == USECASE_PATIENTMONITOR
#include "PatientMonitor/PatientMonitorNoTask.h"
#define runApplicationTaskless PatientMonitorNoTask
#endif
