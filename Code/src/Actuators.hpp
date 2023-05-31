/**
 * @file Actuators.hpp
 * @author Davide Li Calsi
 * @brief This class handles the physical actuators.
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include <Servo.h>

#define VALVE_OPENED 1
#define VALVE_CLOSED 0
#define VALVE_UNCHANGED 2
#define FAN_ON 1
#define FAN_OFF 0
#define FAN_UNCHANGED 2

class Actuators {

    private:

    Servo servo;
    bool valveOpen;
    bool fanOn;

    public:

    /**
     * @brief Construct a new Actuators object. Setups the servo that controls the valve
     * and the fan.
     * 
     */
    Actuators();

    /**
     * @brief Open the valve is the humidity value is too low
     * 
     * @param 
     * @return int 1 for failure, 0 for success
     */
    int openValve(bool open_close);

    /**
     * @brief Attach the servo to the default pint D7
     * 
     */
    void attachServo();

    /**
     * @brief Start the fan in the temperature is too high
     * 
     * @param 
     * @return int 1 for failure, 0 for success
     */
    int coolDown(bool open_close);

    /**
     * @brief Open or close a door, based on the passed parameter.
     * Used in the SMART LOCK use-case.
     * 
     * @param open True if you want to open the door, false otherwise
     * @return int 
     */
    int handleDoor(bool open);

};