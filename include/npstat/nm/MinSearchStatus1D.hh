#ifndef NPSTAT_MINSEARCHSTATUS1D_HH_
#define NPSTAT_MINSEARCHSTATUS1D_HH_

/*!
// \file MinSearchStatus1D.hh
//
// \brief Summary status of a search for a function minimum of a 1-d interval
//
// Author: I. Volobouev
//
// May 2014
*/

namespace npstat {
    enum MinSearchStatus1D {
        MIN_SEARCH_OK = 0,
        MIN_ON_LEFT_EDGE,
        MIN_ON_RIGHT_EDGE,
        MIN_SEARCH_FAILED
    };
}

#endif // NPSTAT_MINSEARCHSTATUS1D_HH_
