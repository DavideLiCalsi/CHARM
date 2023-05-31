#include <Arduino.h>

#include <STM32FreeRTOS.h>
#include <task.h>
#include <stdlib.h>

#include "constants.h"

#define SHOULD_RUN(prob) ( (random(prob) ==0 ? 0:1) )
#define GET_ITERATIONS(min,max,prob) (random(max/100,max) * SHOULD_RUN(prob))

extern PerfCountersMonitor counters;

unsigned long getRandomIterations(int min,int max, int run_prob_range){

    randomSeed(micros());
    unsigned long iterations = GET_ITERATIONS(min,max,run_prob_range);
    return iterations;
}

/**
 * @brief FreeRTOS task whose duty is to stimulate the
 * LSU counter
 * 
 * @param args 
 */
static void LSU_Stimulator(void* args){

    unsigned long i,delay, iterations;

    /* 
    If you specify that ITERATIONS_MAX is 0
    the task does not iterate
    */
    if (configLSU_ITERATIONS_MAX == 0){
        vTaskDelete(NULL);
    }

    while(1){

        __asm inline(
            "mov r8,0x0000  \n"
            "movt r8, 0x2003   \n"
        );

        iterations=getRandomIterations(configLSU_ITERATIONS_MIN,configLSU_ITERATIONS_MAX,3);

        if (iterations!=0){
            for (i=0;i<iterations;++i){

                counters.updateECounter(1,GN_ECOUNTER_LSU_STIM);
                counters.BeginMonitored();

                __asm inline(
                    "ldr r10, [r8]  \n"
                    "ldr r11, [r8]  \n"
                    "ldr r12, [r8]  \n"
                    "ldr r9, [r8]  \n"
                    "ldr r11, [r8]  \n"

                );
            
            counters.EndMonitored();
            }
        }

        vTaskSuspend(NULL);
    }

}


/**
 * @brief Stimulates the Exception counter
 * 
 * @param args 
 */
static void EXC_Stimulator(void* args){

    unsigned long i, delay, iterations;

    /* 
    If you specify that ITERATIONS_MAX is 0
    the task does not iterate
    */
    if (configEXC_ITERATIONS_MAX == 0){
        vTaskDelete(NULL);
    }

    while(1){

        iterations=getRandomIterations(configEXC_ITERATIONS_MIN,configEXC_ITERATIONS_MAX,3);
        
        if (iterations!=0){
            for (i=0;i<iterations; ++i){
                counters.updateECounter(1,GN_ECOUNTER_EXC_STIM);
                counters.BeginMonitored();
                digitalWrite(D0,HIGH);
                digitalWrite(D0,HIGH);
                counters.EndMonitored();
            
            }
        }

        vTaskSuspend(NULL);        
    }

}

/**
 * @brief Stimulates the Folded Instructions
 * counter. As a side effect, it also highly stimulates
 * the Exception overhead counter
 * 
 * @param args 
 */
static void FLD_Stimulator(void* args){

    unsigned long i, delay, iterations;

    /* 
    If you specify that ITERATIONS_MAX is 0
    the task does not iterate
    */
    if (configFLD_ITERATIONS_MAX == 0){
        vTaskDelete(NULL);
    }

    while(1){

        randomSeed(analogRead(0));

        iterations = getRandomIterations( configFLD_ITERATIONS_MIN,  configFLD_ITERATIONS_MAX,3);

        if (iterations!=0)  {

            for (i=0;i<iterations;++i){
                counters.updateECounter(1,GN_ECOUNTER_FLD_STIM);
                counters.BeginMonitored();
                __asm inline(
                    "ADDAL  r0,r0,r1 \n"
                    "ITT    EQ \n"
                    "NOP \n"
                    "NOP   \n" 
                );
                counters.EndMonitored();
            }
        }  

        vTaskSuspend(NULL);
    }
}

/**
 * @brief Stimulates the CPI counter. As a side effect,
 * it also stimulates the Exception overhead
 * 
 * @param args 
 */
static void CPI_Stimulator(void* args){

    unsigned long i;
    unsigned long iterations, delay;

    /* 
    If you specify that ITERATIONS_MAX is 0
    the task does not iterate
    */
    if (configCPI_ITERATIONS_MAX == 0){
        vTaskDelete(NULL);
    }

    while(1){

        randomSeed(analogRead(0));

        iterations = getRandomIterations(configCPI_ITERATIONS_MIN, configCPI_ITERATIONS_MAX,3);
        if (iterations!=0){
            for (i=0;i<iterations; ++i){
                counters.updateECounter(1,GN_ECOUNTER_CPI_STIM);
                counters.BeginMonitored();
                __asm inline(
                    "VMUL.F32 s0,s1,s2  \n"
                    "VSUB.F32 s0,s1,s2  \n"
                    "VADD.F32 s0,s1,s2  \n"
                );
                counters.EndMonitored();
            }
        }

        vTaskSuspend(NULL);
    }
}