#include <cmath>
#include <cassert>

#include "getJESSigma.hh"
#include "JetCorrectionsInterface.hh"

static float l0_pt(const unsigned icorr,
                   const float corrected_pt,
                   const float emf, 
                   const float detectorEta)
{
    assert(corrected_pt >= 0.1f);
    assert(emf >= 0.f);
    assert(emf <= 1.f);
    {
        const float tol = 1.0e-6f;
        const float delta = sqrtf(tol);
        float tmp = emf;
        float xnext = corrected_pt*0.8f;
        float fval, deriv;

        fval = xnext*generic_correction_scale(icorr, xnext, &tmp, detectorEta);
        while (fabsf(fval - corrected_pt)/(fabsf(fval) + fabsf(corrected_pt) + 1.f) > tol)
        {
            const double xdelta = xnext*delta;
            tmp = emf;
            if (fval < corrected_pt)
            {
                float fplus = (xnext+xdelta)*generic_correction_scale(
                    icorr, xnext+xdelta, &tmp, detectorEta);
                deriv = (fplus - fval)/xdelta;
            }
            else
            {
                float fminus = (xnext-xdelta)*generic_correction_scale(
                    icorr, xnext-xdelta, &tmp, detectorEta);
                deriv = (fval - fminus)/xdelta;
            }
            assert(deriv);
            xnext += (corrected_pt - fval)/deriv;
            tmp = emf;
            fval = xnext*generic_correction_scale(
                icorr, xnext, &tmp, detectorEta);
        }
        return xnext;
    }
}

double getJESSigma(const unsigned icorr, const double pt, const double detEta)
{
    float emf = 0.3f;
    const double pt0 = l0_pt(icorr, pt, emf, detEta);
    return generic_correction_uncertainty(icorr, pt0, detEta, 1U);
}
