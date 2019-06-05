#ifndef NPSTAT_DENSITYSCANND_HH_
#define NPSTAT_DENSITYSCANND_HH_

/*!
// \file DensityScanND.hh
//
// \brief Fill multivariate arrays with multivariate density values
//
// Author: I. Volobouev
//
// December 2011
*/

#include <vector>
#include <cassert>
#include <stdexcept>

#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    /**
    // The IndexMapper template parameter is a functor which takes
    // a single unsigned argument (perhaps, with automatic conversion to
    // another type) and returns a double (or something which can be
    // automatically converted to double). IndexMapper can, for example,
    // be set to LinearMapper1d or CircularMapper1d.
    */
    template<class IndexMapper>
    class DensityScanND
    {
    public:
        /**
        // A mapper for each coordinate in the "maps" constructor argument
        // will convert the array index into a proper argument for the scanned
        // density.
        //
        // This functor will NOT make copies of either "fcn" or "maps"
        // parameters. These parameters will be used by reference only
        // (aliased). It is up to the user of this class to ensure proper
        // lifetime of these objects.
        */
        inline DensityScanND(const AbsDistributionND& fcn,
                             const std::vector<IndexMapper>& maps,
                             double normfactor=1.0)
            : fcn_(fcn), mapping_(maps), buf_(fcn.dim()),
              normfactor_(normfactor), dim_(fcn.dim())
        {
            if (!(dim_ && dim_ == maps.size())) throw std::invalid_argument(
                "In npstat::DensityScanND constructor: incompatible arguments");
        }

        inline double operator()(const unsigned* index,
                                 const unsigned indexLen) const
        {
            if (dim_ != indexLen) throw std::invalid_argument(
                "In npstat::DensityScanND::operator(): "
                "incompatible input point dimensionality");
            assert(index);
            double* x = &buf_[0];
            for (unsigned i=0; i<dim_; ++i)
                x[i] = mapping_[i](static_cast<double>(index[i]));
            return normfactor_*fcn_.density(x, dim_);
        }

    private:
        DensityScanND();

        const AbsDistributionND& fcn_;
        const std::vector<IndexMapper>& mapping_;
        mutable std::vector<double> buf_;
        double normfactor_;
        unsigned dim_;
    };
}

#endif // NPSTAT_DENSITYSCANND_HH_
