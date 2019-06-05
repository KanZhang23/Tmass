AM_CPPFLAGS = -I@top_srcdir@/ $(DEPS_CFLAGS)

noinst_LTLIBRARIES = libstat.la

libstat_la_SOURCES = AbsDistribution1D.cc AbsUnfoldND.cc \
    AbsDistributionND.cc amiseOptimalBandwidth.cc CompositeDistribution1D.cc \
    CompositeDistributionND.cc CopulaInterpolationND.cc SymbetaParams1D.cc \
    Distribution1DFactory.cc Distribution1DReader.cc DistributionNDReader.cc \
    Distributions1D.cc DistributionsND.cc CrossCovarianceAccumulator.cc \
    fitSbParameters.cc StatAccumulatorArr.cc HistoAxis.cc ResponseMatrix.cc \
    InterpolatedDistribution1D.cc JohnsonCurves.cc JohnsonKDESmoother.cc \
    LocalPolyFilter1D.cc logLikelihoodPeak.cc PolyFilterCollection1D.cc \
    SbMomentsBigGamma.cc SbMomentsCalculator.cc gaussianResponseMatrix.cc \
    SequentialCopulaSmoother.cc SequentialPolyFilterND.cc StatAccumulator.cc \
    UnitMapInterpolationND.cc WeightedStatAccumulator.cc AbsNtuple.cc \
    QuadraticOrthoPolyND.cc NMCombinationSequencer.cc Filter1DBuilders.cc \
    StatAccumulatorPair.cc GridRandomizer.cc ConstantBandwidthSmoother1D.cc \
    GaussianMixture1D.cc HistoNDCdf.cc scanDensityAsWeight.cc NUHistoAxis.cc \
    distributionReadError.cc WeightedStatAccumulatorPair.cc AbsUnfold1D.cc \
    ProductSymmetricBetaNDCdf.cc DualHistoAxis.cc multinomialCovariance1D.cc \
    StorableMultivariateFunctor.cc StorableMultivariateFunctorReader.cc \
    TruncatedDistribution1D.cc neymanPearsonWindow1D.cc AsinhTransform1D.cc \
    LOrPEMarginalSmoother.cc LeftCensoredDistribution.cc QuantileTable1D.cc \
    RightCensoredDistribution.cc AbsDiscreteDistribution1D.cc convertAxis.cc \
    DiscreteDistribution1DReader.cc DiscreteDistributions1D.cc lorpeMise1D.cc \
    BernsteinFilter1DBuilder.cc BetaFilter1DBuilder.cc AbsFilter1DBuilder.cc \
    continuousDegreeTaper.cc RatioOfNormals.cc AbsCVCopulaSmoother.cc \
    DensityScan1D.cc BoundaryHandling.cc SmoothedEMUnfold1D.cc Copulas.cc \
    PearsonsChiSquared.cc BinnedKSTest1D.cc MultiscaleEMUnfold1D.cc \
    AbsBinnedComparison1D.cc BinnedADTest1D.cc LocalPolyFilter1DReader.cc \
    MemoizingSymbetaFilterProvider.cc UGaussConvolution1D.cc BinSummary.cc \
    SmoothedEMUnfoldND.cc UnfoldingFilterNDReader.cc AbsUnfoldingFilterND.cc \
    UnfoldingBandwidthScannerND.cc DistributionMix1D.cc ScalableGaussND.cc \
    InterpolatedDistro1D1P.cc AbsDistributionTransform1D.cc LogTransform1D.cc \
    DistributionTransform1DReader.cc TransformedDistribution1D.cc \
    WeightTableFilter1DBuilder.cc VerticallyInterpolatedDistribution1D.cc \
    LocalMultiFilter1D.cc LogRatioTransform1D.cc IdentityTransform1D.cc \
    VariableBandwidthSmoother1D.cc AbsMarginalSmootherBase.cc OSDE1D.cc \
    buildInterpolatedHelpers.cc GridInterpolatedDistribution.cc \
    AbsCopulaSmootherBase.cc BernsteinCopulaSmoother.cc distro1DStats.cc \
    SequentialGroupedCopulaSmoother.cc UnfoldingBandwidthScanner1D.cc \
    InterpolatedDistro1DNP.cc volumeDensityFromBinnedRadial.cc \
    statUncertainties.cc LocationScaleFamily1D.cc SinhAsinhTransform1D.cc \
    AbsKDE1DKernel.cc KDE1DHOSymbetaKernel.cc

includedir = ${prefix}/include/npstat/stat

include_HEADERS = AbsBandwidthCV.hh \
    AbsBandwidthGCV.hh \
    AbsBinnedComparison1D.hh \
    AbsBinnedComparison1D.icc \
    AbsCompositeDistroBuilder.hh \
    AbsCompositeDistroBuilder.icc \
    AbsCopulaSmootherBase.hh \
    AbsCopulaSmootherBase.icc \
    AbsCVCopulaSmoother.hh \
    AbsDiscreteDistribution1D.hh \
    AbsDistribution1D.hh \
    AbsDistributionND.hh \
    AbsDistributionND.icc \
    AbsDistributionTransform1D.hh \
    AbsDistro1DBuilder.hh \
    AbsDistro1DBuilder.icc \
    AbsFilter1DBuilder.hh \
    AbsGridInterpolatedDistribution.hh \
    AbsInterpolatedDistribution1D.hh \
    AbsInterpolationAlgoND.hh \
    AbsKDE1DKernel.hh \
    AbsKDE1DKernel.icc \
    AbsLossCalculator.hh \
    AbsMarginalSmootherBase.hh \
    AbsMarginalSmootherBase.icc \
    AbsNtuple.hh \
    AbsNtuple.icc \
    AbsPolyFilter1D.hh \
    AbsPolyFilterND.hh \
    AbsResponseBoxBuilder.hh \
    AbsResponseIntervalBuilder.hh \
    AbsSymbetaFilterProvider.hh \
    AbsUnfold1D.hh \
    AbsUnfoldingFilterND.hh \
    AbsUnfoldND.hh \
    amiseOptimalBandwidth.hh \
    amiseOptimalBandwidth.icc \
    ArchivedNtuple.hh \
    ArchivedNtuple.icc \
    ArrayProjectors.hh \
    ArrayProjectors.icc \
    arrayStats.hh \
    arrayStats.icc \
    AsinhTransform1D.hh \
    BandwidthCVLeastSquares1D.hh \
    BandwidthCVLeastSquares1D.icc \
    BandwidthCVLeastSquaresND.hh \
    BandwidthCVLeastSquaresND.icc \
    BandwidthCVPseudoLogli1D.hh \
    BandwidthCVPseudoLogli1D.icc \
    BandwidthCVPseudoLogliND.hh \
    BandwidthCVPseudoLogliND.icc \
    BandwidthGCVLeastSquares1D.hh \
    BandwidthGCVLeastSquares1D.icc \
    BandwidthGCVLeastSquaresND.hh \
    BandwidthGCVLeastSquaresND.icc \
    BandwidthGCVPseudoLogli1D.hh \
    BandwidthGCVPseudoLogli1D.icc \
    BandwidthGCVPseudoLogliND.hh \
    BandwidthGCVPseudoLogliND.icc \
    BernsteinCopulaSmoother.hh \
    BernsteinFilter1DBuilder.hh \
    betaKernelsBandwidth.hh \
    betaKernelsBandwidth.icc \
    BetaFilter1DBuilder.hh \
    BinnedADTest1D.hh \
    BinnedKSTest1D.hh \
    BinSummary.hh \
    BinSummary.icc \
    BoundaryHandling.hh \
    BoundaryMethod.hh \
    buildInterpolatedCompositeDistroND.hh \
    buildInterpolatedCompositeDistroND.icc \
    buildInterpolatedDistro1DNP.hh \
    buildInterpolatedDistro1DNP.icc \
    buildInterpolatedHelpers.hh \
    CensoredQuantileRegression.hh \
    CensoredQuantileRegression.icc \
    CircularBuffer.hh \
    CircularBuffer.icc \
    Column.hh \
    Column.icc \
    CompositeDistribution1D.hh \
    CompositeDistributionND.hh \
    CompositeDistributionND.icc \
    CompositeDistros1D.hh \
    ConstantBandwidthSmoother1D.hh \
    ConstantBandwidthSmootherND.hh \
    ConstantBandwidthSmootherND.icc \
    continuousDegreeTaper.hh \
    convertAxis.hh \
    CopulaInterpolationND.hh \
    Copulas.hh \
    CrossCovarianceAccumulator.hh \
    CrossCovarianceAccumulator.icc \
    CVCopulaSmoother.hh \
    CVCopulaSmoother.icc \
    DensityAveScanND.hh \
    DensityScan1D.hh \
    DensityScanND.hh \
    DiscreteDistribution1DReader.hh \
    DiscreteDistributions1D.hh \
    discretizationErrorND.hh \
    Distribution1DFactory.hh \
    Distribution1DReader.hh \
    DistributionTransform1DReader.hh \
    DistributionMix1D.hh \
    DistributionNDReader.hh \
    Distributions1D.hh \
    Distributions1D.icc \
    DistributionsND.hh \
    DistributionsND.icc \
    distributionReadError.hh \
    distro1DStats.hh \
    DualHistoAxis.hh \
    DummyCompositeDistroBuilder.hh \
    DummyDistro1DBuilder.hh \
    DummyResponseBoxBuilder.hh \
    DummyResponseIntervalBuilder.hh \
    empiricalCopula.hh \
    empiricalCopulaHisto.hh \
    empiricalCopulaHisto.icc \
    empiricalCopula.icc \
    fillHistoFromText.hh \
    fillHistoFromText.icc \
    Filter1DBuilders.hh \
    FitUtils.hh \
    FitUtils.icc \
    GaussianMixture1D.hh \
    gaussianResponseMatrix.hh \
    GCVCopulaSmoother.hh \
    GCVCopulaSmoother.icc \
    griddedRobustRegression.hh \
    griddedRobustRegression.icc \
    GriddedRobustRegressionStop.hh \
    GridInterpolatedDistribution.hh \
    GridRandomizer.hh \
    HistoAxis.hh \
    HistoND.hh \
    HistoND.icc \
    HistoNDCdf.hh \
    HistoNDFunctorInstances.hh \
    histoStats.hh \
    histoStats.icc \
    histoUtils.hh \
    histoUtils.icc \
    IdentityTransform1D.hh \
    InMemoryNtuple.hh \
    InMemoryNtuple.icc \
    InterpolatedDistribution1D.hh \
    InterpolatedDistro1D1P.hh \
    InterpolatedDistro1DNP.hh \
    interpolateHistoND.hh \
    interpolateHistoND.icc \
    InterpolationFunctorInstances.hh \
    JohnsonCurves.hh \
    JohnsonKDESmoother.hh \
    KDE1D.hh \
    KDE1DCV.hh \
    KDE1DHOSymbetaKernel.hh \
    KDECopulaSmoother.hh \
    KDECopulaSmoother.icc \
    KDEGroupedCopulaSmoother.hh \
    KDEGroupedCopulaSmoother.icc \
    KDEFilterND.hh \
    KDEFilterND.icc \
    kendallsTau.hh \
    kendallsTau.icc \
    LeftCensoredDistribution.hh \
    LocalLogisticRegression.hh \
    LocalLogisticRegression.icc \
    LocalMultiFilter1D.hh \
    LocalMultiFilter1D.icc \
    LocalPolyFilter1D.hh \
    LocalPolyFilter1D.icc \
    LocalPolyFilter1DReader.hh \
    LocalPolyFilterND.hh \
    LocalPolyFilterND.icc \
    LocalQuadraticLeastSquaresND.hh \
    LocalQuadraticLeastSquaresND.icc \
    LocalQuantileRegression.hh \
    LocalQuantileRegression.icc \
    LocationScaleFamily1D.hh \
    LocationScaleTransform1.hh \
    LocationScaleTransform1.icc \
    logLikelihoodPeak.hh \
    LogRatioTransform1D.hh \
    LogTransform1D.hh \
    LOrPECopulaSmoother.hh \
    LOrPECopulaSmoother.icc \
    LOrPEGroupedCopulaSmoother.hh \
    LOrPEGroupedCopulaSmoother.icc \
    LOrPEMarginalSmoother.hh \
    lorpeBackgroundCVDensity1D.hh \
    lorpeBackgroundCVDensity1D.icc \
    lorpeBackground1D.hh \
    lorpeBackground1D.icc \
    lorpeMise1D.hh \
    lorpeSmooth1D.hh \
    lorpeSmooth1D.icc \
    MemoizingSymbetaFilterProvider.hh \
    MemoizingSymbetaFilterProvider.icc \
    mergeTwoHistos.hh \
    mergeTwoHistos.icc \
    mirrorWeight.hh \
    MultiscaleEMUnfold1D.hh \
    multinomialCovariance1D.hh \
    MultivariateSumAccumulator.hh \
    MultivariateSumsqAccumulator.hh \
    MultivariateSumsqAccumulator.icc \
    MultivariateWeightedSumAccumulator.hh \
    MultivariateWeightedSumsqAccumulator.hh \
    MultivariateWeightedSumsqAccumulator.icc \
    neymanPearsonWindow1D.hh \
    NMCombinationSequencer.hh \
    NonparametricCompositeBuilder.hh \
    NonparametricCompositeBuilder.icc \
    NonparametricDistro1DBuilder.hh \
    NonparametricDistro1DBuilder.icc \
    NtHistoFill.hh \
    NtNtupleFill.hh \
    NtRectangularCut.hh \
    NtRectangularCut.icc \
    NtupleBuffer.hh \
    NtupleBuffer.icc \
    NtupleRecordTypes.hh \
    NtupleRecordTypesFwd.hh \
    NtupleReference.hh \
    NUHistoAxis.hh \
    OrderedPointND.hh \
    OrderedPointND.icc \
    orthoPoly1DVProducts.hh \
    orthoPoly1DVProducts.icc \
    OSDE1D.hh \
    OSDE1D.icc \
    PearsonsChiSquared.hh \
    PolyFilterCollection1D.hh \
    productResponseMatrix.hh \
    productResponseMatrix.icc \
    ProductSymmetricBetaNDCdf.hh \
    QuadraticOrthoPolyND.hh \
    QuadraticOrthoPolyND.icc \
    QuantileRegression1D.hh \
    QuantileRegression1D.icc \
    QuantileTable1D.hh \
    RatioOfNormals.hh \
    RatioResponseBoxBuilder.hh \
    RatioResponseBoxBuilder.icc \
    RatioResponseIntervalBuilder.hh \
    RatioResponseIntervalBuilder.icc \
    ResponseMatrix.hh \
    RightCensoredDistribution.hh \
    SampleAccumulator.hh \
    SampleAccumulator.icc \
    SbMomentsCalculator.hh \
    ScalableGaussND.hh \
    scanDensityAsWeight.hh \
    SequentialCopulaSmoother.hh \
    SequentialGroupedCopulaSmoother.hh \
    SequentialPolyFilterND.hh \
    SequentialPolyFilterND.icc \
    SinhAsinhTransform1D.hh \
    SmoothedEMUnfold1D.hh \
    SmoothedEMUnfoldND.hh \
    spearmansRho.hh \
    spearmansRho.icc \
    StatAccumulator.hh \
    StatAccumulatorArr.hh \
    StatAccumulatorPair.hh \
    statUncertainties.hh \
    StatUtils.hh \
    StatUtils.icc \
    StorableHistoNDFunctor.hh \
    StorableHistoNDFunctor.icc \
    StorableInterpolationFunctor.hh \
    StorableInterpolationFunctor.icc \
    StorableMultivariateFunctor.hh \
    StorableMultivariateFunctorReader.hh \
    SymbetaParams1D.hh \
    TransformedDistribution1D.hh \
    TruncatedDistribution1D.hh \
    TwoPointsLTSLoss.hh \
    TwoPointsLTSLoss.icc \
    UGaussConvolution1D.hh \
    UnfoldingBandwidthScanner1D.hh \
    UnfoldingBandwidthScannerND.hh \
    UnfoldingFilterNDReader.hh \
    UnitMapInterpolationND.hh \
    variableBandwidthSmooth1D.hh \
    variableBandwidthSmooth1D.icc \
    VariableBandwidthSmoother1D.hh \
    VerticallyInterpolatedDistribution1D.hh \
    volumeDensityFromBinnedRadial.hh \
    weightedCopulaHisto.hh \
    weightedCopulaHisto.icc \
    WeightedDistro1DPtr.hh \
    WeightedLTSLoss.hh \
    WeightedLTSLoss.icc \
    WeightedSampleAccumulator.hh \
    WeightedSampleAccumulator.icc \
    WeightedStatAccumulator.hh \
    WeightedStatAccumulatorPair.hh \
    WeightTableFilter1DBuilder.hh

EXTRA_DIST = 00README.txt npstat_doxy.hh
