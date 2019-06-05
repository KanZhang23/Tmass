#ifndef NPSTAT_NTRECTANGULARCUT_HH_
#define NPSTAT_NTRECTANGULARCUT_HH_

/*!
// \file NtRectangularCut.hh
//
// \brief Rectangular cuts for homogeneous ntuples
//
// Author: I. Volobouev
//
// November 2011
*/

#include "npstat/nm/BoxND.hh"

namespace npstat {
    /**
    // This convenience class implements simple rectangular cuts
    // on ntuple contents. Cut lower bound is included and cut upper bound
    // is excluded from the passing sample. It is possible to specify
    // min >= max for any rectangle dimension. In this case no ntuple rows
    // will be accepted by the cut.
    //
    // The template parameter should coincide with the ntuple value_type.
    //
    // For use inside "conditionalCycleOverRows", and other similar methods
    // of AbsNtuple.
    */
    template<typename Numeric>
    class NtRectangularCut
    {
        template <typename Num2> friend class NtRectangularCut;

    public:
        /** Default constructor creates an "all pass" cut */
        inline NtRectangularCut() : nCuts_(0) {}

        //@{
        /**
        // For this convenience constructor, there must be 
        // an automatic conversion from type T to Numeric.
        //
        // It is assumed that correctness of the ntuple column numbers
        // has already been verified. Use "validColumn()" ntuple method
        // or other similar checks to ensure this correctness and do not
        // reuse cut objects with ntuples that might have different
        // column structure.
        */
        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4,
                         unsigned long c5, const T& min5, const T& max5);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4,
                         unsigned long c5, const T& min5, const T& max5,
                         unsigned long c6, const T& min6, const T& max6);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4,
                         unsigned long c5, const T& min5, const T& max5,
                         unsigned long c6, const T& min6, const T& max6,
                         unsigned long c7, const T& min7, const T& max7);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4,
                         unsigned long c5, const T& min5, const T& max5,
                         unsigned long c6, const T& min6, const T& max6,
                         unsigned long c7, const T& min7, const T& max7,
                         unsigned long c8, const T& min8, const T& max8);

        template<typename T>
        NtRectangularCut(unsigned long c0, const T& min0, const T& max0,
                         unsigned long c1, const T& min1, const T& max1,
                         unsigned long c2, const T& min2, const T& max2,
                         unsigned long c3, const T& min3, const T& max3,
                         unsigned long c4, const T& min4, const T& max4,
                         unsigned long c5, const T& min5, const T& max5,
                         unsigned long c6, const T& min6, const T& max6,
                         unsigned long c7, const T& min7, const T& max7,
                         unsigned long c8, const T& min8, const T& max8,
                         unsigned long c9, const T& min9, const T& max9);
        //@}

        /**
        // Constructor for a cut of arbitrary dimensionality. Note that
        // it is impossible to specify a box with negative side length.
        */
        template<typename T>
        NtRectangularCut(const std::vector<unsigned long>& ntupleColumns,
                         const BoxND<T>& acceptedBox);

        /**
        // Add a cut on column contents to the set. Returns *this, so that
        // a bunch of calls of this method can be stacked.
        */
        template<typename T>
        NtRectangularCut& addCut(unsigned long cutColumn,
                                 const T& min, const T& max,
                                 bool inverted=false);

        /** Add all cuts from another object of similar kind */
        template<typename T>
        NtRectangularCut& addCut(const NtRectangularCut<T>& another);        

        /** Invert the meaning of all cuts added so far to this object */
        void invert();

        /** The number of cuts defined so far */
        inline unsigned long dim() const {return nCuts_;}

        /** In this method, cutNumber parameter should be less than dim() */
        inline unsigned long cutColumn(const unsigned long cutNumber) const
            {return cutData_.at(cutNumber).col;}

        /**
        // In principle, min can be larger than max. Because of this, we
        // can't just return Interval<Numeric> when inspecting cut values.
        */
        void cutInterval(unsigned long cutNumber,
                         Numeric* minValue, Numeric* maxValue) const;

        /** Was the cut meaning inverted? */
        bool isInverted(unsigned long cutNumber) const;

        /**
        // The following method returns the number of unique columns.
        // It may be useful to check that nUniqueColumns() == dim() after
        // the cut is fully constructed (unless you intentionally include
        // more than one cut for some column).
        */
        unsigned long nUniqueColumns() const;

        /**
        // Convert to column vector / cut box. Intervals in which
        // min >= max are converted into [min, min]. "acceptedBox"
        // method will throw std::invalid_argument if any of the
        // cuts have been inverted.
        */
        std::vector<unsigned long> ntupleColumns() const;
        BoxND<Numeric> acceptedBox() const;

        /** Operator which returns "true" if an ntuple row passes the cut */
        bool operator()(const Numeric* row, unsigned long nCols) const;

        /** Comparison for equality */
        bool operator==(const NtRectangularCut<Numeric>& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const NtRectangularCut<Numeric>& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            NtRectangularCut* cut);
    private:
        struct RCut
        {
            inline RCut(unsigned long icol, Numeric imin,
                        Numeric imax, bool inv)
                : col(icol), min(imin), max(imax), inverted(inv) {}

            inline bool operator==(const RCut& r) const
                {return col == r.col && min == r.min && 
                        inverted == r.inverted &&
                        (max == r.max || (max <= min && r.max <= min));}

            unsigned long col;
            Numeric min;
            Numeric max;
            bool inverted;
        };

        std::vector<RCut> cutData_;
        unsigned long nCuts_;

#ifdef SWIG
    public:
        inline void addCut2(unsigned long cutColumn,
                            const Numeric& min, const Numeric& max,
                            bool inverted=false)
            {addCut(cutColumn, min, max, inverted);}

        inline void addCut2(const NtRectangularCut<Numeric>& another)
            {addCut(another);}
#endif
    };
}

#include "npstat/stat/NtRectangularCut.icc"

#endif // NPSTAT_NTRECTANGULARCUT_HH_
