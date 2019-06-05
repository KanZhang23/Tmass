The code in this directory should not depend on headers from any other
directory of the "npstat" package.

This directory contains implementations for a number of useful mathematical
objects, integration and root finding codes, etc. The classes and functions
can be approximately split into the following subject groups:

* Multidimensional arrays, grids, and related utilities
* Optimization and root finding
* Integration
* Interpolation
* Orthogonal polynomials and filtering
* Searcheable multidimensional structures
* Miscellaneous utilities


Multidimensional arrays, grids, and related utilities
-----------------------------------------------------

ArrayND.hh             -- Multidimensional array template.

ArrayNDScanner.hh      -- A class which can be used to iterate over
                          multidimensional array indices. Useful when
                          the array itself is not available (for example,
                          for iterating over slices, etc).

AbsArrayProjector.hh   -- Interface class used in iterations over array
                          elements.

AbsMultivariateFunctor.hh -- Interface class for a variety of multivariate
                             functor-based calculations.

CoordinateSelector.hh  -- A trivial AbsMultivariateFunctor implementation
                          which selects one of the elements from the input
                          array.

MultivariateFunctorScanner.hh -- Utility for filling array values from
                                 multivariate functors.

AbsVisitor.hh          -- Another interface class used in iterations over
                          array elements. Use this if elements indices is
                          not of interest and only the array value is used.

ArrayRange.hh          -- Used in constructing array subranges.

ArrayShape.hh          -- Defines the shape of multidimensional arrays.

BoxND.hh               -- Represents rectangles, boxes, and hyperboxes

BoxNDScanner.hh        -- A class for iterating over all coordinates in
                          a multidimensional box (like using histogram
                          bin centers).

EigenMethod.hh         -- Helper enum for use with Matrix methods for
                          finding eigenvalues and eigenvectors.

SvdMethod.hh           -- Helper enum for use with Matrix methods for
                          performing singular value decompositions.

GridAxis.hh            -- Can be used to define an axis of a rectangular
                          grid (not necessarily equidistant).

Interval.hh            -- 1-d intervals. Used by BoxND to bulid boxes.

Matrix.hh              -- A simple matrix template. Outsorces most of its
                          more advanced functionality to LAPACK.

matrixIndexPairs.hh    -- A utlitity for enumerating matrix elements on
                          the main diagonal and above.

PointDimensionality.hh -- Compile-time dimensionality detector for classes
                          like std::array.

UniformAxis.hh         -- Can be used to define an equidistant axis of
                          a rectangular grid.

DualAxis.hh            -- Can be used to define either equidistant or
                          non-uniform axis, with O(1) coordinate lookup
                          for equidistant grids.

fillArrayCentersPreservingAreas.hh -- It is assumed that we have an array
                          with the number of elements increased by an integer
                          factor in each dimension in comparison with the
                          original array. This array is filled from the
                          original one in the manner appropriate for
                          subsequent smoothing. This is useful in certain
                          rebinning situations, when some quantity calculated
                          on some grid needs to be smoothed and interpolated
                          on a denser grid.

truncatedInverseSqrt.hh -- Truncate the square root of a symmetric positive
                           semidefinite matrix by keeping eigenvectors
                           corresponding to largest eigenvalues.


Optimization and root finding
-----------------------------

MinSearchStatus1D.hh     -- Status of minimum search on a 1-d interval.

findPeak2D.hh            -- Reliable peak finding on 2-d grids.

goldenSectionSearch.hh   -- Search for 1-d function minimum using the
                            golden section method.

findRootInLogSpace.hh    -- Search for a solution of a single equation of
                            the type f(x) == rhs where x is some type that
                            can be multiplied by a positive double. Uses
                            interval division.

findRootNewtonRaphson.hh -- Search for a solution of a single equation of
                            the type f(x) == rhs using the Newton-Raphson
                            method.

findRootUsingBisections.hh -- Search for a solution of a single equation of
                              the type f(x) == rhs using bisections.

See also "MathUtils.hh" header for codes that find roots of quadratic
and cubic equations.


Integration
-----------

FejerQuadrature.hh         -- 1-d Fejer quadratures (essentially, using
                              Chebyshev polynomials).

GaussHermiteQuadrature.hh  -- 1-d Gaussian quadratures using Hermite
                              polynomials.

GaussLegendreQuadrature.hh -- 1-d Gaussian quadratures using Legendre
                              polynomials.

rectangleQuadrature.hh     -- Gaussian quadratures on rectangles and
                              hyperrectangles using tensor product integration.


Interpolation
-------------

bilinearSection.hh     -- Finds the contours of the intersection of a bilinear
                          interpolation cell (specified by values at the
                          corners of the unit square) with a given constant
                          level. For use in higher-level mapping and
                          contouring algorithms.

interpolate.hh         -- Simple interpolating polynomials (linear, quadratic,
                          cubic) on regularly spaced 1-d grids.

LinInterpolatedTable1D.hh -- Linearly interpolated table in 1-d.

LinInterpolatedTableND.hh -- Interpolated table in multiple dimensions
                             (with multilinear interpolation).

LinearMapper1d.hh      -- Linear functor in 1-d for use in interpolation
                          and extrapolation.

CircularMapper1d.hh    -- Linear coordinate mapper for circular topologies.

LogMapper1d.hh         -- Functor which is linear in log(x). Useful for
                          interpolating functions of strictly positive
                          quantities.

ExpMapper1d.hh         -- Functor which is linear in log(y). Useful for
                          interpolating functions that are strictly positive.

rescanArray.hh         -- A utility for filling one array using values of
                          another. The arrays are treated as values of
                          histogram bins inside the unit box, and
                          interpolations are performed as necessary.


Orthogonal polynomials and filtering
------------------------------------

AbsClassicalOrthoPoly1D.hh -- Base class for classical orthogonal polynomials.

ClassicalOrthoPolys1D.hh   -- Concrete implementations of various classical
                          orthogonal polynomials (Legendre, Jacobi, etc).

ContOrthoPoly1D.hh     -- Continuous orthogonal polynomials for discrete
                          measures (intended for constructing empirical
                          chaos polynomials).

ConvolutionEngine1D.hh -- FFTW double precision interface to be used
                          for implementing convolutions in 1-d.

ConvolutionEngineND.hh -- FFTW double precision interface to be used
                          for implementing convolutions on grids
                          of arbitrary dimensionality.

DiscreteBernsteinPoly1D.hh -- Discrete Bernstein polynomials in one dimension
                          (preserving the partition of unity property).

FourierImage.hh        -- Wrapper class for memory blocks allocated by
                          fftw_malloc and deallocated by fftw_free.
                          Intended for storing results of Fourier transforms.

OrthoPoly1D.hh         -- Discrete orthogonal polynomials in 1-d (typically,
                          for use in linear filters).

OrthoPolyND.hh         -- Discrete orthogonal polynomials of arbitrary
                          dimensionality in hyperrectangular domains.

OrthoPolyMethod.hh     -- Enum for the methods used to construct the
                          continuous polynomials (class ContOrthoPoly1D).

StorablePolySeries1D.hh -- Storable functor for orthogonal polynomial series.

See also "MathUtils.hh" header for codes that calculate series for some
classical orthogonal polynomial systems.


Searcheable multidimensional structures
---------------------------------------

CompareByIndex.hh      -- Comparison functor for objects that support
                          subsripting. Used by k-d tree code.

KDTree.hh              -- Balanced k-d tree implementation. All points
                          must be known in advance.


Miscellaneous utilities
-----------------------

absDifference.hh       -- Proper calculation of absolute value and absolute
                          difference of two values for a number of types
                          including unsigned, complex, etc.

allocators.hh          -- A few utility functions related to memory management.

areAllElementsUnique.hh -- A simple template for checking uniqueness of
                          container values using O(N^2) algorithm.

binomialCoefficient.hh -- Calculation of binomial coefficients which avoids
                          overflows.

closeWithinTolerance.hh -- Closeness comparison for doubles.

ComplexComparesAbs.hh  -- Use ComplexComparesAbs<T>::less in the templated
ComplexComparesFalse.hh   code when it makes sense to do comparison of complex
                          numbers by magnitude and comparison of other types
                          by "operator<". ComplexComparesFalse<T>::less returns
                          "false" for complex types and compares other types
                          with "operator<". "more" methods are similar.

ConstSubscriptMap.hh   -- A variation of std::map template with const
                          subscripting operator.

definiteIntegrals.hh   -- Definite integrals based on exact expressions.

discretizedDistance.hh -- L1 and L2 distances for discretized functions.

EquidistantSequence.hh -- Sequences of points equidistant in 1-d linear
                          or log space.

GeneralizedComplex.hh  -- Define a type which will be set to std::complex<T>
                          for T from float, double, and long double, and to
                          T for all other types.

isMonotonous.hh        -- A few simple templates for checking monotonicity
                          of container values.

lapack.h               -- LAPACK-related declarations (F77 versions).

lapack_interface.hh    -- High-level interface to some of the LAPACK functions. 

MathUtils.hh           -- Miscellaneous utilities.

PairCompare.hh         -- Additional comparison functors for std::pair.

PreciseType.hh         -- PreciseType<T>::type is typedefed to "long double"
                          for primitive scalar types and to
                          "std::complex<long double>" for complex types.

ProperDblFromCmpl.hh   -- ProperDblFromCmpl<T>::type is a floating point type
                          which can be multiplied by T when T is complex,
                          otherwise it is just "double".

PtrBufferHandle.hh     -- Resource handler for an array of pointers to
                          explicitly allocated objects (to complement a vector
                          of pointers). Not for use by application code.

SimpleFunctors.hh      -- Interface classes and concrete simple functors
                          for a variety of functor-based calculations.

SpecialFunctions.hh    -- Special functions.

timestamp.hh           -- Generate a trivial time stamp in the format hh:mm:ss.

Triple.hh              -- A template with three members, similar to
                          std::pair in purpose and functionality.
