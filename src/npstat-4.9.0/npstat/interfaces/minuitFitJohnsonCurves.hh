#ifndef NPSI_MINUITFITJOHNSONCURVES_HH_
#define NPSI_MINUITFITJOHNSONCURVES_HH_

/*!
// \file minuitFitJohnsonCurves.hh
//
// \brief Binned fit of Johnson curves by maximum likelihood
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/stat/HistoND.hh"

namespace npsi {
    /**
    // Fitting Johnson system of curves to histogrammed data
    // using the maximum likelihood method
    //
    */
    class JohnsonFit
    {
    public:
        /**
        // In the constructor, arguments are as follows:
        //
        // "histo" is the 1-d histogram to fit.
        //
        // "qmin" and "qmax" are the quantiles which define the
        // fitted region. For example, set them to 0.05 and 0.95,
        // respectively, if you would like to fit central 90% of
        // the distribution.
        //
        // "minlog" limits the contribution of any single bin
        // into the log-likelihood from below. No bin can contribute
        // less then "minlog" even if the value of the fitted density
        // for that bin is 0. Typically, this parameter should be set
        // to a negative number with a reasonably large magnitude,
        // on the order of log(DBL_MIN).
        //
        */
        template <class Numeric,class Axis>
        JohnsonFit(const npstat::HistoND<Numeric,Axis>& histo,
                   double qmin, double qmax, double minlog);

        /**
        // Check fit convergence. If the fit did not converge,
        // the results will be set by simple array-based calculations.
        */
        inline bool converged() const {return converged_;}

        //@{
        /** Examine fitted value */
        inline double mean() const {return mean_;}
        inline double sigma() const {return sigma_;}
        inline double skewness() const {return skew_;}
        inline double kurtosis() const {return kurt_;}
        //@}

        //@{
        /**
        // Figure out the region fitted. The minimum bin is included
        // and the maximum bin is excluded from the fit.
        */
        inline unsigned minFitBin() const {return minFitBin_;}
        inline unsigned maxFitBin() const {return maxFitBin_;}
        //@}

    private:
        double mean_;
        double sigma_;
        double skew_;
        double kurt_;
        unsigned minFitBin_;
        unsigned maxFitBin_;
        bool converged_;
    };
}

#include "npstat/interfaces/minuitFitJohnsonCurves.icc"

#endif // NPSI_MINUITFITJOHNSONCURVES_HH_
