#ifndef NPSTAT_BOXND_HH_
#define NPSTAT_BOXND_HH_

/*!
// \file BoxND.hh
//
// \brief Template to represent rectangles, boxes, and hyperboxes
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>

#include "geners/ClassId.hh"
#include "npstat/nm/Interval.hh"

namespace npstat {
    /**
    // Class to represent rectangles, boxes, and hyperboxes
    */
    template <typename Numeric>
    struct BoxND : public std::vector<Interval<Numeric> >
    {
        /** Default constructor makes a 0-dimensional box */
        inline BoxND() {}

        /** Interval in each dimension is made by its default constructor */
        inline explicit BoxND(const unsigned long dim) :
            std::vector<Interval<Numeric> >(dim) {}

        /** Use the same interval in each dimension */
        inline BoxND(const unsigned long dim, const Interval<Numeric>& v) :
            std::vector<Interval<Numeric> >(dim, v) {}

        /**
        // Constructor where one of the limits will be 0 and the other
        // will be generated from the given vector (which also determines
        // the dimensionality)
        */
        template <typename Num2>
        explicit BoxND(const std::vector<Num2>& limits);

        /** Converting constructor */
        template <typename Num2>
        explicit BoxND(const BoxND<Num2>& r);

        /**
        // Get the data from a box of a different type. This method
        // works essentially as a converting assignment operator.
        */
        template <typename Num2>
        BoxND& copyFrom(const BoxND<Num2>& r);

        /** Box dimensionality */
        inline unsigned long dim() const {return this->size();}

        /** Box volume */
        Numeric volume() const;

        /**
        // Midpoint for every coordinate. The size of the "coord"
        // array should be at least as large as the box dimensionality.
        */
        void getMidpoint(Numeric* coord, unsigned long coordLen) const;

        //@{
        /**
        // This method return "true" if the corresponding function
        // of the Interval returns "true" for every coordinate.
        // There must be an automatic conversion from Num2 type into Numeric.
        */
        template <typename Num2>
        bool isInsideLower(const Num2* coord, unsigned long coordLen) const;
        template <typename Num2>
        bool isInsideUpper(const Num2* coord, unsigned long coordLen) const;
        template <typename Num2>
        bool isInsideWithBounds(const Num2* coord, unsigned long coordLen) const;
        template <typename Num2>
        bool isInside(const Num2* coord, unsigned long coordLen) const;
        //@}

        //@{
        /** Scaling of all limits by a constant */
        BoxND& operator*=(double r);
        BoxND& operator/=(double r);
        //@}

        //@{
        /** Scaling by a different constant in each dimension */
        BoxND& operator*=(const std::vector<double>& scales);
        BoxND& operator/=(const std::vector<double>& scales);
        //@}

        /**
        // Scaling of all limits by a constant in such a way that the midpoint
        // remains unchanged
        */
        BoxND& expand(double r);

        //@{
        /**
        // Scaling of all limits in such a way that the midpoint
        // remains unchanged, using a different scaling factor
        // in each dimension
        */
        BoxND& expand(const std::vector<double>& scales);
        BoxND& expand(const double* scales, unsigned long lenScales);
        //@}

        //@{
        /** Shifting this object */
        template <typename Num2>
        BoxND& operator+=(const std::vector<Num2>& shifts);
        template <typename Num2>
        BoxND& operator-=(const std::vector<Num2>& shifts);
        template <typename Num2>
        BoxND& shift(const Num2* shifts, unsigned long lenShifts);
        //@}

        /** Moving this object so that the midpoint is (0, 0, ..., 0) */
        BoxND& moveToOrigin();

        /** Overlap volume with another box */
        Numeric overlapVolume(const BoxND& r) const;

        /** A faster way to calculate overlapVolume(r)/volume() */
        double overlapFraction(const BoxND& r) const;

        /** Box with lower limit 0 and upper limit 1 in all coordinates */
        static BoxND unitBox(unsigned long ndim);

        /**
        // Box with lower limit -1 and upper limit 1 in all coordinates.
        // Note that this will produce nonsense in case the Numeric type
        // is unsigned.
        */
        static BoxND sizeTwoBox(unsigned long ndim);

        /**
        // Box with all upper limits set to maximum possible Numeric
        // number and with lower limits set to negative maximum (or to
        // zero with unsigned types)
        */
        static BoxND allSpace(unsigned long ndim);

        //@{
        /** Methods related to I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in, BoxND* box);
    };
}

//@{
/** Binary comparison for equality */
template <typename Numeric>
bool operator==(const npstat::BoxND<Numeric>& l, const npstat::BoxND<Numeric>& r);

template <typename Numeric>
bool operator!=(const npstat::BoxND<Numeric>& l, const npstat::BoxND<Numeric>& r);
//@}

#include "npstat/nm/BoxND.icc"

#endif // NPSTAT_BOXND_HH_
