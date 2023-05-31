from models.Model import Model

import random
import numpy as np # linear algebra
import pandas as pd # data processing, CSV file I/O (e.g. pd.read_csv)

# Import utilities from sklearn
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report, confusion_matrix, precision_score, recall_score
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.preprocessing import PolynomialFeatures

class LogisticModel(Model):

    def get_untrained_model(self):
        model = LogisticRegression(solver='liblinear', random_state=0)
        self.model=model

    def preprocess(self):
        dataset = self.dataset.fillna(0)
        x = dataset[dataset.columns.drop(['LEGIT']) ]
        y = dataset['LEGIT']

        #x= (x - x.min())/(x.max() - x.min())

        self.x_train, self.x_test, self.y_train, self.y_test = train_test_split(x,y, test_size=0.2, stratify=y)
    
        return (self.x_train,self.x_test,self.y_train,self.y_test)

    def train(self,x_train,y_train,importance=False):

        if importance:
            std=np.std(x_train,0)
            std['DWT_SLEEPCNT']=1
            x_scaled=(x_train -np.average(x_train,0))/std
            self.model.fit(x_scaled,y_train)
        else:
            self.model.fit(x_train,y_train)

    def test(self,x_test):
        return self.model.predict(x_test)
    
    def get_importance(self):
        coefs=self.model.coef_[0]
        return [abs(x) for x in coefs]

    
