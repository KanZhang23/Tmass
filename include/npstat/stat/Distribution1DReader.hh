#ifndef NPSTAT_DISTRIBUTION1DREADER_HH_
#define NPSTAT_DISTRIBUTION1DREADER_HH_

/*!
// \file Distribution1DReader.hh
//
// \brief Factory for deserializing one-dimensional distribution functions
//
// Author: I. Volobouev
//
// September 2010
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    class Distribution1DReader : public gs::DefaultReader<AbsDistribution1D>
    {
        typedef gs::DefaultReader<AbsDistribution1D> Base;
        friend class gs::StaticReader<Distribution1DReader>;
        Distribution1DReader();
    };

    /** Factory for deserializing one-dimensional distribution functions */
    typedef gs::StaticReader<Distribution1DReader> StaticDistribution1DReader;
}

#endif // NPSTAT_DISTRIBUTION1DREADER_HH_
