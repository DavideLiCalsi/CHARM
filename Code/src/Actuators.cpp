#include "Actuators.hpp"
#include "constants.h"
#include <Arduino.h>
#include <Servo.h>

#include "PerfCounters.hpp"

#define SERVO_PIN D0
#define FAN_PIN D1

extern PerfCountersMonitor counters;

Actuators::Actuators(){

    valveOpen=false;
    fanOn=false;
}

void Actuators::attachServo(){
  servo.attach(SERVO_PIN);
}

/**
 * @brief Open or close the valve. If you open a valve that is already
 * open, nothing happens (the same goes for a closed valve).
 * 
 * @param open_close True if you wish to close the valve, false otherwise
 * @return int 
 */
int Actuators::openValve(bool open_close){
    
    if ( open_close==true && !valveOpen){

      PRINTF("Opening valve\n");

      counters.updateECounter(1,IS_ECOUNTER_VALVE_OPEN);

      servo.write(90);
      valveOpen=true;

      return VALVE_OPENED;
  
    }
    else if ( open_close==false && valveOpen) {

      PRINTF("Closing valve\n");

      counters.updateECounter(1,IS_ECOUNTER_VALVE_CLOSED);

      servo.write(180);
      valveOpen=false;

      return VALVE_CLOSED;
    }
  
  return VALVE_UNCHANGED;
}

/**
 * @brief Manage the fan. If the fan is already on, and you request to turn it
 * on, nothing happens (the same goes for a fan that is off).
 * 
 * @param open_close True to start the fan, false to stop it.
 * @return int 
 */
int Actuators::coolDown(bool open_close){

    if ( open_close==true && !fanOn){

      PRINTF("Turning Fan on\n");

      counters.updateECounter(1,IS_ECOUNTER_FAN_ON);

      digitalWrite(FAN_PIN,HIGH);

      fanOn = true;

      return FAN_ON;
    }
    else{

      if ( open_close==false && fanOn){

        PRINTF("Turning Fan off");

        counters.updateECounter(1,IS_ECOUNTER_FAN_OFF);

        digitalWrite(FAN_PIN,LOW);

        fanOn = false;

        return FAN_OFF;
      }
    }
  
  return FAN_UNCHANGED;
}

/**
 * @brief DEPRECATED
 * 
 * @param open 
 * @return int 
 */
int Actuators::handleDoor(bool open){
   
   if ( open )
    servo.write(0);
  else
    servo.write(90);
}
