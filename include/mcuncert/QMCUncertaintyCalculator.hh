#ifndef MCUNCERT_QMCUNCERTAINTYCALCULATOR_HH
#define MCUNCERT_QMCUNCERTAINTYCALCULATOR_HH

/*!
// \file QMCUncertaintyCalculator.hh 
//
// \brief AbsUncertaintyCalculator derived class for quasi-Monte Carlo error estimation
//
// Author: Ll.A. Jimenez Rugama
//
// August 2015
*/

#include "AbsUncertaintyCalculator.hh"
#include "cone.hh"
#include <vector>

namespace mcuncert {
    /**
    // 
    // Quasi-Monte Carlo uncertainity estimator and other inegration 
    // parameters estimated when using digital nets. The theory 
    // behind this algorithm is described in http://arxiv.org/abs/1410.8615
    //
    // The bound on the error is estimated according to the discrite
    // Walsh-Fourier coefficients of the function. Thus, the error
    // bound is updated every power of 2 number of points. The error
    // will be guaranteed for all functions lying in a cone of functions
    // as described in http://arxiv.org/abs/1410.8615.
    //
    // The most important parameters in this class are m_min_, l_star,
    // and r_lag. m_min_ is the parameter such that 2^m_min_ is
    // the number of points needed to compute the error estimate
    // for the first time (before it is accounted as infinity).
    // l_star and r_lag are parameters defining the cone of functions
    // for which the error bound provided is guaranteed. For more
    // information about these parameters, one can check the file cone.hh
    //
    **/
    class QMCUncertaintyCalculator : public AbsUncertaintyCalculator
    {
    public:
        QMCUncertaintyCalculator(const unsigned long m_min_ = 8UL , const unsigned l_star = 4U , const unsigned r_lag = 4U);

        inline virtual ~QMCUncertaintyCalculator() {}

        /// Reset to the initial state, as if no points were added
        void clear();

        /// Add one more input point with the corresponding function value
        void addPoint(long double functionValue);

        /// Number of points used
        unsigned long count() const;

        /// Average function value
        long double mean() const;

        /// Uncertainty of the average function value
        long double meanUncertainty() const;

        /// Minimum and maximum function values
        long double min() const;
        long double max() const;

        /// Sum of the function values and sum of their squares
        long double sum() const;
        long double sumsq() const;

        /// Print some useful information for calibration and debugging
        void print() const;

        /// Necessary conditions check for guarantees on error bounds (qMC only)
        bool nec_cond_fail() const;

    private:
        /// Recenter is used to update the values every power of 2.
        void recenter();

        /// Update of the Fast Walsh-Fourier Transform
        void update_transform();

        /// Initialization of the Kappa mapping (heuristically)
        void init_kappanumap();

        /// Update of the Kappa mapping (heuristically)
        void update_kappanumap();

        /// Update the error bounds based on the Kappa mapping and checking necessary conditions
        void update_error_bound();

        /// Enlarge the cone of functions when necessary conditions fail
        void enlarge_cone();

        long double sum_;
        long double sumsq_;
        long double runningMean_;
        long double min_;
        long double max_;
        unsigned long count_; ///< Number of points considered
        unsigned long count_exp_; ///< Counter of the log2 number of points
        unsigned long nextRecenter_; ///< Updates are made each nextRecenter which indeed, is every power of 2

        mcuncert::priv::cone cone_; ///< Set of functions for which the algorithm has guarantees on the error bound
        unsigned long mmin_; ///< Starting power(base 2) of number of points at which we start computing the error bound
        long double abs_error_bound_;
        std::vector< long double > ffwt_;
        std::vector< unsigned long > kappanumap_;
        std::vector< long double > Stilde_; ///< We keep track of all the sums from lstar to actual m.
        bool nec_cond_fail_; ///< Boolean that tells whether the necessary conditions are not met. If true, function has no guarantees on the error.
        std::vector< long double > nec_cond_upper_; ///< Upper bound for necessary conditions
        std::vector< long double > nec_cond_lower_; ///< Lower bound for necessary conditions

        long double nec_cond_aux_; ///< Auxiliary variable used for computations
    };
}

#endif // MCUNCERT_QMCUNCERTAINTYCALCULATOR_HH
