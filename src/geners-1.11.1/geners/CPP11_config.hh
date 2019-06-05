#ifndef GENERS_CPP11_CONFIG_HH_
#define GENERS_CPP11_CONFIG_HH_

#include "geners/platform.hh"

#ifdef __GXX_EXPERIMENTAL_CXX0X__

#ifndef CPP11_STD_AVAILABLE
#define CPP11_STD_AVAILABLE
#endif

#endif // __GXX_EXPERIMENTAL_CXX0X__

#ifdef USING_MS_WINDOWS_CPP

#if _MSC_VER >= 1600
#ifndef CPP11_STD_AVAILABLE
#define CPP11_STD_AVAILABLE
#endif
#endif

#endif // USING_MS_WINDOWS_CPP

#endif // GENERS_CPP11_CONFIG_HH_
