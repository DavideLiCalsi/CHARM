/**
 * @file Exponential.h
 * @author Davide Li Calsi
 * @brief This file contains primitives to sample time instants
 * from an exponential distribution.
 * Recall that, for an exponential distribution:
 * - PROB. DENSITY -> f(t) = lambda * exp(-lambda*t)
 * - CUMUL. DENSITY -> F(t) = 1 - exp(-lambda*t)
 * @version 0.1
 * @date 2022-05-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include <math.h>
#include "constants.h"

#define LAMBDA_FAN 1/configENDO_FREQUENCY_FAN
#define LAMBDA_VALVE 1/configENDO_FREQUENCY_VALVE

/**
 * @brief Inverse of the exponential CDP
 * 
 * @param prob a.k.a. y
 * @param lambda
 * @return float 
 */
float Inverse_CDP(float prob, float lambda){
    return -1 * log(1-prob) / lambda;
}

float sample_exponential(float lambda){
    float U = random(1,10000)*1.0 / 10000;

    return Inverse_CDP(U,lambda);
}