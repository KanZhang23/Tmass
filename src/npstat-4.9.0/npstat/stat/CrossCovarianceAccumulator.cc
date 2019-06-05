#include <cmath>

#include "npstat/stat/CrossCovarianceAccumulator.hh"
#include "geners/binaryIO.hh"
#include "geners/IOException.hh"

namespace npstat {
    CrossCovarianceAccumulator::CrossCovarianceAccumulator(
        const unsigned n1, const unsigned n2)
        : dim1_(n1),
          dim2_(n2),
          count_(0UL),
          sum1_(n1, 0.0L),
          sum2_(n2, 0.0L),
          sumsq1_(n1, 0.0L),
          sumsq2_(n2, 0.0L),
          sumsq_(static_cast<unsigned long>(n1)*n2, 0.0L)
    {
        if (!n1) throw std::invalid_argument(
            "In npstat::CrossCovarianceAccumulator constructor: "
            "zero size of the first array");
        if (!n2) throw std::invalid_argument(
            "In npstat::CrossCovarianceAccumulator constructor: "
            "zero size of the second array");
    }

    void CrossCovarianceAccumulator::reset()
    {
        count_ = 0UL;

        long double* p = &sum1_[0];
        for (unsigned i=0; i<dim1_; ++i)
            p[i] = 0.0L;

        p = &sumsq1_[0];
        for (unsigned i=0; i<dim1_; ++i)
            *p++ = 0.0L;

        p = &sum2_[0];
        for (unsigned i=0; i<dim2_; ++i)
            *p++ = 0.0L;

        p = &sumsq2_[0];
        for (unsigned i=0; i<dim2_; ++i)
            *p++ = 0.0L;

        p = &sumsq_[0];
        for (unsigned i=0; i<dim1_; ++i)
            for (unsigned j=0; j<dim2_; ++j)
                *p++ = 0.0L;
    }

    long double CrossCovarianceAccumulator::sumsq(
        const unsigned i, const unsigned j) const
    {
        if (!(i < dim1_ && j < dim2_)) throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::sumsq: "
            "index out of range");
        return sumsq_[static_cast<unsigned long>(i)*dim2_ + j];
    }

    double CrossCovarianceAccumulator::mean1(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
           "In npstat::CrossCovarianceAccumulator::mean1: no data accumulated");
        return sum1_.at(i)/count_;
    }

    double CrossCovarianceAccumulator::mean2(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
           "In npstat::CrossCovarianceAccumulator::mean2: no data accumulated");
        return sum2_.at(i)/count_;
    }

    double CrossCovarianceAccumulator::stdev1(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::stdev1: no data accumulated");
        if (i >= dim1_)  throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::stdev1: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        const long double var = 
            (sumsq1_[i] - sum1_[i]/count_*sum1_[i])/(count_ - 1);
        if (var > 0.0L)
            return sqrtl(var);
        else
            return 0.0;
    }

    double CrossCovarianceAccumulator::stdev2(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::stdev2: no data accumulated");
        if (i >= dim2_)  throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::stdev2: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        const long double var = 
            (sumsq2_[i] - sum2_[i]/count_*sum2_[i])/(count_ - 1);
        if (var > 0.0L)
            return sqrtl(var);
        else
            return 0.0;
    }

    double CrossCovarianceAccumulator::meanUncertainty1(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::meanUncertainty1: "
          "no data accumulated");
        if (i >= dim1_)  throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::meanUncertainty1: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        const long double var = 
            (sumsq1_[i] - sum1_[i]/count_*sum1_[i])/(count_-1)/count_;
        if (var > 0.0L)
            return sqrtl(var);
        else
            return 0.0;
    }

    double CrossCovarianceAccumulator::meanUncertainty2(const unsigned i) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::meanUncertainty2: "
          "no data accumulated");
        if (i >= dim2_)  throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::meanUncertainty2: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        const long double var = 
            (sumsq2_[i] - sum2_[i]/count_*sum2_[i])/(count_-1)/count_;
        if (var > 0.0L)
            return sqrtl(var);
        else
            return 0.0;
    }

    bool CrossCovarianceAccumulator::operator==(
        const CrossCovarianceAccumulator& r) const
    {
        return dim1_ == r.dim1_ &&
            dim2_ == r.dim2_ &&
            count_ == r.count_ &&
            sum1_ == r.sum1_ &&
            sum2_ == r.sum2_ &&
            sumsq1_ == r.sumsq1_ &&
            sumsq2_ == r.sumsq2_ &&
            sumsq_ == r.sumsq_;
    }

    double CrossCovarianceAccumulator::cov(
        const unsigned i, const unsigned j) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::cov: no data accumulated");
        if (!(i < dim1_ && j < dim2_)) throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::cov: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        else
            return (sumsq_[static_cast<unsigned long>(i)*dim2_ + j] - 
                    sum1_[i]/count_*sum2_[j])/(count_ - 1);
    }

    double CrossCovarianceAccumulator::corr(
        const unsigned i, const unsigned j) const
    {
        if (!count_) throw std::runtime_error(
          "In npstat::CrossCovarianceAccumulator::corr: no data accumulated");
        if (!(i < dim1_ && j < dim2_)) throw std::out_of_range(
            "In npstat::CrossCovarianceAccumulator::corr: "
            "index out of range");
        if (count_ == 1UL)
            return 0.0;
        const long double var1 = 
            (sumsq1_[i] - sum1_[i]/count_*sum1_[i]);
        const long double var2 = 
            (sumsq2_[j] - sum2_[j]/count_*sum2_[j]);
        if (var1 > 0.0L && var2 > 0.0L)
        {
            const long double v = 
                sumsq_[static_cast<unsigned long>(i)*dim2_ + j] - 
                sum1_[i]/count_*sum2_[j];
            return v/sqrtl(var1)/sqrtl(var2);
        }
        else
            return 0.0;
    }

    Matrix<double> CrossCovarianceAccumulator::crossCovMat() const
    {
        if (!count_) throw std::runtime_error(
           "In npstat::CrossCovarianceAccumulator::crossCovMat: "
           "no data accumulated");
        const unsigned long cntm1 = count_ - 1UL;
        Matrix<double> m(dim1_, dim2_);
        if (cntm1)
        {
            double* mdata = const_cast<double*>(m.data());
            const long double* s1 = &sum1_[0];
            const long double* s2 = &sum2_[0];
            const long double* psq = &sumsq_[0];
            for (unsigned i=0; i<dim1_; ++i)
                for (unsigned j=0; j<dim2_; ++j)
                    *mdata++ = (*psq++ - s1[i]/count_*s2[j])/cntm1;
        }
        else
            m.zeroOut();
        return m;
    }

    Matrix<double> CrossCovarianceAccumulator::crossCorrMat() const
    {
        if (!count_) throw std::runtime_error(
           "In npstat::CrossCovarianceAccumulator::crossCorrMat: "
           "no data accumulated");
        const unsigned long cntm1 = count_ - 1UL;
        Matrix<double> m(dim1_, dim2_);
        if (cntm1)
        {
            double* mdata = const_cast<double*>(m.data());
            const long double* s1 = &sum1_[0];
            const long double* s1sq = &sumsq1_[0];
            const long double* s2 = &sum2_[0];
            const long double* s2sq = &sumsq2_[0];
            const long double* psq = &sumsq_[0];
            for (unsigned i=0; i<dim1_; ++i)
            {
                const long double var1 = s1sq[i] - s1[i]/count_*s1[i];
                if (var1 > 0.0L)
                {
                    const long double svar1 = sqrtl(var1);
                    for (unsigned j=0; j<dim2_; ++j)
                    {
                        const long double var2 = s2sq[j] - s2[j]/count_*s2[j];
                        const long double v = *psq++ - s1[i]/count_*s2[j];
                        if (var2 > 0.0L)
                            *mdata++ = v/svar1/sqrtl(var2);
                        else
                            *mdata++ = 0.0;
                    }
                }
                else
                    for (unsigned j=0; j<dim2_; ++j, ++psq)
                        *mdata++ = 0.0;
            }
        }
        else
            m.zeroOut();
        return m;
    }

    bool CrossCovarianceAccumulator::write(std::ostream& os) const
    {
        gs::write_pod(os, dim1_);
        gs::write_pod(os, dim2_);
        gs::write_pod(os, count_);
        gs::write_pod_array(os, &sum1_[0], dim1_);
        gs::write_pod_array(os, &sum2_[0], dim2_);
        gs::write_pod_array(os, &sumsq1_[0], dim1_);
        gs::write_pod_array(os, &sumsq2_[0], dim2_);
        gs::write_pod_array(os, &sumsq_[0],
                        static_cast<unsigned long>(dim1_)*dim2_);
        return !os.fail();
    }

    CrossCovarianceAccumulator* CrossCovarianceAccumulator::read(
        const gs::ClassId& id, std::istream& in)
    {
        static const gs::ClassId current(
            gs::ClassId::makeId<CrossCovarianceAccumulator>());
        current.ensureSameId(id);

        unsigned d1 = 0, d2 = 0;
        gs::read_pod(in, &d1);
        gs::read_pod(in, &d2);
        if (d1 == 0 || d2 == 0)
            return 0;
        CrossCovarianceAccumulator* acc = 
            new CrossCovarianceAccumulator(d1, d2);
        gs::read_pod(in, &acc->count_);
        gs::read_pod_array(in, &acc->sum1_[0], d1);
        gs::read_pod_array(in, &acc->sum2_[0], d2);
        gs::read_pod_array(in, &acc->sumsq1_[0], d1);
        gs::read_pod_array(in, &acc->sumsq2_[0], d2);
        gs::read_pod_array(in, &acc->sumsq_[0],
                       static_cast<unsigned long>(d1)*d2);
        if (in.fail())
        {
            delete acc;
            throw gs::IOReadFailure(
                "In npstat::CrossCovarianceAccumulator::read: "
                "input stream failure");
        }
        return acc;
    }

    void CrossCovarianceAccumulator::accumulate(
        const CrossCovarianceAccumulator& r)
    {
        if (!(dim1_ == r.dim1_ && dim2_ == r.dim2_))
            throw std::invalid_argument(
                "In npstat::CrossCovarianceAccumulator::accumulate: "
                "incompatible size of argument arrays");
        count_ += r.count_;

        long double* p1 = &sum1_[0];
        const long double* p2 = &r.sum1_[0];
        for (unsigned i=0; i<dim1_; ++i)
            *p1++ += *p2++;        

        p1 = &sumsq1_[0];
        p2 = &r.sumsq1_[0];
        for (unsigned i=0; i<dim1_; ++i)
            *p1++ += *p2++;        

        p1 = &sum2_[0];
        p2 = &r.sum2_[0];
        for (unsigned i=0; i<dim2_; ++i)
            *p1++ += *p2++;        

        p1 = &sumsq2_[0];
        p2 = &r.sumsq2_[0];
        for (unsigned i=0; i<dim2_; ++i)
            *p1++ += *p2++;        

        p1 = &sumsq_[0];
        p2 = &r.sumsq_[0];
        for (unsigned i=0; i<dim1_; ++i)
            for (unsigned j=0; j<dim2_; ++j)
                *p1++ += *p2++;
    }
}
