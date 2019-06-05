#ifndef NPSTAT_JOHNSONCURVES_HH_
#define NPSTAT_JOHNSONCURVES_HH_

/*!
// \file JohnsonCurves.hh
//
// \brief Johnson frequency curves
//
// The S_u distribution is rather easy to fit by moments, and the
// fitting procedure works well. The S_b is much harder to fit, and
// the implementation may be rough at the moment. It is, however,
// better than any previously published algorithm.
//
// Author: I. Volobouev
//
// April 2010
*/

#include "npstat/stat/Distribution1DFactory.hh"

namespace npstat {
    /** Johnson S_u (unbounded) curve */
    class JohnsonSu : public AbsScalableDistribution1D
    {
    public:
        JohnsonSu(double location, double scale,
                  double skewness, double kurtosis);
        inline virtual JohnsonSu* clone() const {return new JohnsonSu(*this);}

        inline virtual ~JohnsonSu() {}

        inline double skewness() const {return skew_;}
        inline double kurtosis() const {return kurt_;}
        inline bool isValid() const {return isValid_;}

        inline double getDelta() const {return delta_;}
        inline double getLambda() const {return lambda_;}
        inline double getGamma() const {return gamma_;}
        inline double getXi() const {return xi_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::JohnsonSu";}
        static inline unsigned version() {return 1;}
        static JohnsonSu* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<JohnsonSu>;

        JohnsonSu(double location, double scale,
                  const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        // The I/O helper constructor
        inline JohnsonSu(double location, double scale)
            : AbsScalableDistribution1D(location, scale) {}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        void initialize();

        double skew_;
        double kurt_;

        double delta_;
        double lambda_;
        double gamma_;
        double xi_;

        bool isValid_;
    };

    /** Johnson S_b (bounded) curve */
    class JohnsonSb : public AbsScalableDistribution1D
    {
    public:
        JohnsonSb(double location, double scale,
                  double skewness, double kurtosis);
        inline virtual JohnsonSb* clone() const {return new JohnsonSb(*this);}

        inline virtual ~JohnsonSb() {}

        inline double skewness() const {return skew_;}
        inline double kurtosis() const {return kurt_;}
        inline bool isValid() const {return isValid_;}

        inline double getDelta() const {return delta_;}
        inline double getLambda() const {return lambda_;}
        inline double getGamma() const {return gamma_;}
        inline double getXi() const {return xi_;}

        static bool fitParameters(double skewness, double kurtosis,
                                  double *gamma, double *delta,
                                  double *lambda, double *xi);

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::JohnsonSb";}
        static inline unsigned version() {return 1;}
        static JohnsonSb* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<JohnsonSb>;

        JohnsonSb(double location, double scale,
                  const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        // The I/O helper constructor
        inline JohnsonSb(double location, double scale)
            : AbsScalableDistribution1D(location, scale) {}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledExceedance(double x) const;
        double unscaledQuantile(double x) const;

        double skew_;
        double kurt_;

        double delta_;
        double lambda_;
        double gamma_;
        double xi_;

        bool isValid_;
    };

    /** This class selects an appropriate Johnson curve automatically */
    class JohnsonSystem : public AbsScalableDistribution1D
    {
    public:
        enum CurveType {
            GAUSSIAN = 0,
            LOGNORMAL,
            SU,
            SB,
            INVALID
        };

        JohnsonSystem(double location, double scale,
                      double skewness, double kurtosis);
        JohnsonSystem(const JohnsonSystem&);
        JohnsonSystem& operator=(const JohnsonSystem&);
        virtual ~JohnsonSystem();

        inline virtual JohnsonSystem* clone() const
            {return new JohnsonSystem(*this);}

        inline double skewness() const {return skew_;}
        inline double kurtosis() const {return kurt_;}
        inline CurveType curveType() const {return curveType_;}
        inline bool isValid() const {return curveType_ != INVALID;}

        static CurveType select(double skewness, double kurtosis);

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::JohnsonSystem";}
        static inline unsigned version() {return 1;}
        static JohnsonSystem* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<JohnsonSystem>;

        JohnsonSystem(double location, double scale,
                      const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        // The I/O helper constructor
        inline JohnsonSystem(double location, double scale)
            : AbsScalableDistribution1D(location, scale), fcn_(0) {}

        void initialize();

        inline double unscaledDensity(double x) const
            {if (fcn_) return fcn_->density(x); else return -1.0;}
        inline double unscaledCdf(double x) const
            {if (fcn_) return fcn_->cdf(x); else return -1.0;}
        inline double unscaledExceedance(double x) const
            {if (fcn_) return fcn_->exceedance(x); else return -1.0;}
        inline double unscaledQuantile(double x) const
            {if (fcn_) return fcn_->quantile(x); else return 0.0;}

        AbsScalableDistribution1D* fcn_;
        double skew_;
        double kurt_;
        CurveType curveType_;
    };
}

#endif // NPSTAT_JOHNSONCURVES_HH_
