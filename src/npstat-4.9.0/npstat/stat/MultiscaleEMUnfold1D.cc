#include <algorithm>
#include <climits>
#include <cfloat>

#include "npstat/stat/MultiscaleEMUnfold1D.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npstat {
    MultiscaleEMUnfold1D::MultiscaleEMUnfold1D(
        const Matrix<double>& responseMatrix, const LocalPolyFilter1D& f,
        const int i_symbetaPower, const double i_maxLOrPEDegree,
        const double i_xMinUnfolded, const double i_xMaxUnfolded,
        const BoundaryHandling& filterBoundaryMethod,
        const double i_minBandwidth, const double i_maxBandwidth,
        const unsigned i_nFilters, const unsigned i_itersPerFilter,
        const bool i_useConvolutions, const bool i_useMultinomialCovariance,
        const bool i_smoothLastIter, const double i_convergenceEpsilon,
        const unsigned i_maxIterations)
        : SmoothedEMUnfold1D(responseMatrix, f, i_useConvolutions,
                                    i_useMultinomialCovariance, i_smoothLastIter,
                                    i_convergenceEpsilon, i_maxIterations),
          maxLOrPEDegree_(i_maxLOrPEDegree),
          binwidth_(fabs((i_xMinUnfolded-i_xMaxUnfolded)/responseMatrix.nColumns())),
          symbetaPower_(i_symbetaPower),
          itersPerFilter_(i_itersPerFilter),
          filterBoundaryMethod_(filterBoundaryMethod),
          bwset_(std::min(i_minBandwidth, i_maxBandwidth),
                 std::max(i_minBandwidth, i_maxBandwidth), i_nFilters)
    {
    }

    unsigned MultiscaleEMUnfold1D::preIterate(
        const double* observed, const unsigned lenObserved,
        double** prev, double** next, const unsigned lenUnfolded)
    {
        unsigned iterCount = 0U;
        const unsigned nBw = bwset_.size();
        if (nBw)
        {
            const LocalPolyFilter1D* currentFilt = getFilter();
            const double cEps = convergenceEpsilon();

            for (unsigned ibw=0; ibw<nBw; ++ibw)
            {
                const double bw = bwset_[nBw-ibw-1U];
                CPP11_shared_ptr<const LocalPolyFilter1D> filt =
                    filterProvider_.provideFilter(
                        symbetaPower_, bw, maxLOrPEDegree_, lenUnfolded,
                        binwidth_, filterBoundaryMethod_, UINT_MAX, false);
                setFilter(filt.get());
                bool converged = false;
                for (unsigned it=0U; it<itersPerFilter_ && !converged;
                     ++it, ++iterCount)
                {
                    std::swap(*prev, *next);
                    update(observed,lenObserved,*prev,*next,lenUnfolded,true);
                    converged = probDelta(*prev, *next, lenUnfolded) <= cEps;
                }
            }
            setFilter(currentFilt);
        }
        return iterCount;
    }
}
