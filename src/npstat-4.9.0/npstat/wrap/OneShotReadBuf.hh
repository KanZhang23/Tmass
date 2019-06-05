#ifndef NPSTAT_ONESHOTREADBUF_HH_
#define NPSTAT_ONESHOTREADBUF_HH_

#include <streambuf>

namespace npstat {
    namespace Private {
        struct OneShotReadBuf : public std::streambuf
        {
            inline OneShotReadBuf(char* s, const std::size_t n)
            {
                this->setg(s, s, s + n);
            }
        };
    }
}

#endif // NPSTAT_ONESHOTREADBUF_HH_
