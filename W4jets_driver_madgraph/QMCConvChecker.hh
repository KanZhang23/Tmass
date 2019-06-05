#ifndef QMCCONVCHECKER_HH_
#define QMCCONVCHECKER_HH_

#include <vector>
#include <cmath>
#include <cfloat>
#include <cassert>

#include "mcuncert/QMCUncertaintyCalculator.hh"

#include "AbsConvergenceChecker.hh"

class QMCConvChecker : public AbsConvergenceChecker
{
public:
    // This class constructor has the following input arguments:
    // double dt which is the maximum time we allow for our integration,
    // unsigned nDeltaJes that says how many DeltaJes are being integrated,
    // double tol that is the relative error tolerance at which the 
    // integration will stop if the bound on the error is smaller, and
    // double perc which is the percentage of DeltaJes that need to satisfy tol.
    //
    // Input arguments qmcuncert_m_min, qmcuncert_l_star, and qmcuncert_r_lag
    // are passed to the constructor of QMCUncertaintyCalculator.
    //
    inline QMCConvChecker(const double dt, const unsigned nDeltaJes,
                          const double tol, const double perc,
                          const unsigned qmcuncert_m_min,
                          const unsigned qmcuncert_l_star,
                          const unsigned qmcuncert_r_lag)
        : qmcuncer_(nDeltaJes, mcuncert::QMCUncertaintyCalculator(
                        qmcuncert_m_min, qmcuncert_l_star, qmcuncert_r_lag)),
          tmax_(dt),
          reltol_(tol),
          pertol_(perc),
          jesNumbersize_(nDeltaJes)
    {
        assert(jesNumbersize_);
    }

    inline virtual ~QMCConvChecker() {}

    inline virtual ConvergenceStatus check(
        const double timeElapsed,
        const std::vector<std::vector<npstat::StatAccumulator> >& ) const
    {
        unsigned converged = 0;
        for (unsigned ijes = 0; ijes < jesNumbersize_; ++ijes)
            if (myRelUncert(ijes) < reltol_)
                ++converged;

        if (static_cast<double>(converged)/jesNumbersize_ > pertol_)
            return CS_CONVERGED;
        else if (timeElapsed >= tmax_)
            return CS_OUT_OF_TIME;
        else
            return CS_CONTINUE;
    }

    inline virtual void accumulate(const unsigned i, const double value)
    {
        qmcuncer_.at(i).addPoint(value);
    }

    inline virtual double getRelUncertainty(const unsigned i) const
    {
        assert(i < jesNumbersize_);
        return myRelUncert(i);
    }

private:
    QMCConvChecker();

    inline double myRelUncert(const unsigned ijes) const
    {
        const double absmean = fabs(qmcuncer_[ijes].mean());
        return qmcuncer_[ijes].meanUncertainty()/(absmean + DBL_EPSILON);
    }

    std::vector<mcuncert::QMCUncertaintyCalculator> qmcuncer_;
    double tmax_;
    double reltol_;
    double pertol_;
    unsigned jesNumbersize_;
};

#endif // QMCCONVCHECKER_HH_
