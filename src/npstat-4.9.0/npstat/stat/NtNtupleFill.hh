#ifndef NPSTAT_NTNTUPLEFILL_HH_
#define NPSTAT_NTNTUPLEFILL_HH_

/*!
// \file NtNtupleFill.hh
//
// \brief Fill a homogeneous ntuple using contents of another ntuple.
//
// For use with "cycleOverRows", "conditionalCycleOverRows", and other
// similar methods of AbsNtuple.
//
// Author: I. Volobouev
//
// February 2012
*/

#include <cassert>
#include <stdexcept>

namespace npstat {
    /**
    // This convenience class fills an ntuple using contents of another ntuple
    */
    template <class Ntuple>
    class NtNtupleFill
    {
    public:
        typedef Ntuple ntuple_type;

        /**
        // The constructor assumes that correctness of the ntuple
        // column numbers has already been verified. Column
        // numbers are thus best generated with AbsNtuple
        // methods "columnIndices". Column number columnMap[0]
        // of the source ntuple (whose "cycleOverRows" method will
        // be called) will be mapped into the first column of the
        // destination ntuple, columnMap[1] into the second, etc.
        // The number of "columnMap" elements must be equal either 
        // to the number of columns in the destination ntuple or
        // to zero. In the latter case an automatic trivial
        // sequential column mapping will be generated.
        */
        inline NtNtupleFill(Ntuple* destination,
                            const std::vector<unsigned long>& columnMap)
            : dest_(destination),
              coordCols_(columnMap)
        {
            assert(dest_);
            const unsigned long nCols = dest_->nColumns();
            if (!nCols) throw std::invalid_argument(
                "In npstat::NtNtupleFill constructor: "
                "can not fill zero-column ntuples");
            const unsigned long mapLen = coordCols_.size();
            if (mapLen != 0 &&  mapLen != nCols) throw std::invalid_argument(
                "In npstat::NtNtupleFill constructor: "
                "incompatible number of elements in the column map");
            buffer_.resize(nCols);
        }

    private:
        NtNtupleFill();

        Ntuple* dest_;
        std::vector<unsigned long> coordCols_;
        mutable std::vector<typename Ntuple::value_type> buffer_;

    public:
        template <typename T>
        inline void accumulate(const T* rowContents,
                               const unsigned long nCols) const
        {
            const unsigned long dim = buffer_.size();
            typename Ntuple::value_type * c = &buffer_[0];
            if (coordCols_.empty())
            {
                if (nCols < dim) throw std::invalid_argument(
                    "In npstat::NtNtupleFill::accumulate: "
                    "insufficient number of columns in the source ntuple");
                for (unsigned long i=0; i<dim; ++i)
                    c[i] = static_cast<typename Ntuple::value_type>(
                        rowContents[i]);
            }
            else
            {
                const unsigned long* idx = &coordCols_[0];
                for (unsigned long i=0; i<dim; ++i)
                    c[i] = static_cast<typename Ntuple::value_type>(
                        rowContents[idx[i]]);
            }
            dest_->fill(c, dim);
        }
    };

    //@{
    /** Helper utility function for making NtNtupleFill objects */
    template <typename Ntuple>
    inline NtNtupleFill<Ntuple> make_NtNtupleFill(
        Ntuple& destination,
        const std::vector<unsigned long>& columnMap)
    {
        return NtNtupleFill<Ntuple>(&destination, columnMap);
    }

    template <typename Ntuple>
    inline NtNtupleFill<Ntuple> make_NtNtupleFill(Ntuple& destination)
    {
        std::vector<unsigned long> dummyMap;
        return NtNtupleFill<Ntuple>(&destination, dummyMap);
    }
    //@}
}

#endif // NPSTAT_NTNTUPLEFILL_HH_
