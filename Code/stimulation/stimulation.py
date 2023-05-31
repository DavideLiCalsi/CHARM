from time import sleep
from random import randint, uniform
from threading import Thread
import json

from matplotlib import use

config_file = open('stimulation/config.json',"r")
use_case_config = json.load(config_file)
config_file.close()

disconnect_delay=use_case_config['disconnect_delay']
connect_delay=1/use_case_config['connect_frequency']
read_delay=1/use_case_config['read_frequency']

def bias_param(param):

    if float.is_integer(float(param)):
        return param + randint(- param//4, param//4)
    else:
        return param + uniform(- param/4, param/4)

def send_requests(use_case,periph,target_handles):

    consecutive_reads = use_case_config[use_case]["stress_2_param"]["consecutive_reads"]
    consecutive_reads = bias_param(consecutive_reads)

    for i in range(0,consecutive_reads):

        for handle in target_handles:
            raw_char = periph.readCharacteristic(handle)
            print("Read raw characteristic: ", raw_char)


def find_characteristics_handle(periph, target_address, target_uuids):

    characteristics = periph.getCharacteristics()

    target_handles = []

    for char in characteristics:
        handle = char.getHandle()
        uuid = char.uuid

        if uuid in target_uuids:
            target_handles.append(handle)

    assert len(target_uuids) == len(target_handles), f"Missing handle. Aborting"

    print("Found all target characteristics\n")

    return target_handles


def stimulate_stress_0(use_case,periph, target_address, target_uuids):
    '''
    Stimulate the MCU with the minimum severity level
    
    periph : Peripheral
        The Bluetooth peripheral object
    
    target_address :
        The address to stimulate
    
    target_uuids :
        List of uuids of the characteristics to read
    
    '''

    periph.connect(target_address)
    services = periph.discoverServices()

    print("Discovering services")
    for service in services:
        print(service.getCommonName(),service.binVal)

    print("Scanning target characteristics")

    target_handles = find_characteristics_handle(periph,target_address,target_uuids)

    for handle in target_handles:

        raw_char = periph.readCharacteristic(handle)
        print("Read raw characteristic: ", raw_char)

    sleep( bias_param(disconnect_delay) )

    periph.disconnect()

    time_to_wait = bias_param(connect_delay)
    print("Sleeping for ", time_to_wait, "seconds")
    sleep(time_to_wait)


def stimulate_stress_1(use_case, periph, target_address, target_uuids):
    '''
    Stimulate the MCU with severity level 1
    
    periph : Peripheral
        The Bluetooth peripheral object
    
    target_address :
        The address to stimulate
    
    target_uuids :
        List of uuids of the characteristics to read
    
    '''

    periph.connect(target_address)
    services = periph.discoverServices()

    print("Discovering services")
    for service in services:
        print(service.getCommonName(),service.binVal)

    print("Scanning target characteristics")

    target_handles = find_characteristics_handle(periph,target_address,target_uuids)

    consecutive_reads = use_case_config[use_case]["stress_1_param"]["consecutive_reads"]
    consecutive_reads = bias_param(consecutive_reads)

    for i in range(0,consecutive_reads):

        for handle in target_handles:
            raw_char = periph.readCharacteristic(handle)
            print("Read raw characteristic: ", raw_char)
        
        sleep(bias_param(read_delay))
    
    sleep(bias_param(disconnect_delay))

    periph.disconnect()

    time_to_wait = bias_param(connect_delay)
    print("Sleeping for ", time_to_wait, "seconds")
    sleep(time_to_wait)


def stimulate_stress_2(use_case,periph, target_address, target_uuids):
    '''
    Stimulate the MCU with severity level 2
    
    periph : Peripheral
        The Bluetooth peripheral object
    
    target_address :
        The address to stimulate
    
    target_uuids :
        List of uuids of the characteristics to read
    
    '''

    periph.connect(target_address)
    services = periph.discoverServices()

    print("Discovering services")
    for service in services:
        print(service.getCommonName(),service.binVal)

    print("Scanning target characteristics")

    target_handles = find_characteristics_handle(periph,target_address,target_uuids)

    t1=Thread(target=send_requests,args=[use_case,periph,target_handles])
    t2=Thread(target=send_requests, args=[use_case,periph,target_handles])
    t3=Thread(target=send_requests, args=[use_case,periph,target_handles])
    t4=Thread(target=send_requests, args=[use_case,periph,target_handles])
    t1.start()
    t2.start()
    t3.start()
    t4.start()

    t1.join()
    t2.join()
    t3.join()
    t4.join()

    sleep(bias_param(disconnect_delay))

    periph.disconnect()

    time_to_wait = bias_param(connect_delay)
    print("Sleeping for ", time_to_wait, "seconds")
    sleep(time_to_wait)


def stimulate_stress_3(use_case, periph,target_address, target_uuids):

    '''
    Stimulate the MCU with severity level 3
    
    periph : Peripheral
        The Bluetooth peripheral object
    
    target_address :
        The address to stimulate
    
    target_uuids :
        List of uuids of the characteristics to read
    
    '''

    consecutive_connections = use_case_config[use_case]["stress_3_param"]["consecutive_connections"]
    consecutive_connections = bias_param(consecutive_connections)

    for _ in range(0,consecutive_connections):
        periph.connect(target_address)

        sleep(bias_param(disconnect_delay))

        periph.disconnect()
    
    time_to_wait = bias_param(connect_delay)
    print("Sleeping for ", time_to_wait, "seconds")
    sleep(time_to_wait)


def stimulate(use_case,periph,target_address, target_uuids,stress=0):

    stimulation_dictionary = {
        0: stimulate_stress_0,
        1: stimulate_stress_1,
        2: stimulate_stress_2,
        3: stimulate_stress_3
    }

    stimulation_dictionary[stress](use_case,periph,target_address,target_uuids)
