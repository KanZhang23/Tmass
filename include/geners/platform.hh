#ifndef GENERS_PLATFORM_HH_
#define GENERS_PLATFORM_HH_

#ifdef USING_MS_WINDOWS_CPP
#undef USING_MS_WINDOWS_CPP
#endif

#if (defined(_WIN32) || defined(_WIN64)) && defined(_MSC_VER)
#define USING_MS_WINDOWS_CPP
#endif

#endif // GENERS_PLATFORM_HH_
