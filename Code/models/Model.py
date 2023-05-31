import itertools
from abc import abstractmethod
from cmath import log
import math
from random import Random, randint, random, gauss
from unittest import result

import numpy as np
import pandas as pd
from sympy import Mod

from sklearn.decomposition import PCA
from sklearn.metrics import (accuracy_score, confusion_matrix, precision_score,
                             recall_score,f1_score)
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.preprocessing import FunctionTransformer, PolynomialFeatures
from sklearn.feature_selection import SequentialFeatureSelector


class Model:
    """
    A class that represents a Machine Learning model.
    """

    def __init__(self) -> None:
        self.model=None
        self.dataset=None
        self.data_path=None
        self.x_test=None
        self.y_test=None
        self.x_train=None
        self.y_train=None
        pass

    def load_data(self,data_path:str)->pd.DataFrame:
        """
        Loads data in csv format and returns a pandas DataFrame
        """
        
        assert '.csv' in data_path, f"Data must be in csv format!"

        if self.data_path == data_path:
            return None

        dataset=pd.read_csv(data_path,error_bad_lines=False)
        metadata=dataset['METADATA'].to_list()[0:2]
        dataset=dataset.drop(labels=['METADATA'],axis=1)

        self.dataset=dataset
        metadata=[ x.upper() for x in metadata]
        return sorted(metadata)
    
    def load_dataframe(self,dataframe):
        self.dataset=dataframe.drop(labels=['METADATA'],axis=1)
    
    def load_ith_fold(self,i,k):

        if self.dataset is None:
            raise Exception("Dataset not loaded. Please load one.")
        
        size=len(self.dataset)
        fold_size=size//k

        if i==k-1 and size % fold_size != 0:
            fold_size = size % fold_size

        low=(i//k) * fold_size
        high=low+fold_size

        test_set=self.dataset.iloc[low:high]
        train_set= pd.concat( [self.dataset.iloc[0:low],self.dataset.iloc[high:]], axis=0 )

        y_train=train_set['LEGIT']
        y_test=test_set['LEGIT']
        x_train=train_set.drop(labels=['LEGIT'],axis=1)
        x_test=test_set.drop(labels=['LEGIT'],axis=1)

        return x_train,x_test,y_train,y_test
        


    def set_hyperparameter_counters(self,counters:list):
        """
        Selects which counters should be kept or dropped
        (i.e. the metaparameter "number and types of counters")

        :param counters: List of counter names to drop
        """

        if self.dataset is None:
            print("Please load a valid dataset first")
            raise TypeError

        self.dataset = self.dataset.drop(labels=counters,axis=1)

    def compute_metrics(self,y_predicted,y_true):
        """
        Computes some evaluation metrics from the predicted
        data.
        """

        metrics = {
            'accuracy':0,
            'precision':0,
            'recall':0,
            'f1':0,
            'confusion_matrix':None
        }

        metrics['accuracy']=accuracy_score(y_true,y_predicted)
        metrics['precision']=precision_score(y_true,y_predicted,zero_division=0)
        metrics['recall']=recall_score(y_true,y_predicted,zero_division=0)
        metrics['f1']=f1_score(y_true,y_predicted,zero_division=1)

        y_true_series = pd.Series(y_true, name='Actual')
        y_predicted_series = pd.Series(y_predicted, name='Predicted')
        metrics['confusion_matrix']=confusion_matrix(y_true_series, y_predicted_series)

        return metrics
    
    def report(self,y_predicted, y_true, verbose=0):
        """
        Computes and prints some useful metrics
        """

        metrics = self.compute_metrics(y_predicted,y_true)

        if verbose>0:
            print("ACCURACY: %.4f %%" % (metrics['accuracy']*100 ) )
            print("PRECISION %.4f %%" % (metrics['precision']*100) )
            print("RECALL: %.4f %%" % (metrics['recall']*100) )

        
        matrix=metrics['confusion_matrix']

        if verbose>1:
            print(matrix)
            return metrics
            print("\nCONFUSION MATRIX:")
            print("\n\t\tPREDICTED CLASSES")
            print("\t\tBenign\tMalicious")
            print('TRUE\tBenign\t',matrix[0][0],"\t",matrix[0][1])
            print('CLASSES\tMalicious',matrix[1][0],"\t",matrix[1][1])

        return metrics

    def augment(self,sample,percentage,distribution='uniform'):
        l = len(sample)
        noise=np.zeros(l,sample.dtype)

        for i in range(l):
            min= -5#int(-sample[i]*percentage)
            max= -min#int(sample[i]*percentage)


            if distribution=='uniform':
                noise[i] = randint(min,max)
            
            if distribution=='gaussian':
                noise[i]=round(gauss(0, math.sqrt(max) ))
            
            if sample[i]==0:
                noise[i]=0
        
        return sample+noise
    
    def polynomial_feature_preproc(self,deg,x_train,x_test,y_train,y_test):

        pol=PolynomialFeatures(deg)
        x_train=pol.fit_transform(x_train)
        x_test=pol.transform(x_test)

        return x_train,x_test
    
    def kfold_xval(self,x,y,repetitions=5):
        """
        Performs k-fold cross-validation and returns the
        mean score.
        """

        scores = cross_val_score(self.model,x,y,cv=repetitions)

        return scores.mean()

    @staticmethod
    def BoxCox_single(x,lam):
        """
        Implements the unidimensinal BoxCox transform on variable x with lambda parameter = lam
        """

        if x==0:
            return 0

        if lam==0:
            return log(x)
        else:
            return ( pow(x,lam)-1 )/lam

    def BoxCox_preproc(self,x_train, x_test,param,mode='spherical'):
        """
        This method implements the multivariate BoxCox transformation as
        illustrated here https://profs.sci.univr.it/~bicego/papers/2016_Neurocomputing.pdf.
        Two modes are available:
            -spherical: all the lambda parameters for the trasnformation are the same
            -diagonal: different lambda-param for each direction
        """

        if mode == 'spherical':
            lam=param # a single parameter is needed

            x_train=x_train.drop(labels=['DWT_SLEEPCNT'],axis=1)
            x_test=x_test.drop(labels=['DWT_SLEEPCNT'],axis=1)

            columns=x_train.columns

            for c in columns:

                x_train[c]=x_train[c].apply(lambda x: Model.BoxCox_single(x,lam))
                x_test[c]=x_test[c].apply(lambda x: Model.BoxCox_single(x,lam))
        
        return x_train,x_test

    
    def test_drop_columns(self,num_cols_dropped, x_train,x_test,y_train,y_test):
        """
        Trains and evaluates the model on the provided data by dropping all the 
        possible combinations of n (param num_cols_dropped) columns dropped.
        Returns a list of tuples (dropped_columns,metrics)
        """
        to_drop=[]
        columns=x_train.columns
        result=[]

        for to_drop in itertools.combinations(columns,num_cols_dropped):
            to_drop=list(to_drop)
            print("\n"+"#"*40)
            print("DROPPING COLUMNS",to_drop)
            x_train_new=x_train.drop(labels=to_drop,axis=1)
            x_test_new=x_test.drop(labels=to_drop,axis=1)

            self.train(x_train_new,y_train)
            y_predicted=self.test(x_test_new)
            metrics=self.report(y_predicted,y_test)
            result.append( (to_drop,metrics) )
        
        return result


    def applyPCA(self,x_train,x_test,variance_kept):
        pca=PCA(variance_kept)
        pca.fit(x_train)
        principal_components_train=pca.transform(x_train)
        principal_components_test=pca.transform(x_test)

        print(pca.components_)

        return principal_components_train,principal_components_test

    def importancePCA(self,x_train,variance_kept):
        pca=PCA(variance_kept)
        pca.fit(x_train)
        
        imp=pca.components_[0]
        cols=list(x_train.columns)
        print(pca.components_)

        df = {cols[i]:imp[i] for i in range(len(cols))}
        feature_importances=pd.DataFrame(df,index=[0])
        
        return feature_importances

    def feature_selection(self,direction='forward',tolerance=0.05):
        """
        Uses builtin sklearn methods to perform model-agnostic sequential feature selection
        """

        x=self.x_train
        y=self.y_train

        selector=SequentialFeatureSelector(
            estimator=self.model,
            n_features_to_select="auto",
            tol=tolerance,
            direction=direction,
            cv=10,
            scoring='recall'

        )

        selector.fit(x,y)

        feature_names=x.columns

        return selector.get_feature_names_out(feature_names)

    
    def save_to_log(self,log_path,model_type,metaparameters,metrics):
        log_file=open(log_path,'a')
        accuracy=round(metrics['accuracy'],4)
        precision=round(metrics['precision'],4)
        recall=round(metrics['recall'],4)

        log_entry="("+metaparameters+","+model_type+","+str(accuracy)+ ","+str(precision)+ ","+str(recall)+ ""+")\n"
        log_file.write(log_entry)
        log_file.close()

    @abstractmethod
    def get_untrained_model(self):
        pass
    
    @abstractmethod
    def preprocess(self,dataset):
        pass
    
    @abstractmethod
    def train(self,x_train,y_train):
        pass

    @abstractmethod
    def test(self,x_test,y_test):
        pass

    @abstractmethod
    def get_importance(self):
        pass
