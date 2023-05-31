from models.Model import Model

import random
import numpy as np # linear algebra
import pandas as pd # data processing, CSV file I/O (e.g. pd.read_csv)

# Import utilities from sklearn
from sklearn import tree
from sklearn.metrics import classification_report, confusion_matrix, precision_score, recall_score
from sklearn.model_selection import train_test_split

class DecisionTreeModel(Model):

    def get_untrained_model(self):
        model = tree.DecisionTreeClassifier()
        self.model=model

    def preprocess(self):
        dataset = self.dataset.fillna(0)
        x = dataset[dataset.columns.drop(['LEGIT']) ]
        y = dataset['LEGIT']
    
        self.x_train, self.x_test, self.y_train, self.y_test = train_test_split(x,y, test_size=0.2, stratify=y)
        return (self.x_train,self.x_test,self.y_train,self.y_test)

    def train(self,x_train,y_train):
        self.model.fit(x_train,y_train)

    def test(self,x_test):
        return self.model.predict(x_test)
    
    def get_importance(self):
        return self.model.feature_importances_