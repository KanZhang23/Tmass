#include <cmath>
#include <cfloat>
#include <cassert>
#include <algorithm>

#include "JetPtTF.hh"
#include "geners/GenericIO.hh"

#define WEIGHT_CUT 1.0e-10
#define EXCEEDANCE_CUT 1.0e-3

using namespace npstat;

JetPtTF::JetPtTF(const GridAxis& partonPtAxis,
                 const std::vector<CPP11_shared_ptr<const AbsDistribution1D> >& dv,
                 const double detectorJetPtCut,
                 const double sigmaAtPtCut)
    : partonPtAxis_(partonPtAxis),
      rho0_(dv),
      detectorJetPtCut_(detectorJetPtCut),
      sigmaAtPtCut_(sigmaAtPtCut),
      jetPt_(0.0),
      deltaJES_(0.0),
      jesAtCut_(1.0),
      jes_(1.0),
      scaleFactor_(1.0)
{
    assert(detectorJetPtCut_ > 0.0);
    assert(sigmaAtPtCut_ > 0.0);
    assert(partonPtAxis_.nCoords() == dv.size());
}

double JetPtTF::density(const double partPt) const
{
    if (jetPt_ < detectorJetPtCut_)
        return 0.0;

    if (partPt <= 0.0)
        return 0.0;

    const double x = jes_*jetPt_/partPt;
    const double xcut = jesAtCut_*detectorJetPtCut_/partPt;

    if (x <= xcut)
        return 0.0;

    // Density is linearly interpolated between two nearby parton Pt grid points
    const std::pair<unsigned,double>& intv = partonPtAxis_.getInterval(partPt);
    double dsum = 0.0;
    for (unsigned i=0U; i<2U; ++i)
    {
        const double weight = i ? 1.0 - intv.second : intv.second;
        if (weight > WEIGHT_CUT)
        {
            const AbsDistribution1D& distro(*rho0_[intv.first+i]);
            const double dens = distro.density(x);
            if (dens > 0.0)
            {
                double exc = distro.exceedance(xcut);
                if (exc < EXCEEDANCE_CUT)
                    exc = EXCEEDANCE_CUT;
                dsum += weight*dens/exc;
            }
        }
    }
    return dsum/partPt*scaleFactor_;
}

double JetPtTF::cdf(const double partPt) const
{
    if (jetPt_ <= detectorJetPtCut_)
        return 0.0;

    if (partPt <= 0.0)
        return 0.0;

    const double x = jes_*jetPt_/partPt;
    const double xcut = jesAtCut_*detectorJetPtCut_/partPt;

    if (x <= xcut)
        return 0.0;

    // Density is linearly interpolated between two nearby parton Pt grid points
    const std::pair<unsigned,double>& intv = partonPtAxis_.getInterval(partPt);
    double dsum = 0.0;
    for (unsigned i=0U; i<2U; ++i)
    {
        const double weight = i ? 1.0 - intv.second : intv.second;
        if (weight > WEIGHT_CUT)
        {
            const AbsDistribution1D& distro(*rho0_[intv.first+i]);
            const double cdfmin = distro.cdf(xcut);
            dsum += weight*(distro.cdf(x) - cdfmin)/(1.0 - cdfmin);
        }
    }
    return dsum;
}

double JetPtTF::sampleMCJetPt(const double partPt, const double rnd) const
{
    if (partPt <= 0.0)
        return 0.0;

    const double xcut = jesAtCut_*detectorJetPtCut_/partPt;

    const std::pair<unsigned,double>& intv = partonPtAxis_.getInterval(partPt);
    const double w0 = intv.second;
    const double w1 = 1.0 - w0;
    assert(w0 >= 0.0 && w0 <= 1.0);
    const bool closeToZero = w0 < WEIGHT_CUT;
    const bool closeToOne = w1 < WEIGHT_CUT;

    double xgen = 0.0;
    if (closeToZero || closeToOne)
    {
        unsigned ind = intv.first;
        if (closeToZero) ++ind;
        const AbsDistribution1D& distro(*rho0_[ind]);
        const double cdfmin = distro.cdf(xcut);
        xgen = distro.quantile(rnd*(1.0 - cdfmin) + cdfmin);
    }
    else
    {
        const AbsDistribution1D& d0(*rho0_[intv.first]);
        const double cdfmin0 = d0.cdf(xcut);
        const double exc0 = 1.0 - cdfmin0;

        const AbsDistribution1D& d1(*rho0_[intv.first+1U]);
        const double cdfmin1 = d1.cdf(xcut);
        const double exc1 = 1.0 - cdfmin1;

        double xgen0 = d0.quantile(rnd*exc0 + cdfmin0);
        double xgen1 = d1.quantile(rnd*exc1 + cdfmin1);
        double f0 = w0*(d0.cdf(xgen0) - cdfmin0)/exc0 +
                    w1*(d1.cdf(xgen0) - cdfmin1)/exc1 - rnd;
        double f1 = w0*(d0.cdf(xgen1) - cdfmin0)/exc0 +
                    w1*(d1.cdf(xgen1) - cdfmin1)/exc1 - rnd;
        if (f0 == 0.0)
            xgen = xgen0;
        else if (f1 == 0.0)
            xgen = xgen1;
        else
        {
            assert(f0*f1 < 0.0);
            for (unsigned i=0; i<1000U; ++i)
            {
                if (fabs(xgen0 - xgen1)/std::max(fabs(xgen0), fabs(xgen1)) <
                    8.0*DBL_EPSILON)
                    break;
                const double x = (xgen0 + xgen1)/2.0;
                const double fval = w0*(d0.cdf(x) - cdfmin0)/exc0 +
                                    w1*(d1.cdf(x) - cdfmin1)/exc1 - rnd;
                if (fval == 0.0)
                    break;
                if (fval*f0 > 0.0)
                {
                    xgen0 = x;
                    f0 = fval;
                }
                else
                {
                    xgen1 = x;
                    f1 = fval;
                }
            }
            xgen = (xgen0 + xgen1)/2.0;
        }
    }

    return xgen*partPt;
}

void JetPtTF::setJetInfo(const JetInfo& info)
{
    jetInfo_ = info;
    jetPt_ = jetInfo_.pt();
    updateJES();
}

void JetPtTF::setDeltaJES(const double value)
{
    deltaJES_ = value;
    jesAtCut_ = 1.0 + deltaJES_*sigmaAtPtCut_;
    if (jesAtCut_ < 0.0)
        jesAtCut_ = 0.0;
    updateJES();
}

void JetPtTF::updateJES()
{
    if (jetPt_ > 0.0)
    {
        jes_ = 1.0 + deltaJES_*jetInfo_.sysErr();
        assert(jes_ > 0.0);
        scaleFactor_ = jes_ + jetPt_*deltaJES_*jetInfo_.dErrDPt();
        if (scaleFactor_ < 0.0)
            scaleFactor_ = 0.0;
    }
    else
    {
        jes_ = 1.0;
        scaleFactor_ = 1.0;
    }
}

bool JetPtTF::operator==(const JetPtTF& r) const
{
    const unsigned sz = rho0_.size();
    if (sz != r.rho0_.size())
        return false;
    for (unsigned i=0; i<sz; ++i)
        if (*r.rho0_[i] != *rho0_[i])
            return false;
    return
    partonPtAxis_     == r.partonPtAxis_     &&
    detectorJetPtCut_ == r.detectorJetPtCut_ &&
    sigmaAtPtCut_     == r.sigmaAtPtCut_     &&
    jetInfo_          == r.jetInfo_          &&
    jetPt_            == r.jetPt_            &&   
    deltaJES_         == r.deltaJES_         &&        
    jesAtCut_         == r.jesAtCut_         &&      
    jes_              == r.jes_              &&      
    scaleFactor_      == r.scaleFactor_;
}

bool JetPtTF::write(std::ostream& of) const
{
    const bool status = gs::write_obj(of, partonPtAxis_) &&
                        gs::write_item(of, rho0_) &&
                        gs::write_item(of, jetInfo_);
    if (status)
    {
        gs::write_pod(of, detectorJetPtCut_);
        gs::write_pod(of, sigmaAtPtCut_);
        gs::write_pod(of, jetPt_);
        gs::write_pod(of, deltaJES_);
        gs::write_pod(of, jesAtCut_);
        gs::write_pod(of, jes_);
        gs::write_pod(of, scaleFactor_);
    }
    return status && !of.fail();
}

JetPtTF* JetPtTF::read(const gs::ClassId& id, std::istream& in)
{
    static const gs::ClassId myClassId(gs::ClassId::makeId<JetPtTF>());
    myClassId.ensureSameId(id);

    CPP11_auto_ptr<GridAxis> paxis = gs::read_obj<GridAxis>(in);
    std::vector<CPP11_shared_ptr<const npstat::AbsDistribution1D> > rho;
    gs::restore_item(in, &rho);
    JetInfo jetInf;
    gs::restore_item(in, &jetInf);
    double detectorJetPtCut, sigmaAtPtCut;
    gs::read_pod(in, &detectorJetPtCut);
    gs::read_pod(in, &sigmaAtPtCut);
    if (in.fail()) throw gs::IOReadFailure("In JetPtTF::read: "
                                           "input stream failure");
    CPP11_auto_ptr<JetPtTF> ptf(new JetPtTF(
        *paxis, rho, detectorJetPtCut, sigmaAtPtCut));
    gs::read_pod(in, &ptf->jetPt_);
    gs::read_pod(in, &ptf->deltaJES_);
    gs::read_pod(in, &ptf->jesAtCut_);
    gs::read_pod(in, &ptf->jes_);
    gs::read_pod(in, &ptf->scaleFactor_);
    if (in.fail()) throw gs::IOReadFailure("In JetPtTF::read: "
                                           "input stream failure");
    ptf->jetInfo_ = jetInf;
    return ptf.release();
}
