#include <cfloat>

#include "npstat/stat/StatAccumulatorArr.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    void StatAccumulatorArr::initialize(const unsigned long len)
    {
        if (!len) throw std::invalid_argument(
            "In npstat::StatAccumulatorArr::initialize: "
            "data length must not be zero");
        assert(dim_ == 0);
        assert(count_ == 0);
        std::vector<long double>(len*3,0.0L).swap(accMem_);
        std::vector<double>(len*2).swap(minMaxMem_);
        dim_ = len;
        double* d = &minMaxMem_[0];
        for (unsigned long i=0; i<len; ++i)
        {
            *d++ = DBL_MAX;
            *d++ = -DBL_MAX;
        }
        lastRecenter_ = 0UL;
        nextRecenter_ = 0UL;

        // minMaxMem_[0] is min for array element 0
        // minMaxMem_[1] is max for array element 0
        // Then it repeats for all array elements
        //
        // accMem_[0] is the running mean for array element 0
        // accMem_[1] is the sum of squares for array element 0
        // accMem_[2] is the sum for array element 0
        // Then it repeats for all array elements
    }

    void StatAccumulatorArr::recenter()
    {
        if (count_ > lastRecenter_)
        {
            long double* buf = &accMem_[0];
            for (unsigned long i=0; i<dim_; ++i)
            {
                const long double m = buf[2]/count_;
                buf[1] -= m*buf[2];
                if (buf[1] < 0.0L)
                    buf[1] = 0.0L;
                *buf++ += m;
                buf++;
                *buf++ = 0.0L;
            }
            lastRecenter_ = count_;
            nextRecenter_ = 2UL*count_;
        }
    }

    void StatAccumulatorArr::accumulate(const StatAccumulatorArr& r)
    {
        if (r.dim_ == 0UL) return;
        if (dim_)
        {
            if (r.count_)
            {
                if (dim_ != r.dim_) throw std::invalid_argument(
                    "In npstat::StatAccumulatorArr::accumulate: "
                    "incompatible argument dimensionality");
                long double* buf = &accMem_[0];
                const long double* rbuf = &r.accMem_[0];
                for (unsigned long i=0; i<dim_; ++i)
                {
                    const long double rshift = *rbuf++;
                    long double rsumsq = *rbuf++;
                    const long double rsum = *rbuf++;
                    const long double m = rsum/r.count_;
                    rsumsq -= m*rsum;
                    if (rsumsq < 0.0L)
                        rsumsq = 0.0L;
                    const long double dr = rshift + m - *buf++;
                    rsumsq += dr*dr*r.count_;
                    *buf++ += rsumsq;
                    *buf++ += dr*r.count_;
                }

                double* d = &minMaxMem_[0] - 1;
                const double* rd = &r.minMaxMem_[0] - 1;
                for (unsigned long i=0; i<dim_; ++i)
                {
                    if (*++rd < *++d)
                        *d = *rd;
                    if (*++rd > *++d)
                        *d = *rd;
                }

                count_ += r.count_;
                if (count_ >= nextRecenter_)
                    recenter();
            }
        }
        else
        {
            accMem_ = r.accMem_;
            minMaxMem_ = r.minMaxMem_;
            dim_ = r.dim_;
            count_ = r.count_;
            lastRecenter_ = r.lastRecenter_;
            nextRecenter_ = r.nextRecenter_;
        }
    }

    double StatAccumulatorArr::min(const unsigned long i) const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulatorArr::min: no data accumulated");
        if (i >= dim_) throw std::out_of_range(
            "In npstat::StatAccumulatorArr::min: index out of range");
        return minMaxMem_[2*i];
    }

    double StatAccumulatorArr::max(const unsigned long i) const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulatorArr::max: no data accumulated");
        if (i >= dim_) throw std::out_of_range(
            "In npstat::StatAccumulatorArr::max: index out of range");
        return minMaxMem_[2*i + 1];
    }

    double StatAccumulatorArr::mean(const unsigned long i) const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulatorArr::mean: no data accumulated");
        if (i >= dim_) throw std::out_of_range(
            "In npstat::StatAccumulatorArr::mean: index out of range");
        return accMem_[3*i + 2]/count_ + accMem_[3*i];
    }

    double StatAccumulatorArr::stdev(const unsigned long i) const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulatorArr::stdev: no data accumulated");
        if (i >= dim_) throw std::out_of_range(
            "In npstat::StatAccumulatorArr::stdev: index out of range");
        if (count_ == 1UL)
            return 0.0;
        if (count_ > lastRecenter_)
            (const_cast<StatAccumulatorArr*>(this))->recenter();
        return sqrtl(accMem_[3*i + 1]/(count_ - 1UL));
    }

    double StatAccumulatorArr::meanUncertainty(const unsigned long i) const
    {
        if (!count_) throw std::runtime_error(
            "In npstat::StatAccumulatorArr::meanUncertainty: "
            "no data accumulated");
        if (i >= dim_) throw std::out_of_range(
            "In npstat::StatAccumulatorArr::meanUncertainty: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        if (count_ > lastRecenter_)
            (const_cast<StatAccumulatorArr*>(this))->recenter();
        return sqrtl(accMem_[3*i + 1]/(count_ - 1UL)/count_);
    }

    bool StatAccumulatorArr::write(std::ostream& os) const
    {
        gs::write_pod(os, dim_);
        if (dim_)
        {
            (const_cast<StatAccumulatorArr*>(this))->recenter();
            gs::write_pod(os, count_);
            gs::write_pod_vector(os, minMaxMem_);
            std::vector<long double> storedVec(2UL*dim_);
            long double* stored = &storedVec[0];
            const long double* buf = &accMem_[0];
            for (unsigned long i=0; i<dim_; ++i, ++buf)
            {
                *stored++ = *buf++;
                *stored++ = *buf++;
            }
            gs::write_pod_vector(os, storedVec);
        }
        return !os.fail();
    }

    void StatAccumulatorArr::restore(const gs::ClassId& id, std::istream& in,
                                     StatAccumulatorArr* acc)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<StatAccumulatorArr>());
        current.ensureSameId(id);

        assert(acc);
        gs::read_pod(in, &acc->dim_);
        if (acc->dim_)
        {
            gs::read_pod(in, &acc->count_);
            gs::read_pod_vector(in, &acc->minMaxMem_);
            std::vector<long double> storedVec;
            gs::read_pod_vector(in, &storedVec);
            acc->accMem_.resize(3UL*acc->dim_);
            long double* buf = &acc->accMem_[0];
            const long double* stored = &storedVec[0];
            for (unsigned long i=0; i<acc->dim_; ++i)
            {
                *buf++ = *stored++;
                *buf++ = *stored++;
                *buf++ = 0.0L;
            }
        }
        else
            acc->count_ = 0;
        acc->lastRecenter_ = acc->count_;
        acc->nextRecenter_ = 2UL*acc->count_;
        if (in.fail()) throw gs::IOReadFailure(
            "In npstat::StatAccumulatorArr::restore: input stream failure");
    }

    bool StatAccumulatorArr::operator==(const StatAccumulatorArr& r) const
    {
        if (!(dim_ == r.dim_ && count_ == r.count_))
            return false;
        if (dim_ == 0UL || count_ == 0UL)
            return true;
        (const_cast<StatAccumulatorArr*>(this))->recenter();
        (const_cast<StatAccumulatorArr&>(r)).recenter();
        return minMaxMem_ == r.minMaxMem_ && accMem_ == r.accMem_;
    }
}
