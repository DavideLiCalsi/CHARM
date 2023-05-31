#include <Arduino.h>
#include <math.h>
#include "constants.h"

/**
 * @brief Inverse of the weibull distribution
 * 
 * @param lambda 
 * @param k 
 * @return float 
 */
float Inverse_CDP(float y,float lambda, float k){
    float root = pow(-log(1-y),1/k);
    return lambda * root;
}

float sample_weibull(float lambda, float k){
    float U = random(1,10000) *1.0 / 10000;

    return Inverse_CDP(U,lambda,k);
}