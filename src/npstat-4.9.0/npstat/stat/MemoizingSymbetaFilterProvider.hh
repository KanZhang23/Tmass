#ifndef NPSTAT_MEMOIZINGSYMBETAFILTERPROVIDER_HH_
#define NPSTAT_MEMOIZINGSYMBETAFILTERPROVIDER_HH_

/*!
// \file MemoizingSymbetaFilterProvider.hh
//
// \brief Builds symmetric beta LOrPE filters and remembers
//        these filters when the user sets the corresponding flag
//
// Author: I. Volobouev
//
// November 2013
*/

#include <map>
#include <vector>
#include <string>
#include <cassert>

#include "geners/ClassId.hh"

#include "npstat/stat/AbsSymbetaFilterProvider.hh"

namespace npstat {
    namespace Private {
        class SymbetaFilterParams
        {
        public:
            inline SymbetaFilterParams()
                : bandwidth_(-1.0), degree_(-1.0), binwidth_(-1.0),
                  nbins_(0), excludedBin_(0), symbetaPower_(0),
                  excludeCentralPoint_(false) {}

            SymbetaFilterParams(
                int symbetaPower, double bandwidth, double degree,
                unsigned nbins, double binwidth, const BoundaryHandling& bm,
                unsigned excludedBin, bool excludeCentralPoint);

            bool operator<(const SymbetaFilterParams& r) const;

            inline bool operator>(const SymbetaFilterParams& r) const
                {return r < *this;}

            bool operator==(const SymbetaFilterParams& r) const;

            inline bool operator!=(const SymbetaFilterParams& r) const
                {return !(*this == r);}

            // I/O methods needed for writing
            inline gs::ClassId classId() const {return gs::ClassId(*this);}
            bool write(std::ostream& of) const;

            // I/O methods needed for reading
            static inline const char* classname()
                {return "npstat::Private::SymbetaFilterParams";}
            static inline unsigned version() {return 3;}
            static void restore(const gs::ClassId& id, std::istream& in,
                                SymbetaFilterParams* ptr);

            inline double get_bandwidth() const {return bandwidth_;}
            inline double get_degree() const {return degree_;}
            inline double get_binwidth() const {return binwidth_;}
            inline unsigned get_nbins() const {return nbins_;}
            inline unsigned get_excludedBin() const {return excludedBin_;}
            inline int get_symbetaPower() const {return symbetaPower_;}
            inline const BoundaryHandling& get_bm() const {return bm_;}
            inline bool get_excludeCentralPoint() const 
                {return excludeCentralPoint_;}

        private:
            double bandwidth_;
            double degree_;
            double binwidth_;
            unsigned nbins_;
            unsigned excludedBin_;
            int symbetaPower_;
            BoundaryHandling bm_;
            bool excludeCentralPoint_;
        };
    }

    class MemoizingSymbetaFilterProvider : public SimpleSymbetaFilterProvider
    {
    public:
        inline MemoizingSymbetaFilterProvider() {this->startMemoizing();}

        inline virtual ~MemoizingSymbetaFilterProvider() {}

        virtual CPP11_shared_ptr<const LocalPolyFilter1D> provideFilter(
            int symbetaPower, double bandwidth, double degree,
            unsigned nbins, double binwidth,
            const BoundaryHandling& bm,
            unsigned excludedBin, bool excludeCentralPoint);

        inline unsigned long nMemoized() const {return filterMap_.size();}

        void firstMemoizedInfo(int* symbetaPower, double* bandwidth,
                               double* degree, unsigned* nbins,
                               double* binwidth, BoundaryHandling* bm,
                               unsigned* excludedBin,
                               bool* excludeCentralPoint) const;

        std::vector<double> knownBandwidthValues() const;

        bool operator==(const MemoizingSymbetaFilterProvider& r) const;
        inline bool operator!=(const MemoizingSymbetaFilterProvider& r) const
            {return !(*this == r);}

        // I/O methods needed for writing
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;

        // I/O methods needed for reading
        static inline const char* classname()
            {return "npstat::MemoizingSymbetaFilterProvider";}
        static inline unsigned version() {return 3;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            MemoizingSymbetaFilterProvider* ptr);
    private:
        typedef CPP11_shared_ptr<const LocalPolyFilter1D> FilterPtr;
        typedef std::map<Private::SymbetaFilterParams, FilterPtr> FilterMap;

        FilterMap filterMap_;
    };
}

#include "npstat/stat/MemoizingSymbetaFilterProvider.icc"

#endif // NPSTAT_MEMOIZINGSYMBETAFILTERPROVIDER_HH_
