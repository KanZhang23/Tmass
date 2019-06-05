#ifndef NPSTAT_COLUMN_HH_
#define NPSTAT_COLUMN_HH_

/*!
// \file Column.hh
//
// \brief Address ntuple columns by name or by number
//
// Author: I. Volobouev
//
// November 2010
*/

#include <string>
#include <cassert>

namespace npstat {
    template <typename T>
    class AbsNtuple;

    /**
    // This class serves just one purpose: it allows to address ntuple columns
    // by either name or number in the same function call. Do not create
    // stand-alone objects of this class in your application code and do not
    // include this header directly (it will be included automatically when
    // you include AbsNtuple.hh). Objects of this class are not meant to be
    // reused, and doing so can lead to subtle bugs.
    */
    class Column
    {
    public:
        /** Constructor from unsigned integer */
        inline Column(const unsigned long i)
            : col_(i), name_(0), lastNt_(0), mode_(0) {}

        /** Constructor from c-string */
        inline Column(const char* name)
            : col_(0), name_(name), lastNt_(0), mode_(1) {assert(name);}

        /** Constructor from std::string */
        inline Column(const std::string& name)
            : col_(0), name_(0), st_(name), lastNt_(0), mode_(2) {}

        /**
        // Can't call "isValid" after operator(): this will result
        // in a run-time error
        */
        template <typename T>
        bool isValid(const AbsNtuple<T>& ntuple) const;

        /**
        // This will return the column number in the given ntuple
        // or will throw an exception if the column specification
        // is invalid
        */
        template <typename T>
        unsigned long operator()(const AbsNtuple<T>& ntuple) const;

    private:
        Column();

        mutable unsigned long col_;
        const char* name_;
        std::string st_;
        mutable const void* lastNt_;
        unsigned mode_;
    };
}

#endif // NPSTAT_COLUMN_HH_
