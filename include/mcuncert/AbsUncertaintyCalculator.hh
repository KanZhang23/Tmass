#ifndef MCUNCERT_ABSUNCERTAINTYCALCULATOR_HH_
#define MCUNCERT_ABSUNCERTAINTYCALCULATOR_HH_

/*!
// \file AbsUncertaintyCalculator.hh 
//
// \brief Abstract class for numerical integration
//
// Author: I. Volobouev and Ll.A. Jimenez Rugama
//
// August 2015
*/

namespace mcuncert {
    class AbsUncertaintyCalculator
    {
    public:
        inline virtual ~AbsUncertaintyCalculator() {}

        /// Reset to the initial state, as if no points were added
        virtual void clear() = 0;

        /// Add one more input point with the corresponding function value
        virtual void addPoint(long double functionValue) = 0;

        /// Number of points used
        virtual unsigned long count() const = 0;

        /// Average function value
        virtual long double mean() const = 0;

        /// Uncertainty of the average function value
        virtual long double meanUncertainty() const = 0;

        /// Minimum and maximum function values
        virtual long double min() const = 0;
        virtual long double max() const = 0;

        /// Sum of the function values and sum of their squares
        virtual long double sum() const = 0;
        virtual long double sumsq() const = 0;

        /// Print some useful information for calibration and debugging
        virtual void print() const = 0;

        /// Necessary conditions check for guarantees on error bounds (qMC only)
        virtual bool nec_cond_fail() const = 0;
    };
}

#endif // MCUNCERT_ABSUNCERTAINTYCALCULATOR_HH_
