#ifndef NPSTAT_COMPAREBYINDEX_HH_
#define NPSTAT_COMPAREBYINDEX_HH_

/*!
// \file CompareByIndex.hh
//
// \brief Compare subscriptable objects (e.g., vectors) by their k-th element
//
// Author: I. Volobouev
//
// March 2010
*/

namespace npstat {
    /**
    // Comparison functor for classes which support subscripting and for
    // pointers to such classes. To be used with std::sort and such.
    */
    template <typename T>
    class LessByIndex
    {
    public:
        /** Constructor takes the index of the element to be compared */
        inline explicit LessByIndex(const unsigned i) : i_(i) {}

        inline bool operator()(const T& x, const T& y) const
            {return x[i_] < y[i_];}
        inline bool operator()(const T* x, const T* y) const
            {return (*x)[i_] < (*y)[i_];}
    private:
        LessByIndex();
        unsigned i_;
    };

    /**
    // Comparison functor for classes which support subscripting and for
    // pointers to such classes. To be used with std::sort and such.
    */
    template <typename T>
    class GreaterByIndex
    {
    public:
        /** Constructor takes the index of the element to be compared */
        inline explicit GreaterByIndex(const unsigned i) : i_(i) {}

        inline bool operator()(const T& x, const T& y) const
            {return y[i_] < x[i_];}
        inline bool operator()(const T* x, const T* y) const
            {return (*y)[i_] < (*x)[i_];}
    private:
        GreaterByIndex();
        unsigned i_;
    };
}

#endif // NPSTAT_COMPAREBYINDEX_HH_
