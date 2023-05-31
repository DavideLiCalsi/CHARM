# SAMURAI

Simple Anomaly-based Microntroller-Units Remote Attestation enabling Interrupts

## Files structure

- **src**: contains the source code to run on the MCU. The folder contains 3 (actually 4) subfolders, one for each use-case.
- **stimulation**: contains the python code to stimulate the use-cases
- **dataset**: contains the collected data in csv format
- **experiment.py**: a python script that starts the MCU and configures it to be stimulated in a desired manner. See **Run data collection** for more info.
  

## USE-CASES
We plan on implementing 3 use-cases:

 1. **Industrial Sensor:** it mimicks the behaviour of a Smart Sensor in an inustrial facility. The application measures temperature and humidity in an industrial machinery. If these values surpass a critical threshold, they activate either a fan (to reduce the temperature) or a valve (to introduce moistured air and increase humidity). The application also allows to connect to the board using BLE and read the temperature and humidity values.
 2. **Weather monitor:** a device used for weather forecasts or climate monitoring. It samples humidity and temperature periodically, according to some external requirements. The collected data can be read via BLE.
 3. **Patient Monitor:** a wearable medical device that monitors the sympthoms of a patient affected by Parkinson's disease. The application also allows to use BLE to connect to the device and read the measured parameters.

## Free-RTOS and Interrupts
To correctly handle SPI interrupts, we overwrite the BlueNRG default interrupt handler by changing it from

```c
__weak void SPI_EXTI_Callback(void)
{
  HCI_Isr();

}
```
where HCI_Isr() is the Interrupt Service Routine used to process HCI requests from the chip, to

```c
__weak void SPI_EXTI_Callback(void)
{
  
  BaseType_t checkIfYieldRequired;
  checkIfYieldRequired = xTaskResumeFromISR(htask7);
  portYIELD_FROM_ISR(checkIfYieldRequired);
}
```
The previous piece of code resumes the Free-RTOS task whose hanlde is htask7. In each use-case, this task is simply implemented as 

```c
static void BLEProcesserTask(void* args){

    while(1){
        
        vTaskSuspend(NULL);

        HCI_Isr();

        vTaskResume(htask6);
        
    }

    vTaskDelete(NULL);
}
```

so it basically calls the intended ISR and then wakes up the BLE task of each usecase, which is represented by htask6 in this case.

## Configuration
The constants.h file contains some useful constans to configure some parameters or hyperparameters.

| Parameter | Description | Values |
| --------- | ----------- | ------ |
| USE_FREERTOS | Flag that enables or disables the use Free-RTOS. It was kept for debugging in earlier stages, it should always be set to the enabled value | 1 Enables RTOS, 0 Disables it  |
| MAL_INT_EN | Flag that enables or disables the malicious interrupt from relocating malware. It should only be raised if you are sure about data collection. Too many malicious interrupts might damage the flash memory | 1 Enables the malicious interrupt, 0 Disables it |
| DATA_ONLY | If enabled, the application will not log any form or debugging/logging information. It will only print the counters values. | 1 To disable debug/logging printing, 0 to enable it |
| configUSECASE | Select which usecase to use. | USECASE_SMARTLOCK: run the SmartLock usecase, USECASE_INDUSTRIALSENSOR: run the IndustrialSensor usecase, USECASE_PATIENTMONITOR: run the PatientMonitor, usecase |
| configRELOCATION_TYPE | Which type of malicious action should happen | TRANSIENT vs RELOCATING |
| configMALWARE_SIZE(0 to 3) | The sizes in bytes of roving malware that you want to test | Potentially any integer |
| configENDO_FREQUENCY_FAN | Frequency [Hz] at which the fan endogenous events happen | Any float, works only for the IndustrialSensor use-case |
| configENDO_FREQUENCY_VALVE | Frequency [Hz] at which the valve endogenous events happen | Any float, works only for the IndustrialSensor use-case |
| configENDO_DISTRIBUTION | Distribution of the two previous endogenous events | EXPONENTIAL vs WEIBULL, works only for the IndustrialSensor use-case  |
| configENDO_KEEP_INTERVAL | Iterations during which the temperature and humidity values should be artificially kept anomalous, works only for the IndustrialSensor use-case |
| configMAIN_TASK_DELAY | The delay in ms of the main task | Any integer |

**NOTE**: the parameters whose name start with **config** should not be manually set (although you can do it, if needed). They are automatically added by calling the **experiment.py** script. If you choose to run everything through this script, the other parameters should be modified in the constants_fixed.h. The script will automatically take care of putting everything in constants.h.


## Stimulation

The stimulation folder contains a python script that generates random Bluetooth requests to stimulate the usecases. To run it, on Linux:

```sh
sudo service bluetooth start
sudo python3 UseCaseStimulator.py [-h] [--use_case USE_CASE]
                            [--stress_level STRESS_LEVEL]
                            [--verbose VERBOSE]
```

If you are uncertain, type 

```sh
sudo python3 UseCaseStimulator.py --help
```
You can manually run this script, or simply run experiment.py and let it call the stimulation script. See next section for more.

## Run data collection
To start collecting data, use the script experiment.py. You can simply call it as:

```sh
sudo python3 experiment.py [OPTIONS]
```

You can specify several options to configure the application that you are running and the way in which you stimulate it. 

```sh
usage: experiment.py [-h] [--use_case USE_CASE] [--relocation_type RELOCATION_TYPE] [--malware_size [MALWARE_SIZE ...]]
                     [--exo_frequency_connect EXO_FREQUENCY_CONNECT] [--exo_frequency_read EXO_FREQUENCY_READ] [--exo_delay_disconnect EXO_DELAY_DISCONNECT]
                     [--endo_frequency_fan ENDO_FREQUENCY_FAN] [--endo_frequency_valve ENDO_FREQUENCY_VALVE] [--stress_level STRESS_LEVEL]
                     [--main_task_frequency MAIN_TASK_FREQUENCY] [--endo_distribution ENDO_DISTRIBUTION] [--endo_keep_interval ENDO_KEEP_INTERVAL] [--verbose VERBOSE]

Run an experiment

optional arguments:
  -h, --help            show this help message and exit
  --use_case USE_CASE   The usecase to run for this experiment
  --relocation_type RELOCATION_TYPE
                        Set it to relocating or transient
  --malware_size [MALWARE_SIZE ...]
                        Sizes in bytes of the roving malware. At most 4 sizes are supported.
  --exo_frequency_connect EXO_FREQUENCY_CONNECT
                        Frequency in Hz of BLE connect exogenous events.
  --exo_frequency_read EXO_FREQUENCY_READ
                        Frequency in Hz of BLE read exogenous events.
  --exo_delay_disconnect EXO_DELAY_DISCONNECT
                        Delay in ms of BLE disconnect exogenous events. The delay is measured starting from the last read.
  --endo_frequency_fan ENDO_FREQUENCY_FAN
                        Frequency in Hz of fan endogenous events (for IndustrialSensor usecase)
  --endo_frequency_valve ENDO_FREQUENCY_VALVE
                        Frequency in Hz valve fan endogenous events (for IndustrialSensor usecase)
  --stress_level STRESS_LEVEL
                        The stress level to apply to the stimulation script. Use -1 to apply all stress levels randomically. Current stress levels are 0: Connect,
                        read once, disconnect 1: Connect, read n times, disconnect 2: k threads perform n readings each 3: Connect and disconnect n times
  --main_task_frequency MAIN_TASK_FREQUENCY
                        The frequency in Hz of the main task
  --endo_distribution ENDO_DISTRIBUTION
                        The distribution of endogenous events. Currently supported ones are Exponential and Weibull.
  --endo_keep_interval ENDO_KEEP_INTERVAL
                        Number of sample iterations during which the sampled values must be kept anomalous
  --verbose VERBOSE     The verbosity level of this script
```

To collect data to a csv file, it is recommended to run

```sh
sudo python3 experiment.py [OPTIONS] & pio device monitor >> output_file.csv
```
What this script does, is:

### 1. Build the constants.h header
### 2. Set some stimulation parameters in the stimulation/config.json file
### 3. Upload code to the MCU and start it
### 4. Run the stimulation/UseCaseStimulator.py