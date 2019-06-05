#ifndef NPSTAT_GAUSSIANMIXTURE1D_HH_
#define NPSTAT_GAUSSIANMIXTURE1D_HH_

/*!
// \file GaussianMixture1D.hh
//
// \brief One-dimensional Gaussian mixtures
//
// Author: I. Volobouev
//
// October 2011
*/

#include <vector>
#include <stdexcept>
#include <utility>

#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    class GaussianMixture1D;

    /** A Gaussian mixture component */
    class GaussianMixtureEntry
    {
    public:
        inline GaussianMixtureEntry() : w_(0.0), mean_(0.0), stdev_(1.0) {}

        /** Constructor arguments are weight, mean, and standard deviation */
        inline GaussianMixtureEntry(const double w, const double mean,
                                    const double stdev)
            : w_(w), mean_(mean), stdev_(stdev)
        {
            if (w_ < 0.0) throw std::invalid_argument(
                "In npstat::GaussianMixtureEntry constructor: "
                "component weight must be non-negative");
            if (w_ > 0.0 && stdev_ <= 0.0) throw std::invalid_argument(
                "In npstat::GaussianMixtureEntry constructor: "
                "standard deviation must be positive");
        }

        //@{
        /** Examine object properties */
        inline double weight() const {return w_;}
        inline double mean() const {return mean_;}
        inline double stdev() const {return stdev_;}
        //@}

        bool operator==(const GaussianMixtureEntry& r) const;
        inline bool operator!=(const GaussianMixtureEntry& r) const
            {return !(*this == r);}

        //@{
        /** Method needed for "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& os) const;
        //@}

        static inline const char* classname()
            {return "npstat::GaussianMixtureEntry";}
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in,
                            GaussianMixtureEntry* entry);
    private:
        friend class GaussianMixture1D;

        double w_;
        double mean_;
        double stdev_;
    };

    /** One-dimensional Gaussian mixture distribution */
    class GaussianMixture1D : public AbsScalableDistribution1D
    {
    public:
        /**
        // This constructor takes an array of GaussianMixtureEntry
        // objects together with an overall location and scale parameters
        // which are used to shift and scale the whole distribution.
        // There must be at least one component with positive weight
        // in the array of entries. The sum of weights will be normalized
        // to 1 internally.
        */
        GaussianMixture1D(double location, double scale,
                          const GaussianMixtureEntry* entries,
                          unsigned nEntries);

        /** Constructor from a single Gaussian distribution */
        explicit GaussianMixture1D(const Gauss1D& gauss);

        inline virtual ~GaussianMixture1D() {}

        inline virtual GaussianMixture1D* clone() const
            {return new GaussianMixture1D(*this);}

        /** Number of mixture components */
        inline unsigned nentries() const {return entries_.size();}

        /** Get the mixture component with the given number */
        inline const GaussianMixtureEntry& entry(const unsigned n) const
            {return entries_.at(n);}

        /** Get all mixture components */
        inline const std::vector<GaussianMixtureEntry>& entries() const
            {return entries_;}

        /** Generate random numbers asscording to this distribution */
        virtual unsigned random(AbsRandomGenerator& g, double* r) const;

        /** Mean of the whole mixture */
        double mean() const;

        /** Standard deviation of the whole mixture */
        double stdev() const;

        /**
        // Mean Integrated Squared Error expected for KDE of a sample from
        // this mixture using Hermite polynomial kernel. The formula is from
        // the article "Exact Mean Integrated Squared Error" by J.S. Marron
        // and M.P. Wand, The Annals Of Statistics, Vol 20, pp. 712-736 (1992).
        // Kernel order is maxPolyDegree + 2 if maxPolyDegree is even, and
        // maxPolyDegree + 1 if maxPolyDegree is odd.
        //
        // Due to accumulation of round-off errors, the calculation becomes
        // unreliable for maxPolyDegree above 16 or so.
        */
        double gaussianMISE(unsigned maxPolyDegree, double bandwidth,
                            unsigned long npoints) const;

        /** Bandwidth which minimizes the KDE MISE */
        double miseOptimalBw(unsigned maxPolyDegree, unsigned long npoints,
                             double* mise = 0) const;

        //@{
        /** Methods needed for "geners" I/O */
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;
        //@}

        static inline const char* classname() {return "npstat::GaussianMixture1D";}
        static inline unsigned version() {return 1;}
        static GaussianMixture1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const;

    private:
        friend class ScalableDistribution1DFactory<GaussianMixture1D>;

        class GaussianMISEFunctor;
        friend class GaussianMISEFunctor;

        class GaussianMISEFunctor : public Functor1<long double,double>
        {
        public:
            inline GaussianMISEFunctor(const GaussianMixture1D& mix,
                                       const unsigned maxPolyDegree,
                                       const unsigned long npoints)
                : mix_(mix), npt_(npoints), polyDeg_(maxPolyDegree) {}
            inline virtual ~GaussianMISEFunctor() {}

            inline long double operator()(const double& bandwidth) const
                {return mix_.gmise(bandwidth, polyDeg_, npt_);}

        private:
            const GaussianMixture1D& mix_;
            const unsigned long npt_;
            const unsigned polyDeg_;
        };

        inline GaussianMixture1D(double location, double scale)
            : AbsScalableDistribution1D(location, scale) {}
        GaussianMixture1D(const double location, const double scale,
                          const std::vector<double>& params);
        inline static int nParameters() {return -1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledMean() const;
        double unscaledStdev() const;

        long double U(long double h, unsigned s, unsigned q) const;
        long double gmise(long double h, unsigned deg, unsigned long n) const;

        std::vector<GaussianMixtureEntry> entries_;
        std::vector<double> weightCdf_;
        std::vector<Gauss1D> distros_;
        mutable std::vector<long double> polyterms_;
        mutable std::vector<std::pair<long double,long double> > uterms_;
    };
}

#endif // NPSTAT_GAUSSIANMIXTURE1D_HH_
