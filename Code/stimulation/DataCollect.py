import argparse
import os
import json
from time import sleep

import pandas as pd

constants_path = "src/constants.h"


# Set the argument parser
parser = argparse.ArgumentParser(
    description="Run an experiment"
)

parser.add_argument(
    '--use_case',
    type=str,
    help='The usecase to run for this experiment',
    default='IndustrialSensor'
)

parser.add_argument(
    '--relocation_type',
    type=str,
    help='Set it to relocating or transient',
    default='transient'
)

parser.add_argument(
    '--malware_size',
    nargs='*',
    default=[1024,2048,4096],
    help="Sizes in bytes of the roving malware. At most 4 sizes are supported."
)

parser.add_argument(
    '--exo_frequency_connect',
    type=float,
    default=10.0,
    help="Frequency in Hz of BLE connect exogenous events."
)

parser.add_argument(
    '--exo_frequency_read',
    type=float,
    default=10.0,
    help="Frequency in Hz of BLE read exogenous events."
)

parser.add_argument(
    '--exo_delay_disconnect',
    type=float,
    default=10.0,
    help="Delay in ms of BLE disconnect exogenous events. The delay is measured starting from the last read."
)

parser.add_argument(
    '--endo_frequency_fan',
    type=float,
    default=0.5,
    help="Frequency in Hz of fan endogenous events (for IndustrialSensor usecase)"
)

parser.add_argument(
    '--endo_frequency_valve',
    type=float,
    default=0.125,
    help="Frequency in Hz valve fan endogenous events (for IndustrialSensor usecase)"
)

parser.add_argument(
    '--stress_level',
    type=int,
    default=-1,
    help='The stress level to apply to the stimulation script. Use -1 to apply all stress levels randomically.\n'+ \
        'Current stress levels are\n' + \
        '0: Connect, read once, disconnect\n' + \
        '1: Connect, read n times, disconnect\n' + \
        '2: k threads perform n readings each\n' + \
        '3: Connect and disconnect n times\n' )

parser.add_argument(
    '--main_task_frequency',
    type=float,
    default=8.0,
    help='The frequency in Hz of the main task'
)

parser.add_argument(
    '--endo_distribution',
    type=str,
    default="Weibull",
    help="The distribution of endogenous events. Currently supported ones are Exponential and Weibull."
)

parser.add_argument(
    '--endo_keep_interval',
    type=int,
    default=4,
    help='Number of sample iterations during which the sampled values must be kept anomalous'
)

parser.add_argument(
    "--entropy_level",
    type=str,
    default='HIGH',
    help='The unpredictability of tasks. Works only if the use-case is the general one.'
)

parser.add_argument(
    "--activity_level",
    type=str,
    default='HIGH',
    help='The intensity of tasks\'s effects on counters. Works only if the use-case is the general one.'
)

parser.add_argument(
    "--csv_path",
    type=str,
    default=None,
    help='Path to the csv file that should store data'
)

# Get the arguments passed to the script
args = parser.parse_args()

# Paramaters passed to the external stimulator
# NOTE: use_case is an exception because it is also passed to
# the MCU code
stress_level = args.stress_level
use_case = args.use_case.upper()
exo_frequency_connect = args.exo_frequency_connect
exo_frequency_read = args.exo_frequency_read
exo_delay_disconnect = args.exo_delay_disconnect
endo_keep_interval = args.endo_keep_interval
entropy_level = args.entropy_level
activity_level = args.activity_level
csv_path = args.csv_path

# Paramaters passed to the MCU source code
relocation_type = args.relocation_type
malware_size = args.malware_size
endo_frequency_fan = args.endo_frequency_fan
endo_frequency_valve = args.endo_frequency_valve
endo_distribution = args.endo_distribution
main_task_frequency = args.main_task_frequency


iterations_dictionary = {
    'LSU': {

        # Select entropy, then activity intensity
        'VERY_LOW':{

            'VERY_LOW':(10000,20000),
            'LOW':(50000,60000),
            'MEDIUM':(150000,160000),
            'HIGH':(250000,260000)

        },
        'LOW':{

            'VERY_LOW':(7000,23000),
            'LOW':(45000,65000),
            'MEDIUM':(140000,170000),
            'HIGH':(210000,300000)

        },
        'MEDIUM':{

            'VERY_LOW':(5000,25000),
            'LOW':(40000,70000),
            'MEDIUM':(130000,180000),
            'HIGH':(190000,320000)

        },
        'HIGH':{

            'VERY_LOW':(2000,28000),
            'LOW':(30000,80000),
            'MEDIUM':(110000,200000),
            'HIGH':(160000,350000)
        }
    },

    'FLD':{
        
        # Select entropy, then activity intensity
        'VERY_LOW':{

            'VERY_LOW':(4500,5500),
            'LOW':(10000,12000),
            'MEDIUM':(70000,74000),
            'HIGH':(300000,350000)

        },
        'LOW':{

            'VERY_LOW':(2000,6000),
            'LOW':(10000,20000),
            'MEDIUM':(70000,98000),
            'HIGH':(300000,450000)

        },
        'MEDIUM':{

            'VERY_LOW':(5000,45000),
            'LOW':(45000,95000),
            'MEDIUM':(145000,195000),
            'HIGH':(350000,400000)

        },
        'HIGH':{

            'VERY_LOW':(0,50000),
            'LOW':(50000,100000),
            'MEDIUM':(150000,200000),
            'HIGH':(350000,400000)
        }
    },

    'EXC':{
        
        # Select entropy, then activity intensity
        'VERY_LOW':{

            'VERY_LOW':(4500,5500),
            'LOW':(10000,12000),
            'MEDIUM':(70000,74000),
            'HIGH':(300000,350000)

        },
        'LOW':{

            'VERY_LOW':(2000,6000),
            'LOW':(10000,20000),
            'MEDIUM':(70000,98000),
            'HIGH':(300000,450000)

        },
        'MEDIUM':{

            'VERY_LOW':(5000,45000),
            'LOW':(45000,95000),
            'MEDIUM':(145000,195000),
            'HIGH':(350000,400000)

        },
        'HIGH':{

            'VERY_LOW':(0,50000),
            'LOW':(50000,100000),
            'MEDIUM':(150000,200000),
            'HIGH':(350000,400000)
        }
    },

    'CPI':{
        # Select entropy, then activity intensity
        'VERY_LOW':{

            'VERY_LOW':(4500,5500),
            'LOW':(10000,12000),
            'MEDIUM':(70000,74000),
            'HIGH':(300000,350000)

        },
        'LOW':{

            'VERY_LOW':(2000,6000),
            'LOW':(10000,20000),
            'MEDIUM':(70000,98000),
            'HIGH':(300000,450000)

        },
        'MEDIUM':{

            'VERY_LOW':(2000,20000),
            'LOW':(45000,95000),
            'MEDIUM':(145000,195000),
            'HIGH':(350000,400000)

        },
        'HIGH':{

            'VERY_LOW':(0,50000),
            'LOW':(50000,100000),
            'MEDIUM':(150000,200000),
            'HIGH':(350000,400000)
        }
    }
}

entropy_dictionary = {
    'LSU': {

        # Select entropy
        'VERY_LOW':(80,120),
        'LOW':(60,140),
        'MEDIUM':(20,180),
        'HIGH':(0,200)
    },

    'FLD':{
        'VERY_LOW':(80,120),
        'LOW':(60,140),
        'MEDIUM':(20,180),
        'HIGH':(0,200)
    },

    'EXC':{
        
        'VERY_LOW':(80,120),
        'LOW':(60,140),
        'MEDIUM':(20,180),
        'HIGH':(0,200)
    },

    'CPI':{
        'VERY_LOW':(80,120),
        'LOW':(60,140),
        'MEDIUM':(20,180),
        'HIGH':(0,200)
    }
}


# BEGIN Auxiliary functions definition

def log_metaparameters(csv_path):

    data = None
    metadata=None
    try:
        data = pd.read_csv(csv_path,error_bad_lines=False)
        metadata = data['METADATA']
    except TypeError:
        print("Please specify a valid csv file")
        return
    except FileNotFoundError:
        print("Please specify a valid csv file")
        return
    except KeyError:
        print("Key does not exist")
        return  
    
    # If all values in metadata are
    # nan, we can assume that the metadata were
    # not written yet
    if metadata.isnull().all():
        data['METADATA'][0]=use_case

        if use_case == 'GENERAL':
            data['METADATA'][1]="Activity: " +activity_level
            data['METADATA'][2]="Entropy: " +entropy_level
    
    data.to_csv(csv_path)


def summarize_metaparameters():
    print("ENTROPY:",entropy_level)
    print("ACTIVITY:",activity_level)
    #print("COUNTERS:",counters)
    #print("RUNNING TASKS:",tasks)

def update_stimulation_param(path, \
                            exo_frequency_connect, \
                            exo_frequency_read, \
                            exo_delay_disconnect \
                            ):
    f = open(path)
    new_json = json.load(f)
    f.close()

    new_json['connect_frequency']=exo_frequency_connect
    new_json['read_frequency']=exo_frequency_read
    new_json['disconnect_delay']=exo_delay_disconnect

    json_string = json.dumps(new_json,indent=4)

    with open(path,'w') as f:
        f.write(json_string)
        f.close()

def get_delay(entropy:str,task:str):
    
    return entropy_dictionary[task][entropy]

def get_iterations(density:str, entropy:str, task:str):

    return iterations_dictionary[task][entropy][density]


# END Auxiliary functions definition

summarize_metaparameters()
#log_metaparameters(csv_path)

print("\nGenerating constants.h file...")

os.system("rm " + constants_path)
os.system("cat constants_fixed.h >> " + constants_path)

constants_file = open(constants_path,"a")

constants_file.write("#define configUSECASE USECASE_"+use_case.upper() +"\n")
constants_file.write("#define configRELOCATION_TYPE \""+relocation_type.upper()+"\"\n")

while len(malware_size) < 4:
    malware_size.append(0)

if len(malware_size) > 4:
    malware_size = malware_size[0:4]

for s in malware_size:
    constants_file.write("#define configMALWARE_SIZE"+ str(malware_size.index(s)) + " " + str(s)+"\n")

if use_case == "INDUSTRIALSENSOR":
    constants_file.write("#define configENDO_FREQUENCY_FAN "+ str(endo_frequency_fan) +"\n")
    constants_file.write("#define configENDO_FREQUENCY_VALVE "+ str(endo_frequency_valve) +"\n")
    constants_file.write("#define configENDO_DISTRIBUTION \"" + endo_distribution.upper() + "\"\n")
    constants_file.write('#define configENDO_KEEP_INTERVAL ' + str(endo_keep_interval) + "\n")

constants_file.write("#define configMAIN_TASK_DELAY " + str(1000/main_task_frequency) +"\n")

if use_case == 'GENERAL':

    for name in ['LSU','FLD','CPI','EXC']:
        
        min,max = get_delay(entropy_level,'LSU')
        constants_file.write("#define config"+name+"_STIMULATOR_DELAY_MIN "+ str(min) +"\n")
        constants_file.write("#define config"+name+"_STIMULATOR_DELAY_MAX "+ str(max) +"\n")
        min,max = get_iterations(activity_level,entropy_level,'LSU')
        constants_file.write("#define config"+name+"_ITERATIONS_MIN "+ str(min) +"\n")
        constants_file.write("#define config"+name+"_ITERATIONS_MAX "+ str(max) +"\n")


constants_file.close()

print("constants.h succesfully generated!\n")

if use_case != 'GENERAL':
    print("Updating the config.json file...")
    update_stimulation_param('stimulation/config.json',exo_frequency_connect,exo_frequency_read,exo_delay_disconnect)
    print("config.json successfully updated!\n")

print("Compiling and uploading to the MCU")

upload_command = "pio run --target upload"

print("COMMAND RUN:\n",upload_command)
os.system(upload_command)

print("Running the stimulation script")

bluetooth_start_comm = "sudo service bluetooth start"
stimulation_start_comm = "sudo python3 stimulation/UseCaseStimulator.py " + \
            " --use_case=" + use_case + \
            " --stress_level=" + str(stress_level) + \
            " --verbose=0"

if use_case != 'GENERAL':
    print("COMMAND RUN:\n",bluetooth_start_comm)
    os.system(bluetooth_start_comm)
    print("COMMAND RUN:\n",stimulation_start_comm)
    os.system(stimulation_start_comm)



