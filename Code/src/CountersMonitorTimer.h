#include <TimerInterrupt_Generic.h>
#include <Arduino.h>

extern PerfCountersMonitor counters;

void CountersMonitor_ISR(void){
    Serial.println("Called");
    counters.getPCounters();
    counters.updateTotal();
    counters.disable();
    counters.enable();
}

void SetupTimerInterrupt(){

    STM32Timer timer(TIM3);

    if ( timer.attachInterrupt(100000,CountersMonitor_ISR) )
        Serial.println("Attached timer");
    else
        Serial.println("Failed attaching timer");

    timer.restartTimer();

}  