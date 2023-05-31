from time import sleep
from stimulation.ConfigureExperiment import Experiment
from stimulation.Generator import Generator
import os

experiment = Experiment()
generator=Generator()

# Set the metaparameters, done once in the beginning
experiment._set_metaparameters(generator.generate_metaparameters(entropy='HIGH',activity='VERY_LOW'))

iterations = 30
delay=600

malware_range=[1,8]
for i in range(iterations):
    experiment._set_parameters(generator.generate_parameters(malware_range=malware_range))
    experiment._write_all_parameters()
    experiment.update_MCU()
    sleep(delay)
