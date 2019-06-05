#ifndef NPSTAT_CIRCULARBUFFER_HH_
#define NPSTAT_CIRCULARBUFFER_HH_

/*!
// \file CircularBuffer.hh
//
// \brief Accumulates data in a circular buffer and calculates
//        various descriptive statistics
//
// Author: I. Volobouev
//
// October 2013
*/

#include <vector>

#include "npstat/nm/PreciseType.hh"

namespace npstat {
    template
    <
        typename Numeric, 
        typename Precise=typename PreciseType<Numeric>::type
    >
    class CircularBuffer
    {
    public:
        /** The constructor argument is the size of the buffer */
        explicit CircularBuffer(unsigned long sz);

        /** The size of the buffer */
        inline unsigned long size() const {return len_;}

        /** The total number of times the buffer was filled */
        inline unsigned long nFills() const {return nfills_;}

        //@{
        /** 
        // Element access which will fail if the buffer is not filled.
        // The element with index 0 is the oldest one in the buffer.
        */
        inline const Numeric& at(const unsigned long i) const
            {return data_.at((nfills_ + i) % len_);}

        inline Numeric& at(const unsigned long i)
            {return data_.at((nfills_ + i) % len_);}
        //@}

        //@{
        /**
        // Element access without checking that the buffer is filled.
        // The element with index 0 is the oldest one in the buffer.
        */
        inline const Numeric& operator[](const unsigned long i) const
            {return data_[(nfills_ + i) % len_];}

        inline Numeric& operator[](const unsigned long i)
            {return data_[(nfills_ + i) % len_];}
        //@}

        /** Check if the buffer is empty */
        inline bool empty() const {return !nfills_;}

        /** Check if the buffer has been completely filled */
        inline bool filled() const {return nfills_ >= len_;}

        /** Comparison for equality */
        bool operator==(const CircularBuffer& r) const;

        /** Logical negation of  operator== */
        bool operator!=(const CircularBuffer& r) const;

        //@{
        /** Accumulate the sample */
        void accumulate(const Numeric& value);

        inline CircularBuffer& operator+=(const Numeric& r)
            {accumulate(r); return *this;}
        //@}

        /** Clear all accumulated data */
        inline void reset() {data_.clear(); nfills_ = 0;}

        /** Minimum value in the accumulated sample */
        Numeric min() const;

        /** Maximum value in the accumulated sample */
        Numeric max() const;

        /** Sum of the accumulated values */
        Precise sum() const;

        /** Accumulated sample average */
        Precise mean() const;

        /** Estimate of the population standard deviation */
        Precise stdev() const;

        /** Uncertainty of the population mean */
        Precise meanUncertainty() const;

    private:
        CircularBuffer();

        std::vector<Numeric> data_;
        unsigned long len_;
        unsigned long nfills_;
    };
}

#include "npstat/stat/CircularBuffer.icc"

#endif // NPSTAT_CIRCULARBUFFER_HH_
