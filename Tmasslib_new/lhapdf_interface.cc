#include <cassert>

#include "lhapdf_interface.h"

#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "LHAPDF/LHAPDF.h"

static LHAPDF::PDF* lhapdf_ = 0;
static double qSquaredFactor_ = 0.0;

#ifdef __cplusplus
extern "C" {
#endif

double alphas_lhapdf(const double Q2_in)
{
    assert(lhapdf_);

    const double Q2 = Q2_in*qSquaredFactor_;
    assert(Q2 > 0.0);

    return lhapdf_->alphasQ2(Q2);
}

void calculate_lhapdf(const double x, const double Q2_in,
                      double* gPdf, double qPdf[8])
{
    assert(lhapdf_);

    const double Q2 = Q2_in*qSquaredFactor_;
    assert(Q2 > 0.0);

    *gPdf = lhapdf_->xfxQ2(21, x, Q2);
    unsigned pos = 0;
    for (int i=1; i<5; ++i)
    {
        qPdf[pos++] = lhapdf_->xfxQ2(i, x, Q2);
        qPdf[pos++] = lhapdf_->xfxQ2(-i, x, Q2);
    }
}

void init_lhapdf(const char* pdfname, const double q2factor)
{
    assert(pdfname);
    assert(q2factor > 0.0);

    cleanup_lhapdf();
    lhapdf_ = LHAPDF::mkPDF(std::string(pdfname));
    assert(lhapdf_);
    qSquaredFactor_ = q2factor;
}

void cleanup_lhapdf(void)
{
    delete lhapdf_;
    lhapdf_ = 0;
    qSquaredFactor_ = 0.0;
}

#ifdef __cplusplus
}
#endif
