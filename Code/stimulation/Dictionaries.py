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