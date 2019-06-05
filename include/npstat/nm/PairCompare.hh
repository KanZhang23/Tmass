#ifndef NPSTAT_PAIRCOMPARE_HH_
#define NPSTAT_PAIRCOMPARE_HH_

/*!
// \file PairCompare.hh
//
// \brief Various comparison functors for std::pair and npstat::Triple
//
// Author: I. Volobouev
//
// December 2011
*/

namespace npstat {
    /** "less" functor to compare the first element only */
    template <typename T>
    struct LessByFirst
    {
        inline bool operator()(const T& x, const T& y) const
            {return x.first < y.first;}
        inline bool operator()(const T* x, const T* y) const
            {return x->first < y->first;}
    };

    /** "greater" functor to compare the first element only */
    template <typename T>
    struct GreaterByFirst
    {
        inline bool operator()(const T& x, const T& y) const
            {return y.first < x.first;}
        inline bool operator()(const T* x, const T* y) const
            {return y->first < x->first;}
    };

    /** "less" functor to compare the second element only */
    template <typename T>
    struct LessBySecond
    {
        inline bool operator()(const T& x, const T& y) const
            {return x.second < y.second;}
        inline bool operator()(const T* x, const T* y) const
            {return x->second < y->second;}
    };

    /** "greater" functor to compare the second element only */
    template <typename T>
    struct GreaterBySecond
    {
        inline bool operator()(const T& x, const T& y) const
            {return y.second < x.second;}
        inline bool operator()(const T* x, const T* y) const
            {return y->second < x->second;}
    };

    /** "less" functor to compare the third element only */
    template <typename T>
    struct LessByThird
    {
        inline bool operator()(const T& x, const T& y) const
            {return x.third < y.third;}
        inline bool operator()(const T* x, const T* y) const
            {return x->third < y->third;}
    };

    /** "greater" functor to compare the third element only */
    template <typename T>
    struct GreaterByThird
    {
        inline bool operator()(const T& x, const T& y) const
            {return y.third < x.third;}
        inline bool operator()(const T* x, const T* y) const
            {return y->third < x->third;}
    };
}

#endif // NPSTAT_PAIRCOMPARE_HH_
