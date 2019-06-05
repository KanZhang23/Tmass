#ifndef NPSTAT_LOCALPOLYFILTER1DREADER_HH_
#define NPSTAT_LOCALPOLYFILTER1DREADER_HH_

/*!
// \file LocalPolyFilter1DReader.hh
//
// \brief Factory for deserializing local polynomial filters
//
// Author: I. Volobouev
//
// May 2014
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npstat {
    class LocalPolyFilter1DReader : public gs::DefaultReader<LocalPolyFilter1D>
    {
        typedef gs::DefaultReader<LocalPolyFilter1D> Base;
        friend class gs::StaticReader<LocalPolyFilter1DReader>;
        LocalPolyFilter1DReader();
    };

    /** Factory for deserializing one-dimensional distribution functions */
    typedef gs::StaticReader<LocalPolyFilter1DReader> StaticLocalPolyFilter1DReader;
}

#endif // NPSTAT_LOCALPOLYFILTER1DREADER_HH_
