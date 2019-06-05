#ifndef NPSTAT_DISTRIBUTIONNDREADER_HH_
#define NPSTAT_DISTRIBUTIONNDREADER_HH_

/*!
// \file DistributionNDReader.hh
//
// \brief Factory for deserializing multivariate distribution functions
//
// Author: I. Volobouev
//
// September 2010
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    class DistributionNDReader : public gs::DefaultReader<AbsDistributionND>
    {
        typedef gs::DefaultReader<AbsDistributionND> Base;
        friend class gs::StaticReader<DistributionNDReader>;
        DistributionNDReader();
    };

    /** Factory for deserializing multivariate distribution functions */
    typedef gs::StaticReader<DistributionNDReader> StaticDistributionNDReader;
}

#endif // NPSTAT_DISTRIBUTIONNDREADER_HH_
