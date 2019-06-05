#ifndef BINNEDJETPTFCN_HH_
#define BINNEDJETPTFCN_HH_

#include <cmath>
#include <vector>
#include <cassert>

#include "geners/GenericIO.hh"

#include "npstat/nm/isMonotonous.hh"

#include "JetInfo.hh"

//
// Collection of JetPtTF or JetPtEff objects binned in |eta| which cover
// the complete eta range. It is assumed that the functions are reasonably
// light-weight and that big tables are managed, if necessary, by shared
// pointers employed inside these functions.
//
template<class T>
class BinnedJetPtFcn
{
public:
    inline BinnedJetPtFcn(const std::vector<double>& etaBinLimits,
                          const std::vector<T>& tfs, const bool useDetectorEta)
        : etaBinLimits_(etaBinLimits),
          tfs_(tfs),
          currentEtaBin_(tfs_.size()),
          usingDetEta_(useDetectorEta)
    {
        assert(currentEtaBin_);
        assert(etaBinLimits_.size() == currentEtaBin_ - 1U);
        if (currentEtaBin_ > 2U)
            assert(npstat::isStrictlyIncreasing(etaBinLimits_.begin(),
                                                etaBinLimits_.end()));
    }

    inline void setJetInfo(const JetInfo& info)
    {
        if (info.e() > 0.0)
        {
            const double eta = usingDetEta_ ? info.detectorEta() : info.eta();
            currentEtaBin_ = determineEtaBin(eta);
            tfs_[currentEtaBin_].setJetInfo(info);
        }
        else
            currentEtaBin_ = tfs_.size();
    }

    inline void setDeltaJES(const double value)
    {
        const unsigned sz = tfs_.size();
        for (unsigned i=0; i<sz; ++i)
            tfs_[i].setDeltaJES(value);
    }

    inline const T& fcn() const {return tfs_.at(currentEtaBin_);}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    inline bool operator==(const BinnedJetPtFcn& r) const
    {
        return tfs_ == r.tfs_ && 
               etaBinLimits_ == r.etaBinLimits_ &&
               currentEtaBin_ == r.currentEtaBin_ &&
               usingDetEta_ == r.usingDetEta_;
    }
    inline bool operator!=(const BinnedJetPtFcn& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    inline bool write(std::ostream& of) const
    {
        gs::write_pod(of, currentEtaBin_);
        gs::write_pod(of, usingDetEta_);
        return gs::write_item(of, etaBinLimits_) && gs::write_item(of, tfs_);
    }

    // I/O methods needed for reading
    static inline const char* classname()
    {
        static const std::string myClassName(
            gs::template_class_name<T>("BinnedJetPtFcn"));
        return myClassName.c_str();
    }
    static inline unsigned version() {return 1;}
    static inline BinnedJetPtFcn* read(const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<BinnedJetPtFcn<T> >());
        myClassId.ensureSameId(id);

        unsigned etaBin, udet;
        std::vector<double> etaBinLimits;
        std::vector<T> tfs;

        gs::read_pod(in, &etaBin);
        gs::read_pod(in, &udet);
        gs::restore_item(in, &etaBinLimits);
        gs::restore_item(in, &tfs);

        if (in.fail()) throw gs::IOReadFailure("In BinnedJetPtFcn::read: "
                                               "input stream failure");
        BinnedJetPtFcn* ptr = new BinnedJetPtFcn(etaBinLimits, tfs, udet);
        ptr->currentEtaBin_ = etaBin;
        return ptr;
    }

private:
    BinnedJetPtFcn();

    inline unsigned determineEtaBin(const double eta) const
    {
        const unsigned sz = etaBinLimits_.size();
        unsigned binnum = 0U;
        if (sz)
        {
            const double abseta = fabs(eta);
            const double* limits = &etaBinLimits_[0];
            for (; binnum<sz; ++binnum)
                if (abseta < limits[binnum])
                    break;
        }
        return binnum;
    }

    std::vector<double> etaBinLimits_;
    std::vector<T> tfs_;
    unsigned currentEtaBin_;
    unsigned usingDetEta_;
};

#endif // BINNEDJETPTFCN_HH_
