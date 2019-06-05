#ifndef NPSTAT_ORDEREDPOINTND_HH_
#define NPSTAT_ORDEREDPOINTND_HH_

/*!
// \file OrderedPointND.hh
//
// \brief Multidimensional points which can be sorted according to
//        multiple sorting criteria
//
// Author: I. Volobouev
//
// September 2010
*/

#include <vector>

#include "npstat/nm/PointDimensionality.hh"

namespace npstat {
    template <typename Point> class OrderedPointND;

    template <typename Point>
    bool operator==(const OrderedPointND<Point>& l,
                    const OrderedPointND<Point>& r);

    template <typename Point>
    bool operator!=(const OrderedPointND<Point>& l,
                    const OrderedPointND<Point>& r);

    /**
    // Multivariate point which can remember its sequence number according to
    // multiple sorting criteria. These points can be used, for example, to
    // build composite distributions (i.e., modeled by copula and marginals).
    //
    // Template parameter class Point must be subscriptable. It must also
    // publish "value_type" typedef (std::array is a good candidate).
    */
    template <class Point>
    class OrderedPointND
    {
    public:
        typedef typename Point::value_type value_type;
        enum {dim_size = PointDimensionality<Point>::dim_size};

        /** Default constructor will invoke the default Point constructor */
        OrderedPointND();

        /** Constructor from the underlying point */
        OrderedPointND(const Point& point);

        /** Costructor from a data array for *the underlying point */
        template <typename Num2>
        OrderedPointND(const Num2* data, unsigned dataLen);

        /**
        // Constructor from the underlying point and its numbers in
        // several sorted sequences
        */
        OrderedPointND(const Point& point, const unsigned long *number,
                       unsigned lenNumber);
        //@{
        /** Examine object properties */
        inline const Point& point() const {return point_;}
        inline const value_type* coords() const {return point_.coords_;}
        inline const unsigned long* number() const {return number_;}
        inline unsigned dim() const {return dim_size;}
        inline unsigned size() const {return dim_size;}
        //@}

        /** Non-const subscripting */
        inline value_type& operator[](unsigned i) {return point_[i];}

        /** Const subscripting */
        inline const value_type& operator[](unsigned i) const
            {return point_[i];}

        /** Set point number according to sorting criterion i */
        void setNumber(unsigned i, unsigned long value);

        /** Get point number according to sorting criterion i */
        unsigned long getNumber(unsigned i) const;

        friend bool operator==<>(const OrderedPointND&, const OrderedPointND&);
        friend bool operator!=<>(const OrderedPointND&, const OrderedPointND&);

    private:
        void clearNumber();

        Point point_;
        unsigned long number_[dim_size];
    };

    /**
    // Fill a vector of ordered points using unordered points
    // (but do not order them yet). Note that the output vector
    // is _not_ cleared before filling. Class Point1 should be
    // subscriptable.
    //
    // Parameters "dimsToUse" and "nDimsToUse" specify how
    // dimensions of Point2 should be constructed out of
    // those in Point1.
    */
    template <class Point1, class Point2>
    void fillOrderedPoints(const std::vector<Point1>& data,
                           const unsigned* dimsToUse,
                           unsigned nDimsToUse,
                           std::vector<OrderedPointND<Point2> >* out);
}

#include "npstat/stat/OrderedPointND.icc"

#endif // NPSTAT_ORDEREDPOINTND_HH_
