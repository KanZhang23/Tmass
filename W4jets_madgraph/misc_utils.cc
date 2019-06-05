#include "misc_utils.hh"

#include <cassert>
#include <ctime>
#include <cstdio>

#include <sys/time.h>

double time_in_seconds()
{
    struct timeval tv;
    const int status = gettimeofday(&tv, NULL);
    assert(status == 0);
    const double usec = tv.tv_usec;
    return tv.tv_sec + usec/1.0e6;
}

std::string time_stamp()
{
    struct tm *current;
    time_t now;

    time(&now);
    current = localtime(&now);

    char buf[10];
    sprintf(buf, "%02i:%02i:%02i", current->tm_hour,
            current->tm_min, current->tm_sec);
    return std::string(buf);
}

bool is_power_of_two(unsigned u)
{
    unsigned count = 0;
    for (unsigned i=0; i<sizeof(unsigned)*8 && u && count < 2; ++i)
    {
        count += u & 1;
        u >>= 1;
    }
    return count == 1;
}
