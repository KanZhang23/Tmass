#ifndef GENERS_REGEX_HH_
#define GENERS_REGEX_HH_

#include "geners/CPP11_config.hh"

#ifdef CPP11_STD_AVAILABLE

// Use C++11 regex matching

#include <regex>

namespace gs {
    typedef std::regex Regex;
}

#else // CPP11_STD_AVAILABLE

// Use POSIX extended regex matching

#include <string>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <regex.h>

namespace gs {
    class SearchSpecifier;

    class Regex
    {
        friend class SearchSpecifier;

        std::string re_;
        mutable regex_t* preg_;

        inline void cleanup()
        {
            if (preg_)
            {
                regfree(preg_);
                delete preg_;
                preg_ = 0;
            }
        }

        inline bool matches(const std::string& sentence) const
        {
            if (!preg_)
            {
                preg_ = new regex_t();
                const int ecode = regcomp(preg_, re_.c_str(),
                                          REG_EXTENDED | REG_NOSUB);
                if (ecode)
                {
                    const size_t n = regerror(ecode, preg_, 0, 0);
                    std::string s;
                    s.resize(n+1);
                    regerror(ecode, preg_, const_cast<char*>(s.data()), n+1);
                    std::ostringstream errmess;
                    errmess << "In gs::Regex::matches: failed to compile "
                            << "regular expression \"" << re_.c_str()
                            << "\": " << s.data();
                    throw std::invalid_argument(errmess.str());
                }
            }
            return regexec(preg_, sentence.c_str(), 0, 0, 0) == 0;
        }

    public:
        inline Regex() : preg_(0) {}

        inline Regex(const std::string& re) 
            : re_(re), preg_(new regex_t())
        {
            const int statusCode = regcomp(preg_, re_.c_str(),
                                           REG_EXTENDED | REG_NOSUB);
            if (statusCode)
            {
                const size_t n = regerror(statusCode, preg_, 0, 0);
                std::string s;
                s.resize(n+1);
                regerror(statusCode, preg_, const_cast<char*>(s.data()), n+1);
                cleanup();
                throw std::invalid_argument(s.data());
            }
        }

        inline Regex(const Regex& r) : re_(r.re_), preg_(0) {}

        inline Regex& operator=(const Regex& r)
        {
            if (&r == this)
                return *this;
            re_ = r.re_;
            cleanup();
            return *this;
        }

        inline ~Regex() {cleanup();}
    };
}

#endif // CPP11_STD_AVAILABLE
#endif // GENERS_REGEX_HH_
