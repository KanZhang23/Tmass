#ifndef NPSTAT_DISTRIBUTIONS1D_HH_
#define NPSTAT_DISTRIBUTIONS1D_HH_

/*!
// \file Distributions1D.hh
//
// \brief A number of useful 1-d continuous statistical distributions
//
// Author: I. Volobouev
//
// November 2009
*/

#include "npstat/stat/Distribution1DFactory.hh"

namespace npstat {
    /**
    // The uniform distribution is defined here by a constant density
    // equal to 1 between 0 and 1 and equal to 0 everywhere else
    */
    class Uniform1D : public AbsScalableDistribution1D
    {
    public:
        inline Uniform1D(const double location, const double scale)
            : AbsScalableDistribution1D(location, scale) {}
        inline virtual Uniform1D* clone() const {return new Uniform1D(*this);}

        inline virtual ~Uniform1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Uniform1D";}
        static inline unsigned version() {return 1;}
        static Uniform1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Uniform1D>;

        inline Uniform1D(const double location, const double scale,
                         const std::vector<double>& /* params */)
            : AbsScalableDistribution1D(location, scale) {}
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}
    };

    /** Isosceles triangle distribution: 1 - |x| supported on [-1, 1] */
    class IsoscelesTriangle1D : public AbsScalableDistribution1D
    {
    public:
        inline IsoscelesTriangle1D(const double location, const double scale)
            : AbsScalableDistribution1D(location, scale) {}
        inline virtual IsoscelesTriangle1D* clone() const
            {return new IsoscelesTriangle1D(*this);}

        inline virtual ~IsoscelesTriangle1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname()
            {return "npstat::IsoscelesTriangle1D";}
        static inline unsigned version() {return 1;}
        static IsoscelesTriangle1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<IsoscelesTriangle1D>;

        inline IsoscelesTriangle1D(const double location, const double scale,
                                   const std::vector<double>& /* params */)
            : AbsScalableDistribution1D(location, scale) {}
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}
    };

    /** Exponential distribution. "scale" is the decay time. */
    class Exponential1D : public AbsScalableDistribution1D
    {
    public:
        inline Exponential1D(const double location, const double scale)
            : AbsScalableDistribution1D(location, scale) {}
        inline virtual Exponential1D* clone() const
            {return new Exponential1D(*this);}

        inline virtual ~Exponential1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Exponential1D";}
        static inline unsigned version() {return 1;}
        static Exponential1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Exponential1D>;
        
        inline Exponential1D(const double location, const double scale,
                             const std::vector<double>& /* params */)
            : AbsScalableDistribution1D(location, scale) {}
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;
    };

    /** Logistic distribution */
    class Logistic1D : public AbsScalableDistribution1D
    {
    public:
        inline Logistic1D(const double location, const double scale)
            : AbsScalableDistribution1D(location, scale) {}
        inline virtual Logistic1D* clone() const
            {return new Logistic1D(*this);}

        inline virtual ~Logistic1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Logistic1D";}
        static inline unsigned version() {return 1;}
        static Logistic1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Logistic1D>;
        
        inline Logistic1D(const double location, const double scale,
                          const std::vector<double>& /* params */)
            : AbsScalableDistribution1D(location, scale) {}
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;
    };

    /**
    // A distribution whose density has a simple quadratic shape.
    // The support is from 0 to 1, and the coefficients "a" and "b"
    // are the coefficients for the Legendre polynomials of 1st
    // and 2nd degree translated to the support region. Note that
    // only those values of "a" and "b" that guarantee non-negativity
    // of the density are allowed, otherwise the code will generate
    // a run-time error.
    */
    class Quadratic1D : public AbsScalableDistribution1D
    {
    public:
        Quadratic1D(double location, double scale, double a, double b);
        inline virtual Quadratic1D* clone() const
            {return new Quadratic1D(*this);}

        inline virtual ~Quadratic1D() {}

        inline double a() const {return a_;}
        inline double b() const {return b_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Quadratic1D";}
        static inline unsigned version() {return 2;}
        static Quadratic1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Quadratic1D>;
        
        Quadratic1D(double location, double scale,
                    const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        void verifyNonNegative();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}

        double a_;
        double b_;
    };

    /**
    // A distribution whose density logarithm has a simple quadratic
    // shape. The support is from 0 to 1, and the coefficients "a" and "b"
    // are the coefficients for the Legendre polynomials of 1st and 2nd
    // degree translated to the support region.
    */
    class LogQuadratic1D : public AbsScalableDistribution1D
    {
    public:
        LogQuadratic1D(double location, double scale, double a, double b);
        inline virtual LogQuadratic1D* clone() const
            {return new LogQuadratic1D(*this);}

        inline virtual ~LogQuadratic1D() {}

        inline double a() const {return a_;}
        inline double b() const {return b_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::LogQuadratic1D";}
        static inline unsigned version() {return 2;}
        static LogQuadratic1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<LogQuadratic1D>;

        LogQuadratic1D(double location, double scale,
                       const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        void normalize();
        long double quadInteg(long double x) const;
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}

        long double ref_;
        long double range_;
        double a_;
        double b_;
        double k_;
        double s_;
        double norm_;
    };

    /** The Gaussian (or Normal) distribution */
    class Gauss1D : public AbsScalableDistribution1D
    {
    public:
        inline Gauss1D(const double location, const double scale)
            : AbsScalableDistribution1D(location, scale) {}
        inline virtual Gauss1D* clone() const {return new Gauss1D(*this);}

        inline virtual ~Gauss1D() {}

        // Higher quality generator than the one provided by
        // the quantile function
        virtual unsigned random(AbsRandomGenerator& g,
                                double* generatedRandom) const;

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Gauss1D";}
        static inline unsigned version() {return 1;}
        static Gauss1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Gauss1D>;

        inline Gauss1D(const double location, const double scale,
                       const std::vector<double>& /* params */)
            : AbsScalableDistribution1D(location, scale) {}
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;
    };

    /** Gaussian distribution truncated at some number of sigmas */
    class TruncatedGauss1D : public AbsScalableDistribution1D
    {
    public:
        TruncatedGauss1D(double location, double scale, double nsigma);
        inline virtual TruncatedGauss1D* clone() const
            {return new TruncatedGauss1D(*this);}

        inline virtual ~TruncatedGauss1D() {}

        inline double nsigma() const {return nsigma_;}

        // Higher quality generator than the one provided by
        // the quantile function
        virtual unsigned random(AbsRandomGenerator& g,
                                double* generatedRandom) const;

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::TruncatedGauss1D";}
        static inline unsigned version() {return 1;}
        static TruncatedGauss1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<TruncatedGauss1D>;

        TruncatedGauss1D(double location, double scale,
                         const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return unscaledCdf(-x);}

        double nsigma_;
        double norm_;
        double cdf0_;
    };

    /**
    // Gaussian distribution on the [0, 1] interval, mirrored at the boundaries.
    // This is the Green's function of the diffusion equation on [0, 1]. The
    // interval can be shifted and scaled as for the uniform distribution.
    */
    class MirroredGauss1D : public AbsScalableDistribution1D
    {
    public:
        MirroredGauss1D(double location, double scale,
                        double meanOn0_1, double sigmaOn0_1);

        inline virtual MirroredGauss1D* clone() const
            {return new MirroredGauss1D(*this);}

        inline virtual ~MirroredGauss1D() {}

        inline double meanOn0_1() const {return mu0_;}
        inline double sigmaOn0_1() const {return sigma0_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::MirroredGauss1D";}
        static inline unsigned version() {return 1;}
        static MirroredGauss1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<MirroredGauss1D>;

        MirroredGauss1D(double location, double scale,
                        const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        void validate();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;
        long double ldCdf(double x) const;

        double mu0_;
        double sigma0_;
    };

    /**
    // Bifurcated Gaussian distribution. Different sigmas
    // can be used on the left and on the right, with constructor
    // parameter "leftSigmaFraction" specifying the ratio of
    // the left sigma to the sum of sigmas (this ratio must be
    // between 0 and 1). Different truncations in terms of the
    // number of sigmas can be used as well.
    */
    class BifurcatedGauss1D : public AbsScalableDistribution1D
    {
    public:
        BifurcatedGauss1D(double location, double scale,
                          double leftSigmaFraction,
                          double nSigmasLeft, double nSigmasRight);
        inline virtual BifurcatedGauss1D* clone() const
            {return new BifurcatedGauss1D(*this);}

        inline virtual ~BifurcatedGauss1D() {}

        inline double leftSigmaFraction() const
            {return leftSigma_/(leftSigma_ + rightSigma_);}
        inline double nSigmasLeft() const
            {return nSigmasLeft_;}
        inline double nSigmasRight() const
            {return nSigmasRight_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::BifurcatedGauss1D";}
        static inline unsigned version() {return 1;}
        static BifurcatedGauss1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        BifurcatedGauss1D(double location, double scale);

        friend class ScalableDistribution1DFactory<BifurcatedGauss1D>;

        BifurcatedGauss1D(double location, double scale,
                          const std::vector<double>& params);
        inline static int nParameters() {return 3;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;

        double leftSigma_;
        double rightSigma_;
        double nSigmasLeft_;
        double nSigmasRight_;
        double norm_;
        double leftCdfFrac_;
        double cdf0Left_;
        double cdf0Right_;
    };

    /** Symmetric beta distribution */
    class SymmetricBeta1D : public AbsScalableDistribution1D
    {
    public:
        SymmetricBeta1D(double location, double scale, double power);
        inline virtual SymmetricBeta1D* clone() const
            {return new SymmetricBeta1D(*this);}

        inline virtual ~SymmetricBeta1D() {}

        inline double power() const {return n_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::SymmetricBeta1D";}
        static inline unsigned version() {return 1;}
        static SymmetricBeta1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<SymmetricBeta1D>;

        SymmetricBeta1D(double location, double scale,
                        const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return unscaledCdf(-x);}

        double calculateNorm() const;

        double n_;
        double norm_;
    };

    /** Beta1D density is proportional to x^(apha-1) * (1-x)^(beta-1) */
    class Beta1D : public AbsScalableDistribution1D
    {
    public:
        Beta1D(double location, double scale, double alpha, double beta);
        inline virtual Beta1D* clone() const
            {return new Beta1D(*this);}

        inline virtual ~Beta1D() {}

        inline double alpha() const {return alpha_;}
        inline double beta() const {return beta_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Beta1D";}
        static inline unsigned version() {return 1;}
        static Beta1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Beta1D>;

        Beta1D(double location, double scale,
               const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        double calculateNorm() const;

        double alpha_;
        double beta_;
        double norm_;
    };

    /** Shiftable gamma distribution */
    class Gamma1D : public AbsScalableDistribution1D
    {
    public:
        Gamma1D(double location, double scale, double alpha);
        inline virtual Gamma1D* clone() const
            {return new Gamma1D(*this);}

        inline virtual ~Gamma1D() {}

        inline double alpha() const {return alpha_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Gamma1D";}
        static inline unsigned version() {return 1;}
        static Gamma1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Gamma1D>;

        Gamma1D(double location, double scale,
                const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        void initialize();

        double alpha_;
        double norm_;
    };

    /**
    // Pareto distribution. Location parameter is location of 0, scale
    // parameter is the distance between 0 and the start of the density
    // (like the normal Pareto distribution location parameter).
    */
    class Pareto1D : public AbsScalableDistribution1D
    {
    public:
        Pareto1D(double location, double scale, double powerParameter);
        inline virtual Pareto1D* clone() const {return new Pareto1D(*this);}

        inline virtual ~Pareto1D() {}

        inline double powerParameter() const {return c_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Pareto1D";}
        static inline unsigned version() {return 1;}
        static Pareto1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Pareto1D>;

        Pareto1D(double location, double scale,
                 const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;

        double c_;
        double support_;
    };

    /**
    // Uniform distribution with Pareto tail attached to the right, where
    // the support of the uniform would normally end. Location parameter
    // is location of 0, scale parameter is the width of the uniform part
    // (like the normal Pareto distribution location parameter).
    */
    class UniPareto1D : public AbsScalableDistribution1D
    {
    public:
        UniPareto1D(double location, double scale, double powerParameter);
        inline virtual UniPareto1D* clone() const {return new UniPareto1D(*this);}

        inline virtual ~UniPareto1D() {}

        inline double powerParameter() const {return c_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::UniPareto1D";}
        static inline unsigned version() {return 1;}
        static UniPareto1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<UniPareto1D>;

        UniPareto1D(double location, double scale,
                    const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;

        double c_;
        double support_;
        double amplitude_;
    };

    /** "Huber" distribution */
    class Huber1D : public AbsScalableDistribution1D
    {
    public:
        Huber1D(double location, double scale, double tailWeight);
        inline virtual Huber1D* clone() const {return new Huber1D(*this);}

        inline virtual ~Huber1D() {}

        inline double tailWeight() const {return tailWeight_;}
        inline double tailStart() const {return a_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Huber1D";}
        static inline unsigned version() {return 1;}
        static Huber1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Huber1D>;

        Huber1D(double location, double scale,
                const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return unscaledCdf(-x);}

        double weight(double a) const;

        double tailWeight_;
        double a_;
        double normfactor_;
        double support_;
        double cdf0_;
    };

    /** Cauchy (or Breit-Wigner) distribution */
    class Cauchy1D : public AbsScalableDistribution1D
    {
    public:
        Cauchy1D(double location, double scale);
        inline virtual Cauchy1D* clone() const {return new Cauchy1D(*this);}

        inline virtual ~Cauchy1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Cauchy1D";}
        static inline unsigned version() {return 1;}
        static Cauchy1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Cauchy1D>;

        Cauchy1D(const double location, const double scale,
                 const std::vector<double>& params);
        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return unscaledCdf(-x);}

        double support_;
    };

    /**
    // Log-normal distribution represented by its mean, standard
    // deviation, and skewness. This representation is more useful
    // than other representations encountered in statistical literature.
    */
    class LogNormal : public AbsScalableDistribution1D
    {
    public:
        LogNormal(double mean, double stdev, double skewness);
        inline virtual LogNormal* clone() const {return new LogNormal(*this);}

        inline virtual ~LogNormal() {}

        inline double skewness() const {return skew_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::LogNormal";}
        static inline unsigned version() {return 1;}
        static LogNormal* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<LogNormal>;

        LogNormal(double location, double scale,
                  const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        double skew_;

        double logw_;
        double s_;
        double xi_;
        double emgamovd_;
    };

    /**
    // Moyal Distribution (originally derived by Moyal as an approximation
    // to the Landau distribution)
    */
    class Moyal1D : public AbsScalableDistribution1D
    {
    public:
        Moyal1D(double location, double scale);
        inline virtual Moyal1D* clone() const {return new Moyal1D(*this);}

        inline virtual ~Moyal1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        static inline const char* classname() {return "npstat::Moyal1D";}
        static inline unsigned version() {return 1;}
        static Moyal1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const
            {return AbsScalableDistribution1D::isEqual(r);}

    private:
        friend class ScalableDistribution1DFactory<Moyal1D>;

        Moyal1D(double location, double scale,
                const std::vector<double>& params);

        inline static int nParameters() {return 0;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;

        double xmax_;
        double xmin_;
    };

    /** Student's t-distribution */
    class StudentsT1D : public AbsScalableDistribution1D
    {
    public:
        StudentsT1D(double location, double scale, double nDegreesOfFreedom);
        inline virtual StudentsT1D* clone() const
            {return new StudentsT1D(*this);}

        inline virtual ~StudentsT1D() {}

        inline double nDegreesOfFreedom() const {return nDoF_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::StudentsT1D";}
        static inline unsigned version() {return 1;}
        static StudentsT1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<StudentsT1D>;

        StudentsT1D(double location, double scale,
                    const std::vector<double>& params);
        inline static int nParameters() {return 1;}

        void initialize();
        double effectiveSupport() const;
        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        double nDoF_;
        double normfactor_;
        double power_;
        double bignum_;
    };

    /** Distribution defined by an interpolation table */
    class Tabulated1D : public AbsScalableDistribution1D
    {
    public:
        // The "data" array gives (unnormalized) density values at
        // equidistant intervals. data[0] is density at 0.0, and
        // data[dataLen-1] is density at 1.0. If "dataLen" is less
        // than 2, uniform distribution will be created. Internally,
        // the data is kept in double precision.
        //
        // "interpolationDegree" must be less than 4 and less than "dataLen".
        //
        template <typename Real>
        Tabulated1D(double location, double scale,
                    const Real* data, unsigned dataLen,
                    unsigned interpolationDegree);

        inline Tabulated1D(const double location, const double scale,
                           const std::vector<double>& table,
                           const unsigned interpolationDegree)
            : AbsScalableDistribution1D(location, scale)
        {
            const unsigned long sz = table.size();
            initialize(sz ? &table[0] : (double*)0, sz, interpolationDegree);
        }

        inline virtual Tabulated1D* clone() const
            {return new Tabulated1D(*this);}

        inline virtual ~Tabulated1D() {}

        inline unsigned interpolationDegree() const {return deg_;}
        inline unsigned tableLength() const {return len_;}
        inline const double* tableData() const {return &table_[0];}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::Tabulated1D";}
        static inline unsigned version() {return 1;}
        static Tabulated1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<Tabulated1D>;

        // The following constructor creates interpolator
        // of maximum degree possible
        Tabulated1D(double location, double scale,
                    const std::vector<double>& params);
        inline static int nParameters() {return -1;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}

        template <typename Real> void initialize(
            const Real* data, unsigned dataLen, unsigned interpolationDegree);

        void normalize();
        double interpolate(double x) const;
        double intervalInteg(unsigned intervalNumber) const;
        double interpIntegral(double x0, double x1) const;

        std::vector<double> table_;
        std::vector<double> cdf_;
        double step_;
        unsigned len_;
        unsigned deg_;
    };

    /**
    // Another interpolated distribution. For this one, we will assume
    // that the coordinates correspond to 1-d histogram bin centers.
    */
    class BinnedDensity1D : public AbsScalableDistribution1D
    {
    public:
        // The "data" array gives density values at equidistant intervals.
        // data[0] is density at 0.5/dataLen, and data[dataLen-1] is density
        // at 1.0 - 0.5/dataLen.
        //
        // "interpolationDegree" must be less than 2.
        //
        template <typename Real>
        BinnedDensity1D(double location, double scale,
                        const Real* data, unsigned dataLen,
                        unsigned interpolationDegree);

        inline BinnedDensity1D(const double location, const double scale,
                               const std::vector<double>& table,
                               const unsigned interpolationDegree)
            : AbsScalableDistribution1D(location, scale)
        {
            const unsigned long sz = table.size();
            initialize(sz ? &table[0] : (double*)0, sz, interpolationDegree);
        }

        inline virtual BinnedDensity1D* clone() const
            {return new BinnedDensity1D(*this);}

        inline virtual ~BinnedDensity1D() {}

        inline unsigned interpolationDegree() const {return deg_;}
        inline unsigned tableLength() const {return len_;}
        inline const double* tableData() const {return &table_[0];}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::BinnedDensity1D";}
        static inline unsigned version() {return 1;}
        static BinnedDensity1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<BinnedDensity1D>;

        // The following constructor creates interpolator
        // of maximum degree possible
        BinnedDensity1D(double location, double scale,
                        const std::vector<double>& params);
        inline static int nParameters() {return -1;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}

        template <typename Real> void initialize(
            const Real* data, unsigned dataLen, unsigned interpolationDegree);

        void normalize();
        double interpolate(double x) const;

        std::vector<double> table_;
        std::vector<double> cdf_;
        double step_;
        unsigned len_;
        unsigned deg_;
        unsigned firstNonZeroBin_;
        unsigned lastNonZeroBin_;
    };
}

#include "npstat/stat/Distributions1D.icc"

#endif // NPSTAT_DISTRIBUTIONS1D_HH_
