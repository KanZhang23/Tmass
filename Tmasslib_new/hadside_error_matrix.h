#ifndef HADSIDE_ERROR_MATRIX_H_
#define HADSIDE_ERROR_MATRIX_H_

#include "simple_kinematics.h"
#include "solve_top.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HADERR_W_MSQ = 0,
    HADERR_T_MSQ,
    HADERR_T_X,
    HADERR_T_Y,
    N_HADERR_ROWS
};

enum {
    HADERR_B_ETA = 0,
    HADERR_B_PHI,
    HADERR_B_MSQ,
    HADERR_Q_ETA,
    HADERR_Q_PHI,
    HADERR_Q_MSQ,
    HADERR_QBAR_ETA,
    HADERR_QBAR_PHI,
    HADERR_QBAR_MSQ,
    N_HADERR_COLUMNS
};

void haderr_jacobian(const hadron_side_solution *hsol,
		     double jaco[N_HADERR_ROWS][N_HADERR_COLUMNS]);

/* Incomplete W mass squared gradient using just the light jets */
void hadronic_mwsq_gradient(v3_obj pq, v3_obj pqbar,
                            double mwsq_gradient[N_HADERR_COLUMNS]);

#ifdef __cplusplus
}
#endif

#endif /* HADSIDE_ERROR_MATRIX_H_ */
