#include <set>
#include <cfloat>

#include "geners/mapIO.hh"
#include "geners/IOException.hh"

#include "npstat/stat/MemoizingSymbetaFilterProvider.hh"

#define getit(itemptr) do {\
    if (itemptr) *itemptr = it->first.get_ ## itemptr ();\
} while(0);


namespace npstat {
    namespace Private {

    SymbetaFilterParams::SymbetaFilterParams(
        int symbetaPower, double bandwidth, double degree,
        unsigned nbins, double binwidth,
        const BoundaryHandling& bm,
        unsigned excludedBin, bool excludeCentralPoint)
        : bandwidth_(bandwidth), degree_(degree), binwidth_(binwidth),
          nbins_(nbins), excludedBin_(excludedBin),
          symbetaPower_(symbetaPower),
          bm_(bm),
          excludeCentralPoint_(excludeCentralPoint)
    {
        if (bandwidth < 0.0) throw std::invalid_argument(
            "In npstat::Private::SymbetaFilterParams constructor: "
            "bandwidth can not be negative");
    }

    bool SymbetaFilterParams::write(std::ostream& of) const
    {
        gs::write_pod(of, bandwidth_);
        gs::write_pod(of, degree_);
        gs::write_pod(of, binwidth_);
        gs::write_pod(of, nbins_);
        gs::write_pod(of, excludedBin_);
        gs::write_pod(of, symbetaPower_);
        gs::write_obj(of, bm_);
        unsigned char exclude = excludeCentralPoint_;
        gs::write_pod(of, exclude);
        return !of.fail();
    }

    void SymbetaFilterParams::restore(const gs::ClassId& id, std::istream& in,
                                      SymbetaFilterParams* ptr)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<SymbetaFilterParams>());
        myClassId.ensureSameId(id);

        assert(ptr);

        gs::read_pod(in, &ptr->bandwidth_);
        gs::read_pod(in, &ptr->degree_);
        gs::read_pod(in, &ptr->binwidth_);
        gs::read_pod(in, &ptr->nbins_);
        gs::read_pod(in, &ptr->excludedBin_);
        gs::read_pod(in, &ptr->symbetaPower_);
        gs::restore_obj(in, &ptr->bm_);
        unsigned char exclude = 0;
        gs::read_pod(in, &exclude);
        ptr->excludeCentralPoint_ = exclude;

        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::Private::SymbetaFilterParams::restore: "
            "input stream failure");
    }

    }


    CPP11_shared_ptr<const LocalPolyFilter1D> 
    MemoizingSymbetaFilterProvider::provideFilter(
        const int symbetaPower, const double bandwidth, const double degree,
        const unsigned nbins, const double binwidth,
        const BoundaryHandling& bm,
        const unsigned excludedBin, const bool excludeCentralPoint)
    {
        Private::SymbetaFilterParams par(symbetaPower, bandwidth, degree,
                                         nbins, binwidth, bm,
                                         excludedBin, excludeCentralPoint);
        FilterMap::const_iterator it = filterMap_.find(par);
        if (it != filterMap_.end())
            return it->second;

        FilterPtr ptr = SimpleSymbetaFilterProvider::provideFilter(
            symbetaPower, bandwidth, degree,
            nbins, binwidth, bm,
            excludedBin, excludeCentralPoint);
        if (this->isMemoizing())
            filterMap_[par] = ptr;
        return ptr;
    }

    bool MemoizingSymbetaFilterProvider::write(std::ostream& of) const
    {
        unsigned char memzing = this->isMemoizing();
        gs::write_pod(of, memzing);
        return gs::write_item(of, filterMap_) && !of.fail();
    }

    bool MemoizingSymbetaFilterProvider::operator==(
        const MemoizingSymbetaFilterProvider& r) const
    {
        if (this->isMemoizing() != r.isMemoizing())
            return false;
        if (filterMap_.size() != r.filterMap_.size())
            return false;
        FilterMap::const_iterator ri = r.filterMap_.begin();
        for (FilterMap::const_iterator it = filterMap_.begin();
             it != filterMap_.end(); ++it, ++ri)
        {
            if (it->first != ri->first)
                return false;
            if (*it->second != *ri->second)
                return false;
        }
        return true;
    }

    void MemoizingSymbetaFilterProvider::restore(
        const gs::ClassId& id, std::istream& in,
        MemoizingSymbetaFilterProvider* ptr)
    {
        static const gs::ClassId myClassId(
            gs::ClassId::makeId<MemoizingSymbetaFilterProvider>());
        myClassId.ensureSameId(id);

        assert(ptr);

        unsigned char memzing = 0;
        gs::read_pod(in, &memzing);
        if (memzing)
            ptr->startMemoizing();
        else
            ptr->stopMemoizing();
        return gs::restore_item(in, &ptr->filterMap_);
    }

    void MemoizingSymbetaFilterProvider::firstMemoizedInfo(
        int* symbetaPower, double* bandwidth,
        double* degree, unsigned* nbins, double* binwidth,
        BoundaryHandling* bm, unsigned* excludedBin,
        bool* excludeCentralPoint) const
    {
        if (filterMap_.empty()) throw std::runtime_error(
            "In npstat::MemoizingSymbetaFilterProvider::firstMemoizedInfo: "
            "no items memoized");
        FilterMap::const_iterator it = filterMap_.begin();
        getit(symbetaPower);
        getit(bandwidth);
        getit(degree);
        getit(nbins);
        getit(binwidth);
        getit(bm);
        getit(excludedBin);
        getit(excludeCentralPoint);
    }

    std::vector<double>
    MemoizingSymbetaFilterProvider::knownBandwidthValues() const
    {
        std::set<double> bws;
        for (FilterMap::const_iterator it = filterMap_.begin();
             it != filterMap_.end(); ++it)
            bws.insert(it->first.get_bandwidth());
        return std::vector<double>(bws.begin(), bws.end());
    }
}
