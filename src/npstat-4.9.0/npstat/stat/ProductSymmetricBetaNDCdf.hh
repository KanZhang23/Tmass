#ifndef NPSTAT_PRODUCTSYMMETRICBETANDCDF_HH_
#define NPSTAT_PRODUCTSYMMETRICBETANDCDF_HH_

/*!
// \file ProductSymmetricBetaNDCdf.hh
//
// \brief Multivariate cumulative distribution for product
//        symmetric beta density
//
// Author: I. Volobouev
//
// July 2012
*/

#include <vector>

#include "npstat/nm/AbsMultivariateFunctor.hh"
#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    /**
    // Multivariate cumulative distribution for product symmetric
    // beta density. Can be used as the weight functor with the
    // npstat::mergeTwoHistos function.
    */
    class ProductSymmetricBetaNDCdf : public AbsMultivariateFunctor
    {
    public:
        /**
        // The constructor takes array arguments. Each array element is
        // used for one of the dimensions. "dim" gives the size of all
        // arrays as well as the function dimensionality.
        //
        //  location  -- mean of SymmetricBeta1D in each dimension
        //
        //  scale     -- scale parameter of SymmetricBeta1D in each dimension
        //
        //  power     -- power parameter of SymmetricBeta1D in each dimension
        //
        //  direction -- a flag which tells us what to do with the cumulative
        //               distribution of SymmetricBeta1D. Meanings are as
        //               follows:
        //                >0  -- use exceedance (1 - cdf, weight decreasing)
        //                 0  -- do not use this dimension at all (factor of 1
        //                       will be used everywhere instead of cdf)
        //                <0  -- use cdf (weight increasing)
        */
        ProductSymmetricBetaNDCdf(const double* location, const double* scale,
                                  const double* power, const int* direction,
                                  unsigned dim);

        inline virtual ~ProductSymmetricBetaNDCdf() {}

        virtual double operator()(const double* point, unsigned dim) const;

        inline virtual unsigned minDim() const {return dim_;}

    private:
        std::vector<SymmetricBeta1D> marginals_;
        std::vector<int> directions_;
        unsigned dim_;
    };
}

#endif // NPSTAT_PRODUCTSYMMETRICBETANDCDF_HH_
