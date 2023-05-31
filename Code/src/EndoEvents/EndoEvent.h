#include "constants.h"
#include "Exponential.h"
#include "Weibull.h"
#include <string.h>

#ifdef configENDO_DISTRIBUTION


float GetEndoEvent(float lambda){

    if ( strcmp(configENDO_DISTRIBUTION, "EXPONENTIAL") == 0 )
        return sample_exponential(lambda);
    
    if ( strcmp(configENDO_DISTRIBUTION, "WEIBULL") == 0 )
        return sample_weibull(lambda,50);

    return 1;
    
}

#endif