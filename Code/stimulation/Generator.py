from random import randint

class Generator:

    def __init__(self) -> None:
        self.available_entropies=['VERY_LOW','LOW','MEDIUM','HIGH']
        self.available_activities=['VERY_LOW','LOW','MEDIUM','HIGH']
        self.available_counters=[]
        self.available_tasks=[]
        self.available_malware_types=['TRANSIENT','RELOCATING']
        self.available_malware_sizes_range=(1,25) # size in multiple of Flash pages, 1 Page = 2048 Bytes
        self.page_size=0x800
 

    def generate_metaparameters(self,entropy=None, activity=None):
        index = randint(0,len(self.available_entropies)-1)

        if entropy is None:
            entropy = self.available_entropies[index]

        index = randint(0,len(self.available_activities)-1)

        if activity is None:
            activity = self.available_activities[index]

        # For the time being, keep all counters and tasks
        counters = self.available_counters
        tasks = self.available_tasks

        return {'entropy':entropy,'activity':activity,'counters':counters,'tasks':tasks}

    def generate_parameters(self, malware_range=None, mal_type=None):
        """
        Generates some metaparameters to acquire data.
        """

        if mal_type is None:
            index = randint(0,len(self.available_malware_types)-1)
            malware_type = self.available_malware_types[index]
        else:
            malware_type=mal_type

        malware_sizes = []

        if malware_range is None:
            while (len(malware_sizes) <4):
                min,max=self.available_malware_sizes_range
                new_size = randint(min,max) * self.page_size
                if new_size not in malware_sizes:
                    malware_sizes.append(new_size)
        else:
            sizes_to_keep = list(range(malware_range[0],malware_range[1]))
            print(sizes_to_keep)
            malware_sizes=[self.page_size*size for size in sizes_to_keep]

        return {'malware_type':malware_type, 'malware_sizes':malware_sizes}

