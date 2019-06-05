#include "npstat/stat/Distribution1DFactory.hh"
#include "npstat/stat/Distributions1D.hh"
#include "npstat/stat/JohnsonCurves.hh"
#include "npstat/stat/GaussianMixture1D.hh"
#include "npstat/stat/QuantileTable1D.hh"
#include "npstat/stat/UGaussConvolution1D.hh"

namespace npstat {
    DefaultScalableDistribution1DFactory::
    DefaultScalableDistribution1DFactory()
    {
        (*this)["Uniform1D"] = new ScalableDistribution1DFactory<Uniform1D>();
        (*this)["Gauss1D"] = new ScalableDistribution1DFactory<Gauss1D>();
        (*this)["BifurcatedGauss1D"] =
            new ScalableDistribution1DFactory<BifurcatedGauss1D>();
        (*this)["TruncatedGauss1D"] =
            new ScalableDistribution1DFactory<TruncatedGauss1D>();
        (*this)["SymmetricBeta1D"] =
            new ScalableDistribution1DFactory<SymmetricBeta1D>();
        (*this)["Beta1D"] =
            new ScalableDistribution1DFactory<Beta1D>();
        (*this)["Gamma1D"] =
            new ScalableDistribution1DFactory<Gamma1D>();
        (*this)["Pareto1D"] = new ScalableDistribution1DFactory<Pareto1D>();
        (*this)["UniPareto1D"] = new ScalableDistribution1DFactory<UniPareto1D>();
        (*this)["Huber1D"] = new ScalableDistribution1DFactory<Huber1D>();
        (*this)["Cauchy1D"] = new ScalableDistribution1DFactory<Cauchy1D>();
        (*this)["LogNormal"] = new ScalableDistribution1DFactory<LogNormal>();
        (*this)["BinnedDensity1D"] =
            new ScalableDistribution1DFactory<BinnedDensity1D>();
        (*this)["Tabulated1D"] =
            new ScalableDistribution1DFactory<Tabulated1D>();
        (*this)["JohnsonSu"] = new ScalableDistribution1DFactory<JohnsonSu>();
        (*this)["JohnsonSb"] = new ScalableDistribution1DFactory<JohnsonSb>();
        (*this)["JohnsonSystem"] =
            new ScalableDistribution1DFactory<JohnsonSystem>();
        (*this)["Quadratic1D"] =
            new ScalableDistribution1DFactory<Quadratic1D>();
        (*this)["LogQuadratic1D"] =
            new ScalableDistribution1DFactory<LogQuadratic1D>();
        (*this)["StudentsT1D"] =
            new ScalableDistribution1DFactory<StudentsT1D>();
        (*this)["GaussianMixture1D"] =
            new ScalableDistribution1DFactory<GaussianMixture1D>();
        (*this)["Exponential1D"] =
            new ScalableDistribution1DFactory<Exponential1D>();
        (*this)["QuantileTable1D"] =
            new ScalableDistribution1DFactory<QuantileTable1D>();
        (*this)["UGaussConvolution1D"] =
            new ScalableDistribution1DFactory<UGaussConvolution1D>();
        (*this)["Moyal1D"] = new ScalableDistribution1DFactory<Moyal1D>();
        (*this)["MirroredGauss1D"] =
            new ScalableDistribution1DFactory<MirroredGauss1D>();
        (*this)["IsoscelesTriangle1D"] =
            new ScalableDistribution1DFactory<IsoscelesTriangle1D>();
        (*this)["Logistic1D"] = new ScalableDistribution1DFactory<Logistic1D>();
    }

    DefaultScalableDistribution1DFactory::
    ~DefaultScalableDistribution1DFactory()
    {
        for (std::map<std::string, AbsScalableDistribution1DFactory*>::
                 iterator it = begin(); it != end(); ++it)
            delete it->second;
    }
}
