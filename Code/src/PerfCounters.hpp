#ifndef PERFCOUNTERS_HEADER
#define PERFCOUNTERS_HEADER

#include <Arduino.h>
#include "constants.h"
#include "EventCounterTypes.h"

// Bitmasks to enable/disable load/store count
#define ENABLE_COUNTERS_MASK 0x3F0000

// Macro functions
#define ACTIVE_COUNTERS() ( (DWT->CTRL & ENABLE_COUNTERS_MASK) != 0 )

// Index of the clock counter
#define CLOCK_INDEX 3

class PerfCountersMonitor {

    private:
    unsigned int PerfCounters[NUM_OF_PERFCOUNTERS];
    unsigned int PerfCounters_Total[NUM_OF_PERFCOUNTERS];
    unsigned int EventCounters[NUM_OF_EVENTCOUNTERS];
    unsigned int EventCounters_Total[NUM_OF_EVENTCOUNTERS];

    public:

    PerfCountersMonitor(){
        int i;

        for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){
            PerfCounters_Total[i]=0;
        }

        for (i=0; i < NUM_OF_EVENTCOUNTERS && getECounterType(i) != NULL; ++i){
            EventCounters_Total[i]=0;
        }
    }

    /**
    * @brief Given an index of the PerfCounter array, returns the name of 
    the performance counter stored at index 
    * 
    * @param index Index of the PCounter
    * @return char* The type of PCounter
    */
    char* getPCounterType(int index){
   
        switch(index){

            case 0:
            return "DWT_LSUCNT";
            break;

            case 1:
            return "DWT_CPICNT";
            break;

            case 2:
            return "DWT_EXCCNT";
            break;

            case 3:
            return "DWT_CYCCNT";
            break;

            case 4:
            return "DWT_SLEEPCNT";
            break;

            case 5:
            return "DWT_FOLDCNT";
            break;

            default:
            return NULL;
        }
    }


   /**
    * @brief Given an index of the EventCounter array, returns the name of 
    the performance counter stored at index 
    * 
    * @param index Index of the ECounter
    * @return char* The type of ECounter
    */
    char* getECounterType(int index){
   
        #if configUSECASE == USECASE_INDUSTRIALSENSOR
        return getECounterType_IndustrialSensor(index);
        #endif

        #if configUSECASE == USECASE_SMARTLOCK
        return getECounterType_SmartLock(index);
        #endif

        #if configUSECASE == USECASE_PATIENTMONITOR
        return getECounterType_PatientMonitor(index);
        #endif

        #if configUSECASE == USECASE_WEATHERMONITOR
        return getEcounterType_WeatherMonitor(index);
        #endif

        #if configUSECASE == USECASE_GENERAL
        return getECounterType_None(index);
        #endif

        #if configUSECASE == USECASE_OVH
        return NULL;
        #endif
    }

    /*
    Enable the Performance Counters
    */
    void enable(){
        if (!ACTIVE_COUNTERS()){
            DWT->CTRL = (DWT->CTRL | ENABLE_COUNTERS_MASK | (1<<12) | (1<<9) | (1<<1) );
        }
    }

    /*
    Disable the Performance Counters
    */
    void disable(){
        if (ACTIVE_COUNTERS()){
            DWT->CTRL = DWT->CTRL & ~ENABLE_COUNTERS_MASK;
        }
    }

    /*
    Reads the meaningful performance counters and stores them in the PerfCounters array
    */
    void getPCounters(){
  
        int i=0, mtb_read_entries;

        PerfCounters[i++]+= DWT->LSUCNT;
        PerfCounters[i++]+= DWT->CPICNT;
        PerfCounters[i++]+= DWT->EXCCNT;
        PerfCounters[i++]+= 0;//DWT->CYCCNT;
        PerfCounters[i++]+= DWT->SLEEPCNT;
        PerfCounters[i++]+= DWT->FOLDCNT;

    }

     /**
     * @brief Updates the value of the event counter in position
     * index by adding incr
     * 
     * @param incr The increment in the event counter
     * @param index The index of the parameter to update
     */
    void updateECounter(unsigned int incr, int index){
        
        EventCounters[index]+=incr;
    }

    /**
     * @brief Resets all PCounters and ECounters
     * 
     */
    void resetTotalCounters(){
        int i;

        for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){
            PerfCounters_Total[i]=0;
        }

        for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){
            EventCounters_Total[i]=0;
        }

        PerfCounters_Total[3]=DWT->CYCCNT;
    }

    void resetECounters_Partial(){

        int i;
        for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){
            EventCounters[i]=0;
        }
    }

    void resetPCounters_Partial(){

        int i;
        for(i=0; i<NUM_OF_PERFCOUNTERS && getECounterType(i)!=NULL; ++i){
            PerfCounters[i]=0;
        }
    }

    /*Adds to each performance counter the new measurements*/
    void updateTotal(){
        int i;

        for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){
            PerfCounters_Total[i]+= PerfCounters[i];
            PerfCounters[i]=0;
        }

        for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){
            EventCounters_Total[i]+= EventCounters[i];
            EventCounters[i]=0;
        }
    }
    
    /**
     * @brief Prints the appropriate CSV header to obtain a
     * CSV file for later training
     * 
     */
    void printCSVHeader(){
        int i;

        for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){

                if ( i==0 )
                    SerialPort.printf("%s", getPCounterType(i));
                else
                    SerialPort.printf(",%s", getPCounterType(i));
        }

        for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){

                SerialPort.printf(",%s", getECounterType(i));
        }

        SerialPort.printf(",LEGIT,TIME,METADATA\n");
    }

    /*
    Pretty prints to the serial monitor the meaningful performance counters
    */
    void printTotalCounters(bool isCSV, bool isLegit){
        int i;
        PerfCounters_Total[3]= DWT->CYCCNT-PerfCounters_Total[3];
        if ( !isCSV) {
            
            for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){
                SerialPort.printf("Performance counter %d\n",i);
                SerialPort.printf("Counts: %s\nValue: %u\n\n\n", getPCounterType(i),PerfCounters_Total[i] );
            }
        
            for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){
                SerialPort.printf("Event counter %d\n",i);
                SerialPort.printf("Counts: %s\nValue: %u\n\n\n", getECounterType(i),EventCounters_Total[i] );
            }
        }
        else{     

            for(i=0; i<NUM_OF_PERFCOUNTERS && getPCounterType(i)!=NULL; ++i){

                if (i==0)
                    SerialPort.printf("%u", PerfCounters_Total[i]);
                else
                    SerialPort.printf(",%u", PerfCounters_Total[i]);
            }
            
            for(i=0; i<NUM_OF_EVENTCOUNTERS && getECounterType(i)!=NULL; ++i){
                
                SerialPort.printf(",%u", EventCounters_Total[i]);
            }

            Serial.print( isLegit ? ",1," : ",0,");
        }
        
    }

    /**
     * @brief Signal the beginning of a sequence of instructions whose behaviour is monitored.
     * From a low-level point of view, it resets and enables the Counters.
     * 
     */
    void BeginMonitored(){

        disable();
        enable();
    }

    /**
     * @brief Signal the end of a sequence of instructions whose behaviour is monitored.
     * It reads the Architectural and Applicative counters measured so far and
     * adds them to the total.
     * 
     */
    void EndMonitored(){

        getPCounters();
        updateTotal();
    }
};

#endif