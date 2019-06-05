#include <math.h>

#include "p_struct_function.h"
#include "lhapdf_interface.h"
#include "matrel.h"

#ifdef __cplusplus
extern "C" {
#endif

void pdfterm_qqbar(const particle_obj p1, const particle_obj p2,
                   const double Ecms, double *weight_qq, double *weight_gg)
{
    // Most LHAPDF PDF sets claim validity down to x = 10^-5 or 10^-6,
    // so we are rather conservative here
    static const double min_x = 1.0e-4;

    const particle_obj compound = sum4(p1, p2);

    /* We will neglect the compound transverse momentum */
    const double dtmp  = sqrt(compound.m*compound.m + compound.p.z*compound.p.z);
    const double mom1  = 0.5*(dtmp + compound.p.z);
    const double mom2  = 0.5*(dtmp - compound.p.z);
    const double x1    = mom1/(Ecms/2.0);
    const double x2    = mom2/(Ecms/2.0);

    /* PDF scale used by Herwig */
    const particle_obj qin1 = particle(v3(0.0, 0.0, mom1), 0.0);
    const particle_obj qin2 = particle(v3(0.0, 0.0, -mom2), 0.0);
    const double S = sprod4(qin1, qin2);
    const double T = -sprod4(qin1, p1);
    const double U = -sprod4(qin1, p2);
    const double Q2 = 4.0*S*T*U/(S*S+T*T+U*U);

    if (x1 >= 1.0 || x2 >= 1.0)
    {
        *weight_qq = 0.0;
        *weight_gg = 0.0;
    }
    else
    {
        double Glue1, Glue2, qPdf1[8], qPdf2[8];
        calculate_lhapdf(x1, Q2, &Glue1, qPdf1);
        calculate_lhapdf(x2, Q2, &Glue2, qPdf2);

        double qpdfsum = 0.0;
        unsigned i=0;
        for (; i<8U; ++i)
            qpdfsum += qPdf1[i]*qPdf2[i];

        const double alpha_s_r = alphas_lhapdf(Q2)/alpsks_();
        const double x1d = x1 > min_x ? x1 : min_x;
        const double x2d = x2 > min_x ? x2 : min_x;
        const double common = 4.0*alpha_s_r*alpha_s_r/x1d/x1d/Ecms/x2d/x2d/Ecms;

        /* Dependence of alpha_s on the scale is included */
        *weight_qq = qpdfsum*common;
        *weight_gg = Glue1*Glue2*common;
    }
}

#ifdef __cplusplus
}
#endif
