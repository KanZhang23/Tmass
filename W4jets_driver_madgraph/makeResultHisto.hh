#ifndef MAKERESULTHISTO_HH_
#define MAKERESULTHISTO_HH_

#include "JesIntegResult.hh"

int makeResultHisto(int uid, const char* title, const char* category,
                    const JesIntegResult& result, bool lastSetOnly,
                    double verticalScale, bool useQmcUncertainties);

#endif // MAKERESULTHISTO_HH_
