#ifndef NPSTAT_DISCRETEDISTRIBUTIONS1D_HH_
#define NPSTAT_DISCRETEDISTRIBUTIONS1D_HH_

/*!
// \file DiscreteDistributions1D.hh
//
// \brief A number of useful 1-d discrete statistical distributions
//
// Author: I. Volobouev
//
// May 2013
*/

#include <vector>

#include "npstat/stat/AbsDiscreteDistribution1D.hh"

namespace npstat {
    /** Discrete distribution defined by a table of probabilities */
    class DiscreteTabulated1D : public ShiftableDiscreteDistribution1D
    {
    public:
        //
        // Before applying the shift, probs[0] is the probability at 0,
        // probs[1] is the probability at 1, etc.
        //
        template <typename Real>
        inline DiscreteTabulated1D(const long shift, const Real* probs,
                                   const unsigned probLen)
            : ShiftableDiscreteDistribution1D(shift),
              table_(probs, probs+probLen) {initialize();}

        inline DiscreteTabulated1D(const long shift,
                                   const std::vector<double>& probs)
            : ShiftableDiscreteDistribution1D(shift),
              table_(probs) {initialize();}

        inline virtual ~DiscreteTabulated1D() {}

        inline virtual DiscreteTabulated1D* clone() const
            {return new DiscreteTabulated1D(*this);}

        inline const std::vector<double>& probabilities() const {return table_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::DiscreteTabulated1D";}
        static inline unsigned version() {return 1;}
        static DiscreteTabulated1D* read(const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDiscreteDistribution1D&) const;

    private:
        DiscreteTabulated1D();

        void initialize();

        double unshiftedProbability(long x) const;
        double unshiftedCdf(double x) const;
        double unshiftedExceedance(double x) const;
        long unshiftedQuantile(double x) const;

        std::vector<double> table_;
        std::vector<double> cdf_;
        std::vector<double> exceedance_;
        unsigned firstNonZero_;
        unsigned lastNonZero_;
    };

    /** 
    // Function which pools together two discrete tabulated distributions.
    // Arguments "sampleSize1" and "sampleSize2" define the proportions with
    // which the input distributions are combined. Arguments "first" and
    // "oneAfterLast" define the support of the combined distribution. "First"
    // is the first argument for which the combined density can be be positive
    // (whether it will actually be positive also depends on the supports of
    // the combined distribitions). "OneAfterLast" will be larger by one than
    // the last value of the support.
    */
    DiscreteTabulated1D pooledDiscreteTabulated1D(const DiscreteTabulated1D& d1,
                                                  double sampleSize1,
                                                  const DiscreteTabulated1D& d2,
                                                  double sampleSize2,
                                                  long first, long oneAfterLast);

    /**
    // The Poisson distribution. The calculations of cdf and quantiles,
    // together with generation of random numbers, are implemented by
    // constructing the cdf lookup table. Once such a table is constructed,
    // calculations of this kind become pretty fast. Nevertheless, since
    // construction of the complete cdf table takes some time (especially
    // for large Poisson lambdas), one can imagine situations in which
    // it may be more efficient to set up a Poisson process instead.
    // For example, if you need to generate a few random numbers for many
    // different values of lambda, you might be able to save some CPU time
    // by following the Poisson process route.
    */
    class Poisson1D : public AbsDiscreteDistribution1D
    {
    public:
        explicit Poisson1D(double lambda);

        Poisson1D(const Poisson1D&);
        Poisson1D& operator=(const Poisson1D&);

        inline virtual Poisson1D* clone() const {return new Poisson1D(*this);}

        inline virtual ~Poisson1D() {delete table_;}

        inline double mean() const {return lambda_;}

        double probability(long x) const;
        double cdf(double x) const;
        double exceedance(double x) const;
        long quantile(double x) const;
        unsigned random(AbsRandomGenerator& g, long* generatedRandom) const;
 
        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Poisson1D";}
        static inline unsigned version() {return 1;}
        static Poisson1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        inline virtual bool isEqual(const AbsDiscreteDistribution1D& o) const
        {
            const Poisson1D& r = static_cast<const Poisson1D&>(o);
            return lambda_ == r.lambda_;
        }

    private:
        Poisson1D();

        void buildTable();

        double lambda_;
        DiscreteTabulated1D* table_;
        long minUse_;
        long double lnlambda_;
    };
}

#endif // NPSTAT_DISCRETEDISTRIBUTIONS1D_HH_
