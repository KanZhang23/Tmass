#ifndef NPSTAT_ARRAYND_HH_
#define NPSTAT_ARRAYND_HH_

/*!
// \file ArrayND.hh
//
// \brief Arbitrary-dimensional array template
//
// Author: I. Volobouev
//
// October 2009
*/

#include <cassert>

#include "geners/ClassId.hh"
#include "geners/CPP11_config.hh"

#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/nm/ArrayRange.hh"
#include "npstat/nm/AbsArrayProjector.hh"
#include "npstat/nm/AbsVisitor.hh"
#include "npstat/nm/PreciseType.hh"
#include "npstat/nm/ProperDblFromCmpl.hh"

namespace npstat {
    /**
    // A class for multidimensional array manipulation. A number of methods
    // of this class will work only if dimensionality is limited by
    // CHAR_BIT*sizeof(unsigned long)-1 (which is 31 and 63 on 32- and 64-bit
    // architectures, respectively).
    //
    // Depending on how much space is provided with the "StackLen" template
    // parameter, the array data will be placed either on the stack or on the
    // heap. By default, array data leaves on the heap unless the array has
    // rank 0.
    //
    // Depending on how much space is provided with the "StackDim" template
    // parameter, the array strides will be placed either on the stack or
    // on the heap. By default, strides will be placed on the stack in case
    // the array dimensionality is ten or less.
    //
    // The "Numeric" type must have a default constructor (of course,
    // pointers to arbitrary types can be used as well).
    //
    // Both StackLen and StackDim parameters must be positive.
    */
    template <typename Numeric, unsigned StackLen=1U, unsigned StackDim=10U>
    class ArrayND
    {
        template <typename Num2, unsigned Len2, unsigned Dim2>
        friend class ArrayND;

    public:
        typedef Numeric value_type;
        typedef typename ProperDblFromCmpl<Numeric>::type proper_double;

        /**
        // Default constructor creates an uninitialized array. The
        // following things can be done safely with such an array:
        //
        // 1) Assigning it from another array (initialized or not).
        //
        // 2) Passing it as an argument to the class static method "restore".
        //
        // 3) Calling the "uninitialize" method.
        //
        // 4) Calling the "isShapeKnown" method.
        //
        // 5) Calling the "reshape" method.
        //
        // Any other operation results in an undefined behavior (often,
        // an exception is thrown). Note that initialized array can not
        // be assigned from uninitialized one.
        */
        ArrayND();

        /**
        // Constructor which creates arrays with the given shape.
        // The array data remains undefined. Simple inilitalization
        // of the data can be performed using methods clear() or
        // constFill(SomeValue). More complicated initialization
        // can be done by "linearFill", "functorFill", or by setting
        // every array element to a desired value.
        */
        explicit ArrayND(const ArrayShape& shape);
        ArrayND(const unsigned* shape, unsigned dim);

        /** The copy constructor */
        ArrayND(const ArrayND&);

#ifdef CPP11_STD_AVAILABLE
        /** The move constructor */
        ArrayND(ArrayND&&);
#endif
        /**
        // Converting constructor. It looks more general than the copy
        // constructor, but the actual copy constructor has to be created
        // anyway -- otherwise the compiler will generate an incorrect
        // default copy constructor. Note that existence of this
        // constructor essentially disables data type safety for copying
        // arrays -- but the code significantly gains in convenience.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND(const ArrayND<Num2, Len2, Dim2>&);

        /**
        // Converting constructor where the array values are filled
        // by a functor using values of another array as arguments
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        ArrayND(const ArrayND<Num2, Len2, Dim2>&, Functor f);

        /** Constructor from a subrange of another array */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND(const ArrayND<Num2, Len2, Dim2>& from,
                const ArrayRange& fromRange);

        /** Similar constructor with a transforming functor */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        ArrayND(const ArrayND<Num2, Len2, Dim2>& from,
                const ArrayRange& fromRange, Functor f);

        /**
        // Constructor from a slice of another array. The data of the
        // constructed array remains undefined. The argument "indices"
        // lists either the array indices whose numbers will be fixed
        // when slicing is performed or the indices which will be iterated
        // over during projections (for example, array values may be
        // summed over these indices). These indices will be excluded
        // from the constructed array. The created array can be subsequently
        // used with methods "exportSlice", "importSlice", "project", etc.
        // of the parent array "slicedArray".
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND(const ArrayND<Num2, Len2, Dim2>& slicedArray,
                const unsigned *indices, unsigned nIndices);

        /** Outer product constructor */
        template <typename Num1, unsigned Len1, unsigned Dim1,
                  typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND(const ArrayND<Num1, Len1, Dim1>& a1,
                const ArrayND<Num2, Len2, Dim2>& a2);

        //@{
        /** 
        // Constructor in which the spans are explicitly provided
        // for each dimension. The array data remains undefined.
        */
        explicit ArrayND(unsigned n0);
        ArrayND(unsigned n0, unsigned n1);
        ArrayND(unsigned n0, unsigned n1, unsigned n2);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4, unsigned n5);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4, unsigned n5, unsigned n6);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4, unsigned n5, unsigned n6, unsigned n7);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4, unsigned n5, unsigned n6, unsigned n7,
                unsigned n8);
        ArrayND(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                unsigned n4, unsigned n5, unsigned n6, unsigned n7,
                unsigned n8, unsigned n9);
        //@}

        /** Destructor */
        ~ArrayND();

        /**
        // Assignment operator. The shape of the array on the right
        // must be compatible with the shape of the array on the left.
        // The only exception is when the array on the left has no shape
        // at all (i.e., it was created by the default constructor or
        // its "uninitialize" method was called). In this case the array
        // on the left will assume the shape of the array on the right.
        */
        ArrayND& operator=(const ArrayND&);

#ifdef CPP11_STD_AVAILABLE
        /** The move assignment operator */
        ArrayND& operator=(ArrayND&&);
#endif

        /** Converting assignment operator */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND& operator=(const ArrayND<Num2,Len2,Dim2>&);

        /** Converting assignment method with a transforming functor */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        ArrayND& assign(const ArrayND<Num2, Len2, Dim2>&, Functor f);

        /**
        // The function which can "uninitialize" the array to the same
        // state as produced by the default constructor. Can be applied
        // in order to force the assignment operators to work.
        */
        ArrayND& uninitialize();

        //@{
        /** Change the array shape. All data is lost in the process. */
        ArrayND& reshape(const ArrayShape& newShape);
        ArrayND& reshape(const unsigned* newShape, unsigned dim);
        ArrayND& reshape(unsigned n0);
        ArrayND& reshape(unsigned n0, unsigned n1);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4, unsigned n5);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4, unsigned n5, unsigned n6);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4, unsigned n5, unsigned n6, unsigned n7);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4, unsigned n5, unsigned n6, unsigned n7,
                         unsigned n8);
        ArrayND& reshape(unsigned n0, unsigned n1, unsigned n2, unsigned n3,
                         unsigned n4, unsigned n5, unsigned n6, unsigned n7,
                         unsigned n8, unsigned n9);
        //@}

        /**
        // Change the shape of this array to be identical to the shape of
        // the argument array. The shape of the argument array must be known.
        // All data is lost in the process.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND& reshape(const ArrayND<Num2,Len2,Dim2>& r);

        /**
        // Element access using multidimensional array index
        // (no bounds checking). The length of the index array
        // must be equal to the rank of this object.
        */
        Numeric& value(const unsigned *index, unsigned indexLen);
        const Numeric& value(const unsigned *index, unsigned indexLen) const;

        /**
        // Element access using multidimensional array index
        // (with bounds checking)
        */
        Numeric& valueAt(const unsigned *index, unsigned indexLen);
        const Numeric& valueAt(const unsigned *index, unsigned indexLen) const;

        /** Element access using linear index (no bounds checking) */
        Numeric& linearValue(unsigned long index);
        const Numeric& linearValue(unsigned long index) const;

        /** Element access using linear index (with bounds checking) */
        Numeric& linearValueAt(unsigned long index);
        const Numeric& linearValueAt(unsigned long index) const;

        /** Convert linear index into multidimensional index */
        void convertLinearIndex(unsigned long l, unsigned* index,
                                unsigned indexLen) const;

        /** Convert multidimensional index into linear index */
        unsigned long linearIndex(const unsigned* idx, unsigned idxLen) const;

        // Some inspectors
        /** Total number of data array elements */
        inline unsigned long length() const {return len_;}

        /** Linearized data */
        inline const Numeric* data() const {return data_;}

        /** Check whether the array has been initialized */
        inline bool isShapeKnown() const {return shapeIsKnown_;}

        /** The number of array dimensions */
        inline unsigned rank() const {return dim_;}

        /** Get the complete shape */
        ArrayShape shape() const;

        /** Shape data as a C-style array */
        inline const unsigned *shapeData() const {return shape_;}

        /** Get the complete range */
        ArrayRange fullRange() const;

        /** Get the number of elements in some particular dimension */
        unsigned span(unsigned dim) const;

        /** Maximum span among all dimensions */
        unsigned maximumSpan() const;

        /** Minimum span among all dimensions */
        unsigned minimumSpan() const;

        /** Get the strides */
        inline const unsigned long* strides() const {return strides_;}

        /** Check if all array elements are zero */
        bool isZero() const;

        /**
        // This method checks whether all array elements are
        // non-negative and, in addition, there is at least
        // one positive element
        */
        bool isDensity() const;

        /** This method modifies all the data in one statement */
        template <typename Num2>
        ArrayND& setData(const Num2* data, unsigned long dataLength);

        /** Compare two arrays for equality */
        template <unsigned Len2, unsigned Dim2>
        bool operator==(const ArrayND<Numeric,Len2,Dim2>&) const;

        /** Logical negation of operator== */
        template <unsigned Len2, unsigned Dim2>
        bool operator!=(const ArrayND<Numeric,Len2,Dim2>&) const;

        /** Largest absolute difference with another bin-compatible array */
        template <unsigned Len2, unsigned Dim2>
        double maxAbsDifference(const ArrayND<Numeric,Len2,Dim2>&) const;

        /** operator+ returns a copy of this array */
        ArrayND operator+() const;

        /** operator- applies the unary minus operator to every element */
        ArrayND operator-() const;

        /** addition of two arrays */
        template <unsigned Len2, unsigned Dim2>
        ArrayND operator+(const ArrayND<Numeric,Len2,Dim2>& r) const;

        /** subtraction of two arrays */
        template <unsigned Len2, unsigned Dim2>
        ArrayND operator-(const ArrayND<Numeric,Len2,Dim2>& r) const;

        /** multiplication by a scalar */
        template <typename Num2>
        ArrayND operator*(const Num2& r) const;

        /** division by a scalar */
        template <typename Num2>
        ArrayND operator/(const Num2& r) const;

        //@{
        /**
        // In-place operator. Note that it works faster than the binary
        // version, i.e., A += B is much faster than A = A + B.
        */
        template <typename Num2>
        ArrayND& operator*=(const Num2& r);

        template <typename Num2>
        ArrayND& operator/=(const Num2& r);

        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND& operator+=(const ArrayND<Num2,Len2,Dim2>& r);

        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND& operator-=(const ArrayND<Num2,Len2,Dim2>& r);
        //@}

        /** This method is equivalent to (but faster than) += r*c */
        template <typename Num3, typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND& addmul(const ArrayND<Num2,Len2,Dim2>& r, const Num3& c);

        /** Outer product as a method (see also the outer product constructor) */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND outer(const ArrayND<Num2,Len2,Dim2>& r) const;

        /**
        // Contraction of a pair of indices. Note that the array length
        // must be the same in both dimensions.
        */
        ArrayND contract(unsigned pos1, unsigned pos2) const;

        /**
        // Here, dot product corresponds to outer product followed
        // by the contraction over two indices -- the last index
        // of this object and the first index of the argument.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND dot(const ArrayND<Num2,Len2,Dim2>& r) const;

        /**
        // The intent of this method is to marginalize
        // over a set of indices with a prior. Essentially, we are
        // calculating integrals akin to p(y) = Integral f(y|x) g(x) dx
        // in which all functions are represented on an equidistant grid.
        // If needed, multiplication of the result by the grid cell size
        // should be performed after this function. "indexMap" specifies
        // how the indices of the prior array (which is like g(x)) are
        // mapped into the indices of this array (which is like f(y|x)).
        // The number of elements in the map, "mapLen", must be equal to
        // the rank of the prior. Dimension 0 of the prior corresponds
        // to the dimension indexMap[0] of this array, dimension 1
        // corresponds to indexMap[1], etc.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        ArrayND marginalize(const ArrayND<Num2,Len2,Dim2>& prior,
                            const unsigned* indexMap, unsigned mapLen) const;

        /** Transposed array */
        ArrayND transpose(unsigned pos1, unsigned pos2) const;

        /** Transpose without arguments can be invoked for 2-d arrays only */
        ArrayND transpose() const;

        // The following function would work better with the declaration
        //
        // template <typename Num2 = typename PreciseType<Numeric>::type>
        // Num2 sum() const;
        //
        // However, default template arguments for class methods are supported
        // only in C++11. This comment is also applicable to a number of other
        // functions in this header.

        /**
        // Sum of all array elements which uses Num2 type as accumulator.
        // Typically, the precision and dynamic range of Num2 should be
        // suitably larger than the precision and dynamic range of Numeric.
        // For example, if Numeric is float then Num2 should be double, etc.
        */
        template <typename Num2>
        Num2 sum() const;

        /**
        // Sum of absolute values squared which uses Num2 as accumulator.
        // Function std::abs(Numeric) must exist.
        */
        template <typename Num2>
        Num2 sumsq() const;

        /**
        // Mixed derivative over all directions. Useful for generating
        // densities from distribution functions. The resulting array
        // will have one less point in each dimension. Class Num2 is
        // used as accumulator for calculations. static_cast from
        // Num2 to Numeric must exist. The result is multiplied by the
        // scale factor provided.
        */
        template <typename Num2>
        ArrayND derivative(double scale=1.0) const;

        /**
        // The operation inverse to "derivative". Constructs multivariate
        // cumulative density function.
        */
        template <typename Num2>
        ArrayND cdfArray(double scale=1.0) const;

        /**
        // Calculate just one multivariate cumulative density function
        // value. Point with given index will be included in the sum.
        */
        template <typename Num2>
        Num2 cdfValue(const unsigned *index, unsigned indexLen) const;

        /**
        // The next function turns the array data into the conditional
        // cumulative density function for the last dimension. "Num2"
        // is the type of accumulator class used. The cdf is stored
        // in such a way that the cdf value of 0 is skipped (the first
        // stored value is the sum which includes the 0th bin). The slice
        // is filled with the sum of values. The "useTrapezoids" parameter
        // specifies whether trapezoidal integration formula should be
        // utilized (rectangular integration is used in case
        // "useTrapezoids" value is "false").
        */
        template <typename Num2>
        void convertToLastDimCdf(ArrayND* sumSlice, bool useTrapezoids);

        /**
        // Coarsen this array by summing n nearby elements along
        // the given dimension. The "result" array will have n
        // times less elements along that dimension.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void coarseSum(unsigned idim, unsigned n,
                       ArrayND<Num2,Len2,Dim2>* result) const;

        /**
        // Coarsen this array by averaging n nearby elements along
        // the given dimension. The "result" array will have n
        // times less elements along that dimension.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void coarseAverage(unsigned idim, unsigned n,
                           ArrayND<Num2,Len2,Dim2>* result) const;

        /** Minimum array element */
        Numeric min() const;

        /** Minimum array element and its index */
        Numeric min(unsigned *index, unsigned indexLen) const;

        /** Maximum array element */
        Numeric max() const;

        /** Maximum array element and its index */
        Numeric max(unsigned *index, unsigned indexLen) const;

        /**
        // Closest value accessor (works as if the array allows access
        // with non-integer indices). For example, the second point
        // in some dimension will be accessed in case the coordinate
        // in that dimension is between 0.5 and 1.5. This function can be
        // used, for example, for implementing simple N-D histogramming
        // or for closest value interpolation and extrapolation.
        */
        Numeric& closest(const double *x, unsigned xDim);
        const Numeric& closest(const double *x, unsigned xDim) const;

        /**
        // Multilinear interpolation. Closest value extrapolation is used
        // in case some index is outside of the array bounds. Note that
        // this function works only if the array dimensionality is less
        // than CHAR_BIT*sizeof(unsigned long). x is the "coordinate"
        // which coincides with array index for x equal to unsigned
        // integers.
        */
        Numeric interpolate1(const double *x, unsigned xDim) const;

        /**
        // Multicubic interpolation. Closest value extrapolation is used
        // in case some index is outside of the array bounds. This
        // function is much slower than "interpolate1" (in the current
        // implementation, a recursive algorithm is used).
        */
        Numeric interpolate3(const double *x, unsigned xDim) const;

        /**
        // This method applies a single-argument functor to each
        // element of the array (in-place). The result returned
        // by the functor becomes the new value of the element. There
        // must be a conversion (static cast) from the functor result to
        // the "Numeric" type. The method returns *this which allows
        // for chaining of such methods. Use the transforming constructor
        // if you want a new array instead.
        */
        template <class Functor>
        ArrayND& apply(Functor f);

        /**
        // This method applies a single-argument functor to each
        // element of the array. The result returned by the functor
        // is ignored inside the scan. Depending on what the functor does,
        // the array values may or may not be modified (they can be modified
        // if the functor takes its argument via a non-const reference).
        */
        template <class Functor>
        ArrayND& scanInPlace(Functor f);

        /** This method fills the array data with a constant value */
        ArrayND& constFill(Numeric c);

        /** Zero the array out (every datum becomes Numeric()) */
        ArrayND& clear();

        /**
        // This method fills the array with a linear combination
        // of the index values. For example, a 2-d array element with indices
        // i, k will be set to (coeff[0]*i + coeff[1]*k + c). There must be
        // a conversion (static cast) from double into "Numeric".
        */
        ArrayND& linearFill(const double* coeff, unsigned coeffLen, double c);

        /**
        // This method fills the array from a functor
        // which takes (const unsigned* index, unsigned indexLen)
        // arguments. There must be a conversion (static cast) from
        // the functor result to the "Numeric" type.
        */
        template <class Functor>
        ArrayND& functorFill(Functor f);

        /**
        // This method can be used for arrays with rank
        // of at least 2 whose length is the same in all dimensions.
        // It puts static_cast<Numeric>(1) on the main diagonal and
        // Numeric() everywhere else.
        */
        ArrayND& makeUnit();

        /** This method turns all negative elements into zeros */
        ArrayND& makeNonNegative();

        /**
        // This method accumulates marginals and divides
        // the array (treated as a distribution) by the product of the
        // marginals. Several iterations like this turn the distribution
        // into a copula. If the array contains negative elements, they
        // are turned into zeros before the iterations are performed.
        // The function returns the actual number of iteration performed
        // when the given tolerance was reached for all marginals.
        */
        unsigned makeCopulaSteps(double tolerance, unsigned maxIterations);

        /**
        // Loop over all elements of two compatible arrays and apply
        // a binary functor
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void jointScan(ArrayND<Num2, Len2, Dim2>& other, Functor binaryFunct);

        /** Convenience method for element-by-element in-place multiplication */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        inline ArrayND& inPlaceMul(const ArrayND<Num2,Len2,Dim2>& r)
        {
            jointScan(const_cast<ArrayND<Num2,Len2,Dim2>&>(r),
                      multeq_left<Numeric,Num2>());
            return *this;
        }

        /**
        // Loop over subranges in two arrays in such a way that the functor
        // is called only if the indices on both sides are valid. The topology
        // of both arrays is assumed to be box-like (flat). The starting
        // corner in this object (where cycling begins) is provided by the
        // argument "thisCorner". The "range" argument specifies the width
        // of the processed patch in each dimension. The corner of the "other"
        // array where cycling begins is provided by the "otherCorner"
        // argument. The "arrLen" argument specifies the number of elements
        // in "thisCorner", "range", and "otherCorner" arrays. It should be
        // equal to the rank of either of the two ArrayND arrays.
        //
        // Note that there is no good way for this method to assume constness
        // of this or "other" array: this becomes apparent only after the 
        // functor has been specified. Apply const_cast judiciously as needed,
        // other solutions of this problem are not any better.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void jointSubrangeScan(ArrayND<Num2, Len2, Dim2>& other,
                               const unsigned* thisCorner,
                               const unsigned* range,
                               const unsigned* otherCorner,
                               unsigned arrLen,
                               Functor binaryFunct);

        /**
        // Method similar to "jointSubrangeScan" in which the topology of
        // both arrays is assumed to be hypertoroidal (circular buffer in
        // every dimension)
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void dualCircularScan(ArrayND<Num2, Len2, Dim2>& other,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              unsigned arrLen,
                              Functor binaryFunct);

        /**
        // Method similar to "jointSubrangeScan" in which the topology of
        // this array is assumed to be flat and the other array hypertoroidal
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void flatCircularScan(ArrayND<Num2, Len2, Dim2>& other,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              unsigned arrLen,
                              Functor binaryFunct);

        /**
        // Method similar to "jointSubrangeScan" in which the topology of
        // this array is assumed to be hypertoroidal and the other array flat
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void circularFlatScan(ArrayND<Num2, Len2, Dim2>& other,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              unsigned arrLen,
                              Functor binaryFunct);

        /**
        // This method runs over a subrange of the array
        // and calls the argument functor on every point. This
        // method will not call "clear" or "result" functions of
        // the argument functor.
        */
        template <typename Num2, typename Integer>
        void processSubrange(AbsArrayProjector<Numeric,Num2>& f,
                             const BoxND<Integer>& subrange) const;

        /**
        // Copy a hyperrectangular subrange of this array potentially
        // completely overwriting the destination array. The starting
        // corner in this object where copying begins is provided by
        // the first two arguments. The subrange size is defined by
        // the shape of the destination array.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void exportSubrange(const unsigned* fromCorner, unsigned lenCorner,
                            ArrayND<Num2, Len2, Dim2>* dest) const;

        /** The inverse operation to "exportSubrange" */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void importSubrange(const unsigned* fromCorner, unsigned lenCorner,
                            const ArrayND<Num2, Len2, Dim2>& from);

        /**
        // Check that all elements of this array differ from the
        // corresponding elements of another array by at most "eps".
        // Equivalent to maxAbsDifference(r) <= eps (but usually faster).
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        bool isClose(const ArrayND<Num2,Len2,Dim2>& r, double eps) const;

        //@{
        /** Check compatibility with another shape */
        bool isCompatible(const ArrayShape& shape) const;
        bool isCompatible(const unsigned* shape, unsigned dim) const;
        //@}

        /**
        // Check shape compatibility with another array. Equivalent to
        // but faster than isCompatible(r.shape()).
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        bool isShapeCompatible(const ArrayND<Num2,Len2,Dim2>& r) const;

        /**
        // Joint cycle over the data of this array and the slice.
        // The array to which the "slice" argument refers should normally
        // be created by the slicing constructor using this array as
        // the argument. The "fixedIndices" argument should be the same
        // as the "indices" argument in that constructor. This method
        // is to be used for import/export of slice data and in-place
        // operations (addition, multiplication, etc).
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void jointSliceScan(ArrayND<Num2,Len2,Dim2>& slice,
                            const unsigned *fixedIndices,
                            const unsigned *fixedIndexValues,
                            unsigned nFixedIndices,
                            Functor binaryFunct);

        /**
        // Joint cycle over a slice this array and some memory buffer.
        // The length of the buffer must be equal to the length of the
        // slice, as in "jointSliceScan". This method is to be used
        // for import/export of slice data and in-place operations
        // (addition, multiplication, etc) with memory managed not
        // by ArrayND but in some other manner.
        */
        template <typename Num2, class Functor>
        void jointMemSliceScan(Num2* buffer, unsigned long bufLen,
                               const unsigned *fixedIndices,
                               const unsigned *fixedIndexValues,
                               unsigned nFixedIndices,
                               Functor binaryFunct);

        /** Figure out the slice shape without actually making the slice */
        ArrayShape sliceShape(const unsigned *fixedIndices,
                              unsigned nFixedIndices) const;

        /** Convenience method for exporting a slice of this array */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        inline void exportSlice(ArrayND<Num2,Len2,Dim2>* slice,
                                const unsigned *fixedIndices,
                                const unsigned *fixedIndexValues,
                                unsigned nFixedIndices) const
        {
            assert(slice);
            (const_cast<ArrayND*>(this))->jointSliceScan(
                *slice, fixedIndices, fixedIndexValues, nFixedIndices,
                scast_assign_right<Numeric,Num2>());
        }

        /** 
        // Convenience method for exporting a slice of this array
        // into a memory buffer
        */
        template <typename Num2>
        inline void exportMemSlice(Num2* buffer, unsigned long bufLen,
                                   const unsigned *fixedIndices,
                                   const unsigned *fixedIndexValues,
                                   unsigned nFixedIndices) const
        {
            (const_cast<ArrayND*>(this))->jointMemSliceScan(
                buffer, bufLen, fixedIndices, fixedIndexValues,
                nFixedIndices, scast_assign_right<Numeric,Num2>());
        }

        /** Convenience method for importing a slice into this array */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        inline void importSlice(const ArrayND<Num2,Len2,Dim2>& slice,
                                const unsigned *fixedIndices,
                                const unsigned *fixedIndexValues,
                                unsigned nFixedIndices)
        {
            jointSliceScan(const_cast<ArrayND<Num2,Len2,Dim2>&>(slice),
                           fixedIndices, fixedIndexValues, nFixedIndices,
                           scast_assign_left<Numeric,Num2>());
        }

        /**
        // Convenience method for importing a slice into this array
        // from a memory buffer
        */
        template <typename Num2>
        inline void importMemSlice(const Num2* buffer, unsigned long bufLen,
                                   const unsigned *fixedIndices,
                                   const unsigned *fixedIndexValues,
                                   unsigned nFixedIndices)
        {
            jointMemSliceScan(const_cast<Num2*>(buffer), bufLen,
                              fixedIndices, fixedIndexValues, nFixedIndices,
                              scast_assign_left<Numeric,Num2>());
        }

        /**
        // This method applies the values in the slice to all other
        // coresponding values in the array. This can be used, for example,
        // to multiply/divide by some factor which varies across the slice.
        // The slice values will be used as the right functor argument.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void applySlice(ArrayND<Num2,Len2,Dim2>& slice,
                        const unsigned *fixedIndices, unsigned nFixedIndices,
                        Functor binaryFunct);

        /**
        // Convenience method which multiplies the array by a scale factor
        // which varies across the slice
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        inline ArrayND& multiplyBySlice(const ArrayND<Num2,Len2,Dim2>& slice,
                                        const unsigned *fixedIndices,
                                        unsigned nFixedIndices)
        {
            applySlice(const_cast<ArrayND<Num2,Len2,Dim2>&>(slice),
                       fixedIndices, nFixedIndices,
                       multeq_left<Numeric,Num2>());
            return *this;
        }

        //@{
        /**
        // This method fills a projection. The array to which
        // "projection" argument points should normally be created by
        // the slicing constructor using this array as an argument.
        // "projectedIndices" should be the same as "indices" specified
        // during the slice creation.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void project(ArrayND<Num2,Len2,Dim2>* projection,
                     AbsArrayProjector<Numeric,Num3>& projector,
                     const unsigned *projectedIndices,
                     unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void project(ArrayND<Num2,Len2,Dim2>* projection,
                     AbsVisitor<Numeric,Num3>& projector,
                     const unsigned *projectedIndices,
                     unsigned nProjectedIndices) const;
        //@}

        //@{
        /**
        // Similar method to "project", but projections are added to
        // (or subtracted from) the existing projection data instead of
        // replacing them
        */
        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void addToProjection(ArrayND<Num2,Len2,Dim2>* projection,
                             AbsArrayProjector<Numeric,Num3>& projector,
                             const unsigned *projectedIndices,
                             unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void subtractFromProjection(ArrayND<Num2,Len2,Dim2>* projection,
                                    AbsArrayProjector<Numeric,Num3>& projector,
                                    const unsigned *projectedIndices,
                                    unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void addToProjection(ArrayND<Num2,Len2,Dim2>* projection,
                             AbsVisitor<Numeric,Num3>& projector,
                             const unsigned *projectedIndices,
                             unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2, typename Num3>
        void subtractFromProjection(ArrayND<Num2,Len2,Dim2>* projection,
                                    AbsVisitor<Numeric,Num3>& projector,
                                    const unsigned *projectedIndices,
                                    unsigned nProjectedIndices) const;
        //@}

        /**
        // Rotation. Place the result into another array. The elements
        // with indices 0 in the current array will become elements with
        // indices "shifts" in the rotated array.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void rotate(const unsigned* shifts, unsigned lenShifts,
                    ArrayND<Num2, Len2, Dim2>* rotated) const;

        /**
        // Fill another array with all possible mirror images
        // of this one. This other array must have twice the span
        // in each dimension.
        */
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void multiMirror(ArrayND<Num2, Len2, Dim2>* out) const;

        //@{
        /**
        // Fortran-style subscripting without bounds checking (of course,
        // with indices starting at 0).
        */
        Numeric& operator()();
        const Numeric& operator()() const;

        Numeric& operator()(unsigned i0);
        const Numeric& operator()(unsigned i0) const;

        Numeric& operator()(unsigned i0, unsigned i1);
        const Numeric& operator()(unsigned i0, unsigned i1) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2) const;

        Numeric& operator()(unsigned i0, unsigned i1,
                            unsigned i2, unsigned i3);
        const Numeric& operator()(unsigned i0, unsigned i1,
                                  unsigned i2, unsigned i3) const;

        Numeric& operator()(unsigned i0, unsigned i1,
                            unsigned i2, unsigned i3, unsigned i4);
        const Numeric& operator()(unsigned i0, unsigned i1,
                                  unsigned i2, unsigned i3, unsigned i4) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                            unsigned i3, unsigned i4, unsigned i5);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                                  unsigned i3, unsigned i4, unsigned i5) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                            unsigned i3, unsigned i4, unsigned i5,
                            unsigned i6);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                                  unsigned i3, unsigned i4, unsigned i5,
                                  unsigned i6) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                            unsigned i3, unsigned i4, unsigned i5,
                            unsigned i6, unsigned i7);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                                  unsigned i3, unsigned i4, unsigned i5,
                                  unsigned i6, unsigned i7) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                            unsigned i3, unsigned i4, unsigned i5,
                            unsigned i6, unsigned i7, unsigned i8);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                                  unsigned i3, unsigned i4, unsigned i5,
                                  unsigned i6, unsigned i7, unsigned i8) const;

        Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                            unsigned i3, unsigned i4, unsigned i5,
                            unsigned i6, unsigned i7, unsigned i8,
                            unsigned i9);
        const Numeric& operator()(unsigned i0, unsigned i1, unsigned i2,
                                  unsigned i3, unsigned i4, unsigned i5,
                                  unsigned i6, unsigned i7, unsigned i8,
                                  unsigned i9) const;
        //@}

        //@{
        /**
        // Fortran-style subscripting with bounds checking (of course,
        // with indices starting at 0).
        */
        Numeric& at();
        const Numeric& at() const;

        Numeric& at(unsigned i0);
        const Numeric& at(unsigned i0) const;

        Numeric& at(unsigned i0, unsigned i1);
        const Numeric& at(unsigned i0, unsigned i1) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2) const;

        Numeric& at(unsigned i0, unsigned i1,
                    unsigned i2, unsigned i3);
        const Numeric& at(unsigned i0, unsigned i1,
                          unsigned i2, unsigned i3) const;

        Numeric& at(unsigned i0, unsigned i1,
                    unsigned i2, unsigned i3, unsigned i4);
        const Numeric& at(unsigned i0, unsigned i1,
                          unsigned i2, unsigned i3, unsigned i4) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                    unsigned i3, unsigned i4, unsigned i5);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                          unsigned i3, unsigned i4, unsigned i5) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                    unsigned i3, unsigned i4, unsigned i5,
                    unsigned i6);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                          unsigned i3, unsigned i4, unsigned i5,
                          unsigned i6) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                    unsigned i3, unsigned i4, unsigned i5,
                    unsigned i6, unsigned i7);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                          unsigned i3, unsigned i4, unsigned i5,
                          unsigned i6, unsigned i7) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                    unsigned i3, unsigned i4, unsigned i5,
                    unsigned i6, unsigned i7, unsigned i8);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                          unsigned i3, unsigned i4, unsigned i5,
                          unsigned i6, unsigned i7, unsigned i8) const;

        Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                    unsigned i3, unsigned i4, unsigned i5,
                    unsigned i6, unsigned i7, unsigned i8,
                    unsigned i9);
        const Numeric& at(unsigned i0, unsigned i1, unsigned i2,
                          unsigned i3, unsigned i4, unsigned i5,
                          unsigned i6, unsigned i7, unsigned i8,
                          unsigned i9) const;
        //@}

        //@{
        /**
        // Subscripting by continuous coordinate.
        // Works similar to the "closest" method.
        */
        Numeric& cl();
        const Numeric& cl() const;

        Numeric& cl(double x0);
        const Numeric& cl(double x0) const;

        Numeric& cl(double x0, double x1);
        const Numeric& cl(double x0, double x1) const;

        Numeric& cl(double x0, double x1, double x2);
        const Numeric& cl(double x0, double x1, double x2) const;

        Numeric& cl(double x0, double x1,
                    double x2, double x3);
        const Numeric& cl(double x0, double x1,
                          double x2, double x3) const;

        Numeric& cl(double x0, double x1,
                    double x2, double x3, double x4);
        const Numeric& cl(double x0, double x1,
                          double x2, double x3, double x4) const;

        Numeric& cl(double x0, double x1, double x2,
                    double x3, double x4, double x5);
        const Numeric& cl(double x0, double x1, double x2,
                          double x3, double x4, double x5) const;

        Numeric& cl(double x0, double x1, double x2,
                    double x3, double x4, double x5,
                    double x6);
        const Numeric& cl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6) const;

        Numeric& cl(double x0, double x1, double x2,
                    double x3, double x4, double x5,
                    double x6, double x7);
        const Numeric& cl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7) const;

        Numeric& cl(double x0, double x1, double x2,
                    double x3, double x4, double x5,
                    double x6, double x7, double x8);
        const Numeric& cl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7, double x8) const;

        Numeric& cl(double x0, double x1, double x2,
                    double x3, double x4, double x5,
                    double x6, double x7, double x8,
                    double x9);
        const Numeric& cl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7, double x8,
                          double x9) const;
        //@}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            ArrayND* array);
    private:
        Numeric localData_[StackLen];
        Numeric* data_;

        unsigned long localStrides_[StackDim];
        unsigned long *strides_;

        unsigned localShape_[StackDim];
        unsigned *shape_;

        unsigned long len_;
        unsigned dim_;

        bool shapeIsKnown_;
        bool dataIsExternal_;

        // Basic initialization from unsigned* shape and dimensionality
        void buildFromShapePtr(const unsigned*, unsigned);

        // Basic initialization from unsigned* shape, dimensionality,
        // and external data buffer which we will manage but will not own
        void buildExtFromShapePtr(const unsigned*, unsigned, Numeric*);

        // Build strides_ array out of the shape_ array
        void buildStrides();

        // Recursive implementation of nested loops for "linearFill"
        void linearFillLoop(unsigned level, double s0,
                            unsigned long idx, double shift,
                            const double* coeffs);

        // Recursive implementation of nested loops for "functorFill"
        template <class Functor>
        void functorFillLoop(unsigned level, unsigned long idx,
                             Functor f, unsigned* farg);

        // Recursive implementation of nested loops for "interpolate3"
        Numeric interpolateLoop(unsigned level, const double *x,
                                const Numeric* base) const;

        // Recursive implementation of nested loops for the outer product
        template <typename Num1, unsigned Len1, unsigned Dim1,
                  typename Num2, unsigned Len2, unsigned Dim2>
        void outerProductLoop(unsigned level, unsigned long idx0,
                              unsigned long idx1, unsigned long idx2,
                              const ArrayND<Num1, Len1, Dim1>& a1,
                              const ArrayND<Num2, Len2, Dim2>& a2);

        // Recursive implementation of nested loops for contraction
        void contractLoop(unsigned thisLevel, unsigned resLevel,
                          unsigned pos1, unsigned pos2,
                          unsigned long idxThis, unsigned long idxRes,
                          ArrayND& result) const;

        // Recursive implementation of nested loops for transposition
        void transposeLoop(unsigned level, unsigned pos1, unsigned pos2,
                           unsigned long idxThis, unsigned long idxRes,
                           ArrayND& result) const;

        // Recursive implementation of nested loops for the dot product
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void dotProductLoop(unsigned level, unsigned long idx0,
                            unsigned long idx1, unsigned long idx2,
                            const ArrayND<Num2, Len2, Dim2>& r,
                            ArrayND& result) const;

        // Recursive implementation of nested loops for marginalization
        template <typename Num2, unsigned Len2, unsigned Dim2>
        Numeric marginalizeInnerLoop(unsigned long idx,
                                     unsigned levelPr, unsigned long idxPr,
                                     const ArrayND<Num2,Len2,Dim2>& prior,
                                     const unsigned* indexMap) const;
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void marginalizeLoop(unsigned level, unsigned long idx,
                             unsigned levelRes, unsigned long idxRes,
                             const ArrayND<Num2,Len2,Dim2>& prior,
                             const unsigned* indexMap, ArrayND& res) const;

        // Recursive implementation of nested loops for range copy
        // with functor modification of elements
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void copyRangeLoopFunct(unsigned level, unsigned long idx0,
                                unsigned long idx1,
                                const ArrayND<Num2, Len2, Dim2>& r,
                                const ArrayRange& range, Functor f);

        // Loop over subrange in such a way that the functor is called
        // only if indices on both sides are valid. The topology of both
        // arrays is that of the hyperplane (flat).
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void commonSubrangeLoop(unsigned level, unsigned long idx0,
                                unsigned long idx1,
                                const unsigned* thisCorner,
                                const unsigned* range,
                                const unsigned* otherCorner,
                                ArrayND<Num2, Len2, Dim2>& other,
                                Functor binaryFunct);

        // Similar loop with the topology of the hypertorus for both
        // arrays (all indices of both arrays are wrapped around)
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void dualCircularLoop(unsigned level, unsigned long idx0,
                              unsigned long idx1,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              ArrayND<Num2, Len2, Dim2>& other,
                              Functor binaryFunct);

        // Similar loop in which the topology of this array is assumed
        // to be flat and the topology of the destination array is that
        // of the hypertorus. Due to the asymmetry of the topologies,
        // "const" function is not provided (use const_cast as appropriate).
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void flatCircularLoop(unsigned level, unsigned long idx0,
                              unsigned long idx1,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              ArrayND<Num2, Len2, Dim2>& other,
                              Functor binaryFunct);

        // Similar loop in which the topology of this array is assumed
        // to be hypertoroidal and the topology of the destination array
        // is flat.
        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void circularFlatLoop(unsigned level, unsigned long idx0,
                              unsigned long idx1,
                              const unsigned* thisCorner,
                              const unsigned* range,
                              const unsigned* otherCorner,
                              ArrayND<Num2, Len2, Dim2>& other,
                              Functor binaryFunct);

        // Slice compatibility verification
        template <typename Num2, unsigned Len2, unsigned Dim2>
        unsigned long verifySliceCompatibility(
            const ArrayND<Num2,Len2,Dim2>& slice,
            const unsigned *fixedIndices,
            const unsigned *fixedIndexValues,
            unsigned nFixedIndices) const;

        // Buffer compatibility verification with a slice
        unsigned long verifyBufferSliceCompatibility(
            unsigned long bufLen,
            const unsigned *fixedIndices,
            const unsigned *fixedIndexValues,
            unsigned nFixedIndices,
            unsigned long* sliceStrides) const;

        // Recursive implementation of nested loops for slice operations
        template <typename Num2, class Functor>
        void jointSliceLoop(unsigned level, unsigned long idx0,
                            unsigned level1, unsigned long idx1,
                            Num2* sliceData, const unsigned long* sliceStrides,
                            const unsigned *fixedIndices,
                            const unsigned *fixedIndexValues,
                            unsigned nFixedIndices, Functor binaryFunctor);

        // Recursive implementation of nested loops for "applySlice"
        template <typename Num2, class Functor>
        void scaleBySliceInnerLoop(unsigned level, unsigned long idx0,
                                   Num2& scale,
                                   const unsigned* projectedIndices,
                                   unsigned nProjectedIndices,
                                   Functor binaryFunct);

        template <typename Num2, unsigned Len2, unsigned Dim2, class Functor>
        void scaleBySliceLoop(unsigned level, unsigned long idx0,
                              unsigned level1, unsigned long idx1,
                              ArrayND<Num2,Len2,Dim2>& slice,
                              const unsigned *fixedIndices,
                              unsigned nFixedIndices,
                              Functor binaryFunct);

        // Recursive implementation of nested loops for projections
        template <typename Num2>
        void projectInnerLoop(unsigned level, unsigned long idx0,
                              unsigned* currentIndex,
                              AbsArrayProjector<Numeric,Num2>& projector,
                              const unsigned* projectedIndices,
                              unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2,
                  typename Num3, class Op>
        void projectLoop(unsigned level, unsigned long idx0,
                         unsigned level1, unsigned long idx1,
                         unsigned* currentIndex,
                         ArrayND<Num2,Len2,Dim2>* projection,
                         AbsArrayProjector<Numeric,Num3>& projector,
                         const unsigned* projectedIndices,
                         unsigned nProjectedIndices, Op fcn) const;

        // Note that "projectLoop2" is almost identical to "projectLoop"
        // while "projectInnerLoop2" is almost identical to "projectInnerLoop".
        // It would make a lot of sense to combine these functions into
        // the same code and then partially specialize the little piece
        // where the "AbsVisitor" or "AbsArrayProjector" is actually called.
        // Unfortunately, "AbsVisitor" and "AbsArrayProjector" are
        // templates themselves, and it is not possible in C++ to partially
        // specialize a function template (that is, even if we can specialize
        // on "AbsVisitor" vs. "AbsArrayProjector", there is no way to
        // specialize on their parameter types).
        template <typename Num2, unsigned Len2, unsigned Dim2,
                  typename Num3, class Op>
        void projectLoop2(unsigned level, unsigned long idx0,
                          unsigned level1, unsigned long idx1,
                          ArrayND<Num2,Len2,Dim2>* projection,
                          AbsVisitor<Numeric,Num3>& projector,
                          const unsigned* projectedIndices,
                          unsigned nProjectedIndices, Op fcn) const;

        template <typename Num2>
        void projectInnerLoop2(unsigned level, unsigned long idx0,
                               AbsVisitor<Numeric,Num2>& projector,
                               const unsigned* projectedIndices,
                               unsigned nProjectedIndices) const;

        template <typename Num2, typename Integer>
        void processSubrangeLoop(unsigned level, unsigned long idx0,
                                 unsigned* currentIndex,
                                 AbsArrayProjector<Numeric,Num2>& f,
                                 const BoxND<Integer>& subrange) const;

        // Sum of all points with the given index and below
        template <typename Accumulator>
        Accumulator sumBelowLoop(unsigned level, unsigned long idx0,
                                 const unsigned* limit) const;

        // Loop for "convertToLastDimCdf"
        template <typename Accumulator>
        void convertToLastDimCdfLoop(ArrayND* sumSlice, unsigned level,
                                     unsigned long idx0,
                                     unsigned long idxSlice,
                                     bool useTrapezoids);

        // Convert a coordinate into index.
        // No checking whether "idim" is within limits.
        unsigned coordToIndex(double coord, unsigned idim) const;

        // Verify that projection array is compatible with this one
        template <typename Num2, unsigned Len2, unsigned Dim2>
        void verifyProjectionCompatibility(
            const ArrayND<Num2,Len2,Dim2>& projection,
            const unsigned *projectedIndices,
            unsigned nProjectedIndices) const;

        template <typename Num2, unsigned Len2, unsigned Dim2>
        void coarseSum2(unsigned idim, ArrayND<Num2,Len2,Dim2>* result) const;

        template <typename Num2, unsigned Len2, unsigned Dim2>
        void coarseSum3(unsigned idim, ArrayND<Num2,Len2,Dim2>* result) const;

        template <typename Num2, unsigned Len2, unsigned Dim2>
        void coarseSumN(unsigned idim, unsigned n,
                        ArrayND<Num2,Len2,Dim2>* result) const;

#ifdef CPP11_STD_AVAILABLE
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data,
                                            const unsigned* shape, unsigned dim);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, const ArrayShape& shape);

        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0);

        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1);

        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4,
                                            unsigned n5);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4,
                                            unsigned n5, unsigned n6);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4,
                                            unsigned n5, unsigned n6, unsigned n7);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4,
                                            unsigned n5, unsigned n6, unsigned n7,
                                            unsigned n8);
        template <typename Num2>
        friend ArrayND<Num2> externalMemArrayND(Num2* data, unsigned n0, unsigned n1,
                                            unsigned n2, unsigned n3, unsigned n4,
                                            unsigned n5, unsigned n6, unsigned n7,
                                            unsigned n8, unsigned n9);
#endif // CPP11_STD_AVAILABLE

#ifdef SWIG
    // Additional ArrayND methods necessary to construct a reasonable
    // python interface. SWIG has a major problem we have to address
    // here: it converts all access by reference into access by pointer.
    // However, in python we can't use pointers explicitly. Of course,
    // additional methods like "dereference" or "assign" can be written
    // for pointers, but the interface becomes ugly. Therefore, methods
    // below access array elements by value. Another problem we have to
    // consider is that SWIG support for wrapping template methods in
    // template classes is weak.
    //
    // Note that various "get" methods introduced below are renamed by
    // SWIG so that in python they end up named just like the original
    // C++ methods which return references. The "set" methods are
    // unchanged.
    //
    // Do not use these methods in pure C++ programs.
    //
    public:
        inline void setValue(const unsigned *index, unsigned indexLen,
                             Numeric v)
            {valueAt(index, indexLen) = v;}
        inline Numeric getValue(const unsigned *index, unsigned indexLen) const
            {return valueAt(index, indexLen);}

        inline void setLinearValue(unsigned long index, Numeric v)
            {linearValueAt(index) = v;}
        inline Numeric getLinearValue(unsigned long index) const
            {return linearValueAt(index);}

        inline void setClosest(const double *x, unsigned xDim, Numeric v) 
            {closest(x, xDim) = v;}
        const Numeric getClosest(const double *x, unsigned xDim) const
            {return closest(x, xDim);}

        inline void set(Numeric v)
            {at() = v;}
        inline Numeric get() const
            {return at();}

        inline void set(unsigned i0, Numeric v)
            {at(i0) = v;}
        inline Numeric get(unsigned i0) const
            {return at(i0);}

        inline void set(unsigned i0, unsigned i1, Numeric v)
            {at(i0,i1) = v;}
        inline Numeric get(unsigned i0, unsigned i1) const
            {return at(i0,i1);}

        inline void set(unsigned i0, unsigned i1, unsigned i2, Numeric v)
            {at(i0,i1,i2) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2) const
            {return at(i0,i1,i2);}

        inline void set(unsigned i0, unsigned i1,
                        unsigned i2, unsigned i3, Numeric v)
            {at(i0,i1,i2,i3) = v;}
        inline Numeric get(unsigned i0, unsigned i1,
                           unsigned i2, unsigned i3) const
            {return at(i0,i1,i2,i3);}

        inline void set(unsigned i0, unsigned i1,
                        unsigned i2, unsigned i3, unsigned i4, Numeric v)
            {at(i0,i1,i2,i3,i4) = v;}
        inline Numeric get(unsigned i0, unsigned i1,
                           unsigned i2, unsigned i3, unsigned i4) const
            {return at(i0,i1,i2,i3,i4);}

        inline void set(unsigned i0, unsigned i1, unsigned i2,
                        unsigned i3, unsigned i4, unsigned i5, Numeric v)
            {at(i0,i1,i2,i3,i4,i5) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2,
                           unsigned i3, unsigned i4, unsigned i5) const
            {return at(i0,i1,i2,i3,i4,i5);}

        inline void set(unsigned i0, unsigned i1, unsigned i2,
                        unsigned i3, unsigned i4, unsigned i5,
                        unsigned i6, Numeric v)
            {at(i0,i1,i2,i3,i4,i5,i6) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2,
                           unsigned i3, unsigned i4, unsigned i5,
                           unsigned i6) const
            {return at(i0,i1,i2,i3,i4,i5,i6);}

        inline void set(unsigned i0, unsigned i1, unsigned i2,
                        unsigned i3, unsigned i4, unsigned i5,
                        unsigned i6, unsigned i7, Numeric v)
            {at(i0,i1,i2,i3,i4,i5,i6,i7) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2,
                           unsigned i3, unsigned i4, unsigned i5,
                           unsigned i6, unsigned i7) const
            {return at(i0,i1,i2,i3,i4,i5,i6,i7);}

        inline void set(unsigned i0, unsigned i1, unsigned i2,
                        unsigned i3, unsigned i4, unsigned i5,
                        unsigned i6, unsigned i7, unsigned i8, Numeric v)
            {at(i0,i1,i2,i3,i4,i5,i6,i7,i8) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2,
                           unsigned i3, unsigned i4, unsigned i5,
                           unsigned i6, unsigned i7, unsigned i8) const
            {return at(i0,i1,i2,i3,i4,i5,i6,i7,i8);}

        inline void set(unsigned i0, unsigned i1, unsigned i2,
                        unsigned i3, unsigned i4, unsigned i5,
                        unsigned i6, unsigned i7, unsigned i8,
                        unsigned i9, Numeric v)
            {at(i0,i1,i2,i3,i4,i5,i6,i7,i8,i9) = v;}
        inline Numeric get(unsigned i0, unsigned i1, unsigned i2,
                           unsigned i3, unsigned i4, unsigned i5,
                           unsigned i6, unsigned i7, unsigned i8,
                           unsigned i9) const
            {return at(i0,i1,i2,i3,i4,i5,i6,i7,i8,i9);}

        inline void setCl(Numeric v)
            {cl() = v;}
        inline Numeric getCl() const
            {return cl();}

        inline void setCl(double x0, Numeric v)
            {cl(x0) = v;}
        inline Numeric getCl(double x0) const
            {return cl(x0);}

        inline void setCl(double x0, double x1, Numeric v)
            {cl(x0, x1) = v;}
        inline Numeric getCl(double x0, double x1) const
            {return cl(x0, x1);}

        inline void setCl(double x0, double x1, double x2, Numeric v)
            {cl(x0, x1, x2) = v;}
        inline Numeric getCl(double x0, double x1, double x2) const
            {return cl(x0, x1, x2);}

        inline void setCl(double x0, double x1,
                          double x2, double x3, Numeric v)
            {cl(x0, x1, x2, x3) = v;}
        inline Numeric getCl(double x0, double x1,
                             double x2, double x3) const
            {return cl(x0, x1, x2, x3);}

        inline void setCl(double x0, double x1,
                          double x2, double x3, double x4, Numeric v)
            {cl(x0, x1, x2, x3, x4) = v;}
        inline Numeric getCl(double x0, double x1,
                             double x2, double x3, double x4) const
            {return cl(x0, x1, x2, x3, x4);}

        inline void setCl(double x0, double x1, double x2,
                          double x3, double x4, double x5, Numeric v)
            {cl(x0, x1, x2, x3, x4, x5) = v;}
        inline Numeric getCl(double x0, double x1, double x2,
                             double x3, double x4, double x5) const
            {return cl(x0, x1, x2, x3, x4, x5);}

        inline void setCl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, Numeric v)
            {cl(x0, x1, x2, x3, x4, x5, x6) = v;}
        inline Numeric getCl(double x0, double x1, double x2,
                             double x3, double x4, double x5,
                             double x6) const
            {return cl(x0, x1, x2, x3, x4, x5, x6);}

        inline void setCl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7, Numeric v)
            {cl(x0, x1, x2, x3, x4, x5, x6, x7) = v;}
        inline Numeric getCl(double x0, double x1, double x2,
                             double x3, double x4, double x5,
                             double x6, double x7) const
            {return cl(x0, x1, x2, x3, x4, x5, x6, x7);}

        inline void setCl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7, double x8, Numeric v)
            {cl(x0, x1, x2, x3, x4, x5, x6, x7, x8) = v;}
        inline Numeric getCl(double x0, double x1, double x2,
                             double x3, double x4, double x5,
                             double x6, double x7, double x8) const
            {return cl(x0, x1, x2, x3, x4, x5, x6, x7, x8);}

        inline void setCl(double x0, double x1, double x2,
                          double x3, double x4, double x5,
                          double x6, double x7, double x8,
                          double x9, Numeric v)
            {cl(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) = v;}
        inline Numeric getCl(double x0, double x1, double x2,
                             double x3, double x4, double x5,
                             double x6, double x7, double x8,
                             double x9) const
            {return cl(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);}

        inline void setData2(const Numeric* data, unsigned long dataLength)
            {setData(data, dataLength);}

        inline double maxAbsDifference2(const ArrayND& r) const
            {return maxAbsDifference(r);}

        inline bool isEqual2(const ArrayND& r) const
            {return *this == r;}

        inline bool notEqual2(const ArrayND& r) const
            {return !(*this == r);}

        inline ArrayND plus2(const ArrayND& r) const
            {return *this + r;}

        inline ArrayND minus2(const ArrayND& r) const
            {return *this - r;}

        inline ArrayND mul2(const double r) const
            {return *this * static_cast<proper_double>(r);}

        inline ArrayND div2(const double r) const
            {return *this / static_cast<proper_double>(r);}

        inline ArrayND& imul2(const double r)
            {*this *= static_cast<proper_double>(r); return *this;}

        inline ArrayND& idiv2(const double r)
            {*this /= static_cast<proper_double>(r); return *this;}

        inline ArrayND& iadd2(const ArrayND& r)
            {*this += r; return *this;}

        inline ArrayND& isub2(const ArrayND& r)
            {*this -= r; return *this;}

        inline ArrayND& addmul2(const ArrayND& r, const double c)
            {return addmul(r, static_cast<proper_double>(c));}

        inline ArrayND& inPlaceMul2(const ArrayND& r)
            {return inPlaceMul(r);}

        inline ArrayND outer2(const ArrayND& r) const
            {return outer(r);}

        inline ArrayND dot2(const ArrayND& r) const
            {return dot(r);}

        inline ArrayND marginalize2(const ArrayND<proper_double>& r,
                                    const unsigned* indexMap,
                                    const unsigned mapLen) const
            {return marginalize(r, indexMap, mapLen);}

        inline Numeric sum2() const
            {return static_cast<Numeric>((*this).template sum<
                typename PreciseType<Numeric>::type>());}

        inline double sumsq2() const
            {return (*this).template sumsq<long double>();}

        inline ArrayND derivative2(double scale=1.0) const
            {return (*this).template derivative<
                typename PreciseType<Numeric>::type>(scale);}

        inline ArrayND cdfArray2(double scale=1.0) const
            {return (*this).template cdfArray<
                typename PreciseType<Numeric>::type>(scale);}

        inline Numeric cdfValue2(const unsigned *index, unsigned indexLen) const
            {return static_cast<Numeric>((*this).template cdfValue<
                typename PreciseType<Numeric>::type>(index, indexLen));}

        inline void convertToLastDimCdf2(ArrayND* sumSlice, bool b)
            {(*this).template convertToLastDimCdf<
                typename PreciseType<Numeric>::type>(sumSlice, b);}

        inline bool isClose2(const ArrayND& r, double eps) const
            {return isClose(r, eps);}

        inline bool isShapeCompatible2(const ArrayND& r) const
            {return isShapeCompatible(r);}

        inline void exportSlice2(ArrayND* slice,
                                 const unsigned *fixedIndices,
                                 const unsigned *fixedIndexValues,
                                 unsigned nFixedInd) const
            {exportSlice(slice, fixedIndices, fixedIndexValues, nFixedInd);}

        inline void exportMemSlice2(Numeric* slice, unsigned long len,
                                    const unsigned *fixedIndices,
                                    const unsigned *fixedIndexValues,
                                    unsigned nFixedInd) const
            {exportMemSlice(slice, len, fixedIndices,
                            fixedIndexValues, nFixedInd);}

        inline void importSlice2(const ArrayND& slice,
                                 const unsigned *fixedIndices,
                                 const unsigned *fixedIndexValues,
                                 unsigned nFixedInd)
            {importSlice(slice, fixedIndices, fixedIndexValues, nFixedInd);}

        inline void importMemSlice2(const Numeric* slice, unsigned long len,
                                    const unsigned *fixedIndices,
                                    const unsigned *fixedIndexValues,
                                    unsigned nFixedInd)
            {importMemSlice(slice, len, fixedIndices,
                            fixedIndexValues, nFixedInd);}

        inline ArrayND& multiplyBySlice2(const ArrayND<proper_double>& slice,
                                         const unsigned *fixedIndices,
                                         unsigned nFixedIndices)
            {return multiplyBySlice(slice, fixedIndices, nFixedIndices);}

        inline ArrayND subrange(const ArrayRange& range)
            {return ArrayND(*this, range);}

        inline ArrayND slice(const unsigned *index, unsigned indexLen)
            {return ArrayND(*this, index, indexLen);}

        inline void rotate2(const unsigned* shifts, unsigned lenShifts,
                            ArrayND* rotated) const
            {rotate(shifts, lenShifts, rotated);}

        inline void exportSubrange2(const unsigned* corner, unsigned lenCorner,
                                    ArrayND* to) const
            {exportSubrange(corner, lenCorner, to);}

        inline void importSubrange2(const unsigned* corner, unsigned lenCorner,
                                    const ArrayND& from)
            {importSubrange(corner, lenCorner, from);}

        inline void multiMirror2(ArrayND* out) const
            {multiMirror(out);}

#endif // SWIG
    };

#ifdef CPP11_STD_AVAILABLE
    //@{
    /**
    // This function allows you to manage external data memory
    // using ArrayND objects. It can be used, for example, to create
    // arrays which live completely on the stack and still manage
    // sizeable chunks of already allocated memory. It is up to the
    // user of this function to ensure that the size of the memory
    // buffer is consistent with the shape of the array.
    //
    // Note that the responsibility to manage external data passes
    // to another ArrayND through a move constructor or through
    // a move assignment operator but not through a normal copy
    // constructor or assignment operator. Because of this feature,
    // this function works only with compilers supporting C++11.
    */
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data,
                                        const unsigned* shape, unsigned dim);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, const ArrayShape& shape);

    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0);

    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1);

    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4,
                                        unsigned n5);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4,
                                        unsigned n5, unsigned n6);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4,
                                        unsigned n5, unsigned n6, unsigned n7);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4,
                                        unsigned n5, unsigned n6, unsigned n7,
                                        unsigned n8);
    template <typename Numeric>
    ArrayND<Numeric> externalMemArrayND(Numeric* data, unsigned n0, unsigned n1,
                                        unsigned n2, unsigned n3, unsigned n4,
                                        unsigned n5, unsigned n6, unsigned n7,
                                        unsigned n8, unsigned n9);
    //@}
#endif // CPP11_STD_AVAILABLE
}

#include "npstat/nm/ArrayND.icc"

#endif // NPSTAT_ARRAYND_HH_
