#ifndef NPSTAT_MULTIVARIATESUMACCUMULATOR_HH_
#define NPSTAT_MULTIVARIATESUMACCUMULATOR_HH_

/*!
// \file MultivariateSumAccumulator.hh
//
// \brief Accumulator of array sums for use with AbsNtuple method cycleOverRows
//        and similar
//
// Author: I. Volobouev
//
// April 2011
*/

#include <vector>
#include <cassert>
#include <stdexcept>

namespace npstat {
    /**
    // Class for accumulating multivariate sums and calculating
    // corresponding averages. The first call to the "accumulate"
    // method after construction (or upon reset) will be used to
    // determine the input array length. All subsequent "accumulate"
    // calls until the next "reset" must use the same array length.
    */
    template <typename Precise = long double>
    class MultivariateSumAccumulator
    {
    public:
        typedef Precise precise_type;

        /** Only default constructor is necessary */
        inline MultivariateSumAccumulator() : dim_(0), count_(0) {}

        /** Dimensionality of the data */
        inline unsigned long dim() const {return dim_;}

        /** Number of times "accumulate" was called since last reset */
        inline unsigned long count() const {return count_;}

        /** Get the sums accumulated so far */
        inline const Precise* data() const {return dim_ ? &data_[0] : 0;}

        /** Accumulate sums for an array of numbers */
        template<typename T>
        inline void accumulate(const T* data, const unsigned long len)
        {
            if (dim_)
            {
                if (len != dim_) throw std::invalid_argument(
                    "In npstat::MultivariateSumAccumulator::accumulate: "
                    "unexpected data length");
            }
            else
            {
                // First call of this method for this particular object
                // after construction or reset
                if (!len) throw std::invalid_argument(
                    "In npstat::MultivariateSumAccumulator::accumulate: "
                    "data length must be positive");
                data_.resize(len);
                dim_ = len;
            }
            assert(data);
            Precise* buf = &data_[0];
            for (unsigned long i=0; i<len; ++i)
                buf[i] += data[i];
            ++count_;
        }

        //@{
        /** Add the sample from another accumulator */
        inline void accumulate(const MultivariateSumAccumulator& r)
        {
            if (r.count_)
            {
                if (dim_)
                {
                    if (r.dim_ != dim_) throw std::invalid_argument(
                        "In npstat::MultivariateSumAccumulator::accumulate: "
                        "incompatible accumulator dimensions");
                }
                else
                {
                    data_.resize(r.dim_);
                    dim_ = r.dim_;
                }
                Precise* buf = &data_[0];
                const Precise* data = &r.data_[0];
                for (unsigned long i=0; i<dim_; ++i)
                    buf[i] += data[i];
                count_ += r.count_;
            }
        }

        inline MultivariateSumAccumulator& operator+=(
            const MultivariateSumAccumulator& r)
                {accumulate(r); return *this;}
        //@}

        /** Reset all accumulators */
        inline void reset()
        {
            data_.clear();
            dim_ = 0;
            count_ = 0;
        }

        /** Retrieve the sum for a particular index */
        inline const Precise& sum(const unsigned long i) const
            {return data_.at(i);}

        /** Retrieve the mean for a particular index */
        inline Precise mean(const unsigned long i) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::MultivariateSumAccumulator::mean: "
                "no data accumulated");
            return data_.at(i)/count_;
        }

        /** Retrieve the vector of means */
        inline std::vector<Precise> meanVector() const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::MultivariateSumAccumulator::meanVector: "
                "no data accumulated");
            std::vector<Precise> v(data_);
            Precise* buf = &v[0];
            for (unsigned long i=0; i<dim_; ++i)
                buf[i] /= count_;
            return v;
        }

        /** Comparison for equality */
        inline bool operator==(const MultivariateSumAccumulator& r) const
        {
            if (count_ == 0UL && r.count_ == 0UL)
                return true;
            if (count_ != r.count_)
                return false;
            if (dim_ != r.dim_)
                return false;
            return data_ == r.data_;
        }

        /** Logical negation of operator== */
        inline bool operator!=(const MultivariateSumAccumulator& r) const
            {return !(*this == r);}

    private:
        std::vector<Precise> data_;
        unsigned long dim_;
        unsigned long count_;
    };
}

#endif // NPSTAT_MULTIVARIATESUMACCUMULATOR_HH_
