from models.Model import Model

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

from sklearn.model_selection import train_test_split

class DNN(Model):

    def get_untrained_model(self):
        model=keras.Sequential( 
            [
                layers.Input(11,name='Input'),
                layers.Dense(32,activation='relu'),
                layers.Dense(1,activation='sigmoid',name='Output')
            ]
        )
        
        model.compile(
            optimizer=tf.keras.optimizers.Adam(learning_rate=1e-3),
            loss=tf.keras.losses.BinaryCrossentropy(),
            metrics=[tf.keras.metrics.Recall(),
            tf.keras.metrics.FalseNegatives()]
        )

        self.model=model
        return model
    
    def preprocess(self):
        dataset = self.dataset.fillna(0)
        x = dataset[dataset.columns.drop(['LEGIT']) ]
        y = dataset['LEGIT']

        #poly = PolynomialFeatures(7)
        #x=poly.fit_transform(x)

        self.x_train, self.x_test, self.y_train, self.y_test = train_test_split(x,y, test_size=0.2, stratify=y)
    
        return (self.x_train,self.x_test,self.y_train,self.y_test)
    
    def train(self,x_train,y_train,importance=False):

        x_val, self.x_test, y_val, self.y_test = train_test_split(self.x_test, self.y_test,test_size=0.2,stratify=self.y_test)
        self.model.fit(
            x=x_train,
            y=y_train,
            validation_data=(x_val,y_val),
            batch_size=32,
            epochs=100,
            callbacks=[
                tf.keras.callbacks.EarlyStopping(monitor='val_loss',patience=10,restore_best_weights=True)
            ]
        )
    
    def test(self,x_test):
        predictions=self.model.predict(x_test)
        
        predictions= [ int(p[0]) for p in predictions]
        print(predictions[0])

        return predictions
