#ifndef NPSTAT_HISTOND_HH_
#define NPSTAT_HISTOND_HH_

/*! 
// \file HistoND.hh
//
// \brief Arbitrary-dimensional histogram template
//
// Author: I. Volobouev
//
// July 2010
*/

#include "npstat/nm/ArrayND.hh"
#include "npstat/stat/HistoAxis.hh"

namespace npstat {
    /**
    // (Almost) arbitrary-dimensional histogram with binning determined
    // by the second template parameter (typically HistoAxis or NUHistoAxis).
    // The dimensionality must not exceed CHAR_BIT*sizeof(unsigned long)-1
    // which is normally 31/63 on 32/64-bit systems.
    //
    // The template parameter class (Numeric) must be such that it can be
    // used as the template parameter of ArrayND class. For a typical usage
    // pattern, Numeric should also support operator += between itself and
    // the weights with which the histogram is filled (see, however, the
    // description of the "dispatch" method which is not subject to
    // this recommendation).
    //
    // If the "fillC" method is used to accumulate the data then the weights
    // must support multiplication by a double, and then it must be possible
    // to use the "+=" operator to add such a product to Numeric.
    //
    // Note that there are no methods which would allow the user to examine
    // the bin contents of the histogram using bin numbers. This is
    // intentional: almost always such examinations are performed in a loop
    // over indices, and it is more efficient to grab a reference to the 
    // underlying array using the "binContents()" method and then examine
    // that array directly.
    */
    template <typename Numeric, class Axis=HistoAxis>
    class HistoND
    {
        template <typename Num2, class Axis2> friend class HistoND;

    public:
        typedef Numeric value_type;
        typedef Axis axis_type;

        enum RebinType {
            SAMPLE = 0,
            SUM,
            AVERAGE
        };

        /** Main constructor for arbitrary-dimensional histograms */
        explicit HistoND(const std::vector<Axis>& axes, const char* title=0,
                         const char* accumulatedDataLabel=0);

        /** Convenience constructor for 1-d histograms */
        explicit HistoND(const Axis& xAxis, const char* title=0,
                         const char* accumulatedDataLabel=0);

        /** Convenience constructor for 2-d histograms */
        HistoND(const Axis& xAxis, const Axis& yAxis,
                const char* title=0, const char* accumulatedDataLabel=0);

        /** Convenience constructor for 3-d histograms */
        HistoND(const Axis& xAxis, const Axis& yAxis, const Axis& zAxis,
                const char* title=0, const char* accumulatedDataLabel=0);

        /** Convenience constructor for 4-d histograms */
        HistoND(const Axis& xAxis, const Axis& yAxis,
                const Axis& zAxis, const Axis& tAxis,
                const char* title=0, const char* accumulatedDataLabel=0);

        /** Convenience constructor for 5-d histograms */
        HistoND(const Axis& xAxis, const Axis& yAxis,
                const Axis& zAxis, const Axis& tAxis, const Axis& vAxis,
                const char* title=0, const char* accumulatedDataLabel=0);

        /**
        // Simple constructor for uniformly binned histograms without
        // axis labels. Sequence size returned by the size() method of
        // both "shape" and "boundingBox" arguments must be the same.
        */
        HistoND(const ArrayShape& shape, const BoxND<double>& boundingBox,
                const char* title=0, const char* accumulatedDataLabel=0);

        /**
        // Converting constructor. The functor will be applied to all bins
        // of the argument histogram to fill the bins of the constructed
        // histogram. If the title and data label are not provided, they
        // will be cleared.
        */
        template <typename Num2, class Functor>
        HistoND(const HistoND<Num2,Axis>& h, const Functor& f,
                const char* title=0, const char* accumulatedDataLabel=0);

        /**
        // A slicing constructor. The new histogram will be created by
        // slicing another histogram. See the description of the slicing
        // constructor in the "ArrayND" class for the meaning of arguments
        // "indices" and "nIndices". The data of the newly created histogram
        // is cleared.
        */
        template <typename Num2>
        HistoND(const HistoND<Num2,Axis>& h, const unsigned *indices,
                unsigned nIndices, const char* title=0);

        /**
        // A constructor that inserts a new axis into a histogram
        // (as if the argument histogram was a slice of the new histogram).
        // The "newAxisNumber" argument specifies the number of the
        // new axis in the axis sequence of the constructed histogram.
        // If the "newAxisNumber" exceeds the number of axes of the
        // argument histogram, the new axis will become last. The data
        // of the newly created histogram is cleared.
        */
        template <typename Num2>
        HistoND(const HistoND<Num2,Axis>& h, const Axis& newAxis,
                unsigned newAxisNumber, const char* title=0);

        /**
        // Create a rebinned histogram with the same axis coverage.
        // Note that not all such operations will be meaningful if the
        // bin contents do not belong to one of the floating point types.
        // The "newBinCounts" argument specifies the new number of bins
        // along each axis. The length of this array (provided by the
        // "lenNewBinCounts" argument) should be equal to the input
        // histogram dimensionality.
        //
        // The "shifts" argument can be meaningfully specified with the
        // "rType" argument set to "SAMPLE". These shifts will be added
        // to the bin centers of the created histogram when the bin contents
        // are looked up in the input histogram. This can be useful in case
        // the bin center lookup without shifts would fall exactly on the
        // bin edge. Naturally, the length of the "shifts" array should be
        // equal to the input histogram dimensionality.
        */
        template <typename Num2>
        HistoND(const HistoND<Num2,Axis>& h, RebinType rType,
                const unsigned *newBinCounts, unsigned lenNewBinCounts, 
                const double* shifts=0, const char* title=0);

        /** Copy constructor */
        HistoND(const HistoND&);

        /** 
        // Assignment operator. Works even when the binning of the two
        // histograms is not compatible.
        */
        HistoND& operator=(const HistoND&);

        /** Histogram dimensionality */
        inline unsigned dim() const {return dim_;}

        /** Histogram title */
        inline const std::string& title() const {return title_;}

        /** Label associated with accumulated data */
        inline const std::string& accumulatedDataLabel() const
            {return accumulatedDataLabel_;}

        /** Retrive a reference to the array of bin contents */
        inline const ArrayND<Numeric>& binContents() const {return data_;}

        /** Retrive a reference to the array of overflows */
        inline const ArrayND<Numeric>& overflows() const {return overflow_;}

        /** Inspect histogram axes */
        inline const std::vector<Axis>& axes() const {return axes_;}

        /** Inspect a histogram axis for the given dimension */
        inline const Axis& axis(const unsigned i) const
            {return axes_.at(i);}

        /** Total number of bins */
        inline unsigned long nBins() const {return data_.length();}

        /** Total number of fills performed */
        inline unsigned long nFillsTotal() const {return fillCount_;}

        /** Total number of fills which fell inside the histogram range */
        inline unsigned long nFillsInRange() const
            {return fillCount_ - overCount_;}

        /** Total number of fills which fell outside the histogram range */
        inline unsigned long nFillsOver() const {return overCount_;}

        /** 
        // This method returns "true" if the method isUniform()
        // of each histogram axis returns "true" 
        */
        bool isUniformlyBinned() const;

        /** Modify the histogram title */
        inline void setTitle(const char* newtitle)
            {title_ = newtitle ? newtitle : ""; ++modCount_;}

        /** Modify the label associated with accumulated data */
        inline void setAccumulatedDataLabel(const char* newlabel)
            {accumulatedDataLabel_ = newlabel ? newlabel : ""; ++modCount_;}

        /** Modify the label for the histogram axis with the given number */
        inline void setAxisLabel(const unsigned axisNum, const char* newlabel)
            {axes_.at(axisNum).setLabel(newlabel); ++modCount_;}

        /**
        // This method returns width/area/volume/etc. of a single bin.
        // 1.0 is returned for a dimensionless histogram.
        */
        double binVolume(unsigned long binNumber=0) const;

        /**
        // Position of the bin center. Length of the "coords" array
        // (filled on return) should be equal to the dimensionality
        // of the histogram.
        */
        void binCenter(unsigned long binNumber,
                       double* coords, unsigned lenCoords) const;

        /**
        // Convenience function which fills out a vector of bin centers
        // in the same order as the linear order of binContents().
        // The class "Point" must have a subscript operator, default
        // constructor, copy constructor, and the size() method (use,
        // for example, std::array).
        */
        template <class Point>
        void allBinCenters(std::vector<Point>* centers) const;

        /** Bounding box for the given bin */
        void binBox(unsigned long binNumber, BoxND<double>* box) const;

        /** Bounding box for the whole histogram */
        BoxND<double> boundingBox() const;

        /**
        // Volume of the histogram bounding box (this direct call is faster
        // than calling boundingBox().volume() ). This function returns 1.0
        // for 0-dim histogram, axis interval length for 1-d histogram, etc.
        */
        double volume() const;

        /** Integral of the histogram */
        double integral() const;

        /** Clear the histogram contents (both bins and overflows) */
        void clear();

        /** This method clears the bin contents but not overflows */
        void clearBinContents();

        /** This method clears overflows but not the bin contents */
        void clearOverflows();

        /** Comparison for equality */
        bool operator==(const HistoND&) const;

        /** Logical negation of operator== */
        bool operator!=(const HistoND&) const;

        /** 
        // Check data for equality (both bin contents and overflows).
        // Do not compare axes, labels, fill counts, etc.
        */
        bool isSameData(const HistoND&) const;

        /**
        // Fill function for histograms of arbitrary dimensionality.
        // The length of the "coords" array should be equal to the
        // histogram dimensionality. The Numeric type must have the "+="
        // operator defined with the Num2 type on the right side.
        */
        template <typename Num2>
        void fill(const double* coords, unsigned coordLength,
                  const Num2& weight);

        //@{
        /**
        // Convenience "fill" method for histograms of corresponding
        // dimensionality
        */
        template <typename Num2>
        void fill(const Num2& weight);

        template <typename Num2>
        void fill(double x0, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3,
                  const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  double x5, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  double x5, double x6, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  double x5, double x6, double x7, const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  double x5, double x6, double x7, double x8,
                  const Num2& weight);

        template <typename Num2>
        void fill(double x0, double x1, double x2, double x3, double x4,
                  double x5, double x6, double x7, double x8, double x9,
                  const Num2& weight);
        //@}

        /**
        // Location-based dispatch method. The provided binary functor
        // will be called with the approprite histogram bin value as the
        // first argument and the weight as the second (functor return value
        // is ignored). This allows for a very general use of the histogram
        // binning functionality. For example, with a proper functor, the
        // histogram bins can be filled with pointers to an arbitrary class
        // (this is the only way to use classes which do not have default
        // constructors as bin contents) and the functor can be used to
        // dispatch class methods. Depending on the exact nature of the
        // functor, multiple things might be modified as the result of this
        // call: the bin value, the weight, and the functor internal state.
        */
        template <typename Num2, class Functor>
        void dispatch(const double* coords, unsigned coordLength,
                      Num2& weight, Functor& f);

        //@{
        /**
        // Convenience "dispatch" method for histograms of corresponding
        // dimensionality
        */
        template <typename Num2, class Functor>
        void dispatch(Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, Num2& weight,
                      Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3,
                      Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      double x5, Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      double x5, double x6, Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      double x5, double x6, double x7, Num2& weight,
                      Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      double x5, double x6, double x7, double x8,
                      Num2& weight, Functor& f);

        template <typename Num2, class Functor>
        void dispatch(double x0, double x1, double x2, double x3, double x4,
                      double x5, double x6, double x7, double x8, double x9,
                      Num2& weight, Functor& f);
        //@}

        /**
        // The "examine" functions allow the user to access bin contents
        // when bins are addressed by their coordinates. Use "binContents()"
        // to access the data by bin numbers. Overflow bins will be accessed
        // if the given coordinates fall outside the histogram range.
        */
        const Numeric& examine(const double* coords,
                               unsigned coordLength) const;

        //@{
        /**
        // Convenience "examine" method for histograms of corresponding
        // dimensionality
        */
        const Numeric& examine() const;

        const Numeric& examine(double x0) const;

        const Numeric& examine(double x0, double x1) const;

        const Numeric& examine(double x0, double x1, double x2) const;

        const Numeric& examine(double x0, double x1, double x2,
                               double x3) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4, double x5) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4, double x5, double x6) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4, double x5, double x6,
                               double x7) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4, double x5, double x6, double x7,
                               double x8) const;

        const Numeric& examine(double x0, double x1, double x2, double x3,
                               double x4, double x5, double x6, double x7,
                               double x8, double x9) const;
        //@}

        /**
        // The "closestBin" functions are similar to the "examine" functions
        // but always return a valid bin and never overflow. This can be
        // useful for implementing lookup tables with constant extrapolation
        // outside of the histogram range.
        */
        const Numeric& closestBin(const double* coords,
                                  unsigned coordLength) const;

        //@{
        /**
        // Convenience "closestBin" method for histograms of corresponding
        // dimensionality
        */
        const Numeric& closestBin() const;

        const Numeric& closestBin(double x0) const;

        const Numeric& closestBin(double x0, double x1) const;

        const Numeric& closestBin(double x0, double x1, double x2) const;

        const Numeric& closestBin(double x0, double x1, double x2,
                                  double x3) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4, double x5) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4, double x5, double x6) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4, double x5, double x6,
                                  double x7) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4, double x5, double x6, double x7,
                                  double x8) const;

        const Numeric& closestBin(double x0, double x1, double x2, double x3,
                                  double x4, double x5, double x6, double x7,
                                  double x8, double x9) const;
        //@}

        /**
        // The "fillC" functions are similar to the "fill" methods but
        // they preserve the centroid of the deposit. Note that, if the
        // histogram dimensionality is high, "fillC" works significantly
        // slower than the corresponding "fill". Also note that there
        // must be at least 2 bins in each dimension in order for this
        // function to work.
        //
        // A word of caution. What is added to the bins is the input weight
        // multiplied by another weight calculated using the bin proximity.
        // If the input weight is just 1 (which happens quite often in
        // practice), the product of the weights is normally less than 1.
        // If the histogram template parameter is one of the integer types,
        // operator += will convert this product to 0 before adding it to
        // the bin! Therefore, it is best to use "fillC" only with floating
        // point template parameters (float, double, etc).
        // 
        // Currently, the "fillC" methods work sensibly only in the case
        // the binning is uniform (i.e., the second template parameter is
        // HistoAxis rather than, let say, NUHistoAxis). They typically
        // will not even compile if the binning is not uniform.
        */
        template <typename Num2>
        void fillC(const double* coords, unsigned coordLength,
                   const Num2& weight);

        //@{
        /**
        // Convenience "fillC" method for histograms of corresponding
        // dimensionality
        */
        template <typename Num2>
        void fillC(const Num2& weight);

        template <typename Num2>
        void fillC(double x0, const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3,
                   const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   double x5, const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   double x5, double x6, const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   double x5, double x6, double x7, const Num2& weight);

        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   double x5, double x6, double x7, double x8,
                   const Num2& weight);
        
        template <typename Num2>
        void fillC(double x0, double x1, double x2, double x3, double x4,
                   double x5, double x6, double x7, double x8, double x9,
                   const Num2& weight);
        //@}

        /**
        // Fill from another histogram. Compatibility of axis limits
        // will not be checked, but compatibility of array shapes will be.
        */
        template <typename Num2>
        HistoND& operator+=(const HistoND<Num2,Axis>& r);

        /**
        // Subtract contents of another histogram. Equivalent to multiplying
        // the contents of the other histogram by -1 and then adding them.
        // One of the consequences of this approach is that, for histograms
        // "a" and "b", the sequence of operations "a += b; a -= b;" does not
        // leave histogram "a" unchanged: although its bin contents will
        // remain the same (up to round-off errors), the fill counts will
        // increase by twice the fill counts of "b".
        */
        template <typename Num2>
        HistoND& operator-=(const HistoND<Num2,Axis>& r);

        //@{
        /** Method to set contents of individual bins (no bounds checking) */
        template <typename Num2>
        void setBin(const unsigned *index, unsigned indexLen, const Num2& v);

        template <typename Num2>
        void setBin(const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    unsigned i8, const Num2& v);

        template <typename Num2>
        void setBin(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    unsigned i8, unsigned i9, const Num2& v);

        template <typename Num2>
        inline void setLinearBin(const unsigned long index, const Num2& v)
            {data_.linearValue(index) = v; ++modCount_;}
        //@}

        //@{
        /** Method to set contents of individual bins with bounds checking */
        template <typename Num2>
        void setBinAt(const unsigned *index, unsigned indexLen, const Num2& v);

        template <typename Num2>
        void setBinAt(const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, unsigned i5, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, unsigned i5, unsigned i6, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                      const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                      unsigned i8, const Num2& v);

        template <typename Num2>
        void setBinAt(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                      unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                      unsigned i8, unsigned i9, const Num2& v);

        template <typename Num2>
        inline void setLinearBinAt(const unsigned long index, const Num2& v)
            {data_.linearValueAt(index) = v; ++modCount_;}
        //@}

        /** This method sets all bin contents in one fell swoop */
        template <typename Num2>
        void setBinContents(const Num2* data, unsigned long dataLength,
                            bool clearOverflows=true);

        /** This method sets all overflows in one fell swoop */
        template <typename Num2>
        void setOverflows(const Num2* data, unsigned long dataLength);

        /**
        // Setting bin contents to some constant value.
        // The Num2 type should allow automatic conversion to Numeric.
        */
        template <typename Num2>
        inline void setBinsToConst(const Num2& value)
            {data_.constFill(value); ++modCount_;}

        /**
        // Setting overflows to some constant value.
        // The Num2 type should allow automatic conversion to Numeric.
        */
        template <typename Num2>
        inline void setOverflowsToConst(const Num2& value)
            {overflow_.constFill(value); ++modCount_;}

        /**
        // This member function instructs the histogram to recalculate
        // the number of fills from data. It may be useful to call this
        // function after "setBinContents" in case the contents are filled
        // from another histogram.
        */
        void recalculateNFillsFromData();

        //@{
        /**
        // This method is intended for data format conversion
        // programs only, not for typical histogramming use
        */
        inline void setNFillsTotal(const unsigned long i)
            {fillCount_ = i; ++modCount_;}
        inline void setNFillsOver(const unsigned long i)
            {overCount_ = i; ++modCount_;}
        //@}

        /** In-place multiplication by a scalar (scaling) */
        template <typename Num2>
        HistoND& operator*=(const Num2& r);

        /** In-place division by a scalar */
        template <typename Num2>
        HistoND& operator/=(const Num2& r);

        //@{
        /** Multiplication by a value which is different for every bin */
        template <typename Num2>
        void scaleBinContents(const Num2* data, unsigned long dataLength);

        template <typename Num2>
        void scaleOverflows(const Num2* data, unsigned long dataLength);
        //@}

        //@{
        /**
        // In-place addition of a scalar to all bins. Equivalent to calling
        // the "fill" function with the same weight once for every bin.
        */
        template <typename Num2>
        void addToBinContents(const Num2& weight);

        template <typename Num2>
        void addToOverflows(const Num2& weight);
        //@}

        //@{
        /**
        // In-place addition of an array. Equivalent to calling the "fill"
        // function once for every bin with the weight taken from the
        // corresponding array element.
        */
        template <typename Num2>
        void addToBinContents(const Num2* data, unsigned long dataLength);

        template <typename Num2>
        void addToOverflows(const Num2* data, unsigned long dataLength);
        //@}

        /**
        // Add contents of all bins inside the given box to the accumulator.
        // Note that Numeric type must support multiplication by a double
        // in order for this function to work (it calculates the overlap
        // fraction of each bin with the box and multiplies bin content
        // by that fraction for subsequent accumulation). The operation
        // Acc += Numeric must be defined.
        */
        template <typename Acc>
        void accumulateBinsInBox(const BoxND<double>& box, Acc* acc,
                                 bool calculateAverage = false) const;

        //@{
        /**
        // Code for projecting one histogram onto another. For now,
        // this is done for bin contents only, not for overflows.
        // The projection should be created in advance from this
        // histogram with the aid of the slicing constructor. The indices
        // used in that constructor should be provided here as well.
        //
        // Note that you might want to recalculate the number of fills
        // from data after performing all projections needed.
        */
        template <typename Num2, typename Num3>
        void addToProjection(HistoND<Num2,Axis>* projection,
                             AbsArrayProjector<Numeric,Num3>& projector,
                             const unsigned *projectedIndices,
                             unsigned nProjectedIndices) const;

        template <typename Num2, typename Num3>
        void addToProjection(HistoND<Num2,Axis>* projection,
                             AbsVisitor<Numeric,Num3>& projector,
                             const unsigned *projectedIndices,
                             unsigned nProjectedIndices) const;
        //@}

        /** Transpose the histogram axes and bin contents */
        HistoND transpose(unsigned axisNum1, unsigned axisNum2) const;

        /**
        // This method returns the number of modifications
        // performed on the histogram since its creation. This number
        // is always increasing during the lifetime of the histogram
        // object. Its main property is as follows: if the method
        // "getModCount" returns the same number twice, there should
        // be no changes in the histogram object (so that a drawing
        // program does not need to redraw the histogram image).
        //
        // This number is pure transient, it is not serialized and
        // does not participate in histogram comparisons for equality.
        */
        inline unsigned long getModCount() const {return modCount_;}

        /**
        // Indicate that the histogram contents have changed. Should
        // be used by any code which directly modifies histogram bins
        // (after using const_cast on the relevant reference).
        */
        inline void incrModCount() {++modCount_;}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static HistoND* read(const gs::ClassId& id, std::istream& in);

    private:
        HistoND();

        // Special constructor which speeds up the "transpose" operation.
        // Does not do full error checking (some of it is done in transpose).
        HistoND(const HistoND& r, unsigned ax1, unsigned ax2);

        template <typename Num2>
        void fillPreservingCentroid(const Num2& weight);

        template <typename Acc>
        void accumulateBinsLoop(unsigned level, const BoxND<double>& box,
                                unsigned* idx, Acc* accumulator,
                                double overlapFraction, long double* wsum) const;
        std::string title_;
        std::string accumulatedDataLabel_;
        ArrayND<Numeric> data_;
        ArrayND<Numeric> overflow_;
        std::vector<Axis> axes_;
        mutable std::vector<double> weightBuf_;
        mutable std::vector<unsigned> indexBuf_;
        unsigned long fillCount_;
        unsigned long overCount_;
        unsigned long modCount_;
        unsigned dim_;

#ifdef SWIG
    // Simplified non-template methods for the python API. Not for use in C++.
    public:
        inline Numeric examine2(const std::vector<double>& c) const
        {
            const unsigned sz = c.size();
            return examine(sz ? &c[0] : (double*)0, sz);
        }

        inline Numeric examine2() const
            {return examine();}

        inline Numeric examine2(double x0) const
            {return examine(x0);}

        inline Numeric examine2(double x0, double x1) const
            {return examine(x0, x1);}

        inline Numeric examine2(double x0, double x1, double x2) const
            {return examine(x0, x1, x2);}

        inline Numeric examine2(double x0, double x1, double x2,
                                double x3) const
            {return examine(x0, x1, x2, x3);}

        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4) const
            {return examine(x0, x1, x2, x3, x4);}

        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4, double x5) const
            {return examine(x0, x1, x2, x3, x4, x5);}

        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4, double x5, double x6) const
            {return examine(x0, x1, x2, x3, x4, x5, x6);}

        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4, double x5, double x6,
                                double x7) const
            {return examine(x0, x1, x2, x3, x4, x5, x6, x7);}
        
        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4, double x5, double x6, double x7,
                                double x8) const
            {return examine(x0, x1, x2, x3, x4, x5, x6, x7, x8);}

        inline Numeric examine2(double x0, double x1, double x2, double x3,
                                double x4, double x5, double x6, double x7,
                                double x8, double x9) const
            {return examine(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);}

        inline Numeric closestBin2(const std::vector<double>& c) const
        {
            const unsigned sz = c.size();
            return closestBin(sz ? &c[0] : (double*)0, sz);
        }

        inline Numeric closestBin2() const
            {return closestBin();}

        inline Numeric closestBin2(double x0) const
            {return closestBin(x0);}

        inline Numeric closestBin2(double x0, double x1) const
            {return closestBin(x0, x1);}

        inline Numeric closestBin2(double x0, double x1, double x2) const
            {return closestBin(x0, x1, x2);}

        inline Numeric closestBin2(double x0, double x1, double x2,
                                   double x3) const
            {return closestBin(x0, x1, x2, x3);}

        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4) const
            {return closestBin(x0, x1, x2, x3, x4);}

        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4, double x5) const
            {return closestBin(x0, x1, x2, x3, x4, x5);}

        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4, double x5, double x6) const
            {return closestBin(x0, x1, x2, x3, x4, x5, x6);}

        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4, double x5, double x6,
                                   double x7) const
            {return closestBin(x0, x1, x2, x3, x4, x5, x6, x7);}
        
        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4, double x5, double x6, double x7,
                                   double x8) const
            {return closestBin(x0, x1, x2, x3, x4, x5, x6, x7, x8);}

        inline Numeric closestBin2(double x0, double x1, double x2, double x3,
                                   double x4, double x5, double x6, double x7,
                                   double x8, double x9) const
            {return closestBin(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);}

        inline void setBin2(const std::vector<unsigned>& index, Numeric v)
        {
            const unsigned sz = index.size();
            setBinAt(sz ? &index[0] : (unsigned*)0, sz, v);
        }

        inline void setBin2(Numeric v)
            {setBinAt(v);}

        inline void setBin2(unsigned i0, Numeric v)
            {setBinAt(i0, v);}

        inline void setBin2(unsigned i0, unsigned i1, Numeric v)
            {setBinAt(i0, i1, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, Numeric v)
            {setBinAt(i0, i1, i2, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    Numeric v)
            {setBinAt(i0, i1, i2, i3, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, i5, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, i5, i6, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, i5, i6, i7, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    unsigned i8, Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, i5, i6, i7, i8, v);}

        inline void setBin2(unsigned i0, unsigned i1, unsigned i2, unsigned i3,
                    unsigned i4, unsigned i5, unsigned i6, unsigned i7,
                    unsigned i8, unsigned i9, Numeric v)
            {setBinAt(i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, v);}

        inline void setLinearBin2(const unsigned long index, Numeric v)
            {data_.linearValueAt(index) = v; ++modCount_;}

        inline void setBinsToConst2(const Numeric value)
            {data_.constFill(value); ++modCount_;}

        inline void setOverflowsToConst2(const Numeric value)
            {overflow_.constFill(value); ++modCount_;}

        inline void fill2(const std::vector<double>& c, const Numeric weight)
        {
            const unsigned sz = c.size();
            fill(sz ? &c[0] : (double*)0, sz, weight);
        }

        inline void fill2(const Numeric weight)
            {fill(weight);}

        inline void fill2(double x0, const Numeric weight)
            {fill(x0, weight);}

        inline void fill2(double x0, double x1, const Numeric weight)
            {fill(x0, x1, weight);}

        inline void fill2(double x0, double x1, double x2, const Numeric weight)
            {fill(x0, x1, x2, weight);}

        inline void fill2(double x0, double x1, double x2, double x3,
                          const Numeric weight)
            {fill(x0, x1, x2, x3, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          const Numeric weight)
            {fill(x0, x1, x2, x3, x4, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          double x5, const Numeric weight)
            {fill(x0, x1, x2, x3, x4, x5, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          double x5, double x6, const Numeric weight)
            {fill(x0, x1, x2, x3, x4, x5, x6, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          double x5, double x6, double x7, const Numeric weight)
            {fill(x0, x1, x2, x3, x4, x5, x6, x7, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          double x5, double x6, double x7, double x8,
                          const Numeric weight)
            {fill(x0, x1, x2, x3, x4, x5, x6, x7, x8, weight);}

        inline void fill2(double x0, double x1, double x2, double x3, double x4,
                          double x5, double x6, double x7, double x8, double x9,
                          const Numeric weight)
            {fill(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, weight);}

        inline void fillC2(const std::vector<double>& c, const Numeric weight)
        {
            const unsigned sz = c.size();
            fillC(sz ? &c[0] : (double*)0, sz, weight);
        }

        inline void fillC2(const Numeric weight)
            {fillC(weight);}

        inline void fillC2(double x0, const Numeric weight)
            {fillC(x0, weight);}
        
        inline void fillC2(double x0, double x1, const Numeric weight)
            {fillC(x0, x1, weight);}
        
        inline void fillC2(double x0, double x1, double x2,
                           const Numeric weight)
            {fillC(x0, x1, x2, weight);}

        inline void fillC2(double x0, double x1, double x2, double x3,
                           const Numeric weight)
            {fillC(x0, x1, x2, x3, weight);}
        
        inline void fillC2(double x0, double x1, double x2,
                           double x3, double x4, const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, weight);}
        
        inline void fillC2(double x0, double x1, double x2, double x3,
                           double x4, double x5, const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, x5, weight);}
        
        inline void fillC2(double x0, double x1, double x2, double x3,
                           double x4, double x5, double x6,
                           const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, x5, x6, weight);}
        
        inline void fillC2(double x0, double x1, double x2,
                           double x3, double x4, double x5,
                           double x6, double x7, const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, x5, x6, x7, weight);}

        inline void fillC2(double x0, double x1, double x2, double x3,
                           double x4, double x5, double x6, double x7,
                           double x8, const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, x5, x6, x8, weight);}
        
        inline void fillC2(double x0, double x1, double x2, double x3,
                           double x4, double x5, double x6, double x7,
                           double x8, double x9, const Numeric weight)
            {fillC(x0, x1, x2, x3, x4, x5, x6, x8, x9, weight);}
#endif // SWIG
    };

    /**
    // Reset negative histogram bins to zero and then divide histogram
    // bin contents by the histogram integral. If the "knownNonNegative"
    // argument is true, it will be assumed that there are no negative
    // bins, and their explicit reset is unnecessary.
    //
    // This function will throw std::runtime_error in case the histogram
    // is empty after all negative bins are reset.
    //
    // This function is not a member of the HistoND class itself because
    // these operations do not necessarily make sense for all bin types.
    // Making such operation a member would make creation of HistoND
    // scripting API (e.g., for python) more difficult.
    */
    template <typename Histo>
    void convertHistoToDensity(Histo* histogram, bool knownNonNegative=false);

    /**
    // Generate a density scanning map for subsequent use with
    // the "DensityScanND" template. Naturally, only histograms
    // with uniform binning can be used here.
    */
    template <typename Histo>
    std::vector<LinearMapper1d> densityScanHistoMap(const Histo& histo);

    /**
    // Generate a density scanning map for subsequent use with the
    // "DensityScanND" template when a density is to be convolved with
    // the histogram data. Only histograms with uniform binning
    // can be used here.
    //
    // The "doubleDataRange" should be set "true" in case the data
    // will be mirrored (or just empty range added) to avoid circular
    // spilling after convolution.
    */
    template <typename Histo>
    std::vector<CircularMapper1d> convolutionHistoMap(const Histo& histo,
                                                      bool doubleDataRange);
}

#include "npstat/stat/HistoND.icc"

#endif // NPSTAT_HISTOND_HH_
