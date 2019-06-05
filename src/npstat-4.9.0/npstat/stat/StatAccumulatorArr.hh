#ifndef NPSTAT_STATACCUMULATORARR_HH_
#define NPSTAT_STATACCUMULATORARR_HH_

/*!
// \file StatAccumulatorArr.hh
//
// \brief Single-pass accumulator of statistical summaries for collections
//        of arrays
//
// Author: I. Volobouev
//
// July 2011
*/

#include <cmath>
#include <vector>
#include <cassert>
#include <stdexcept>

#include "geners/ClassId.hh"

namespace npstat {
    /**
    // Single-pass accumulator of statistical summaries for some set of arrays.
    // Equivalent in functionality to a vector of "StatAccumulator" objects
    // but has a more convenient interface.
    */
    class StatAccumulatorArr
    {
    public:
        inline StatAccumulatorArr() : dim_(0), count_(0) {}

        /** "dim" is the expected array length */
        inline explicit StatAccumulatorArr(const unsigned long dim)
            : dim_(0), count_(0) {initialize(dim);}

        /** Array length */
        inline unsigned long dim() const {return dim_;}

        /** Number of observations processed so far */
        inline unsigned long count() const {return count_;}

        /** Minimum value in the processed sample for the given dimension */
        double min(unsigned long i) const;

        /** Maximum value in the processed sample for the given dimension */
        double max(unsigned long i) const;

        /** Accumulated sample average for the given dimension */
        double mean(unsigned long i) const;

        /**
        // Estimate of the population standard deviation
        // for the given dimension
        */
        double stdev(unsigned long i) const;

        /** Uncertainty of the population mean for the given dimension */
        double meanUncertainty(unsigned long i) const;

        /** Process an array */
        template<typename T>
        inline void accumulate(const T* data, const unsigned long len)
        {
            if (dim_)
            {
                if (len != dim_) throw std::invalid_argument(
                    "In npstat::StatAccumulatorArr::accumulate: "
                    "incompatible data length");
            }
            else
                initialize(len);
            assert(data);
            long double* buf = &accMem_[0];
            double* minmax = &minMaxMem_[0] - 1;
            for (unsigned long i=0; i<len; ++i)
            {
                if (*data < *++minmax)
                    *minmax = *data;
                if (*data > *++minmax)
                    *minmax = *data;
                const long double delta = *data++ - *buf++;
                *buf++ += delta*delta;
                *buf++ += delta;
            }
            if (++count_ >= nextRecenter_)
                recenter();
        }

        //@{
        /** Add the sample from another accumulator */
        void accumulate(const StatAccumulatorArr& acc);

        inline StatAccumulatorArr& operator+=(const StatAccumulatorArr& r)
            {accumulate(r); return *this;}

        template<typename T>
        inline StatAccumulatorArr& operator+=(const std::vector<T>& r)
            {accumulate(&r.at(0), r.size()); return *this;}
        //@}

        /** Clear all accumulators */
        inline void reset()
        {
            dim_ = 0;
            count_ = 0;
        }

        /** Minimum values in the processed sample */
        template<typename T>
        inline void min(T* arr, const unsigned long len) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::min: no data accumulated");
            if (len != dim_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::min: "
                "incompatible buffer length");
            assert(arr);
            const double* minmax = &minMaxMem_[0];
            for (unsigned long i=0; i<len; ++i, ++minmax)
                *arr++ = static_cast<T>(*minmax++);
        }

        /** Maximum values in the processed sample */
        template<typename T>
        inline void max(T* arr, const unsigned long len) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::max: no data accumulated");
            if (len != dim_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::max: "
                "incompatible buffer length");
            assert(arr);
            const double* minmax = &minMaxMem_[0] + 1;
            for (unsigned long i=0; i<len; ++i, ++minmax)
                *arr++ = static_cast<T>(*minmax++);
        }

        /** Accumulated sample averages */
        template<typename T>
        inline void mean(T* arr, const unsigned long len) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::mean: no data accumulated");
            if (len != dim_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::mean: "
                "incompatible buffer length");
            assert(arr);
            const long double* buf = &accMem_[0];
            for (unsigned long i=0; i<len; ++i)
            {
                const long double shift = *buf++; buf++;
                *arr++ = static_cast<T>(shift + *buf++/count_);
            }
        }

        /** Estimates of the population standard deviation */
        template<typename T>
        inline void stdev(T* arr, const unsigned long len) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::stdev: no data accumulated");
            if (len != dim_)  throw std::runtime_error(
                "In npstat::StatAccumulatorArr::stdev: "
                "incompatible buffer length");
            assert(arr);
            const unsigned long countm1 = count_ - 1;
            if (countm1)
            {
                (const_cast<StatAccumulatorArr*>(this))->recenter();
                const long double* buf = &accMem_[0] + 1;
                for (unsigned long i=0; i<len; ++i, ++buf, ++buf)
                    *arr++ = static_cast<T>(sqrtl(*buf++/countm1));
            }
            else
                for (unsigned long i=0; i<len; ++i)
                    *arr++ = 0;
        }

        /** Uncertainties of the population means */
        template<typename T>
        inline void meanUncertainty(T* arr, const unsigned long len) const
        {
            if (!count_) throw std::runtime_error(
                "In npstat::StatAccumulatorArr::meanUncertainty: "
                "no data accumulated");
            if (len != dim_)  throw std::runtime_error(
                "In npstat::StatAccumulatorArr::meanUncertainty: "
                "incompatible buffer length");
            assert(arr);
            const unsigned long countm1 = count_ - 1;
            if (countm1)
            {
                (const_cast<StatAccumulatorArr*>(this))->recenter();
                const long double* buf = &accMem_[0] + 1;
                for (unsigned long i=0; i<len; ++i, ++buf, ++buf)
                    *arr++ = static_cast<T>(sqrtl(*buf++/countm1/count_));
            }
            else
                for (unsigned long i=0; i<len; ++i)
                    *arr++ = 0;
        }

        /**
        // The effect of "operator*=" is the same as if all values
        // were multiplied by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLongDouble>
        inline StatAccumulatorArr& operator*=(const ConvertibleToLongDouble& r)
        {
            if (count_)
            {
                const long double factor(static_cast<long double>(r));
                const double scale(factor);
                long double* buf = &accMem_[0];
                double* minmax = &minMaxMem_[0];
                const bool swapminmax = scale < 0.0;
                for (unsigned long i=0; i<dim_; ++i)
                {
                    *buf++ *= factor;
                    *buf++ *= factor*factor;
                    *buf++ *= factor;
                    if (swapminmax)
                    {
                        const double tmp(minmax[0]*scale);
                        minmax[0] = minmax[1]*scale;
                        minmax[1] = tmp;
                        minmax += 2;
                    }
                    else
                    {
                        *minmax++ *= scale;
                        *minmax++ *= scale;
                    }
                }
            }
            return *this;
        }

        /**
        // The effect of "operator/=" is the same as if all values
        // were divided by "r" for each "accumulate" call
        */
        template<typename ConvertibleToLongDouble>
        inline StatAccumulatorArr& operator/=(const ConvertibleToLongDouble& r)
        {
            const long double denom = static_cast<long double>(r);
            if (denom == 0.0L) throw std::domain_error(
                "In npstat::StatAccumulatorArr::operator/=: "
                "division by zero");
            return operator*=(1.0L/denom);
        }

        /** Binary multiplication by a double */
        template<typename ConvertibleToLDouble>
        inline StatAccumulatorArr operator*(const ConvertibleToLDouble& r) const
        {
            StatAccumulatorArr acc(*this);
            acc *= r;
            return acc;
        }

        /** Binary division by a double */
        template<typename ConvertibleToLDouble>
        inline StatAccumulatorArr operator/(const ConvertibleToLDouble& r) const
        {
            StatAccumulatorArr acc(*this);
            acc /= r;
            return acc;
        }

        /** Binary sum with another accumulator */
        inline StatAccumulatorArr operator+(const StatAccumulatorArr& r) const
        {
            StatAccumulatorArr acc(*this);
            acc += r;
            return acc;
        }

        /** Comparison for equality */
        bool operator==(const StatAccumulatorArr& r) const;

        /** Logical negation of operator== */
        inline bool operator!=(const StatAccumulatorArr& r) const
            {return !(*this == r);}

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::StatAccumulatorArr";}
        static inline unsigned version() {return 2;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            StatAccumulatorArr* acc);
    private:
        void initialize(unsigned long len);
        void recenter();

        std::vector<long double> accMem_;
        std::vector<double> minMaxMem_;
        unsigned long dim_;
        unsigned long count_;
        unsigned long lastRecenter_;
        unsigned long nextRecenter_;

#ifdef SWIG
    public:
        inline StatAccumulatorArr mul2(const double r) const
            {return operator*(r);}

        inline StatAccumulatorArr div2(const double r) const
            {return operator/(r);}

        inline StatAccumulatorArr& imul2(const double r)
            {*this *= r; return *this;}

        inline StatAccumulatorArr& idiv2(const double r)
            {*this /= r; return *this;}

        inline void minArray(double* arr, const unsigned long len) const
            {min(arr, len);}

        inline void maxArray(double* arr, const unsigned long len) const
            {max(arr, len);}        

        inline void meanArray(double* arr, const unsigned long len) const
            {mean(arr, len);}        

        inline void stdevArray(double* arr, const unsigned long len) const
            {stdev(arr, len);}        

        inline void meanUncertaintyArray(double* arr,
                                         const unsigned long len) const
            {meanUncertainty(arr, len);}        

        inline StatAccumulatorArr& operator+=(const std::vector<double>& r)
            {accumulate(&r.at(0), r.size()); return *this;}
#endif
    };
}

#endif // NPSTAT_STATACCUMULATORARR_HH_
