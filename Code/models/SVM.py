from yaml import ScalarEvent
from models.Model import Model

import pandas as pd

from sklearn import svm
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA

import numpy as np
import math
from random import Random, randint, random, gauss

class SVM(Model):

    def get_untrained_model(self,C=1,kernel_type='linear'):

        if kernel_type == 'linear':
            model=svm.LinearSVC(C=C)
        else:
            model = svm.SVC(C=C,kernel=kernel_type,tol=1e-2)
        self.model=model

    def preprocess(self,standardize=True):
        dataset = self.dataset.fillna(0)
        cols=dataset.columns.drop(['LEGIT'])
        x = dataset[dataset.columns.drop(['LEGIT']) ]
        y = dataset['LEGIT']

        x_train, x_test, self.y_train, self.y_test = train_test_split(x,y, test_size=0.2, stratify=y)

        if standardize:
            scaler=StandardScaler()

            scaler.fit(x_train)
            self.x_train= pd.DataFrame( scaler.transform(x_train),columns=cols )
            self.x_test=pd.DataFrame( scaler.transform(x_test),columns=cols )
        else:
            self.x_train=x_train
            self.x_test=x_test

        #print(x_train)
        return (self.x_train,self.x_test,self.y_train,self.y_test)

    def train(self,x_train,y_train):
        self.model.fit(x_train,y_train)

    def test(self,x_test):
        return self.model.predict(x_test)
    
    def get_importance(self):
        coef=self.model.coef_[0]
        coef =[abs(x) for x in coef]
        return coef

    def importancePCA(self,x_train,variance_kept):
        pca=PCA(variance_kept)
        pca.fit(x_train)
        
        imp=pca.components_[0]
        cols=list(range(len(list(x_train.columns))))
        print(pca.components_)

        df = {cols[i]:imp[i] for i in range(len(cols))}
        feature_importances=pd.DataFrame(df,index=[0])
        
        return feature_importances
    
    def augment(self,sample,percentage,distribution='uniform'):
        l = len(sample)
        noise=np.zeros(l,sample.dtype)

        for i in range(l):
            min= -0.05 # int(-sample[i]*percentage)
            max= -min #int(sample[i]*percentage)


            if distribution=='uniform':
                noise[i] = random()*2*max+ min
            
            if distribution=='gaussian':
                noise[i]=round(gauss(0, math.sqrt(max) ))
            
            if sample[i]==0:
                noise[i]=0
        
        return sample+noise
