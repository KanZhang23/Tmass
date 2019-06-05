#ifndef NPSTAT_NTHISTOFILL_HH_
#define NPSTAT_NTHISTOFILL_HH_

/*!
// \file NtHistoFill.hh
//
// \brief Fill a histogram using contents of a homogeneous ntuple
//
// Author: I. Volobouev
//
// November 2011
*/

#include <cassert>
#include <stdexcept>
#include <climits>

namespace npstat {
    /**
    // This convenience class fills a histogram using ntuple contents.
    // For use inside "cycleOverRows", "conditionalCycleOverRows", and
    // other similar methods of AbsNtuple.
    //
    // The second template parameter switches between use of "fill"
    // and "fillC" methods of the Histogram class
    */
    template <class Histogram, bool useFillC=false>
    class NtHistoFill
    {
    public:
        typedef Histogram histo_type;

        /**
        // This constructor assumes that correctness of the
        // ntuple column numbers has already been verified.
        // Column numbers "coordColumns" which map ntuple column numbers
        // into histogram axes are thus best generated with AbsNtuple
        // methods "columnIndices". Column number coordColumns[0]
        // will be mapped into the first histogram axis, coordColumns[1]
        // into the second, etc. Ntuple method "validColumn" can be
        // used to generate the weight column number. If the weight
        // column is not specified, 1 will be used as the weight.
        */
        inline NtHistoFill(Histogram* histo,
                           const std::vector<unsigned long>& coordColumns,
                           const unsigned long weightColumn = ULONG_MAX)
            : histo_(histo),
              coordCols_(coordColumns),
              coords_(histo_->dim()),
              weightCol_(weightColumn),
              dim_(histo_->dim()),
              one_(static_cast<typename Histogram::value_type>(1))
        {
            assert(histo_);
            if (!dim_) throw std::invalid_argument(
                "In npstat::NtHistoFill constructor: "
                "can not fill zero-dimensional histograms");
            if (dim_ != coordColumns.size()) throw std::invalid_argument(
                "In npstat::NtHistoFill constructor: "
                "incompatible number of columns");
            weightColumnSpecified_ = weightColumn != ULONG_MAX;
        }

        static inline bool callsFillC() {return useFillC;}

        static inline const char* histoClassname()
        {
            static const gs::ClassId hid(gs::ClassId::makeId<Histogram>());
            return hid.name().c_str();
        }

    private:
        template <typename T, bool fillC>
        struct HFill
        {
            inline static void fill(Histogram* h, const double* c,
                                    const unsigned dim, const T& weight)
                {h->fill(c, dim, weight);}
        };

        template <typename T>
        struct HFill<T, true>
        {
            inline static void fill(Histogram* h, const double* c,
                                    const unsigned dim, const T& weight)
                {h->fillC(c, dim, weight);}
        };

        NtHistoFill();

        Histogram* histo_;
        std::vector<unsigned long> coordCols_;
        mutable std::vector<double> coords_;
        unsigned long weightCol_;
        unsigned dim_;
        bool weightColumnSpecified_;
        typename Histogram::value_type one_;

    public:
        template <typename T>
        inline void accumulate(const T* rowContents,
                               const unsigned long nCols) const
        {
            const unsigned long* idx = &coordCols_[0];
            double* c = &coords_[0];
            for (unsigned i=0; i<dim_; ++i)
                c[i] = static_cast<double>(rowContents[idx[i]]);
            if (weightColumnSpecified_)
            {
                if (weightCol_ >= nCols)
                    throw std::out_of_range(
                        "In npstat::NtHistoFill::accumulate: "
                        "weight column index out of range");
                HFill<T,useFillC>::fill(
                    histo_, c, dim_, rowContents[weightCol_]);
            }
            else
                HFill<typename Histogram::value_type,useFillC>::fill(
                    histo_, c, dim_, one_);
        }
    };
}

#endif // NPSTAT_NTHISTOFILL_HH_
