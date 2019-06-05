#ifndef MCUNCERT_PSEUDOMCUNCERTAINTYCALCULATOR_HH
#define MCUNCERT_PSEUDOMCUNCERTAINTYCALCULATOR_HH

/*!
// \file AbsUncertaintyCalculator.hh 
//
// \brief AbsUncertaintyCalculator derived class for Monte Carlo error estimation
//
// Author: I. Volobouev
//
// August 2015
*/

#include "AbsUncertaintyCalculator.hh"

namespace mcuncert {
    class PseudoMCUncertaintyCalculator : public AbsUncertaintyCalculator
    {
    public:
        PseudoMCUncertaintyCalculator();

        inline virtual ~PseudoMCUncertaintyCalculator() {}

        // Reset to the initial state, as if no points were added
        void clear();

        // Add one more input point with the corresponding function value
        void addPoint(long double functionValue);

        // Number of points used
        unsigned long count() const;

        // Average function value
        long double mean() const;

        // Uncertainty of the average function value
        long double meanUncertainty() const;

        // Minimum and maximum function values
        long double min() const;
        long double max() const;

        // Sum of the function values and sum of their squares
        long double sum() const;
        long double sumsq() const;

        // Prints useful information for debugging
        void print() const;

        // Necessary conditions check for guarantees on error bounds
        bool nec_cond_fail() const;

    private:
        void recenter();

        long double sum_;
        long double sumsq_;
        long double runningMean_;
        long double min_;
        long double max_;
        unsigned long count_;
        unsigned long nextRecenter_;
    };
}

#endif // MCUNCERT_PSEUDOMCUNCERTAINTYCALCULATOR_HH
