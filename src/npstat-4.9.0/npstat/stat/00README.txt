The code in this directory can depend on headers from the "nm" and "rng"
directories in the "npstat" package.

The classes implemented in this directory can be split into several
subgroups by their purpose:

* Data representation
* Descriptive statistics
* Statistical distributions
* Fitting of parametric models
* Statistical testing
* Local filtering
* Nonparametric density estimation
* Deconvolution density estimation (Unfolding)
* Nonparametric regression
* Interpolation of statistical distributions
* Miscellaneous data analysis techniques
* Convenience API
* Utilities
* I/O


Data representation
-------------------

AbsNtuple.hh -- interface class for ntuples. Implemented in InMemoryNtuple.hh,
    ArchivedNtuple.hh. To be used by application code.

NtHistoFill.hh, NtNtupleFill.hh, NtRectangularCut.hh -- convenience classes
    and functors for use inside "cycleOverRows", "conditionalCycleOverRows",
    and other similar ntuple methods.

Column.hh -- helper class for AbsNtuple.hh. Can be used to refer to ntuple
    columns either by name or by column number. Used by AbsNtuple.hh, and
    should not be used separately.

ArchivedNtuple.hh -- class for storing huge ntuples which do not fit in the
    computer memory. To be used by application code.

HistoAxis.hh -- representation of a histogram axis with equidistant bins.

NUHistoAxis.hh -- representation of a histogram axis with non-uniform bins.

DualHistoAxis.hh -- histogram axis which works reasonably well for both
    uniform and non-uniform bins.

convertAxis.hh -- conversion functions between histogram and grid axes.

HistoND.hh -- arbitrary-dimensional histogram template.

interpolateHistoND.hh -- interpolation of histogram data to coordinate
    values between bin centers.

mergeTwoHistos.hh -- a utility for smooth blending of two histograms.

ProductSymmetricBetaNDCdf.hh -- an interpolation function for blending
    histograms.

InMemoryNtuple.hh -- class for storing small ntuples which completely fit
    in the computer memory. Works faster than ArchivedNtuple.

NtupleBuffer.hh -- data buffer for data exchange between ArchivedNtuple and
    disk-based archive.

OrderedPointND.hh -- a multidimensional point which can be sorted according
    to multiple sorting criteria.

StorableMultivariateFunctor.hh -- Base class for storable multivariate functors.

StorableHistoNDFunctor.hh  -- Adaptation that represents HistoND as
    a StorableMultivariateFunctor.

HistoNDFunctorInstances.hh -- A number of concrete typedefs for
    StorableHistoNDFunctor template.

StorableInterpolationFunctor.hh  -- Adaptation that represents
    LinInterpolatedTableND as a StorableMultivariateFunctor.

InterpolationFunctorInstances.hh -- A number of concrete typedefs for
    StorableInterpolationFunctor template.


Descriptive statistics
----------------------

ArrayProjectors.hh -- helper classes for making lower-dimensional projections
    of the ArrayND class. Calculate various statistics over projected
    dimensions (mean, median, etc).

arrayStats.hh -- means, standard deviations, and covariance matrices of
    multivariate arrays. skewness and kurtosis for 1-d arrays.

statUncertainties.hh -- uncertainties of various sample statistics.

histoStats.hh -- means and covariance matrices for histograms.

kendallsTau.hh -- Kendall's rank correlation coefficient, from a sample
    of points or from copula.

spearmansRho.hh -- Spearman's rank correlation coefficient, from a sample
    of points or from copula.

logLikelihoodPeak.hh -- summary of a 1-d log-likelihood curve.

MultivariateSumAccumulator.hh           -- classes in these files are
MultivariateSumsqAccumulator.hh            intended for calculating
MultivariateWeightedSumAccumulator.hh      covariance and correlation
MultivariateWeightedSumsqAccumulator.hh    matrices of multivariate data
                                           in a numerically stable manner.
    These classes can serve as appropriate accumulator functors in various
    "cycleOverRows" methods of AbsNtuple or as separate tools.

SampleAccumulator.hh -- accumulator of items for the purpose of calculating
    statistical summaries. For use inside histograms, etc.

WeightedSampleAccumulator.hh -- similar class for use with weighted items.

CircularBuffer.hh -- accumulator of items with fixed length. When this
    length is exceeded, the oldest items are discarded. Can calculate
    statistical summaries (mean, stdev) that do not require element sorting.

StatAccumulator.hh -- simple, single-pass calculator of mean and standard
    deviation. Updates a running average, so it works a bit better than
    a "naive" version.

StatAccumulatorPair.hh -- simple, single-pass calculator of mean, standard
    deviation, and covariance for two variables.

StatAccumulatorArr.hh -- simple, single-pass calculator of mean and standard
    deviation for array sets.

CrossCovarianceAccumulator.hh -- single-pass calculator of covariances
    and correlations for samples with elements represented by arrays.

WeightedStatAccumulator.hh -- single-pass calculator of mean and standard
    deviation for weighted points.

WeightedStatAccumulatorPair.hh -- simple, single-pass calculator of mean,
    standard deviation, and covariance for weighted points.

BinSummary.hh -- a five-number summary for SampleAccumulator, StatAccumulator,
    WeightedSampleAccumulator, and WeightedSampleAccumulator which can be
    used for making box plots, etc. Allows for direct manipulation of
    the center value, upper and lower ranges, and min/max value.


Statistical distributions
-------------------------

AbsDistribution1D.hh -- interface classes for 1-d parametric and tabulated
    statistical distributions.

AbsDiscreteDistribution1D.hh -- interface classes for 1-d discrete
    statistical distributions. Implemented in DiscreteDistributions1D.hh.

AbsDistributionTransform1D.hh -- interface class for 1-d coordinate
    transforms intended for subsequent use in constructing distributions
    (in particular, with the TransformedDistribution1D class).

AsinhTransform1D.hh -- asinh transform (just like the one used to generate
    Johnson's S_U curves).

SinhAsinhTransform1D.hh -- transform y = sinh(a + b*asinh(x)).

LogRatioTransform1D.hh -- log(y/(1-y)) transform with scale and location
    adjustment (just like the one used to generate Johnson's S_B curves).

LogTransform1D.hh -- transform y = log(1 + x).

AbsDistributionND.hh -- interface classes for multivariate statistical
    distributions.

CompositeDistribution1D.hh -- represents univariate statistical distributions
    whose cumulative distribution functions F(x) can be built by composition
    of two other cumulative distribution functions: F(x) = G(H(x)).

CompositeDistributionND.hh -- represents multivariate statistical
    distributions decomposed into copula and marginals.

Copulas.hh -- distributions which are copulas.

CompositeDistros1D.hh -- several implementations of CompositeDistribution1D.hh
    using concrete distributions.

Distributions1D.hh -- a number of continuous 1-d statistical distributions.

DistributionsND.hh -- a number of continuous multivariate statistical
    distributions.

DiscreteDistributions1D.hh -- several discrete statistical distributions,
    including Poisson and tabulated.

DistributionMix1D.hh -- Mixtures of one-dimensional statistical distributions.

distro1DStats.hh -- a utility for empirical calculation of distribution mean,
    standard deviation, skewness, and kurtosis.

GaussianMixture1D.hh -- One-dimensional Gaussian mixtures.

GridRandomizer.hh -- class which knows how to map the unit hypercube into
    a distribution whose density is represented on an n-d grid (and, therefore,
    how to generate corresponding random numbers). Not intended for direct use
    by application code (use class "BinnedDensityND" instead).

IdentityTransform1D.hh -- Identity coordinate transformation.

LocationScaleFamily1D.hh -- Creates a location-scale family from
                            a (typically non-scalable) 1-d distribution.

LocationScaleTransform1.hh -- Coordinate transformation of the type
    y = (x - mu)/sigma, with mu and sigma depending on a single parameter
    and calculated by two separate functors.

JohnsonCurves.hh -- Johnson frequency curves.

RatioOfNormals.hh -- Distribution generated when one Gaussian variable
    is divided by another.

SbMomentsCalculator.hh -- Calculator of moments for S_b curves. Not for use
    by application code.

ScalableGaussND.hh -- multivariate Gaussian with diagonal covariance matrix.

TransformedDistribution1D.hh -- 1-d distributions in x but whose density,
    cdf, etc are specified in the y (transformed) space.

TruncatedDistribution1D.hh -- 1-d distributions with truncated support.

LeftCensoredDistribution.hh  -- left-censored distribution.

RightCensoredDistribution.hh -- right-censored distribution.

UGaussConvolution1D.hh -- convolution of uniform and Gaussian distributions.


Fitting of parametric models
----------------------------

FitUtils.hh -- fitting of 1-d histograms. See also headers
    minuitFitJohnsonCurves.hh and MinuitDensityFitFcn1D.hh in the
    "interfaces" directory.


Statistical testing
-------------------

AbsBinnedComparison1D.hh -- interface for comparisons of binned distributions

BinnedADTest1D.hh -- binned version of the Anderson-Darling test

BinnedKSTest1D.hh -- binned version of the Kolmogorov-Smirnov test

PearsonsChiSquared.hh -- chi-squared goodness-of-fit test


Local filtering
---------------

AbsFilter1DBuilder.hh -- interface classes for building local polynomial
    filters in 1d. Implemented in Filter1DBuilders.hh, BetaFilter1DBuilder.hh,
    WeightTableFilter1DBuilder.hh, BernsteinFilter1DBuilder.hh. Used by
    LocalPolyFilter1D.hh.

AbsPolyFilter1D.hh -- interface class for building univariate smoothers
    that can be optimized by cross-validation. Implemented in
    ConstantBandwidthSmoother1D.hh, LocalPolyFilter1D.hh.

AbsPolyFilterND.hh -- interface class for building multivariate smoothers
    that can be optimized by cross-validation. Implemented in
    LocalPolyFilterND.hh, SequentialPolyFilterND.hh, and KDEFilterND.hh.
    Used by AbsBandwidthCV.hh, BandwidthCVLeastSquaresND.hh,
    BandwidthCVPseudoLogliND.hh.

AbsSymbetaFilterProvider.hh -- interface class for building local polynomial
    filters in 1d using kernels from the symmetric beta family.

MemoizingSymbetaFilterProvider.hh -- implements AbsSymbetaFilterProvider
    interface and allows for filter storage and lookup.

BernsteinFilter1DBuilder.hh -- concrete class for building filters which
    smooth densities using Bernstein polynomials.

BetaFilter1DBuilder.hh -- concrete class for building filters which
    smooth densities using beta functions (Bernstein polynomials of
    non-integer degree).

betaKernelsBandwidth.hh -- optimal bandwidth for density estimation with beta
    kernels.

BoundaryHandling.hh -- user API for handling LOrPE boundary methods.

BoundaryMethod.hh -- enums for handling LOrPE boundary methods.

continuousDegreeTaper.hh -- a method for generating tapers with effectively
    continuous degree. Intended for use with LocalPolyFilter1D.

Filter1DBuilders.hh -- concrete classes for building local polynomial
    filters in 1d. They differ by their treatment of the weight function
    and boundary effects.

WeightTableFilter1DBuilder.hh -- concrete classes for building local
    polynomial filters in 1d from density scans.

KDEFilterND.hh -- KDE filtering (Nadaraya-Watson regression) on a regularly
    spaced 1-d grid.

LocalPolyFilter1D.hh -- local polynomial filtering (regression) on a regularly
    spaced 1-d grid.

LocalPolyFilterND.hh -- local polynomial filtering (regression) on a regularly
    spaced multivariate grid.

LocalMultiFilter1D.hh -- local polynomial filtering with separate polynomials
    from an orthogonal set.

LocalQuadraticLeastSquaresND.hh -- local quadratic polynomial filtering for
    an irregular set of points (possibly including uncertainties).

lorpeSmooth1D.hh -- high level driver for LocalPolyFilter1D, etc. Intended
    for density reconstruction from histograms.

lorpeBackground1D.hh -- high level driver for fitting mixed models in which
    signal is parameterized and background is nonparametric.

lorpeBackgroundCVDensity1D.hh -- linearization of cross-validation calculations
    for fitting mixed models with nonparametric background.

QuadraticOrthoPolyND.hh -- local quadratic polynomial filtering on a grid.
    In comparison with LocalPolyFilterND.hh, supports a finer interface to
    filtering functionality (direct support of an AbsDistributionND as
    a weight, calculations of gradient and hessian for the fitted surface,
    fitting is performed on functors rather than ArrayND objects, etc).
    Used by LocalLogisticRegression.hh.

SequentialPolyFilterND.hh -- similar to LocalPolyFilterND.hh, but the
    filtering is performed sequentially for each dimension using 1-d filters.

SymbetaPolyCollection1D.hh -- class that builds LocalPolyFilter1D objects
    and memoizes local polynomials for the bandwidth values used.


Nonparametric density estimation
--------------------------------

AbsBandwidthCV.hh -- interface classes for calculating 1-d and multivariate
    cross-validation criteria for bandwidth and taper selection. Interfaces
    declared in this file are implemented in BandwidthCVLeastSquares1D.hh,
    BandwidthCVLeastSquaresND.hh, BandwidthCVPseudoLogli1D.hh, and
    BandwidthCVPseudoLogliND.hh. These interfaces are used by classes in
    CVCopulaSmoother.hh.

AbsBandwidthGCV.hh -- interface classes for calculating 1-d and multivariate
    cross-validation criteria for bandwidth and taper selection. Interfaces
    declared in this file are implemented in BandwidthGCVLeastSquares1D.hh,
    BandwidthGCVLeastSquaresND.hh, BandwidthGCVPseudoLogli1D.hh, and
    BandwidthGCVPseudoLogliND.hh. These interfaces are used by classes in
    GCVCopulaSmoother.hh. The difference with the series of classes defined
    in "AbsBandwidthCV.hh" is that the grouping (i.e., the binning) of data
    is explicitly acknowledged, so that a substantially different set of
    filters (removing the whole group) can be used for cross-validation.

AbsCompositeDistroBuilder.hh -- interface class for building composite
    distrubutions (which consist of copula and marginals) out of data samples.
    Implemented in DummyCompositeDistroBuilder.hh and
    NonparametricCompositeBuilder.hh.

AbsDistro1DBuilder.hh -- interface class for building 1-d distributions out
    of data samples. Implemented in DummyDistro1DBuilder.hh and
    NonparametricDistro1DBuilder.hh.

AbsCopulaSmootherBase.hh -- interface class for building copulas out of data
    samples. Implemented in AbsCVCopulaSmoother.hh. Used by
    NonparametricCompositeBuilder.hh.

AbsKDE1DKernel.hh -- interface class for simple, brute-force KDE in 1-d
    without discretization or boundary correction. Implemented in
    KDE1DHOSymbetaKernel.hh.

AbsMarginalSmootherBase.hh -- interface class for building 1-d smoothers of
    histograms. Implemented in JohnsonKDESmoother.hh, LOrPEMarginalSmoother.hh,
    ConstantBandwidthSmoother1D.hh, and VariableBandwidthSmoother1D.hh. Used
    by NonparametricCompositeBuilder.hh.

AbsResponseIntervalBuilder.hh  -- interface class for making cuts in the
    inivariate response space when density estimation is performed in
    the regression context. Implemented in DummyResponseIntervalBuilder.hh
    and RatioResponseIntervalBuilder.hh.

AbsResponseBoxBuilder.hh -- interface class for making cuts in the
    multivariate response space when density estimation is performed
    in the regression context. Implemented in DummyResponseBoxBuilder.hh
    and RatioResponseBoxBuilder.hh.

amiseOptimalBandwidth.hh -- function for selecting optimal LOrPE bandwidth
    values by optimizing AMISE on a reference distribution. Used in
    JohnsonKDESmoother.cc and ConstantBandwidthSmoother1D.cc.
    Can also be used by application code.

BandwidthCVLeastSquares1D.hh -- class for calculating KDE or LOrPE
    cross-validation MISE approximations for 1-d density estimates.

BandwidthCVLeastSquaresND.hh -- class for calculating KDE or LOrPE
    cross-validation MISE approximations for multivariate density estimates.

BandwidthCVPseudoLogli1D.hh -- class for calculating KDE or LOrPE
    cross-validation pseudo log likelihood, for 1-d density estimates.

BandwidthCVPseudoLogliND.hh -- Class for calculating KDE or LOrPE
    cross-validation pseudo log likelihood, for multivariate density estimates.

buildInterpolatedCompositeDistroND.hh -- Multivariate density estimation
    in the regression context, with interpolation.

buildInterpolatedDistro1DNP.hh -- Univariate density estimation
    in the regression context, with interpolation.

ConstantBandwidthSmoother1D.hh -- 1-d KDE implementation with constant
    bandwidth. Implements AbsMarginalSmoother interface.

ConstantBandwidthSmootherND.hh -- multivariate KDE implementation with
    constant bandwidth.

CVCopulaSmoother.hh -- an interface to copula smoothers which use constant
    bandwidth LOrPE and select bandwidth by cross-validation. Implemented
    in LOrPECopulaSmoother.hh, SequentialCopulaSmoother.hh,
    KDECopulaSmoother.hh, and BernsteinCopulaSmoother.hh. Could be used by
    application code if it needs to develop its own cross-validation method
    for nonparametric copula estimation.

GCVCopulaSmoother.hh -- an interface to copula smoothers working with
    grouped data and using substantially different filters for
    cross-validation. Implemented in KDEGroupedCopulaSmoother.hh,
    LOrPEGroupedCopulaSmoother.hh, and SequentialGroupedCopulaSmoother.hh.

empiricalCopula.hh -- functions for building empirical copulas by constructing
    kd-tree for the data points and then doing lookups in this tree.

empiricalCopulaHisto.hh -- function for building empirical copula densities
    by ordering the data points in multiple dimensions.

weightedCopulaHisto.hh -- function for building empirical copula densities
    by ordering weighted data points in multiple dimensions.

HistoNDCdf.hh -- multivariate cumulative distribution function built
    from a histogram. Its "coveringBox" method can be used to make
    k-NN type density estimates (and for other purposes).

JohnsonKDESmoother.hh -- 1-d KDE implementation with adaptive bandwidth
    (see comments in the header file for details). Implements
    AbsMarginalSmoother interface. See also "fitCompositeJohnson.hh"
    header in the "interfaces" directory for an alternative approach.

KDE1D.hh -- Convenience class which aggregates the kernel and the data
    for brute-force 1-d KDE without boundary correction.

KDE1DCV.hh -- Cross-validation utilities for brute-force KDE in 1-d.

KDE1DHOSymbetaKernel.hh -- high order Gaussian or symmetric beta kernels
    for brute-force KDE in 1-d.

KDECopulaSmoother.hh -- constant bandwidth multivariate KDE copula
    smoother in which the bandwidth is selected by cross-validation.
    Implements CVCopulaSmoother.

LOrPECopulaSmoother.hh -- constant bandwidth multivariate LOrPE copula
    smoother in which the bandwidth is selected by cross-validation.
    Implements CVCopulaSmoother.

LOrPEMarginalSmoother.hh -- 1-d LOrPE for fitting margins of composite
    distributions. Basically, interfaces "lorpeSmooth1D" to AbsMarginalSmoother.

lorpeMise1D.hh -- Deterministic MISE calculation for reconstructing
    an arbitrary 1-d density.

NonparametricCompositeBuilder.hh -- an implementation of
    AbsCompositeDistroBuilder. Uses separate density estimation procedures
    for copula and marginals.

orthoPoly1DVProducts.hh -- utility functions for calculating certain
    statistical properties of 1-d orthogonal polynomials.

OSDE1D.hh -- orthogonal series density estimation in one dimension.

PolyFilterCollection1D.hh -- collection of LocalPolyFilter1D objects with
    the same kernel but different bandwidth values. Intended for use with
    bandwidth scans (for example, in cross-validation scenarios).

QuantileTable1D.hh -- density function defined by its quantile table.
    Can be easily constructed using "empiricalQuantile" function from
    StatUtils.hh.

SequentialCopulaSmoother.hh -- similar to LOrPECopulaSmoother, but the
    filters are limited to tensor products of univariate filters.

variableBandwidthSmooth1D.hh -- KDE with adaptive bandwidth. Used by
    JohnsonKDESmoother.hh.


Deconvolution density estimation (Unfolding)
--------------------------------------------

AbsUnfold1D.hh -- interface class for deconvolution density estimation in 1-d
    (a.k.a. unfolding).

AbsUnfoldND.hh -- interface class for multivariate deconvolution density
    estimation (a.k.a. unfolding).

AbsUnfoldingFilterND.hh -- interface class for smoothers used in multivariate
    unfolding.

gaussianResponseMatrix.hh  -- helper function for building response matrices
    for one-dimensional unfolding problems.

MultiscaleEMUnfold1D.hh -- a variation of 1-d unfolding algorithm with
    multiscale filtering and, potentially, faster convergence.

productResponseMatrix.hh   -- helper function for building sparse response
    matrices for multivariate unfolding problems.

ResponseMatrix.hh -- sparse response matrix representation for multivariate
    unfolding.

SmoothedEMUnfold1D.hh -- expectation-maximization (a.k.a. D'Agostini)
    unfolding with smoothing for 1-d distributions.

SmoothedEMUnfoldND.hh -- expectation-maximization unfolding with smoothing
    for multivariate distributions.

UnfoldingBandwidthScanner1D.hh -- class which gets various information from
    1-d unfolding results in a convenient form.

UnfoldingBandwidthScannerND.hh -- class which gets various information from
    multivariate unfolding results in a convenient form.


Nonparametric regression
------------------------

LocalLogisticRegression.hh -- facilities for performing local linear and
    quadratic logistic regression. The interface is designed for use
    together with Minuit. See also the header "minuitLocalRegression.hh"
    in the "interfaces" directory.

QuantileRegression1D.hh -- nonparametric quantile regression with one
    predictor. Supports polynomials of arbitrary degrees. Useful for
    constructing Neyman belts. See also "minuitLocalQuantileRegression1D.hh"
    header in the "interfaces" directory.

LocalQuantileRegression.hh -- multivariate local linear or quadratic
    quantile regression. Can be used to fit histograms or collections
    of points. See also "minuitQuantileRegression.hh" header in the
    "interfaces" directory.

CensoredQuantileRegression.hh -- multivariate local linear or quadratic
    quantile regression which can be used for data samples affected by
    a one-sided cut.

griddedRobustRegression.hh -- framework for local robust regression
    (in particular, for local least trimmed squares).

GriddedRobustRegressionStop.hh -- simple functor for stopping robust
    regression sequence.

AbsLossCalculator.hh -- abstract class for calculating local loss for
    local robust regression. Implemented in "WeightedLTSLoss.hh" and
    "TwoPointsLTSLoss.hh".

WeightedLTSLoss.hh -- functor for calculating local least trimmed squares
    with one point removed.

TwoPointsLTSLoss.hh -- functor for calculating local least trimmed squares
    with two points or 1-d stripes removed.


Interpolation of statistical distributions
------------------------------------------

AbsGridInterpolatedDistribution.hh -- interface class for interpolating
    between probability distributions placed at the points of a rectangular
    parameter grid. Implemented in GridInterpolatedDistribution.hh. To be
    used by application code.

AbsInterpolatedDistribution1D.hh -- interface class for univariate density
    interpolation algorithms. Implemented by InterpolatedDistribution1D.hh
    and VerticallyInterpolatedDistribution1D.hh.

AbsInterpolationAlgoND.hh -- interface class for multivariate density
    interpolation algorithms. Implemented by CopulaInterpolationND.hh
    and UnitMapInterpolationND.hh. Used by GridInterpolatedDistribution.hh.

CopulaInterpolationND.hh -- interpolation of distributions represented by
    CompositeDistributionND. Copulas and quantile functions of the marginals
    are combined with externally provided weights.

UnitMapInterpolationND.hh -- interpolation of distributions mapped to the
    unit cube by conditional quantile functions.

GridInterpolatedDistribution.hh -- class which represents a complete
    multivariate distribution interpolated in parameters. Constructed
    incrementally, by adding distributions to the grid points.

InterpolatedDistribution1D.hh -- 1-d continuous statistical distribution
    which interpolates between other distributions by performing linear
    interpolation of the quantile function.

InterpolatedDistro1D1P.hh -- 1-d continuous statistical distribution
    interpolation on a 1-d parameter grid, with linear interpolation
    of weights between parameter values. Supports both interpolation
    of quantile functions and vertical interpolation.

InterpolatedDistro1DNP.hh -- 1-d continuous statistical distribution
    interpolation on a multivariate parameter grid, with multilinear
    interpolation of weights between parameter values. Supports both
    interpolation of quantile functions and vertical interpolation.

UnitMapInterpolationND.hh -- this class interpolates between multivariate
    distributions by interpolating between their unit maps.


Miscellaneous data analysis techniques
--------------------------------------

neymanPearsonWindow1D.hh -- search for likelihood ratio maximum and
    determination of optimal cuts in 1-d based on likelihood ratio
    between signal and background.


Convenience API
---------------

DensityScan1D.hh -- utility class for discretizing 1-d densities.

DensityScanND.hh -- functor for filling multidimensional arrays with
    multivariate density scans. Calculates the density in the bin center.

discretizationErrorND.hh -- function for calculating the ISE due to
    discretization of multivariate densities.

DensityAveScanND.hh -- functor for filling multidimensional arrays with
    multivariate density scans. Integrates the density over the bin area.

Distribution1DFactory.hh -- creation of a number of 1-d distributions from
    a uniform interface.

scanDensityAsWeight.hh -- determines density support and scans a multivariate
    density in a manner suitable for subsequent construction of orthogonal
    polynomial systems.


Utilities
---------

buildInterpolatedHelpers.hh -- utilities for nonparametric interpolation
    of statistical distributions. Not for use by application code.

histoUtils.hh -- utilities related to special ways of filling histograms, etc.

mirrorWeight.hh -- helper function for scanning multivariate densities.
    Used by LocalPolyFilterND and KDEFilterND codes.

multinomialCovariance1D.hh -- helper function for building multinomial
    distribution covariance matrices.

NMCombinationSequencer.hh -- helper class for a certain type of integer
    permutations (distinct ways of choosing M out of N objects).

StatUtils.hh -- a few useful functions which did not fit naturally
    anywhere else.

SymbetaParams1D.hh -- collects the parameters of symmetric beta kernels.
                              
volumeDensityFromBinnedRadial.hh -- convert a density which was obtained
    from a histogram of radius values into the density per unit area
    (or per unit volume or hypervolume).

WeightedDistro1DPtr.hh -- associates a pointer to AbsDistribution1D with
    a weight. Not for use by application code.


I/O
---

Distribution1DReader.hh -- factory for deserializing 1-d distribution functions.

DistributionNDReader.hh -- factory for deserializing N-d distribution functions.

distributionReadError.hh -- this code throws an appropriate exception if
    input I/O operations fail for a distribution previously stored on disk.

DiscreteDistribution1DReader.hh -- factory for deserializing 1-d discrete
    distributions.

DistributionTransform1DReader.hh -- factory for deserializing 1-d transforms.

fillHistoFromText.hh    -- utility for filling histograms from text files
    similar utility for ntuples in declared in the AbsNtuple.hh header).

LocalPolyFilter1DReader.hh -- a reader factory for classes derived from
    LocalPolyFilter1D.

NtupleRecordTypes.hh    -- mechanisms for locating parts of the ArchivedNtuple
NtupleRecordTypesFwd.hh    in the archive. Not for use by application code.

NtupleReference.hh      -- special reference type for ArchivedNtuple.

StorableMultivariateFunctorReader.hh  -- factory for deserializing for
    storable multivariate functors.

UnfoldingFilterNDReader.hh -- reader factory for classes derived from
    AbsUnfoldingFilterND.
