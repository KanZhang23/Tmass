#ifndef MISC_UTILS_HH_
#define MISC_UTILS_HH_

#include <cmath>
#include <string>
#include <algorithm>

inline double relative_delta(const double x, const double y)
{
    return x || y ? fabs(x - y)/std::max(fabs(x), fabs(y)) : 0.0;
}

// gettimeofday translated into seconds
double time_in_seconds();

// Return the string representing current time in the format hh:mm:ss
std::string time_stamp();

bool is_power_of_two(unsigned u);

#endif // MISC_UTILS_HH_
