#ifndef NPSTAT_MULTIVARIATEWEIGHTEDSUMACCUMULATOR_HH_
#define NPSTAT_MULTIVARIATEWEIGHTEDSUMACCUMULATOR_HH_

/*!
// \file MultivariateWeightedSumAccumulator.hh
//
// \brief Accumulator of weighted array sums for use with AbsNtuple method
//        weightedCycleOverRows and similar
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
    // Class for accumulating weighted multivariate sums and calculating
    // corresponding averages. The first call to the "accumulate"
    // method after construction (or upon reset) will be used to
    // determine the input array length. All subsequent "accumulate"
    // calls until the next "reset" must use the same array length.
    */
    template<typename Precise = long double>
    class MultivariateWeightedSumAccumulator
    {
    public:
        typedef Precise precise_type;

        /** Only default constructor is necessary */
        inline MultivariateWeightedSumAccumulator()
            : weightSum_(), maxWeight_(0.0), dim_(0), ncalls_(0) {}

        /** The sum of weights so far accumulated */
        inline Precise weightSum() const {return weightSum_;}

        /** Dimensionality of the data */
        inline unsigned long dim() const {return dim_;}

        /** Number of times "accumulate" was called since last reset */
        inline unsigned long ncalls() const {return ncalls_;}

        /** Maximum weight among those so far accumulated */
        inline double maxWeight() const {return maxWeight_;}

        /** Get the sums accumulated so far */
        inline const Precise* data() const
            {return dim_ ? &data_[0] : (Precise*)0;}

        /** Calculate average weight */
        inline Precise averageWeight() const
        {
            if (!ncalls_) throw std::runtime_error(
                "In npstat::MultivariateWeightedSumAccumulator::averageWeight:"
                " no data accumulated");
            return weightSum_/ncalls_;
        }

        /** Accumulate sums for a weighted array of numbers */
        template<typename T>
        inline void accumulate(const T* data, const unsigned long len,
                               const double w)
        {
            if (dim_)
            {
                if (len != dim_) throw std::invalid_argument(
                    "In npstat::MultivariateWeightedSumAccumulator::accumulate:"
                    " unexpected data length");
            }
            else
            {
                // First call of this method for this particular object
                // after construction or reset
                if (!len) throw std::invalid_argument(
                    "In npstat::MultivariateWeightedSumAccumulator::accumulate:"
                    " data length must be positive");
                data_.resize(len);
                dim_ = len;
            }
            if (w < 0.0) throw std::invalid_argument(
                "In npstat::MultivariateWeightedSumAccumulator::accumulate:"
                " weight must be non-negative");
            if (w > 0.0)
            {
                assert(data);
                Precise* buf = &data_[0];
                for (unsigned long i=0; i<len; ++i)
                    buf[i] += data[i]*w;
                weightSum_ += w;
                if (w > maxWeight_)
                    maxWeight_ = w;
            }
            ++ncalls_;
        }

        /** Reset all accumulators */
        inline void reset()
        {
            data_.clear();
            weightSum_ = Precise();
            maxWeight_ = 0.0;
            dim_ = 0UL;
            ncalls_ = 0UL;
        }

        /** Retrieve the sum for a particular index */
        inline const Precise& sum(const unsigned long i) const
            {return data_.at(i);}

        /** Retrieve the mean for a particular index */
        inline Precise mean(const unsigned long i) const
        {
            if (weightSum_ == Precise()) throw std::runtime_error(
                "In npstat::MultivariateWeightedSumAccumulator::mean:"
                " no data accumulated");
            return data_.at(i)/weightSum_;
        }

        /** Retrieve the vector of means */
        inline std::vector<Precise> meanVector() const
        {
            if (weightSum_ == Precise()) throw std::runtime_error(
                "In npstat::MultivariateWeightedSumAccumulator::meanVector:"
                " no data accumulated");
            std::vector<Precise> v(data_);
            Precise* buf = &v[0];
            for (unsigned long i=0; i<dim_; ++i)
                buf[i] /= weightSum_;
            return v;
        }

    private:
        std::vector<Precise> data_;
        Precise weightSum_;
        double maxWeight_;
        unsigned long dim_;
        unsigned long ncalls_;
    };
}

#endif // NPSTAT_MULTIVARIATEWEIGHTEDSUMACCUMULATOR_HH_
