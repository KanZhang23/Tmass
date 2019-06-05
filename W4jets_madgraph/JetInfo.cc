#include "rk/rkIO.hh"
#include "JetInfo.hh"

bool JetInfo::operator==(const JetInfo& r) const
{
    return p4_ == r.p4_ && etaDet_ == r.etaDet_ && 
           sysErr_ == r.sysErr_ && derrDpt_ == r.derrDpt_;
}

bool JetInfo::write(std::ostream& of) const
{
    gs::write_item(of, p4_, false);
    gs::write_pod(of, etaDet_);
    gs::write_pod(of, sysErr_);
    gs::write_pod(of, derrDpt_);
    return !of.fail();
}

void JetInfo::restore(const gs::ClassId& id, std::istream& in, JetInfo* ptr)
{
    static const gs::ClassId myClassId(gs::ClassId::makeId<JetInfo>());
    myClassId.ensureSameId(id);

    rk::P4 p;
    double etaDet, sysErr, derrDpt;

    gs::restore_item(in, &p, false);
    gs::read_pod(in, &etaDet);
    gs::read_pod(in, &sysErr);
    gs::read_pod(in, &derrDpt);

    if (in.fail()) throw gs::IOReadFailure("In JetInfo::restore: "
                                           "input stream failure");
    assert(ptr);
    *ptr = JetInfo(p, etaDet, sysErr, derrDpt);
}
