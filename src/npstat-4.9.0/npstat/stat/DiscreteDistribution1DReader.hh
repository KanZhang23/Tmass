#ifndef NPSTAT_DISCRETEDISTRIBUTION1DREADER_HH_
#define NPSTAT_DISCRETEDISTRIBUTION1DREADER_HH_

/*!
// \file DiscreteDistribution1DReader.hh
//
// \brief Factory for deserializing one-dimensional discrete distributions
//
// Author: I. Volobouev
//
// May 2013
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/AbsDiscreteDistribution1D.hh"

namespace npstat {
    class DiscreteDistribution1DReader : public gs::DefaultReader<AbsDiscreteDistribution1D>
    {
        typedef gs::DefaultReader<AbsDiscreteDistribution1D> Base;
        friend class gs::StaticReader<DiscreteDistribution1DReader>;
        DiscreteDistribution1DReader();
    };

    /** Factory for deserializing one-dimensional discrete distributions */
    typedef gs::StaticReader<DiscreteDistribution1DReader> StaticDiscreteDistribution1DReader;
}

#endif // NPSTAT_DISCRETEDISTRIBUTION1DREADER_HH_
