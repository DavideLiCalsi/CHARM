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

warnings.filterwarnings('ignore')


def getFlagsFromList(l):

    flags={
        'BoxCox':False,
        'Architectural':False,
        'Poly':False
    }

    for f in flags.keys():
        if f in l:
            flags[f]=True
    
    return flags


# Set the argument parser
parser = argparse.ArgumentParser(
    description="Feed data to a Machine Learning model"
)

parser.add_argument(
    '--data_path',
    type=str,
    default='dataset/IS_data.csv',
    help='Path to the csv file containing the data to process.'
)

parser.add_argument(
    '--model',
    type=str,
    default='DecisionTree',
    help='Model to use'
)

parser.add_argument(
    '--log_path',
    type=str,
    default='log.txt',
    help='Where to output the results'
)

args=parser.parse_args()
data_path = args.data_path
model_type=args.model
log_path=args.log_path
model=None

if model_type.upper() == 'LOGISTIC':
    model=LogisticModel()
elif model_type.upper() == 'DECISIONTREE':
    model=DecisionTreeModel()
elif model_type.upper() == 'SVM':
    model=SVM()

assert model is not None, f"You specified an invalid model"

def Test_PCA_Drop_Col(model):

    model.get_untrained_model()
    meta=model.load_data(data_path)

    metaparameters=""

    for m in meta:
        if not pd.isna(m) :
            metaparameters+=str(m) + ","


    model.set_hyperparameter_counters([])
    x_train,x_test,y_train,y_test=model.preprocess()

    ###### Using PCA ######
    kept_variance=0.95

    accuracies=[]
    precisions=[]
    recalls=[]

    while kept_variance>=0.7:
        model.get_untrained_model()
        x_train_pca,x_test_pca=model.applyPCA(x_train,x_test,kept_variance)
        print("\nAPPLYING PCA...Variance kept:",round(kept_variance,2),"Columns kept",x_train_pca.shape[1],"/",x_train.shape[1])
        model.train(x_train_pca,y_train)
        y_predicted=model.test(x_test_pca)
        metrics=model.report(y_predicted,y_test)
        accuracies.append((kept_variance, round(metrics['accuracy'],4) ))
        precisions.append((kept_variance, round(metrics['precision'],4 ) ))
        recalls.append((kept_variance, round(metrics['recall'],4) ))
        kept_variance-=0.05

    ### Let's try dropping some columns ###

    print("\nPERFORMANCE WITH ALL COUNTERS\n")
    print(x_train)
    to_drop=[]
    x_train=x_train.drop(labels=to_drop,axis=1)
    x_test=x_test.drop(labels=to_drop,axis=1)
    model.train(x_train,y_train)
    y_predicted=model.test(x_test)
    metrics=model.report(y_predicted,y_test)
    plotter=Plotter(Path('figures').joinpath(metaparameters.replace(',','_')).joinpath(model_type))
    plotter.PlotAccPrecRec(accuracies,precisions,recalls,model_type,metaparameters)


    accuracy=round(metrics['accuracy'],4)
    precision=round(metrics['precision'],4)
    recall=round(metrics['recall'],4)

    log_file=open(log_path,'a')

    log_entry="("+metaparameters+model_type+","+str(accuracy)+ ","+str(precision)+ ","+str(recall)+ ""+")\n"
    log_file.write(log_entry)
    log_file.close()

    accuracies=[]
    precisions=[]
    recalls=[]
    counters_dropped=[]

    for i in range(1,3):
        res=model.test_drop_columns(i,x_train,x_test,y_train,y_test)

        precisions= [ x[1]['precision'] for x in res ]
        recalls= [ x[1]['recall'] for x in res ]
        accuracies= [ x[1]['accuracy'] for x in res ]
        counters_dropped =  [x[0] for x in res]
        counters_itegers=[ plotter.counterSubset2Int(x,x_train.columns) for x in counters_dropped]

        accuracies.sort()
        precisions.sort()
        recalls.sort()

        plotter.plotCountersDropSample(accuracies,precisions,recalls,counters_itegers,i,0.20,model_type,metaparameters)

        for r in res:
            model.save_to_log('./log.txt',model_type,metaparameters+str(r[0]),r[1])

def Test_PCA_Architectural(model):

    model.get_untrained_model()
    meta=model.load_data(data_path)

    metaparameters=""

    for m in meta:
        if not pd.isna(m) :
            metaparameters+=str(m) + ","


    model.set_hyperparameter_counters(['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME'])
    x_train,x_test,y_train,y_test=model.preprocess()

    ###### Using PCA ######
    kept_variance=0.95

    accuracies=[]
    precisions=[]
    recalls=[]

    while kept_variance>=0.7:
        x_train_pca,x_test_pca=model.applyPCA(x_train,x_test,kept_variance)
        print("\nAPPLYING PCA...Variance kept:",round(kept_variance,2),"Columns kept",x_train_pca.shape[1],"/",x_train.shape[1])
        model.train(x_train,y_train)
        y_predicted=model.test(x_test)
        metrics=model.report(y_predicted,y_test)
        accuracies.append((kept_variance, round(metrics['accuracy'],4) ))
        precisions.append((kept_variance, round(metrics['precision'],4 ) ))
        recalls.append((kept_variance, round(metrics['recall'],4) ))
        kept_variance-=0.05

    ### Let's try dropping some columns ###

    print("\nPERFORMANCE WITH ALL COUNTERS\n")
    print(x_train)
    to_drop=[]
    x_train=x_train.drop(labels=to_drop,axis=1)
    x_test=x_test.drop(labels=to_drop,axis=1)
    model.train(x_train,y_train)
    y_predicted=model.test(x_test)
    metrics=model.report(y_predicted,y_test)
    plotter=Plotter(Path('figures').joinpath(metaparameters.replace(',','_')).joinpath(model_type).joinpath('arch_only'))
    plotter.PlotAccPrecRec(accuracies,precisions,recalls,model_type,metaparameters)


    accuracy=round(metrics['accuracy'],4)
    precision=round(metrics['precision'],4)
    recall=round(metrics['recall'],4)

    log_file=open(log_path,'a')

    log_entry="("+metaparameters+model_type+","+str(accuracy)+ ","+str(precision)+ ","+str(recall)+ ""+")\n"
    log_file.write(log_entry)
    log_file.close()

    accuracies=[]
    precisions=[]
    recalls=[]
    counters_dropped=[]

    for i in range(1,3):
        res=model.test_drop_columns(i,x_train,x_test,y_train,y_test)

        precisions= [ x[1]['precision'] for x in res ]
        recalls= [ x[1]['recall'] for x in res ]
        accuracies= [ x[1]['accuracy'] for x in res ]
        counters_dropped =  [x[0] for x in res]
        counters_itegers=[ plotter.counterSubset2Int(x,x_train.columns) for x in counters_dropped]

        accuracies.sort()
        precisions.sort()
        recalls.sort()

        plotter.plotCountersDropSample(accuracies,precisions,recalls,counters_itegers,i,0.20,model_type,metaparameters)

        for r in res:
            model.save_to_log('./log.txt',model_type,metaparameters+str(r[0]),r[1])

def Test_PCA_LSU_CYCCnt(model):

    model.get_untrained_model()
    meta=model.load_data(data_path)

    metaparameters=""

    for m in meta:
        if not pd.isna(m) :
            metaparameters+=str(m) + ","


    model.set_hyperparameter_counters(['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','DWT_CPICNT', 'DWT_EXCCNT',  'DWT_SLEEPCNT','DWT_FOLDCNT'])
    x_train,x_test,y_train,y_test=model.preprocess()

    ###### Using PCA ######
    kept_variance=0.95

    accuracies=[]
    precisions=[]
    recalls=[]

    while kept_variance>=0.7:
        model.get_untrained_model()
        x_train_pca,x_test_pca=model.applyPCA(x_train,x_test,kept_variance)
        print("\nAPPLYING PCA...Variance kept:",round(kept_variance,2),"Columns kept",x_train_pca.shape[1],"/",x_train.shape[1])
        model.train(x_train,y_train)
        y_predicted=model.test(x_test)
        metrics=model.report(y_predicted,y_test)
        accuracies.append((kept_variance, round(metrics['accuracy'],4) ))
        precisions.append((kept_variance, round(metrics['precision'],4 ) ))
        recalls.append((kept_variance, round(metrics['recall'],4) ))
        kept_variance-=0.05

    ### Let's try dropping some columns ###

    print("\nPERFORMANCE WITH ALL COUNTERS\n")
    print(x_train)
    to_drop=[]
    x_train=x_train.drop(labels=to_drop,axis=1)
    x_test=x_test.drop(labels=to_drop,axis=1)
    model.train(x_train,y_train)
    y_predicted=model.test(x_test)
    metrics=model.report(y_predicted,y_test)
    plotter=Plotter(Path('figures').joinpath(metaparameters.replace(',','_')).joinpath(model_type).joinpath('minimal'))
    plotter.PlotAccPrecRec(accuracies,precisions,recalls,model_type,metaparameters)


    accuracy=round(metrics['accuracy'],4)
    precision=round(metrics['precision'],4)
    recall=round(metrics['recall'],4)

    log_file=open(log_path,'a')

    log_entry="("+metaparameters+model_type+","+str(accuracy)+ ","+str(precision)+ ","+str(recall)+ ""+")\n"
    log_file.write(log_entry)
    log_file.close()

    accuracies=[]
    precisions=[]
    recalls=[]
    counters_dropped=[]

    for i in range(1,1):
        res=model.test_drop_columns(i,x_train,x_test,y_train,y_test)

        precisions= [ x[1]['precision'] for x in res ]
        recalls= [ x[1]['recall'] for x in res ]
        accuracies= [ x[1]['accuracy'] for x in res ]
        counters_dropped =  [x[0] for x in res]
        counters_itegers=[ plotter.counterSubset2Int(x,x_train.columns) for x in counters_dropped]

        accuracies.sort()
        precisions.sort()
        recalls.sort()

        plotter.plotCountersDropSample(accuracies,precisions,recalls,counters_itegers,i,0.20,model_type,metaparameters)

        for r in res:
            model.save_to_log('./log.txt',model_type,metaparameters+str(r[0]),r[1])

def const_malware(model,paths):
    # CHECK THE EFFECT OF KEEPING A CONST. SIZE MALWARE

    metric_list=[]
    model.get_untrained_model()
    metaparameters=""
    
    for p in paths:
        meta=model.load_data(p)

        if metaparameters=="":
            for m in meta:
                if not pd.isna(m) :
                    metaparameters+=str(m) + ","
        
        model.set_hyperparameter_counters([])
        x_train,x_test,y_train,y_test=model.preprocess()
        model.train(x_train,y_train)
        y_predicted=model.test(x_test)
        metrics=model.report(y_predicted,y_test)
        metric_list.append(metrics)

    print(metric_list)
    plotter=Plotter(Path('figures').joinpath(metaparameters.replace(',','_')).joinpath(model_type))
    plotter.PlotConstMalwareSize(metric_list,model_type,metaparameters)

def Test_Counter_Contributions(model,paths,model_name,architectural=True):

    importance_dict = {i:[] for i in range(0,11)}

    for p in paths:
        model.get_untrained_model()
        meta=model.load_data(p)

        metaparameters=""

        for m in meta:
            if not pd.isna(m) :
                metaparameters+=str(m) + ","

        counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
        if not architectural:
            counters_to_drop=[]
    
        model.set_hyperparameter_counters(counters_to_drop)
        x_train,x_test,y_train,y_test=model.preprocess()

        ### First compute the reference metrics ###
        to_drop=[]
        x_train=x_train.drop(labels=to_drop,axis=1)
        x_test=x_test.drop(labels=to_drop,axis=1)
        
        if model.__class__ == LogisticModel:
            model.train(x_train,y_train,True)
        else:
            model.train(x_train,y_train)
    
        importance = model.get_importance()

        for i,c in enumerate(importance):
            importance_dict[i].append(c)


    print(importance_dict)

    averages={i:np.average(importance_dict[i]) for i in importance_dict}
    stds={i:np.std(importance_dict[i]) for i in importance_dict}

    for c in averages:
        print(c,"- Mean:",averages[c], " - Std:",stds[c])
    
    plt=Plotter('figures')

    plt.PlotImportances(averages,stds,x_train.columns,model_name)

    averages={k: v for k, v in sorted(averages.items(), key=lambda item: item[1],reverse=True)}

    return averages

def TestKeepImportantCounters(logistic_imp,dt_imp,svm_imp,counters,paths):

    m1=LogisticModel()
    m2=DecisionTreeModel()
    m3=SVM()
    recalls=[]

    for n in range(1,11):
        logistic_keep_counters=list(logistic_imp.keys())[0:n]
        dt_keep_counters=list(dt_imp.keys())[0:n]
        svm_keep_counters=list(svm_imp.keys())[0:n]

        logisic_recall=0
        dt_recall=0
        svm_recall=0
        logistic_precision=0

        for p in paths:
            m1.get_untrained_model()
            m2.get_untrained_model()
            m3.get_untrained_model()

            m1.load_data(p)
            m2.load_data(p)
            m3.load_data(p)

            svm_keep_str=[]
            logistic_keep_str=[]
            dt_keep_str=[]

            for i in range(0,n):
                svm_keep_str.append( counters[svm_keep_counters[i]] )
                dt_keep_str.append( counters[logistic_keep_counters[i]] )
                logistic_keep_str.append( counters[dt_keep_counters[i]] )
    
            svm_drop=[c for c in counters if c not in svm_keep_str]
            logistic_drop=[c for c in counters if c not in logistic_keep_str]
            dt_drop=[c for c in counters if c not in dt_keep_str]
            print(logistic_drop,dt_drop,svm_drop)

            m1.set_hyperparameter_counters(logistic_drop)
            m2.set_hyperparameter_counters(dt_drop)
            m3.set_hyperparameter_counters(svm_drop)

            x_train1,x_test1,y_train1,y_test1 = m1.preprocess()
            x_train2,x_test2,y_train2,y_test2 = m2.preprocess()
            x_train3,x_test3,y_train3,y_test3 = m3.preprocess()

            print(m1)
            m1.train(x_train1,y_train1)
            m2.train(x_train2,y_train2)
            m3.train(x_train3,y_train3)

            pred_logistic=m1.test(x_test1)
            pred_dt=m2.test(x_test2)
            pred_svm=m3.test(x_test3)

            metrics_logistic=m1.report(pred_logistic,y_test1)
            metrics_dt=m2.report(pred_dt,y_test2)
            metrics_svm=m3.report(pred_svm,y_test3)

            logisic_recall+=metrics_logistic['recall']
            dt_recall+=metrics_dt['recall']
            svm_recall+=metrics_svm['recall']
            logistic_precision+=metrics_logistic['precision']
        
        logisic_recall = logisic_recall/len(paths)
        logistic_precision = logistic_precision/len(paths)

        dt_recall = dt_recall/len(paths)
        svm_recall = svm_recall/len(paths)

        recalls+=[logisic_recall,dt_recall,svm_recall]

    plt=Plotter('figures')

    plt.PlotImportantCounters(recalls,['Logistic','DecisionTree','SVM'])
            
def Test_PrecvsRecall(logistic,decisiontree,svm,paths):
    logistic.get_untrained_model()
    decisiontree.get_untrained_model()
    svm.get_untrained_model()

    precisions=[]
    recalls=[]
    plotter=Plotter('figures')
    for p in paths:
        logistic.get_untrained_model()
        decisiontree.get_untrained_model()
        svm.get_untrained_model()
        meta=logistic.load_data(p)
        decisiontree.load_data(p)
        svm.load_data(p)

        x_train,x_test,y_train,y_test=logistic.preprocess()
        logistic.train(x_train,y_train)
        y_predicted=logistic.test(x_test)
        metrics=logistic.report(y_predicted,y_test)

        precisions.append(metrics['precision'])
        recalls.append(metrics['recall'])

        x_train,x_test,y_train,y_test=decisiontree.preprocess()
        decisiontree.train(x_train,y_train)
        y_predicted=decisiontree.test(x_test)
        metrics=decisiontree.report(y_predicted,y_test)

        precisions.append(metrics['precision'])
        recalls.append(metrics['recall'])

        x_train,x_test,y_train,y_test=svm.preprocess()
        svm.train(x_train,y_train)
        y_predicted=svm.test(x_test)
        metrics=svm.report(y_predicted,y_test)

        precisions.append(metrics['precision'])
        recalls.append(metrics['recall'])

    plotter.PlotPrecRecallScatter(precisions,recalls)


def InteractionPlotCounter(model,dropped_model,paths,counters):

    recalls={c:[] for c in counters}
    dropped_recalls={c:[] for c in counters}
    plotter=Plotter('figures')

    for removed_counter in counters:
        for p in paths:

            for i in range(10):
                model.get_untrained_model()
                dropped_model.get_untrained_model()

                meta=model.load_data(p)

                x_train,x_test,y_train,y_test=model.preprocess()
                model.train(x_train,y_train)
                y_predicted=model.test(x_test)
                metrics=model.report(y_predicted,y_test)

                recalls[removed_counter].append(metrics['recall'])

                meta=dropped_model.load_data(p)

                dropped_model.set_hyperparameter_counters([removed_counter])

                x_train,x_test,y_train,y_test=dropped_model.preprocess()
                dropped_model.train(x_train,y_train)
                y_predicted=dropped_model.test(x_test)
                metrics=dropped_model.report(y_predicted,y_test)

                dropped_recalls[removed_counter].append(metrics['recall'])

    plotter.PlotInteraction(dropped_recalls,recalls)

def TestAllDatasets(model,paths,iterations,options=[]):

    metric_list=[]

    matrix_rec=[[0,0,0,0] for i in range(4)]
    matrix_prec=[[0,0,0,0] for i in range(4)]
    matrix_acc=[[0,0,0,0] for i in range(4)]

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
            'recall':0
        }
        for i in range(0,iterations):
            model.get_untrained_model()
            meta=model.load_data(p)
            activity=meta[0].removeprefix('ACTIVITY=')
            entropy=meta[1].removeprefix('ENTROPY=')
            activity=inverse_map[activity]
            entropy=inverse_map[entropy]

            print(meta)
            
            counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
            if not flags['Architectural']:
                counters_to_drop=[]
    
            model.set_hyperparameter_counters(counters_to_drop)

            x_train,x_test,y_train,y_test=model.preprocess()

            if flags['Poly']:
                x_train,x_test=model.polynomial_feature_preproc(3,x_train,x_test,None,None)
            
            if flags['BoxCox']:
                x_train,x_test=model.BoxCox_preproc(x_train,x_test,3)

            model.train(x_train,y_train)
            y_predicted=model.test(x_test)
            metrics=model.report(y_predicted,y_test)
            curr_metrics['accuracy']+=metrics['accuracy']
            curr_metrics['precision']+=metrics['precision']
            curr_metrics['recall']+=metrics['recall']
        
        curr_metrics['accuracy']=curr_metrics['accuracy']/iterations
        curr_metrics['precision']=curr_metrics['precision']/iterations
        curr_metrics['recall']=curr_metrics['recall']/iterations
        metric_list.append(curr_metrics)

        print(curr_metrics)
        matrix_acc[activity][entropy]=curr_metrics['accuracy']
        matrix_prec[activity][entropy]=curr_metrics['precision']
        matrix_rec[activity][entropy]=curr_metrics['recall']
    
    plotter=Plotter( Path('figures') )

    #plotter.PlotAllDatasets(metric_list,paths,model_type,architectural)
    plotter.PlotAllDatasetsMatrix(matrix_acc,matrix_prec,matrix_rec,model_type,options)

def TestAugmentationDatasets(model,paths,iterations,factor,architectural=False,always_augment=True):

    metric_list=[]

    matrix_rec=[[0,0,0,0] for i in range(4)]
    matrix_prec=[[0,0,0,0] for i in range(4)]
    matrix_acc=[[0,0,0,0] for i in range(4)]

    inverse_map={
        'VERY_LOW':0,
        'LOW':1,
        'MEDIUM':2,
        'HIGH':3
    }

    x_train=None

    for p in paths:

        curr_metrics={
            'accuracy':0,
            'precision':0,
            'recall':0
        }
        for i in range(0,iterations):
            model.get_untrained_model()
            meta=model.load_data(p)

            activity=meta[0].removeprefix('ACTIVITY=')
            entropy=meta[1].removeprefix('ENTROPY=')
            activity=inverse_map[activity]
            entropy=inverse_map[entropy]
            
            counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
            if not architectural:
                print("Using all counters")
                counters_to_drop=[]
    
            model.set_hyperparameter_counters(counters_to_drop)

            if always_augment or x_train is None:
                
                x_train,x_test,y_train,y_test=model.preprocess()

                if model_type=='SVM':
                    y_train_list=y_train.values

                augmented=[]
                y_augmented=[]
                for index,record in x_train.iterrows():

                    if y_train[index]==1:
                        for i in range(factor):
                            augmented.append(model.augment(record,0.000001,distribution='gaussian'))
                            y_augmented.append(y_train[index])
            
            
                augmented=pd.DataFrame(augmented)
                y_augmented=pd.DataFrame(y_augmented)
                x_train=pd.concat([x_train,augmented],axis=0)
                y_train=pd.concat([y_train,y_augmented],axis=0)
            #return


            model.train(x_train,y_train)
            y_predicted=model.test(x_test)
            metrics=model.report(y_predicted,y_test)
            curr_metrics['accuracy']+=metrics['accuracy']
            curr_metrics['precision']+=metrics['precision']
            curr_metrics['recall']+=metrics['recall']
        
        curr_metrics['accuracy']=curr_metrics['accuracy']/iterations
        curr_metrics['precision']=curr_metrics['precision']/iterations
        curr_metrics['recall']=curr_metrics['recall']/iterations
        metric_list.append(curr_metrics)

        matrix_acc[activity][entropy]=curr_metrics['accuracy']
        matrix_prec[activity][entropy]=curr_metrics['precision']
        matrix_rec[activity][entropy]=curr_metrics['recall']
    
    plotter=Plotter( Path('figures') )

    #plotter.PlotAllDatasets(metric_list,paths,model_type,architectural)
    plotter.PlotAllDatasetsMatrix(matrix_acc,matrix_prec,matrix_rec,model_type,['DataAugmentation_'+str(factor)])

def TestPCAImportance(model,paths):

    ###### Using PCA ######
    kept_variance=0.95

    if model_type != 'SVM':
        importances=pd.DataFrame(columns=['DWT_LSUCNT','DWT_CPICNT','DWT_EXCCNT','DWT_CYCCNT','DWT_SLEEPCNT','DWT_FOLDCNT','LSU_stim','CPI_stim','FLD_stim','EXC_stim','TIME'])
    else:
        cols=list(range(len(['DWT_LSUCNT','DWT_CPICNT','DWT_EXCCNT','DWT_CYCCNT','DWT_SLEEPCNT','DWT_FOLDCNT','LSU_stim','CPI_stim','FLD_stim','EXC_stim','TIME'])))
        importances=pd.DataFrame(columns=cols)

    
    for p in paths:

        kept_variance=0.95
        while kept_variance>=0.95:
            model.get_untrained_model()
            model.load_data(p)
            x_train,x_test,y_train,y_test=model.preprocess()
            
            curr_importance=model.importancePCA(x_train,kept_variance)
            importances=pd.concat([importances,curr_importance],axis=0)
            kept_variance-=0.05

    print(importances)
    
    
    averages=importances.mean(0).to_dict()#np.average(importances,0)
    stds=np.std(importances,0).to_dict()
    print("AVG",averages,"STD",stds)
    Plotter('figure').PlotImportances(averages,stds,x_train.columns,model_type)
    return averages

def Test_Merge_Datasets(model,paths,iterations,options):

    data=None
    curr_metrics={
            'accuracy':0,
            'precision':0,
            'recall':0
        }
    flags=getFlagsFromList(options)

    for p in paths:

        if data is None:
            data=pd.read_csv(p,error_bad_lines=False)
        else:
            print("Loading ",p)
            next=pd.read_csv(p,error_bad_lines=False)
            data=pd.concat([data,next])
    
    for i in range(iterations):
        model.get_untrained_model()
        model.load_dataframe(data)

        counters_to_drop=['LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim','TIME']
        if not flags['Architectural']:
            counters_to_drop=[]
        

        if flags['Poly']:
            x_train,x_test=model.polynomial_feature_preproc(3,x_train,x_test,None,None)
            
        if flags['BoxCox']:
            x_train,x_test=model.BoxCox_preproc(x_train,x_test,3)
    
        model.set_hyperparameter_counters(counters_to_drop)

        x_train,x_test,y_train,y_test=model.preprocess()

        model.train(x_train,y_train)
        y_predicted=model.test(x_test)
        metrics=model.report(y_predicted,y_test)
        curr_metrics['accuracy']+=metrics['accuracy']
        curr_metrics['precision']+=metrics['precision']
        curr_metrics['recall']+=metrics['recall']
    
    curr_metrics['accuracy']=curr_metrics['accuracy']/iterations
    curr_metrics['precision']=curr_metrics['precision']/iterations
    curr_metrics['recall']=curr_metrics['recall']/iterations

    print(curr_metrics)


# Organizing tests

# Useful data-structs
all_paths=['dataset/Data_21Lug2022.csv','dataset/Data_20Lug2022.csv','dataset/Data_19Lug2022.csv','dataset/Data_18Lug2022.csv','dataset/Data_17Lug2022.csv','dataset/Data_20Giu2022.csv','dataset/Data_12Giu2022.csv','dataset/Data_13Giu2022.csv','dataset/Data_05Giu2022.csv','dataset/Data_04Giu2022.csv','dataset/Data_18Giu2022.csv','dataset/Data_31Mag2022.csv']
counters=['DWT_LSUCNT','DWT_CPICNT','DWT_EXCCNT','DWT_CYCCNT','DWT_SLEEPCNT','DWT_FOLDCNT','LSU_stim','CPI_stim','FLD_stim','EXC_stim','TIME']

all_paths=['dataset/Data_31Lug2022.csv','dataset/Data_30Lug2022.csv','dataset/Data_29Lug2022.csv','dataset/Data_27Lug2022.csv','dataset/Data_26Lug2022.csv','dataset/Data_25Lug2022.csv','dataset/Data_24Lug2022.csv','dataset/Data_23Lug2022.csv','dataset/Data_22Lug2022.csv','dataset/Data_21Lug2022.csv','dataset/Data_20Lug2022.csv','dataset/Data_19Lug2022.csv','dataset/Data_18Lug2022.csv','dataset/Data_17Lug2022.csv','dataset/Data_12Giu2022.csv','dataset/Data_18Giu2022.csv']
options=['Architectural']

TestAllDatasets(model,all_paths,10,options)
exit(0)

# Test the model after performing data-augmentation
# TestAugmentationDatasets(model,all_paths,3,14,True)
# exit(0)

Test_Merge_Datasets(model,all_paths,10,options)
exit(0)

options.append('Poly')
TestAllDatasets(model,all_paths,10,options)
exit(0)

logistic_importances=Test_Counter_Contributions(LogisticModel(),all_paths,'Logistic',False)
dt_importances=Test_Counter_Contributions(DecisionTreeModel(),all_paths,'DecisionTree',False)
svm_importances=Test_Counter_Contributions(SVM(),all_paths,'SVM',False)
TestKeepImportantCounters(logistic_importances,dt_importances,svm_importances,counters,all_paths)
exit(0)

# Test the model after performing data-augmentation
TestAugmentationDatasets(model,all_paths,4,3,True)
exit(0)

#options.append('BoxCox')
TestAllDatasets(model,all_paths,10,options)
exit(0)

# Test the model after performing data-augmentation
TestAugmentationDatasets(model,all_paths,4,11,True)
exit(0)

# Compute the importance of each counter for each model and test what happens if you drop the
# n most important ones
logistic_importances=Test_Counter_Contributions(LogisticModel(),all_paths,'Logistic',False)
dt_importances=Test_Counter_Contributions(DecisionTreeModel(),all_paths,'DecisionTree',False)
svm_importances=Test_Counter_Contributions(SVM(),all_paths,'SVM',False)
TestKeepImportantCounters(logistic_importances,dt_importances,svm_importances,counters,all_paths)
exit(0)

# Compute the importance of each counter using PCA and test what happens if you drop the
# n most important ones
importances=TestPCAImportance(model,all_paths)
TestKeepImportantCounters(importances,importances,importances,counters,all_paths)
exit(0)


#const_malware(model,paths)
logistic_importances=Test_Counter_Contributions(model,all_paths,False)
dt_importances=Test_Counter_Contributions(DecisionTreeModel(),all_paths,False)
svm_importances=Test_Counter_Contributions(SVM(),all_paths,False)
TestKeepImportantCounters(logistic_importances,dt_importances,svm_importances,counters,paths)
exit()
#Test_PrecvsRecall(LogisticModel(),DecisionTreeModel(),SVM(),all_paths)
#Test_PCA_Drop_Col(model)

if model_type.upper() == 'LOGISTIC':
    d_model=LogisticModel()
elif model_type.upper() == 'DECISIONTREE':
    d_model=DecisionTreeModel()
elif model_type.upper() == 'SVM':
    d_model=SVM()

InteractionPlotCounter(model,d_model,all_paths,counters)

