#include "FanTask.h"
#include "ServoTask.h"
#include "LegitActivity.h"

#include "constants.h"

/** Boolean flag, set to 1 if you want to log the measured temperature and humidity
 */
#define INDUSTRIAL_SENSOR_LOG_FLAG 1

void IndustrialSensorNoTask(){

    BTLE.update();

    if(SensorService.isConnected() == TRUE)
    {
    
        counters.updateECounter(millis() - previousSecond,1);

        //Update time
        SensorService.Update_Time_Characteristics();

        if((millis() - previousSecond) >= 1000)
        {
            //Update environnemental data
            //Data are set with random values but can be replaced with data from sensors.
            previousSecond = millis();
            SensorService.Temp_Update(floor(temperature));
            SensorService.Press_Update(random(95000,105000));
            SensorService.Humidity_Update(humidity);
        }
    }
    else
    {
        //Keep the Bluetooth module in discoverable mode
        SensorService.setConnectable();
  
    }

    counters.updateECounter(SensorService.read_requests, 2);
    counters.updateECounter(SensorService.connection_requests, 3);
    counters.updateECounter(SensorService.disconnection_requests, 4);

    counters.disable();
    counters.enable();

    get_Measurements(&temperature, &humidity);
  
    counters.getPCounters();
    counters.updateECounter(1,0);
    counters.updateTotal();

    if (temperature > TEMP_THRESHOLD)
        coolDown(NULL);

    if (humidity > HUMIDITY_TREHSOLD)
        openValve(NULL);
}