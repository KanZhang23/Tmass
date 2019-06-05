#ifndef NPSTAT_SYMBETAPARAMS1D_HH_
#define NPSTAT_SYMBETAPARAMS1D_HH_

/*!
// \file SymbetaParams1D.hh
//
// \brief Parameters of 1-d filters from the symmetric beta family
//
// Author: I. Volobouev
//
// July 2014
*/

#include "npstat/stat/BoundaryHandling.hh"

namespace npstat {
    /**
    // Parameters of simple 1-d filters from the symmetric beta family,
    // excluding the number of points and the bandwidth. Together with
    // the bandwidth, these are needed to specify the filter. This is
    // useful, for example, in the unfolding studies. The parameter
    // values are intended for passing to "symbetaLOrPEFilter1D" or
    // another similar function.
    */
    class SymbetaParams1D
    {
    public:
        SymbetaParams1D(int symbetaPower, double maxDegree,
                        double binWidth, const BoundaryHandling& bm);

        inline double maxDegree() const {return maxDegree_;}
        inline double binWidth() const {return binWidth_;}
        inline int symbetaPower() const {return symbetaPower_;}
        inline const BoundaryHandling& boundaryMethod() const {return bm_;}

    private:
        SymbetaParams1D();

        double maxDegree_;
        double binWidth_;
        int symbetaPower_;
        BoundaryHandling bm_;
    };
}

#endif // NPSTAT_SYMBETAPARAMS1D_HH_
