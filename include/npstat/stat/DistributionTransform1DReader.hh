#ifndef NPSTAT_DISTRIBUTIONTRANSFORM1DREADER_HH_
#define NPSTAT_DISTRIBUTIONTRANSFORM1DREADER_HH_

/*!
// \file DistributionTransform1DReader.hh
//
// \brief Factory for deserializing one-dimensional distribution transforms
//
// Author: I. Volobouev
//
// April 2015
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/AbsDistributionTransform1D.hh"

namespace npstat {
    class DistributionTransform1DReader : public gs::DefaultReader<AbsDistributionTransform1D>
    {
        typedef gs::DefaultReader<AbsDistributionTransform1D> Base;
        friend class gs::StaticReader<DistributionTransform1DReader>;
        DistributionTransform1DReader();
    };

    /** Factory for deserializing one-dimensional distribution functions */
    typedef gs::StaticReader<DistributionTransform1DReader> StaticDistributionTransform1DReader;
}

#endif // NPSTAT_DISTRIBUTIONTRANSFORM1DREADER_HH_
