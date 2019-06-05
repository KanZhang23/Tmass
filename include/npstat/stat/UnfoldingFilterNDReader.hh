#ifndef NPSTAT_UNFOLDINGFILTERNDREADER_HH_
#define NPSTAT_UNFOLDINGFILTERNDREADER_HH_

/*!
// \file UnfoldingFilterNDReader.hh
//
// \brief Factory for deserializing multivariate unfolding filters
//
// Author: I. Volobouev
//
// June 2014
*/

#include "geners/AbsReader.hh"
#include "npstat/stat/AbsUnfoldingFilterND.hh"

namespace npstat {
    class UnfoldingFilterNDReader : public gs::DefaultReader<AbsUnfoldingFilterND>
    {
        typedef gs::DefaultReader<AbsUnfoldingFilterND> Base;
        friend class gs::StaticReader<UnfoldingFilterNDReader>;
        UnfoldingFilterNDReader();
    };

    /** Factory for deserializing multivariate distribution functions */
    typedef gs::StaticReader<UnfoldingFilterNDReader> StaticUnfoldingFilterNDReader;
}

#endif // NPSTAT_UNFOLDINGFILTERNDREADER_HH_
