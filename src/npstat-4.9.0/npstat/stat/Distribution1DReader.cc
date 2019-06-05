#include "npstat/stat/Distribution1DReader.hh"

#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/JohnsonCurves.hh"
#include "npstat/stat/CompositeDistribution1D.hh"
#include "npstat/stat/GaussianMixture1D.hh"
#include "npstat/stat/DistributionMix1D.hh"
#include "npstat/stat/TruncatedDistribution1D.hh"
#include "npstat/stat/QuantileTable1D.hh"
#include "npstat/stat/LeftCensoredDistribution.hh"
#include "npstat/stat/RightCensoredDistribution.hh"
#include "npstat/stat/RatioOfNormals.hh"
#include "npstat/stat/UGaussConvolution1D.hh"
#include "npstat/stat/InterpolatedDistro1D1P.hh"
#include "npstat/stat/InterpolatedDistro1DNP.hh"
#include "npstat/stat/TransformedDistribution1D.hh"
#include "npstat/stat/LocationScaleFamily1D.hh"

#define add_reader(Derived) do {\
     const gs::ClassId& id(gs::ClassId::makeId<Derived >());\
     (*this)[id.name()] = new gs::ConcreteReader<AbsDistribution1D,Derived >();\
} while(0);

namespace npstat {
    Distribution1DReader::Distribution1DReader()
    {
        add_reader(BinnedDensity1D);
        add_reader(JohnsonSu);
        add_reader(JohnsonSb);
        add_reader(JohnsonSystem);
        add_reader(Gauss1D);
        add_reader(BifurcatedGauss1D);
        add_reader(LogNormal);
        add_reader(Cauchy1D);
        add_reader(Uniform1D);
        add_reader(Quadratic1D);
        add_reader(LogQuadratic1D);
        add_reader(TruncatedGauss1D);
        add_reader(SymmetricBeta1D);
        add_reader(Beta1D);
        add_reader(Gamma1D);
        add_reader(Pareto1D);
        add_reader(Huber1D);
        add_reader(StudentsT1D);
        add_reader(Tabulated1D);
        add_reader(CompositeDistribution1D);
        add_reader(GaussianMixture1D);
        add_reader(Exponential1D);
        add_reader(TruncatedDistribution1D);
        add_reader(QuantileTable1D);
        add_reader(LeftCensoredDistribution);
        add_reader(RightCensoredDistribution);
        add_reader(RatioOfNormals);
        add_reader(DistributionMix1D);
        add_reader(UGaussConvolution1D);
        add_reader(Moyal1D);
        add_reader(InterpolatedDistro1D1P);
        add_reader(InterpolatedDistro1DNP);
        add_reader(MirroredGauss1D);
        add_reader(TransformedDistribution1D);
        add_reader(LocationScaleFamily1D);
        add_reader(IsoscelesTriangle1D);
        add_reader(Logistic1D);
    }
}
