from os import system
from sys import flags
from importlib_metadata import distribution
from numpy import average, matrix
from models import *

from pathlib import *
import argparse
import pandas as pd

from models.LogisticModel import LogisticModel
from models.DecisionTreeModel import DecisionTreeModel
from models.SVM import SVM
from models.Plotter import *

import warnings
import json

warnings.filterwarnings('ignore')

repetitions=30
BoxCox_lambda=0.5


def getFlagsFromList(l):
    """
    Translates a list of flags into a dictionary.
    """

    flags={
        'BoxCox':False,
        'Architectural':False,
        'Poly':False
    }

    for f in flags.keys():
        if f in l:
            flags[f]=True
    
    return flags

# *** Specify arguments here *** 

# Path to the dataset
data_path = "dataset"
model_type="Logistic"
model=None
all_paths=['dataset/Data_31Lug2022.csv','dataset/Data_30Lug2022.csv','dataset/Data_29Lug2022.csv','dataset/Data_27Lug2022.csv','dataset/Data_26Lug2022.csv','dataset/Data_25Lug2022.csv','dataset/Data_24Lug2022.csv','dataset/Data_23Lug2022.csv','dataset/Data_22Lug2022.csv','dataset/Data_21Lug2022.csv','dataset/Data_20Lug2022.csv','dataset/Data_19Lug2022.csv','dataset/Data_18Lug2022.csv','dataset/Data_17Lug2022.csv','dataset/Data_12Giu2022.csv','dataset/Data_18Giu2022.csv']


def SelectModel(model_type):
    
    global model
    
    if model_type.upper() == 'LOGISTIC':
        print("Selected Logistic Regression")
        model=LogisticModel()
    elif model_type.upper() == 'DECISIONTREE':
        print("Selected Decision Tree")
        model=DecisionTreeModel()
    elif model_type.upper() == 'SVM':
        print("Selected SVM")
        model=SVM()
    elif model_type.upper() == 'DNN':
        print("Selected Deep Neural Network")
        model=DNN()

# Save matrix as json

def SaveResults(file_name,a,p,r,f):
    
    inverse_map={
        'VERY_LOW':0,
        'LOW':1,
        'MEDIUM':2,
        'HIGH':3
    }
    
    result_dict={}
    
    for activity in {"VERY_LOW","LOW","MEDIUM","HIGH"}:
        result_dict[activity]={}
        
        for entropy in {"VERY_LOW","LOW","MEDIUM","HIGH"}:
            result_dict[activity][entropy]={}
            
            activity_num=inverse_map[activity]
            entropy_num=inverse_map[entropy]
            
            result_dict[activity][entropy]['accuracy']=a[activity_num][entropy_num]
            result_dict[activity][entropy]['precision']=p[activity_num][entropy_num]
            result_dict[activity][entropy]['recall']=r[activity_num][entropy_num]
            result_dict[activity][entropy]['f1']=f[activity_num][entropy_num]
    
    with open(file_name,"w") as f:
        json.dump(result_dict,f,indent=4)
    f.close()

def KFoldCrossValidation(model,paths,iterations,options=[]):
    """
    Performs cross-validation k-fold
    """

    metric_list=[]

    matrix_rec=[[0,0,0,0] for i in range(4)]
    matrix_prec=[[0,0,0,0] for i in range(4)]
    matrix_acc=[[0,0,0,0] for i in range(4)]
    matrix_f1=[[0,0,0,0] for i in range(4)]

    inverse_map={
        'VERY_LOW':0,
        'LOW':1,
        'MEDIUM':2,
        'HIGH':3
    }

    flags=getFlagsFromList(options)

    for p in paths:
    
        print(f"Loading data from {p}")

        curr_metrics={
            'accuracy':0,
            'precision':0,
            'recall':0,
            'f1':0
        }
        
        meta=None
        
        for i in range(0,iterations-1):
        
            model.get_untrained_model()
            new_meta=model.load_data(p)
            
            if not new_meta is None:
                meta=new_meta
            
            activity=meta[0].removeprefix('ACTIVITY=')
            entropy=meta[1].removeprefix('ENTROPY=')
            activity=inverse_map[activity]
            entropy=inverse_map[entropy]
            
            counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
            if not flags['Architectural']:
                counters_to_drop=[]
    
            model.set_hyperparameter_counters(counters_to_drop)

            x_train,x_test,y_train,y_test=model.load_ith_fold(i,iterations)

            if flags['Poly']:
                x_train,x_test=model.polynomial_feature_preproc(3,x_train,x_test,None,None)
            
            if flags['BoxCox']:
                print(f"Applying BoxCox with {BoxCox_lambda}")
                x_train,x_test=model.BoxCox_preproc(x_train,x_test,BoxCox_lambda)
                print("End preprocessing")
                
            model.train(x_train,y_train)
            print("TRAINED!")
            y_predicted=model.test(x_test)
            print("PREDICTED!")
            metrics=model.report(y_predicted,y_test,verbose=2)
            curr_metrics['accuracy']+=metrics['accuracy']
            curr_metrics['precision']+=metrics['precision']
            curr_metrics['recall']+=metrics['recall']
            curr_metrics['f1']+=metrics['f1']
        
        curr_metrics['accuracy']=curr_metrics['accuracy']/iterations
        curr_metrics['precision']=curr_metrics['precision']/iterations
        curr_metrics['recall']=curr_metrics['recall']/iterations
        curr_metrics['f1']=curr_metrics['f1']/iterations
        metric_list.append(curr_metrics)

        matrix_acc[activity][entropy]=curr_metrics['accuracy']
        matrix_prec[activity][entropy]=curr_metrics['precision']
        matrix_rec[activity][entropy]=curr_metrics['recall']
        matrix_f1[activity][entropy]=curr_metrics['f1']
    
    return matrix_acc, matrix_prec, matrix_rec, matrix_f1

def hyperparam_select_lambda(model_type,values):
    
    global BoxCox_lambda

    SelectModel(model_type)
    
    options=['Architectural','BoxCox']
    k=10
    
    for v in values:
        BoxCox_lambda=v
        
        print(f"Testing lambda={v}")
        a,p,r,f=KFoldCrossValidation(model,all_paths,k,options)
        
        print(f)
        
        SaveResults(f'Results/CrossValidation/{model_type}_Architectural_BoxCox_{v}.json',a,p,r,f)

def TestAllDatasets(model,paths,iterations,options=[]):
    """
    Test all the datasets in paths for iterations times using the input model
    and the set flags.
    """

    metric_list=[]

    matrix_rec=[[0,0,0,0] for i in range(4)]
    matrix_prec=[[0,0,0,0] for i in range(4)]
    matrix_acc=[[0,0,0,0] for i in range(4)]
    matrix_f1=[[0,0,0,0] for i in range(4)]

    inverse_map={
        'VERY_LOW':0,
        'LOW':1,
        'MEDIUM':2,
        'HIGH':3
    }

    flags=getFlagsFromList(options)

    for p in paths:

        curr_metrics={
            'accuracy':0,
            'precision':0,
            'recall':0,
            'f1':0
        }
        
        meta=None
        
        for i in range(0,iterations):
            model.get_untrained_model()
            new_meta=model.load_data(p)
            
            if not new_meta is None:
                meta=new_meta
            
            activity=meta[0].removeprefix('ACTIVITY=')
            entropy=meta[1].removeprefix('ENTROPY=')
            activity=inverse_map[activity]
            entropy=inverse_map[entropy]
            
            counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
            if not flags['Architectural']:
                counters_to_drop=[]
    
            model.set_hyperparameter_counters(counters_to_drop)

            x_train,x_test,y_train,y_test=model.preprocess(standardize=False)

            if flags['Poly']:
                x_train,x_test=model.polynomial_feature_preproc(3,x_train,x_test,None,None)
            
            if flags['BoxCox']:
                x_train,x_test=model.BoxCox_preproc(x_train,x_test,BoxCox_lambda)

            model.train(x_train,y_train)
            y_predicted=model.test(x_test)

            metrics=model.report(y_predicted,y_test)
            curr_metrics['accuracy']+=metrics['accuracy']
            curr_metrics['precision']+=metrics['precision']
            curr_metrics['recall']+=metrics['recall']
            curr_metrics['f1']+=metrics['f1']
        
        curr_metrics['accuracy']=curr_metrics['accuracy']/iterations
        curr_metrics['precision']=curr_metrics['precision']/iterations
        curr_metrics['recall']=curr_metrics['recall']/iterations
        curr_metrics['f1']=curr_metrics['f1']/iterations
        metric_list.append(curr_metrics)

        matrix_acc[activity][entropy]=curr_metrics['accuracy']
        matrix_prec[activity][entropy]=curr_metrics['precision']
        matrix_rec[activity][entropy]=curr_metrics['recall']
        matrix_f1[activity][entropy]=curr_metrics['f1']
    
    return matrix_acc, matrix_prec, matrix_rec, matrix_f1
           
def test_BoxCox(model_type,values):
    
    global BoxCox_lambda

    SelectModel(model_type)
    
    options=['Architectural','BoxCox']
    k=10
    
    for v in values:
        BoxCox_lambda=v
        
        print(f"Testing lambda={v}")
        
        a,p,r,f=TestAllDatasets(model,all_paths,k,options)
        
        print(f)
        
        SaveResults(f'Results/TestScores/{model_type}_Architectural_BoxCox_{v}.json',a,p,r,f)

model_type='SVM'

values=[ 0.1,0.3]

test_BoxCox(model_type,values)
