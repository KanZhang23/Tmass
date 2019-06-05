#ifndef NPSTAT_DISTRIBUTIONSND_HH_
#define NPSTAT_DISTRIBUTIONSND_HH_

/*!
// \file DistributionsND.hh
//
// \brief A number of useful multivariate continuous statistical distributions
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/stat/AbsDistributionND.hh"
#include "npstat/stat/GridRandomizer.hh"
#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    /**
    // A generic product distribution. The argument distributions
    // will be cloned internally.
    */
    class ProductDistributionND : public AbsDistributionND
    {
    public:
        ProductDistributionND(const AbsDistribution1D** marginals,
                              unsigned nMarginals);

        ProductDistributionND(const ProductDistributionND& r);
        ProductDistributionND& operator=(const ProductDistributionND& r);

        virtual ~ProductDistributionND();

        inline virtual ProductDistributionND* clone() const
            {return new ProductDistributionND(*this);}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned bufLen, double* x) const;
        inline bool mappedByQuantiles() const {return true;}

        /** 
        // Check whether all marginals are instances of
        // AbsScalableDistribution1D
        */
        bool isScalable() const;

        /** Get the marginal distribution with the given index */
        inline AbsDistribution1D* getMarginal(const unsigned i)
            {return marginals_.at(i);}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::ProductDistributionND";}
        static inline unsigned version() {return 1;}
        static ProductDistributionND* read(const gs::ClassId& id,
                                           std::istream& in);
    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        inline explicit ProductDistributionND(const unsigned n)
            : AbsDistributionND(n) {}
        void cleanup();

        std::vector<AbsDistribution1D*> marginals_;
    };

    /** Multivariate uniform distribution */
    class UniformND : public AbsScalableDistributionND
    {
    public:
        inline UniformND(const double* location, const double* scale,
                         const unsigned dim)
            : AbsScalableDistributionND(location, scale, dim) {}

        inline virtual ~UniformND() {}

        inline virtual UniformND* clone() const {return new UniformND(*this);}
        inline bool mappedByQuantiles() const {return true;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;

        static inline const char* classname() {return "npstat::UniformND";}
        static inline unsigned version() {return 1;}
        static UniformND* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND& r) const
            {return AbsScalableDistributionND::isEqual(r);}

    private:
        double unscaledDensity(const double* x) const;
        void unscaledUnitMap(const double* rnd, unsigned bufLen,
                             double* x) const;
    };

    /**
    // Multivariate symmetric distribution proportional to pow(1.0 - R^2, n)
    // with the support region defined by 0 <= R <= 1, where R is the
    // distance (after scaling) to the distribution location point.
    // Can be scaled separately in each dimension.
    */
    class ScalableSymmetricBetaND : public AbsScalableDistributionND
    {
    public:
        ScalableSymmetricBetaND(const double* location, const double* scale,
                                unsigned dim, double n);
        inline virtual ScalableSymmetricBetaND* clone() const
            {return new ScalableSymmetricBetaND(*this);}

        inline virtual ~ScalableSymmetricBetaND() {}

        inline bool mappedByQuantiles() const {return false;}
        inline double power() const {return power_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname()
            {return "npstat::ScalableSymmetricBetaND";}

        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        double unscaledDensity(const double* x) const;
        void unscaledUnitMap(const double* rnd, unsigned bufLen,
                             double* x) const;

        double radialQuantile(double cdf) const;

        double power_;
        double norm_;
    };

    /**
    // Multidimensional Huber-like function. Gaussian near the
    // coordinate origin and exponential when radius is above
    // certain threshold. "tailWeight" parameter is the weight
    // of the exponential tail. It must be between 0 and 1.
    */
    class ScalableHuberND : public AbsScalableDistributionND
    {
    public:
        ScalableHuberND(const double* location, const double* scale,
                        unsigned dim, double tailWeight);
        inline virtual ScalableHuberND* clone() const
            {return new ScalableHuberND(*this);}

        inline virtual ~ScalableHuberND() {}

        inline bool mappedByQuantiles() const {return false;}

        inline double tailWeight() const {return tWeight_;}
        inline double transition() const {return a_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname()
            {return "npstat::ScalableHuberND";}

        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        double unscaledDensity(const double* x) const;
        void unscaledUnitMap(const double* rnd, unsigned bufLen,
                             double* x) const;

        // Weight of the Gaussian part
        double norm1(double a) const;

        // Weight of the exponential part
        double norm2(double a) const;

        double radialQuantile(double cdf) const;

        double tWeight_;
        double a_;
        double normfactor_;
        double const1_;
        double const2_;
    };

    /** Product of symmetric beta distributions in each dimension */
    struct ProductSymmetricBetaND : 
        public HomogeneousProductDistroND<SymmetricBeta1D>
    {
        ProductSymmetricBetaND(const double* location, const double* scale,
                               unsigned dim, double power);
        inline virtual ProductSymmetricBetaND* clone() const
            {return new ProductSymmetricBetaND(*this);}

        inline virtual ~ProductSymmetricBetaND() {}

        inline double power() const {return marginals_[0].power();}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname()
            {return "npstat::ProductSymmetricBetaND";}

        static inline unsigned version() {return 1;}
    };

    /** Distribution defined by the interpolation table of its radial profile */
    class RadialProfileND : public AbsScalableDistributionND
    {
    public:
        /**
        // "location", "scale", and "dim" are the usual multivariate
        // density parameters which define the distribution location,
        // scale in each dimension, and the number of dimensions.
        //
        // The "data" array gives density values, (d Prob)/(dx_1 ... dx_n),
        // at equidistant radial intervals. data[0] is density at r = 0.0,
        // and data[dataLen-1] is density at r = 1.0. If "dataLen" is
        // less than 2, uniform distribution will be created. The total
        // probability content will be automatically normalized to 1, so
        // the "data" values do not have to be normalized by the user.
        // Internally, the profile is kept in double precision.
        //
        // "interpolationDegree" is the order of the radial profile
        // interpolation polynomial. It must be less than 4 and less
        // than "dataLen".
        */
        template <typename Real>
        RadialProfileND(const double* location, const double* scale,
                        unsigned dim,
                        const Real* data, unsigned dataLen,
                        unsigned interpolationDegree);

        inline virtual RadialProfileND* clone() const
            {return new RadialProfileND(*this);}

        inline virtual ~RadialProfileND() {}

        inline bool mappedByQuantiles() const {return false;}

        inline unsigned interpolationDegree() const {return deg_;}
        inline unsigned profileLength() const {return len_;}
        inline const double* profileData() const {return &profile_[0];}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;

        static inline const char* classname()
            {return "npstat::RadialProfileND";}
        static inline unsigned version() {return 1;}
        static RadialProfileND* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        inline RadialProfileND(const double* location, const double* scale,
                               const unsigned dim)
            : AbsScalableDistributionND(location, scale, dim) {}

        double unscaledDensity(const double* x) const;
        void unscaledUnitMap(const double* rnd, unsigned bufLen,
                             double* x) const;
        void calculateQuadratureCoeffs();
        void normalize();
        double radialPofile(double r) const;
        double intervalInteg(unsigned intervalNumber,
                             double stepFraction=1.0) const;
        double intervalIntegLowD(unsigned intervalNumber,
                                 double stepFraction=1.0) const;
        double radialQuantile(double cdf) const;

        std::vector<double> profile_;
        std::vector<double> cdf_;
        double ndSphereVol_;
        double step_;
        unsigned len_;
        unsigned deg_;

        // Coefficients for Gaussian quadratures
        double xg_[4], wg_[4];
    };

    /**
    // Distribution defined by an interpolation table inside
    // the unit box (which can be shifted and scaled). All grid
    // points are inside the box, in the bin centers. Currently,
    // interpolationDegree could be only 0 (faster, no interpolation)
    // or 1 (multilinear interpolation).
    */
    class BinnedDensityND : public AbsScalableDistributionND
    {
    public:
        template <typename Num1, unsigned Len1, unsigned Dim1>
        BinnedDensityND(const double* location, const double* scale,
                        unsigned locationAndScaleLength,
                        const ArrayND<Num1,Len1,Dim1>& histogram,
                        const unsigned interpolationDegree);

        inline virtual BinnedDensityND* clone() const
            {return new BinnedDensityND(*this);}

        inline virtual ~BinnedDensityND() {}

        inline bool mappedByQuantiles() const {return true;}

        inline const ArrayND<double>& gridData() const
            {return randomizer_.gridData();}

        inline unsigned interpolationDegree() const
            {return randomizer_.interpolationDegree();}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::BinnedDensityND";}
        static inline unsigned version() {return 1;}
        static BinnedDensityND* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        inline double unscaledDensity(const double* x) const
            {return randomizer_.density(x, dim_);}

        inline void unscaledUnitMap(const double* rnd,
                                    const unsigned bufLen, double* x) const
            {randomizer_.generate(rnd, bufLen, x);}

        GridRandomizer randomizer_;
    };
}

#include "npstat/stat/DistributionsND.icc"

#endif // NPSTAT_DISTRIBUTIONSND_HH_
