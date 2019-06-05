#ifndef NPSTAT_ARRAYPROJECTORS_HH_
#define NPSTAT_ARRAYPROJECTORS_HH_

/*!
// \file ArrayProjectors.hh
//
// \brief Helper templates for making lower-dimensional array projections
//
// Author: I. Volobouev
//
// March 2010
*/

#include <vector>
#include <stdexcept>

#include "npstat/nm/AbsVisitor.hh"
#include "npstat/nm/PreciseType.hh"

namespace npstat {
    /** Maximum value projector */
    template <typename T>
    class ArrayMaxProjector : public AbsVisitor<T, T>
    {
    public:
        inline ArrayMaxProjector() : isClear_(true) {}
        inline virtual ~ArrayMaxProjector() {}

        inline void clear() {isClear_ = true;}
        inline T result()
        {
            if (isClear_) throw std::runtime_error(
                "In npstat::ArrayMaxProjector: no data processed");
            return v_;
        }
        void process(const T& value);

    private:
        T v_;
        bool isClear_;
    };

    /** Minimum value projector */
    template <typename T>
    class ArrayMinProjector : public AbsVisitor<T, T>
    {
    public:
        inline ArrayMinProjector() : isClear_(true) {}
        inline virtual ~ArrayMinProjector() {}

        inline void clear() {isClear_ = true;}
        inline T result()
        {
            if (isClear_) throw std::runtime_error(
                "In npstat::ArrayMinProjector: no data processed");
            return v_;
        }
        void process(const T& value);

    private:
        T v_;
        bool isClear_;
    };

    /** Sum projector */
    template
    <
        typename T, 
        typename Result,
        typename Accumulator=typename PreciseType<T>::type
    >
    class ArraySumProjector : public AbsVisitor<T, Result>
    {
    public:
        inline ArraySumProjector() : sum_(Accumulator()), n_(0UL) {}
        inline virtual ~ArraySumProjector() {}

        inline void clear() {sum_ = Accumulator(); n_ = 0UL;}
        inline virtual Result result() {return static_cast<Result>(sum_);}
        inline void process(const T& value) {sum_ += value; ++n_;}

    protected:
        Accumulator sum_;
        unsigned long n_;
    };

    /** Mean projector */
    template
    <
        typename T,
        typename Result,
        typename Accumulator=typename PreciseType<T>::type
    >
    class ArrayMeanProjector : public ArraySumProjector<T, Result, Accumulator>
    {
    public:
        inline ArrayMeanProjector() : 
            ArraySumProjector<T, Result, Accumulator>() {}
        inline virtual ~ArrayMeanProjector() {}

        inline Result result()
        {
            if (!this->n_) throw std::runtime_error(
                "In npstat::ArrayMeanProjector: no data processed");
            return static_cast<Result>(this->sum_/(1.0*this->n_));
        }
    };

    /** Median projector */
    template <typename T>
    class ArrayMedianProjector : public AbsVisitor<T, T>
    {
    public:
        inline ArrayMedianProjector() {}
        inline virtual ~ArrayMedianProjector() {}

        inline void clear() {v_.clear();}
        inline void process(const T& value) {v_.push_back(value);}
        virtual T result();

    protected:
        T medSorted(const T *array, unsigned long n);
        std::vector<T> v_;
    };

    /**
    // Range projector. Range is defined here as the difference
    // between 75th and 25th percentiles scaled so that it equals
    // sigma for Gaussian distribution.
    */
    template <typename T>
    class ArrayRangeProjector : public ArrayMedianProjector<T>
    {
    public:
        inline ArrayRangeProjector() : ArrayMedianProjector<T>() {}
        inline virtual ~ArrayRangeProjector() {}
        T result();
    };

    /** Standard deviation projector using an accurate two-pass algorithm */
    template <typename T, typename Result>
    class ArrayStdevProjector : public AbsVisitor<T, Result>
    {
    public:
        inline virtual ~ArrayStdevProjector() {}

        inline void clear() {v_.clear();}
        inline void process(const T& value) {v_.push_back(value);}
        Result result();

    private:
        std::vector<T> v_;        
    };
}

#include "npstat/stat/ArrayProjectors.icc"

#endif // NPSTAT_ARRAYPROJECTORS_HH_
