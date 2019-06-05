#ifndef NPSTAT_DISTRIBUTIONREADERROR_HH_
#define NPSTAT_DISTRIBUTIONREADERROR_HH_

/*!
// \file distributionReadError.hh
//
// \brief  Throw geners I/O exceptions in a standardized manner
//
// Author: I. Volobouev
//
// February 2012
*/

namespace npstat {
    /**
    // Simple utility function which throws an appropriate IOException
    // when "read" function fails for some I/O-capable statistical
    // distribution class
    */
    void distributionReadError(std::istream& in, const char* classname);
}

#endif // NPSTAT_DISTRIBUTIONREADERROR_HH_
