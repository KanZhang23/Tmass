#ifndef MCUNCERT_CONE_HH
#define MCUNCERT_CONE_HH

/*!
// \file cone.hh 
//
// \brief class that defines the set of functions for which 
//        the error estimated in QMCUncertaintyCalculator is
//        guaranteed.
//
// Author: Ll.A. Jimenez Rugama
//
// August 2015
*/

#include "AbsUncertaintyCalculator.hh"
#include <stdexcept>
#include <complex>

const long double alpha = (5.0L*pow(2.0L,4.0L))/(1.0L+1.0L/pow(2.0L,4.0L)); ///< If p=q=beta=1, and rlag=4, this alpha makes C(0)=5

namespace mcuncert {
    /**
    // 
    // In this class the user specifies the parameters that define
    // the cone of functions. This is the cone for which we guarantee
    // the error estimated by the QMCUncertaintyCalculator class.
    //
    // A cone of functions is completely determined by the positive
    // integers lstar and rlag, and the positive functions omega 
    // circle (omgcirc) and omega hat (omghat). Following the notation
    // from the article in http://arxiv.org/abs/1410.8615, in this class
    // we also defined cfrag to ease the computations and understanding
    // of the error bound.
    //
    // In this particular case, we parametrized omgcirc and omghat as 
    // functions of the form gamma*2^(-p*m) and beta*2^(-q*m). This can clearly
    // be modified according to the needs of the user. Note that the 
    // initialization of the cone of functions is initialized by default
    // in the QMCUncertaintyCalculator class.
    //
    // For more information about the meaning of this cone of functions,
    // the user can refer to the documentation of the mcuncert package
    // inside the folder doc.
    //
    **/
    namespace priv{
        class cone
        {
        public:
            unsigned lstar;
            unsigned rlag;
            long double beta; ///< Constant factor for omega_hat
            long double gamma; ///< Constant factor for omega_circle
            long double q; ///< Exponent factor for omega_hat
            long double p; ///< Exponent factor for omega_circle

            cone() : lstar(4U), rlag(4U), beta(1.0L), gamma(alpha), q(1.0L), p(1.0L) {
                if (this->omgcirc(rlag)*omghat(rlag) >= 1.0L) throw std::invalid_argument(
                    "In mcuncert::cone::cone: "
                    "Parametrization input error, omgcirc(rlag)*omghat(rlag) should be strictly smaller than 1.");
            }

            cone( unsigned a , unsigned b ) : lstar(a), rlag(b), beta(1.0L), gamma(alpha), q(1.0L), p(1.0L) {
                if (this->omgcirc(rlag)*omghat(rlag) >= 1.0L) throw std::invalid_argument(
                    "In mcuncert::cone::cone: "
                    "Parametrization input error, omgcirc(rlag)*omghat(rlag) should be strictly smaller than 1.");
            }

            inline virtual ~cone() {}

            long double cfrag(unsigned long m) const;

            long double omgcirc(unsigned long m) const;

            long double omghat(unsigned long m) const;

        };
    }
}

#endif // MCUNCERT_CONE_HH
