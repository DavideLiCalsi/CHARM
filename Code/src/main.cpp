#include <Arduino.h>
#include <Servo.h>

#include "SensorSetup.h"
#include <Crypto.h>

#include "constants.h"
#include "Actuators.hpp"
#include "PerfCounters.hpp"

#if USE_FREERTOS == 1

#include "STM32FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOSConfig_Default.h"
#include "FreeRTOSSetup.h"
#include "croutine.h"
#include "CountersMonitorTimer.h"

#else

#include "NoFreeRTOSSetup.h"

#endif


#include <SPI.h>
#include <SPBTLE_RF.h>
#include <bluenrg_utils.h>
#include <sensor_service.h>

/*
##########################################
            GLOBAL VARIABLES
##########################################
*/
bool should_open_door=false;

//Integer array that stores the performance counters measured in an interval
unsigned int PerfCounters[NUM_OF_PERFCOUNTERS];

//Integer array that stores the sum of performance counters
unsigned int PerfCounters_Total[NUM_OF_PERFCOUNTERS];

//Object whose aim is to monitor and collect the performance counters
PerfCountersMonitor counters;

//ITM buffer
volatile int32_t ITM_RxBuffer= ITM_RXBUFFER_EMPTY;

//  Flag that is raised when attestation is going on
bool attesting=false;

//  Floats storing temperature and humidity values that are measured
float temperature, humidity;

//  Store the acceleration and angular speed measure by the gyroscope
int32_t acc[3];
int32_t ang_speed[3];

//  Semaphores
SemaphoreHandle_t temp_hum_sem;
SemaphoreHandle_t critical;

//  Task handles
TaskHandle_t htask1, htask2, htask3, htask4, htask5, htask6, htask7;

#define IDLE_HANDLE
#define ATTEST_HANDLE
#define MAL_INT_HANDLE
#define MEASURE_BLE_HANDLE

#define CLOCKS_PER_SEC SystemCoreClock

// Flag raised when the malware relocates, to log it in the csv file.
bool malicious_flag = false;

// Servo
Servo servo;

// Actuator object
Actuators actuators;

/*
##########################################
            IDLE TASK
##########################################
*/

static void idle(void* args){

  while(1){
    //Do nothing
    delay(50);
  }
}

typedef struct {
  volatile uint32_t FP_CTRL;
  volatile uint32_t FP_REMAP;
  // Number Implemented determined by FP_CTRL
  volatile uint32_t FP_COMP[];
} sFpbUnit;

static sFpbUnit *const FPB = (sFpbUnit *)0xE0002000;

void fpb_dump_breakpoint_config(void) {
  const uint32_t fp_ctrl = FPB->FP_CTRL;
  const uint32_t fpb_enabled = fp_ctrl & 0x1;
  const uint32_t revision = (fp_ctrl >> 28) & 0xF;
  const uint32_t num_code_comparators =
      (((fp_ctrl >> 12) & 0x7) << 4) | ((fp_ctrl >> 4) & 0xF);

  Serial.printf("FPB Revision: %d, Enabled: %d, Hardware Breakpoints: %d\n",
              revision, (int)fpb_enabled, (int)num_code_comparators);

  for (size_t i = 0; i < num_code_comparators; i++) {
    const uint32_t fp_comp = FPB->FP_COMP[i];
    const bool enabled = fp_comp & 0x1;
    const uint32_t replace = fp_comp >> 30;

    uint32_t instruction_address = fp_comp & 0x1FFFFFFC;
    if (replace == 0x2) {
      instruction_address |= 0x2;
    }

    Serial.printf("  FP_COMP[%d] Enabled %d, Replace: %d, Address 0x%x\n",
                (int)i, (int)enabled, (int)replace, instruction_address);
  }
}

typedef struct {
  volatile uint32_t COMP;
  volatile uint32_t MASK;
  volatile uint32_t FUNCTION;
  volatile uint32_t RSVD;
} sDwtCompCfg;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t CYCCNT;
  volatile uint32_t CPICNT;
  volatile uint32_t EXCCNT;
  volatile uint32_t SLEEPCNT;
  volatile uint32_t LSUCNT;
  volatile uint32_t FOLDCNT;
  volatile const  uint32_t PCSR;
  sDwtCompCfg COMP_CONFIG[];
} sDWTUnit;

static sDWTUnit *const DWT_ = (sDWTUnit *)0xE0001000;

void dwt_dump(void) {
  Serial.printf("DWT Dump:");
  Serial.printf(" DWT_CTRL=0x%x", DWT_->CTRL);

  const size_t num_comparators = (DWT_->CTRL>>28) & 0xF;
  Serial.printf("   NUMCOMP=0x%x", num_comparators);

  for (size_t i = 0; i < num_comparators; i++) {
    const sDwtCompCfg *config = &DWT_->COMP_CONFIG[i];

    Serial.printf(" Comparator %d Config", (int)i);
    Serial.printf("  0x%08x DWT_FUNC%d: 0x%08x",
                &config->FUNCTION, (int)i, config->FUNCTION);
    Serial.printf("  0x%08x DWT_COMP%d: 0x%08x",
                &config->COMP, (int)i, config->COMP);
    Serial.printf("  0x%08x DWT_MASK%d: 0x%08x\n",
                &config->MASK, (int)i, config->MASK);
  }
}


void dwt_reset(void) {
  const size_t num_comparators = (DWT->CTRL>>28) & 0xF;

  for (size_t i = 0; i < num_comparators; i++) {
    sDwtCompCfg *config = &DWT_->COMP_CONFIG[i];
    config->FUNCTION = config->COMP = config->MASK = 0;
  }
}

clock_t clock(){
    return DWT->CYCCNT;
}

static void ArchOVHTask(void* args){

    clock_t begin,end;
    
    begin=clock();
    counters.enable();
    end=clock();
    Serial.printf("Enabling DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %lu milliseconds\n\n", 1000*(end-begin)*1.0/CLOCKS_PER_SEC );

    begin=clock();
    counters.disable();
    end=clock();
    Serial.printf("Disabling DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %lu milliseconds\n\n", 1000*(end-begin)*1.0/CLOCKS_PER_SEC );

    begin=clock();
    counters.getPCounters();
    end=clock();
    Serial.printf("Reading DWT counters required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %lu milliseconds\n\n", 1000*(end-begin)*1.0/CLOCKS_PER_SEC );

    begin=clock();
    counters.updateTotal();
    end=clock();
    Serial.printf("Summing DWT counters to total required %lu clock cycles\n", end-begin);
    Serial.printf("That corresponds to %lu milliseconds\n\n", 1000*(end-begin)*1.0/CLOCKS_PER_SEC );


}


/*
##########################################
            SETUP + LOOP 
##########################################
*/

void setup() {
  
  //NVIC_SetPriority(SPI1_IRQn,configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
  //NVIC_SetPriority(SPI2_IRQn,configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
  //NVIC_SetPriority(SPI3_IRQn,configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);

  SerialPort.begin(9600);
  
  ArchOVHTask(NULL);

  // Seed the PNRG
  randomSeed(42);
  random(10);
  
  // Initialize I2C bus.
  DEV_I2C.begin();

  pinMode(LED1,OUTPUT);

  PRINTF("*Starting*");

  // Setup all the sensors
  if ( !SensorSetup() )
    PRINTF("Error during setup of a sensor");
  
  /*
    Set the correct priority grouping to handle interrupts 
    correctly under FreeRTOS.
  */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);

  // These pins will control the two actuators
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  
  PRINTF("Initialization complete\n");
  PRINTF("Status of counters: %d\n\n", ACTIVE_COUNTERS());

  PRINTF("Creating tasks");

  if ( setupRTOS() != 0 ){
    PRINTF("Failed setup of FreeRTOS");
  }

  // SetupTimerInterrupt(); : does not work

  #if configPRINT_CSV == 1
  counters.printCSVHeader();
  #endif

  delay(1000);
  vTaskStartScheduler();
  
  // This line is unreachable
  Serial.println("Finished setup");
}

void loop() {
  
  #if USE_FREERTOS == 0

  runApplicationTaskless();

  #endif
  
}