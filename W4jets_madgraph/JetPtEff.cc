#include <cassert>

#include "JetPtEff.hh"

JetPtEff::JetPtEff(CPP11_shared_ptr<const npstat::LinInterpolatedTableND<float> > t,
                   const double detectorPtCut, const double sigmaAtPtCut)
    : table_(t),
      detectorJetPtCut_(detectorPtCut),
      sigmaAtPtCut_(sigmaAtPtCut),
      deltaJES_(0.0),
      jesAtCut_(1.0)
{
    assert(table_->dim() == 2);
    assert(detectorJetPtCut_ > 0.0);
    assert(sigmaAtPtCut_ > 0.0);
}

void JetPtEff::setDeltaJES(const double value)
{
    deltaJES_ = value;
    jesAtCut_ = 1.0 + deltaJES_*sigmaAtPtCut_;
    if (jesAtCut_ < 0.0)
        jesAtCut_ = 0.0;
}

double JetPtEff::operator()(const double partonPt) const
{
    const double mcPtCut = jesAtCut_*detectorJetPtCut_;
    double eff = (*table_)(mcPtCut, partonPt);
    if (eff < 0.0)
        eff = 0.0;
    else if (eff > 1.0)
        eff = 1.0;
    return eff;
}

bool JetPtEff::operator==(const JetPtEff& r) const
{
    return *table_ == *r.table_ &&
        detectorJetPtCut_ == r.detectorJetPtCut_ &&
        sigmaAtPtCut_ == r.sigmaAtPtCut_ &&
        deltaJES_ == r.deltaJES_ &&
        jesAtCut_ == r.jesAtCut_;
}

bool JetPtEff::write(std::ostream& of) const
{
    const bool status = gs::write_item(of, *table_);
    if (status)
    {
        gs::write_pod(of, detectorJetPtCut_);
        gs::write_pod(of, sigmaAtPtCut_);
        gs::write_pod(of, deltaJES_);
        gs::write_pod(of, jesAtCut_);
    }
    return status && !of.fail();
}

JetPtEff* JetPtEff::read(const gs::ClassId& id, std::istream& in)
{
    static const gs::ClassId myClassId(gs::ClassId::makeId<JetPtEff>());
    myClassId.ensureSameId(id);

    CPP11_auto_ptr<npstat::LinInterpolatedTableND<float> > ptab = 
        gs::read_item<npstat::LinInterpolatedTableND<float> >(in);
    double detectorJetPtCut, sigmaAtPtCut, deltaJES, jesAtCut;
    gs::read_pod(in, &detectorJetPtCut);
    gs::read_pod(in, &sigmaAtPtCut);
    gs::read_pod(in, &deltaJES);
    gs::read_pod(in, &jesAtCut);
    if (in.fail()) throw gs::IOReadFailure("In JetPtEff::read: "
                                           "input stream failure");
    CPP11_shared_ptr<const npstat::LinInterpolatedTableND<float> > p2(
        ptab.release());
    JetPtEff* pt = new JetPtEff(p2, detectorJetPtCut, sigmaAtPtCut);
    pt->deltaJES_ = deltaJES;
    pt->jesAtCut_ = jesAtCut;
    return pt;
}
