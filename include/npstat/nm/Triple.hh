#ifndef NPSTAT_TRIPLE_HH_
#define NPSTAT_TRIPLE_HH_

/*!
// \file Triple.hh
//
// \brief A simple analogue of std::pair with three components instead of two
//
// Author: I. Volobouev
//
// June 2012
*/

namespace npstat {
    /** A simple analogue of std::pair with three components */
    template <class First, class Second, class Third>
    struct Triple
    {
        typedef First  first_type;
        typedef Second second_type;
        typedef Third  third_type;

        /**
        // Default constructor of the triple calls default constructors
        // of each component type
        */
        inline Triple() : first(First()), second(Second()), third(Third()) {}

        /** Construct a triple by combining three different objects */
        inline Triple(const First& f, const Second& s, const Third& t)
            : first(f), second(s), third(t) {}

        /** Converting copy constructor */
        template<class F2, class S2, class T2>
        inline Triple(const Triple<F2, S2, T2>& r)
            : first(r.first), second(r.second), third(r.third) {}

        /** Converting assignment operator */
        template<class F2, class S2, class T2>
        inline Triple& operator=(const Triple<F2, S2, T2>& r)
        {
            if (static_cast<void*>(this) != static_cast<const void*>(&r))
            {
                first = r.first;
                second = r.second;
                third = r.third;
            }
            return *this;
        }

        /** Comparison for equality */
        inline bool operator==(const Triple& r) const
            {return first == r.first && second == r.second && third == r.third;}

        /** Logical negation of operator== */
        inline bool operator!=(const Triple& r) const
            {return !(*this == r);}

        //@{
        /**
        // Ordering comparison. Initially, first elements are compared.
        // If they are equal, second elements are compared, etc.
        */
        inline bool operator<(const Triple& r) const
        {
            if (first < r.first) return true;
            if (r.first < first) return false;
            if (second < r.second) return true;
            if (r.second < second) return false;
            return third < r.third;
        }

        inline bool operator>(const Triple& r) const
        {
            if (first > r.first) return true;
            if (r.first > first) return false;
            if (second > r.second) return true;
            if (r.second > second) return false;
            return third > r.third;
        }
        //@}

        First first;    ///< First element of the triple
        Second second;  ///< Second element of the triple
        Third third;    ///< Third element of the triple
    };

    /** Utility function for triples similar to std::make_pair */
    template <class First, class Second, class Third>
    inline Triple<First, Second, Third> make_Triple(
        const First& f, const Second& s, const Third& t)
    {
        return Triple<First, Second, Third>(f, s, t);
    }
}

#endif // NPSTAT_TRIPLE_HH_
