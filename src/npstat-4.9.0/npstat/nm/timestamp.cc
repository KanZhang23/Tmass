#include "npstat/nm/timestamp.hh"

#include <ctime>
#include <cstdio>

namespace npstat {
    std::string timestamp()
    {
        struct tm *current;
        time_t now;
	
        time(&now);
        current = localtime(&now);

        char buf[16];
        sprintf(buf, "%02i:%02i:%02i", current->tm_hour,
                current->tm_min, current->tm_sec);
        return std::string(buf);
    }
}
