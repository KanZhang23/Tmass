#ifndef JETPTTFDENSITY_HH_
#define JETPTTFDENSITY_HH_

#include "JetPtTF.hh"
#include "JetPtEff.hh"

// Adapter for using JetPtTF with "importanceSamplingDistro"
class JetPtTFDensity
{
public:
    inline explicit JetPtTFDensity(const JetPtTF& tf) : tf_(tf) {}

    inline double operator()(const double partonPt) const
        {return tf_.density(partonPt);}

private:
    JetPtTFDensity();

    const JetPtTF& tf_;
};

// Adapter for using a product of JetPtTF and JetPtEff
// with "importanceSamplingDistro"
class JetPtEffDensity
{
public:
    inline JetPtEffDensity(const JetPtTF& tf, const JetPtEff& eff)
        : tf_(tf), eff_(eff) {}

    inline double operator()(const double partonPt) const
        {return tf_.density(partonPt)*eff_(partonPt);}

private:
    JetPtEffDensity();

    const JetPtTF& tf_;
    const JetPtEff& eff_;
};

#endif // JETPTTFDENSITY_HH_
