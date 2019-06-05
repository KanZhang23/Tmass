#include <cmath>
#include <vector>
#include <cassert>
#include <sstream>
#include <algorithm>

#include "geners/CPP11_shared_ptr.hh"
#include "geners/CPP11_auto_ptr.hh"
#include "geners/stringArchiveIO.hh"
#include "geners/Reference.hh"

#include "npstat/nm/LinInterpolatedTableND.hh"

#include "npstat/stat/GridInterpolatedDistribution.hh"
#include "npstat/stat/volumeDensityFromBinnedRadial.hh"

#include "npstat/rng/convertToSphericalRandom.hh"
#include "npstat/rng/AbsRandomGenerator.hh"

#include "tfs_2015.h"
#include "topmass_utils.h"

#define DR_DIM_NUMBER 0
#define PTRATIO_DIM_NUMBER 1

typedef CPP11_shared_ptr<npstat::GridInterpolatedDistribution> TFPtr;
typedef CPP11_shared_ptr<npstat::LinInterpolatedTableND<float,npstat::UniformAxis> > EffPtr;

// Variables loaded from the TF file
static std::vector<TFPtr>  b_tfs_v;
static std::vector<TFPtr>  q_tfs_v;
static std::vector<TFPtr>  q_tfs_lomass;
static std::vector<EffPtr> b_effs_v;
static std::vector<EffPtr> q_effs_v;
static std::vector<EffPtr> q_effs_lomass;

static double drBinWidth_normal = 0.0;
static double drBinWidth_lomass = 0.0;

// Other static variables set by the "load_tfs_2015" function
static std::vector<double> etaBinLimits;
static double minPtExceedance = 0.0;
static double partonMassSplit = 0.0;
static double drStretchFactor = 0.0;
static double jetPtCutUsed = 0.0;

// A useful array of pointers which allows us to use either
// b_tfs_v or q_tfs_v depending on the value of the "isB" argument
static std::vector<TFPtr>* tfs_[2] = {0, 0};
static std::vector<EffPtr>* effs_[2] = {0, 0};

// Initialization flag
static bool initialized = false;


inline static npstat::GridInterpolatedDistribution&
get_tf_by_eta(const double jetEta, const int isB)
{
    const double abseta = fabs(jetEta);
    const unsigned len = etaBinLimits.size();
    unsigned ibin = 0;
    for (; abseta >= etaBinLimits[ibin] && ibin < len; ++ibin) {;}
    const unsigned bBin = isB ? 1U : 0U;
    return *((*tfs_[bBin])[ibin]);
}


inline static npstat::GridInterpolatedDistribution&
get_lomass_tf_by_eta(const double jetEta)
{
    const double abseta = fabs(jetEta);
    const unsigned len = etaBinLimits.size();
    unsigned ibin = 0;
    for (; abseta >= etaBinLimits[ibin] && ibin < len; ++ibin) {;}
    return *q_tfs_lomass[ibin];
}


inline static const
npstat::LinInterpolatedTableND<float,npstat::UniformAxis>&
get_eff_by_eta(const double eta, const int isB)
{
    const double abseta = fabs(eta);
    const unsigned len = etaBinLimits.size();
    unsigned ibin = 0;
    for (; abseta >= etaBinLimits[ibin] && ibin < len; ++ibin) {;}
    const unsigned bBin = isB ? 1U : 0U;
    return *((*effs_[bBin])[ibin]);
}


inline static const
npstat::LinInterpolatedTableND<float,npstat::UniformAxis>&
get_lomass_eff_by_eta(const double eta)
{
    const double abseta = fabs(eta);
    const unsigned len = etaBinLimits.size();
    unsigned ibin = 0;
    for (; abseta >= etaBinLimits[ibin] && ibin < len; ++ibin) {;}
    return *q_effs_lomass[ibin];
}


#ifdef __cplusplus
extern "C" {
#endif

int load_tfs_2015(const char* gssaFile, const double minExceedance,
                  const double* etaBinBoundaries, const unsigned nEtaBins,
                  const double pMassSplit, const double drStretch,
                  const double jetPtCut, const int interpolateParam)
{
    // We will normally disable TF interpolation between parameter values.
    // Otherwise the TF code simply becomes too slow.
    const bool disableParamInterpolation = !interpolateParam;

    assert(!initialized);

    assert(gssaFile);
    gs::StringArchive* ar = gs::readCompressedStringArchiveExt(gssaFile);
    if (!ar)
        return 1;
    CPP11_auto_ptr<gs::StringArchive> arHandler(ar);

    assert(minExceedance > 0.0);
    assert(minExceedance <= 1.0);
    minPtExceedance = minExceedance;

    assert(nEtaBins);
    if (nEtaBins > 1)
    {
        assert(etaBinBoundaries);

        etaBinLimits.reserve(nEtaBins-1);
        for (unsigned i=0; i<nEtaBins-1; ++i)
        {
            assert(etaBinBoundaries[i] > 0.0);
            etaBinLimits.push_back(etaBinBoundaries[i]);
        }
        std::sort(etaBinLimits.begin(), etaBinLimits.end());
    }

    // Load transfer functions
    tfs_[0] = &q_tfs_v;
    tfs_[1] = &b_tfs_v;

    for (unsigned isB=0; isB<2; ++isB)
    {
        tfs_[isB]->reserve(nEtaBins);
        for (unsigned etaBin=0; etaBin<nEtaBins; ++etaBin)
        {
            std::ostringstream os;
            os << "isB " << isB << " etaBin " << etaBin;
            const std::string& title = os.str();
            gs::Reference<npstat::GridInterpolatedDistribution> ref(
                *ar, title.c_str(), "Ttbar TFs");
            assert(ref.unique());
            TFPtr tf = ref.getShared(0);
            tf->useSingleCell(disableParamInterpolation);
            tfs_[isB]->push_back(tf);
        }
    }

    q_tfs_lomass.reserve(nEtaBins);
    for (unsigned etaBin=0; etaBin<nEtaBins; ++etaBin)
    {
        std::ostringstream os;
        os << "1GeV etaBin " << etaBin;
        const std::string& title = os.str();
        gs::Reference<npstat::GridInterpolatedDistribution> ref(
            *ar, title.c_str(), "Ttbar TFs");
        assert(ref.unique());
        TFPtr tf = ref.getShared(0);
        tf->useSingleCell(disableParamInterpolation);
        q_tfs_lomass.push_back(tf);
    }

    // Load efficiencies
    effs_[0] = &q_effs_v;
    effs_[1] = &b_effs_v;

    for (unsigned isB=0; isB<2; ++isB)
    {
        effs_[isB]->reserve(nEtaBins);
        for (unsigned etaBin=0; etaBin<nEtaBins; ++etaBin)
        {
            std::ostringstream os;
            os << "ttbar " << (isB ? 'b' : 'q') << " eff, eta bin " << etaBin;
            const std::string& title = os.str();
            gs::Reference<npstat::LinInterpolatedTableND<float,npstat::UniformAxis> > ref(
                *ar, title.c_str(), "");
            assert(ref.unique());
            effs_[isB]->push_back(ref.getShared(0));
        }
    }

    q_effs_lomass.reserve(nEtaBins);
    for (unsigned etaBin=0; etaBin<nEtaBins; ++etaBin)
    {
        std::ostringstream os;
        os << "Eta bin " << etaBin << " ttbar low mass eff";
        const std::string& title = os.str();
        gs::Reference<npstat::LinInterpolatedTableND<float,npstat::UniformAxis> > ref(
            *ar, title.c_str(), "");
        assert(ref.unique());
        q_effs_lomass.push_back(ref.getShared(0));
    }

    // Load some other parameters
    {    
        gs::Reference<double> ref(*ar, "drBinWidth", "Ttbar TFs");
        assert(ref.unique());
        ref.restore(0, &drBinWidth_normal);
        assert(drBinWidth_normal > 0.0);
    }

    {
        gs::Reference<double> ref(*ar, "drBinWidth_1GeV", "Ttbar TFs");
        assert(ref.unique());
        ref.restore(0, &drBinWidth_lomass);
        assert(drBinWidth_lomass > 0.0);
    }

    // Copy the parameters provided as the function arguments
    partonMassSplit = pMassSplit;
    assert(partonMassSplit > 0.0);

    drStretchFactor = drStretch;
    assert(drStretchFactor > 0.0);

    jetPtCutUsed = jetPtCut;
    assert(jetPtCutUsed > 0.0);

    initialized = true;
    return 0;
}


// Current assumptions about the arguments:
//
// nPredictors == 2
// pred[0] is the ID94 (pseudo-jet) mass
// pred[1] is the parton Pt
//
// nX == 2
// x[DR_DIM_NUMBER] is the dR between the parton and the jet
// x[PTRATIO_DIM_NUMBER] is the pt ratio times jes (jetPT*jes/partonPt)
// xcut = tf_pt_cutoff*jes_at_cut/partonPt
//
// In the above, tf_pt_cutoff is the jet energy cutoff for the observed jets.
// tf_pt_cutoff*jes_at_cut is then the corresponding MC pt cutoff. xcut is
// the corresponding cutoff on the Pt ratio taking JES into account.
//
// Note that jes and jes_at_cut are not equal. They correspond to the
// same value of deltaJES (since JES = 1 + deltaJES*sigmaJES), but sigmaJES
// depends on jet Pt and, therefore, is different for the observed jet and
// for the cutoff.
//
double bare_transfer_function(const double jetEta, const int isB,
                              const double* pred, const unsigned nPredictors,
                              const double* x, const unsigned nX,
                              const double xCut)
{
    assert(initialized);

    // Check that the inputs make sense
    assert(pred);
    assert(x);
    assert(x[PTRATIO_DIM_NUMBER] >= xCut);

    const double mParton = pred[0];
    if (!isB && mParton < partonMassSplit)
    {
        npstat::GridInterpolatedDistribution& tf(get_lomass_tf_by_eta(jetEta));
        tf.setGridCoords(pred+1, nPredictors-1);
        double exc = tf.marginalExceedance(PTRATIO_DIM_NUMBER, xCut);
        if (exc < minPtExceedance)
            exc = minPtExceedance;
        return tf.density(x, nX)/exc*npstat::volumeDensityFromBinnedRadial(
            2U, drBinWidth_lomass, x[DR_DIM_NUMBER]);
    }
    else
    {
        npstat::GridInterpolatedDistribution& tf(get_tf_by_eta(jetEta, isB));
        tf.setGridCoords(pred, nPredictors);
        double exc = tf.marginalExceedance(PTRATIO_DIM_NUMBER, xCut);
        if (exc < minPtExceedance)
            exc = minPtExceedance;
        return tf.density(x, nX)/exc*npstat::volumeDensityFromBinnedRadial(
            2U, drBinWidth_normal, x[DR_DIM_NUMBER]);
    }
}

double bare_ptratio_exceedance(const double jetEta, const int isB,
                               const double* pred, const unsigned nPredictors,
                               const double ptRat, const double xCut)
{
    assert(initialized);
    assert(pred);

    double exc = 1.0;
    if (ptRat > xCut)
    {
        const double mParton = pred[0];
        const bool lomass = !isB && mParton < partonMassSplit;
        npstat::GridInterpolatedDistribution& tf(lomass ? get_lomass_tf_by_eta(jetEta) :
                                                 get_tf_by_eta(jetEta, isB));
        if (lomass)
            tf.setGridCoords(pred+1, nPredictors-1);
        else
            tf.setGridCoords(pred, nPredictors);
        const double excCut = tf.marginalExceedance(PTRATIO_DIM_NUMBER, xCut);
        const double excRat = tf.marginalExceedance(PTRATIO_DIM_NUMBER, ptRat);
        exc = excRat/excCut;
        assert(exc <= 1.0);
    }
    return exc;
}

int bare_tf_random(const double jetEta, const int isB, const double xCut,
                   const double* pred, const unsigned nPredictors,
                   void* randomGenerator, const unsigned maxtries,
                   double* ptRatio, double* deltaEta, double* deltaPhi)
{
    assert(initialized);

    assert(pred);
    assert(ptRatio);
    assert(deltaEta);
    assert(deltaPhi);

    npstat::AbsRandomGenerator* npstatRng = 0;
    if (randomGenerator)
        npstatRng = (npstat::AbsRandomGenerator*)randomGenerator;

    const double mParton = pred[0];
    const bool lomass = !isB && mParton < partonMassSplit;
    npstat::GridInterpolatedDistribution& tf(lomass ? get_lomass_tf_by_eta(jetEta) :
                                             get_tf_by_eta(jetEta, isB));
    if (lomass)
        tf.setGridCoords(pred+1, nPredictors-1);
    else
        tf.setGridCoords(pred, nPredictors);

    *ptRatio = -1.0;
    *deltaEta = 1000.0;
    *deltaPhi = 0.0;

    double x[2], rnd[2];
    for (unsigned itry=0; itry<maxtries; ++itry)
    {
        if (npstatRng)
        {
            rnd[DR_DIM_NUMBER] = (*npstatRng)();
            rnd[PTRATIO_DIM_NUMBER] = (*npstatRng)();
        }
        else
        {
            rnd[DR_DIM_NUMBER] = uniform_random();
            rnd[PTRATIO_DIM_NUMBER] = uniform_random();
        }
        if (rnd[DR_DIM_NUMBER] < 1.e-8)
            rnd[DR_DIM_NUMBER] = 1.e-8;

        tf.unitMap(rnd, sizeof(x)/sizeof(x[0]), x);
        if (x[PTRATIO_DIM_NUMBER] >= xCut)
        {
            double randomAngle;
            if (npstatRng)
                randomAngle = (*npstatRng)();
            else
                randomAngle = uniform_random();
            randomAngle *= (2.*M_PI);

            *ptRatio = x[PTRATIO_DIM_NUMBER];
            *deltaEta = x[DR_DIM_NUMBER]*cos(randomAngle);
            *deltaPhi = x[DR_DIM_NUMBER]*sin(randomAngle);
            return 0;
        }
    }

    return 1;
}

v3_obj randomize_sampling_fromrand(const v3_obj orig, const int isB,
                                   const double mparton,
                                   const double rnd_eta, const double rnd_phi,
                                   double *density)
{
    assert(initialized);

    const double pt = Pt(orig);
    const double eta = Eta(orig);
    const double phi = Phi(orig);
    const bool lomass = !isB && mparton < partonMassSplit;

    npstat::GridInterpolatedDistribution& tf(lomass ? get_lomass_tf_by_eta(eta) :
                                             get_tf_by_eta(eta, isB));
    double predictors[2], drBinWidth;
    predictors[0] = mparton;
    predictors[1] = pt;
    if (lomass)
    {
        tf.setGridCoords(predictors+1, sizeof(predictors)/sizeof(predictors[0])-1);
        drBinWidth = drBinWidth_lomass;
    }
    else
    {
        tf.setGridCoords(predictors, sizeof(predictors)/sizeof(predictors[0]));
        drBinWidth = drBinWidth_normal;
    }

    double rnd[2], randomdir[2];
    rnd[0] = rnd_eta;
    rnd[1] = rnd_phi;
    double rnd_r = npstat::convertToSphericalRandom(rnd, 2, randomdir);

    // If the random number for r is exactly 0, we often have problems
    assert(rnd_r >= 0.0);
    if (rnd_r < 1.0e-8)
        rnd_r = 1.0e-8;
    const double dr = tf.marginalQuantile(DR_DIM_NUMBER, rnd_r);

    if (density)
    {
        *density = tf.marginalDensity(DR_DIM_NUMBER, dr)*
            npstat::volumeDensityFromBinnedRadial(2U, drBinWidth, dr)/
            (drStretchFactor*drStretchFactor);
        assert(*density > 0.0);
    }

    const double d_eta = dr*drStretchFactor*randomdir[0];
    const double d_phi = dr*drStretchFactor*randomdir[1];
    const double neweta = eta - d_eta;
    const double newphi = phi - d_phi;

    // Keep the same pt, not momentum
    // const double newpt  = mom(orig) / cosh(neweta);

    return pt_eta_phi(pt, neweta, newphi);
}

//
// Predictors for "normal" (high parton mass) efficiency:
//   
//  0 - Jet Pt Cut
//  1 - Parton Mass
//  2 - Parton Pt
//
// Predictors for "low mass" efficiency:
//
//  0 - Jet Pt Cut
//  1 - Parton Pt
//
// We multiply the detector (observed) jet by the jes to get the
// jet in the "reference" (essentially, MC) space. However,
// the selection cut is fixed in the space of observed jets.
// That means (reference cut) = (cut in observed space)*jes.
// Therefore, larger JES means smaller efficiency.
//
double single_parton_eff(const particle_obj parton, const int isB,
                         const double deltaJES, const double sigma_at_standard_cut)
{
    assert(initialized);
    assert(sigma_at_standard_cut > 0.0);

    const double pt = Pt(parton.p);
    const double eta = Eta(parton.p);
    const double jes = 1.0 + deltaJES*sigma_at_standard_cut;
    const double cut = jes*jetPtCutUsed;

    double eff;
    if (!isB && parton.m < partonMassSplit)
        eff = get_lomass_eff_by_eta(eta)(cut, pt);
    else
        eff = get_eff_by_eta(eta, isB)(cut, parton.m, pt);

    // Check that the efficiency makes sense. It can be very
    // slightly larger than 1 (due to round-offs) but not more
    // than that.
    assert(eff >= 0.0);
    assert(eff <= 1.01);
    if (eff > 1.0)
        eff = 1.0;

    return eff;
}

double get_jet_pt_cutoff(void)
{
    assert(initialized);
    return jetPtCutUsed;
}
    
double get_min_pt_exceedance(void)
{
    assert(initialized);
    return minPtExceedance;
}

#ifdef __cplusplus
}
#endif
