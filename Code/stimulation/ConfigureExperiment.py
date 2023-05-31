import os
from stimulation.Dictionaries import *

all_tasks = ['LSU', 'CPI','EXC','FLD']
all_counters = ["DWT_LSUCNT","DWT_CPICNT","DWT_EXCCNT","DWT_CYCCNT","DWT_SLEEPCNT","DWT_FOLDCNT","LSU_stim","CPI_stim","FLD_stim","EXC_stim"]

class Experiment:

    def __init__(self) -> None:
        # The metaparameters
        self.entropy='HIGH'
        self.activity='HIGH'
        self.counters=[] # The counters specified here are dropped
        self.tasks=[] # The tasks specified here are dropped
        self.print_csv=True # Flag raised if the csv header needs to be printed

        # The parameters
        self.malware_type='TRANSIENT'
        self.malware_sizes=[2048,4*2048,6*2048,10*2048]
    
    def _set_metaparameters(self,metaparameters):
        
        if 'entropy' in metaparameters:
            self.entropy=metaparameters['entropy']
        if 'activity' in metaparameters:
            self.activity=metaparameters['activity']
        if 'counters' in metaparameters:
            self.counters=metaparameters['counters']
        if 'tasks' in metaparameters:
            self.tasks=metaparameters['tasks']
        

    def get_delay(self,task:str):

        return entropy_dictionary[task][self.entropy]

    def get_iterations(self,task:str):
            
        if task not in self.tasks:
            return iterations_dictionary[task][self.entropy][self.activity]
        else:
            return (0,0)

    def _set_parameters(self,parameters):
        
        if 'malware_type' in parameters:
            self.malware_type=parameters['malware_type']
        if 'malware_sizes' in parameters:
            self.malware_sizes=parameters['malware_sizes']
    
    def report_all_parameters(self):
        print("The following parameters will be updated to target:")
        print("ENTROPY:",self.entropy)
        print("ACTIVITY:",self.activity)
        counter_list = [t for t in all_counters if t not in self.counters]
        print("COUNTERS:",counter_list)
        task_list = [t for t in all_tasks if t not in self.tasks]
        print("RUNNING TASKS:",task_list)
        print("MLAWARE TYPE:",self.malware_type)
        print("MALWARE SIZES:",self.malware_sizes)

    def _write_all_parameters(self,constants_path='src/constants.h'):

        self.report_all_parameters()
        print("\nGenerating constants.h file...",end="")

        os.system("rm " + constants_path)
        os.system("cat constants_fixed.h >> " + constants_path)

        constants_file = open(constants_path,"a")

        if self.print_csv:
            self.print_csv=False
            constants_file.write("#define configPRINT_CSV 1\n")
        else:
            constants_file.write("#define configPRINT_CSV 0\n")

        constants_file.write("#define configUSECASE USECASE_GENERAL\n")
        for name in ['LSU','FLD','CPI','EXC']:
        
            min,max = self.get_delay('LSU')
            constants_file.write("#define config"+name+"_STIMULATOR_DELAY_MIN "+ str(min) +"\n")
            constants_file.write("#define config"+name+"_STIMULATOR_DELAY_MAX "+ str(max) +"\n")
            min,max = self.get_iterations('LSU')
            constants_file.write("#define config"+name+"_ITERATIONS_MIN "+ str(min) +"\n")
            constants_file.write("#define config"+name+"_ITERATIONS_MAX "+ str(max) +"\n")
        
        constants_file.write("#define configRELOCATION_TYPE \""+self.malware_type.upper()+"\"\n")

        while len(self.malware_sizes) < 4:
            self.malware_sizes.append(0)

        if len(self.malware_sizes) > 4:
            self.malware_sizes = self.malware_sizes[0:4]

        for s in self.malware_sizes:
            constants_file.write("#define configMALWARE_SIZE"+ str(self.malware_sizes.index(s)) + " " + str(s)+"\n")
        
        constants_file.close()
        print("DONE!")

        print("All parameters were succesfully updated!")
    
    def update_MCU(self):
        os.system("pio run --target upload")

        