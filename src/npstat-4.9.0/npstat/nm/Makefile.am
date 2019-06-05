AM_CPPFLAGS = -I@top_srcdir@/ $(DEPS_CFLAGS)

noinst_LTLIBRARIES = libnm.la

libnm_la_SOURCES = ArrayShape.cc ArrayRange.cc SpecialFunctions.cc \
    ConvolutionEngine1D.cc EquidistantSequence.cc GaussHermiteQuadrature.cc \
    GaussLegendreQuadrature.cc MathUtils.cc OrthoPoly1D.cc GridAxis.cc \
    rectangleQuadrature.cc LinInterpolatedTable1D.cc findPeak2D.cc \
    bilinearSection.cc ConvolutionEngineND.cc FourierImage.cc SvdMethod.cc \
    binomialCoefficient.cc UniformAxis.cc ArrayNDScanner.cc DualAxis.cc \
    DiscreteBernsteinPoly1D.cc definiteIntegrals.cc EigenMethod.cc \
    goldenSectionSearch.cc timestamp.cc OrthoPolyMethod.cc ContOrthoPoly1D.cc \
    lapack_interface.cc AbsClassicalOrthoPoly1D.cc ClassicalOrthoPolys1D.cc \
    matrixIndexPairs.cc truncatedInverseSqrt.cc FejerQuadrature.cc \
    StorablePolySeries1D.cc

includedir = ${prefix}/include/npstat/nm

include_HEADERS = AbsArrayProjector.hh \
    AbsClassicalOrthoPoly1D.hh \
    AbsClassicalOrthoPoly1D.icc \
    absDifference.hh \
    AbsMultivariateFunctor.hh \
    AbsVisitor.hh \
    allocators.hh \
    areAllElementsUnique.hh \
    ArrayND.hh \
    ArrayND.icc \
    ArrayNDScanner.hh \
    ArrayRange.hh \
    ArrayShape.hh \
    bilinearSection.hh \
    binomialCoefficient.hh \
    BoxND.hh \
    BoxND.icc \
    BoxNDScanner.hh \
    BoxNDScanner.icc \
    CircularMapper1d.hh \
    ClassicalOrthoPolys1D.hh \
    closeWithinTolerance.hh \
    CompareByIndex.hh \
    ComplexComparesAbs.hh \
    ComplexComparesFalse.hh \
    ConstSubscriptMap.hh \
    ContOrthoPoly1D.hh \
    ContOrthoPoly1D.icc \
    ConvolutionEngine1D.hh \
    ConvolutionEngine1D.icc \
    ConvolutionEngineND.hh \
    ConvolutionEngineND.icc \
    CoordinateSelector.hh \
    definiteIntegrals.hh \
    DiscreteBernsteinPoly1D.hh \
    discretizedDistance.hh \
    discretizedDistance.icc \
    DualAxis.hh \
    EigenMethod.hh \
    EquidistantSequence.hh \
    ExpMapper1d.hh \
    FejerQuadrature.hh \
    FejerQuadrature.icc \
    fillArrayCentersPreservingAreas.hh \
    findPeak2D.hh \
    findRootInLogSpace.hh \
    findRootInLogSpace.icc \
    findRootNewtonRaphson.hh \
    findRootNewtonRaphson.icc \
    findRootUsingBisections.hh \
    findRootUsingBisections.icc \
    FourierImage.hh \
    GaussHermiteQuadrature.hh \
    GaussHermiteQuadrature.icc \
    GaussLegendreQuadrature.hh \
    GaussLegendreQuadrature.icc \
    GeneralizedComplex.hh \
    goldenSectionSearch.hh \
    goldenSectionSearch.icc \
    GridAxis.hh \
    interpolate.hh \
    interpolate.icc \
    Interval.hh \
    Interval.icc \
    isMonotonous.hh \
    KDTree.hh \
    KDTree.icc \
    lapack.h \
    lapack_interface.hh \
    lapack_interface.icc \
    lapack_interface_double.icc \
    lapack_interface_float.icc \
    LinearMapper1d.hh \
    LinInterpolatedTable1D.hh \
    LinInterpolatedTable1D.icc \
    LinInterpolatedTableND.hh \
    LinInterpolatedTableND.icc \
    LogMapper1d.hh \
    MathUtils.hh \
    MathUtils.icc \
    Matrix.hh \
    matrixIndexPairs.hh \
    Matrix.icc \
    MinSearchStatus1D.hh \
    MultivariateFunctorScanner.hh \
    OrthoPoly1D.hh \
    OrthoPoly1D.icc \
    OrthoPolyMethod.hh \
    OrthoPolyND.hh \
    OrthoPolyND.icc \
    PairCompare.hh \
    PointDimensionality.hh \
    PreciseType.hh \
    ProperDblFromCmpl.hh \
    PtrBufferHandle.hh \
    rectangleQuadrature.hh \
    rescanArray.hh \
    rescanArray.icc \
    SimpleFunctors.hh \
    SpecialFunctions.hh \
    StorablePolySeries1D.hh \
    SvdMethod.hh \
    timestamp.hh \
    Triple.hh \
    truncatedInverseSqrt.hh \
    UniformAxis.hh

EXTRA_DIST = 00README.txt
