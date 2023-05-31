from gettext import find
from time import sleep
from random import randint, random
import argparse

from stimulation import *

from bluepy.btle import Scanner, DefaultDelegate, Peripheral

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)

# Set the argument parser
parser = argparse.ArgumentParser(
    description="Stimulate one of the use-cases."
)
parser.add_argument(
    '--use_case',
    type=str,
    help='Which use-case you want to stimulate with this script',
    default='IndustrialSensor'
)
parser.add_argument(
    '--stress_level',
    type=int,
    default=-1,
    help='The stress level to apply to the stimulation. Use -1 to apply all stress levels randomically'
)
parser.add_argument(
    "--verbose",
    type=int,
    default=0,
    help='The verbosity level of this script'
)

args = parser.parse_args()

stress_level = args.stress_level
verbose = args.verbose
use_case = args.use_case


target_found = False
target_address = use_case_config['target_address']
target_uuids = use_case_config[use_case]['target_uuids']

scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(3.0)

# Scan the available bluetooth devices
for dev in devices:
    print("Device" ,dev.addr,"RSSI= ", dev.addrType," db", dev.rssi)

    if dev.addr == target_address:
        target_found = True

assert target_found, f"Target device was not found.\n"

periph = Peripheral()

while True:
    
    if stress_level != -1:
        stimulate(use_case,periph,target_address,target_uuids,stress_level)
    else:
        stimulate(use_case,periph,target_address,target_uuids,randint(0,3))