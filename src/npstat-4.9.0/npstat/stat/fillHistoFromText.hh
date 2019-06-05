#ifndef NPSTAT_FILLHISTOFROMTEXT_HH_
#define NPSTAT_FILLHISTOFROMTEXT_HH_

/*! 
// \file fillHistoFromText.hh
//
// \brief A utility for filling histograms from data contained in text files
//
// Author: I. Volobouev
//
// November 2014
*/

#include <iostream>

namespace npstat {
    /**
    // Fill a histogram with data from a text file. The text file must
    // contain columns of numbers. The array "columnsToUse" defines which
    // columns to histogram. The dimensionality of this array is assumed
    // to be equal to the dimensionality of the histogram.
    //
    // Empty lines, lines which consist of pure white space, and lines
    // which start with an arbitrary amount of white space (including
    // none) followed by '#' are ignored (considered comments).
    //
    // "true" is returned on success, "false" on failure. 
    */
    template <typename Histo>
    bool fillHistoFromText(std::istream& asciiStream,
                           Histo* histo, const unsigned* columnsToUse,
                           bool hasCommasBetweenValues=false);
}

#include "npstat/stat/fillHistoFromText.icc"

#endif // NPSTAT_FILLHISTOFROMTEXT_HH_
