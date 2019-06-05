#ifndef QQBAR_DELTAR_EFFICIENCY_H_
#define QQBAR_DELTAR_EFFICIENCY_H_

#include "linear_interpolator_nd.h"

#ifdef __cplusplus
extern "C" {
#endif
	    
/* The following function does not assume the ownership of the pointer */
void set_qqbar_deltaR_interpolator(const Interpolator_data_nd *in);

double qqbar_deltaR_efficiency(double deltaR, double m1_ratio, double m2_ratio);
	    
#ifdef __cplusplus
}
#endif
	
#endif /* QQBAR_DELTAR_EFFICIENCY_H_ */
