#ifndef NPSTAT_COPULAS_HH_
#define NPSTAT_COPULAS_HH_

/*!
// \file Copulas.hh
//
// \brief Concrete copula distributions
//
// Author: I. Volobouev
//
// September 2010
*/

#include "npstat/nm/Matrix.hh"

#include "npstat/stat/AbsDistributionND.hh"
#include "npstat/stat/Distributions1D.hh"

namespace npstat {
    // Forward declarations
    class NMCombinationSequencer;


    /** The Gaussian copula */
    class GaussianCopula : public AbsDistributionND
    {
    public:
        explicit GaussianCopula(const Matrix<double>& covmat);
        virtual inline ~GaussianCopula() {}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned bufLen, double* x) const;
        inline bool mappedByQuantiles() const {return false;}

        virtual inline GaussianCopula* clone() const
            {return new GaussianCopula(*this);}
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::GaussianCopula";}
        static inline unsigned version() {return 1;}
        static GaussianCopula* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual inline bool isEqual(const AbsDistributionND& ri) const
        {
            const GaussianCopula& o = static_cast<const GaussianCopula&>(ri);
            return norm_ == o.norm_ && form_ == o.form_ && 
                   sqrCov_ == o.sqrCov_;
        }

    private:
        inline explicit GaussianCopula(unsigned nDim)
            : AbsDistributionND(nDim) {}

        Matrix<double> form_;
        Matrix<double> sqrCov_;
        double norm_;
        mutable std::vector<double> buf_;
    };


    /**
    // The Farlie-Gumbel-Morgenstern copula. It is basically defined by its
    // values at all vertices of the n-dimensional unit cube. All values
    // inside the cube are obtained by multilinear interpolation. Naturally,
    // the values at the vertices can not be arbitrary: they must satisfy
    // certain constraints for the function to be a copula, so the
    // representation in terms of independent parameters is a bit complicated.
    // Then those parameters themselves must be selected in such a way that
    // the function is non-negative in each of the 2^n vertices of the cube,
    // so this representation is far from ideal. It is described in the book
    // by R.B. Nelsen, "An introduction to Copulas", 2nd Ed. (2006), page 108.
    // There should be a better way to do this...
    */
    class FGMCopula : public AbsDistributionND
    {
    public:
        /**
        // The order of parameters is the following: theta_1_2,
        // theta_1_3, ..., theta_1_dim, theta_2_3, ..., theta_2_dim, ...
        // theta_dim-1_dim, theta_1_2_3, ...
        //
        // There must be 2^dim - dim - 1 parameters total. Maximum
        // dimensionality is 31. Invalid parameter set (in particular,
        // a set resulting in possible negative values of the calculated
        // copula) will result in std::invalid_argument exception thrown.
        */
        FGMCopula(unsigned dim, const double* params, unsigned nParams);
        virtual inline ~FGMCopula() {}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned bufLen, double* x) const;
        inline bool mappedByQuantiles() const {return true;}

        virtual inline FGMCopula* clone() const {return new FGMCopula(*this);}
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::FGMCopula";}
        static inline unsigned version() {return 1;}
        static FGMCopula* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual inline bool isEqual(const AbsDistributionND& ri) const
        {
            const FGMCopula& o = static_cast<const FGMCopula&>(ri);
            return cornerValues_ == o.cornerValues_;
        }

    private:
        inline explicit FGMCopula(unsigned nDim) : AbsDistributionND(nDim) {}

        static double interpolate(const double* corners,
                                  const double* x, unsigned dim);
        static double density0(NMCombinationSequencer* sequencers,
                               const double* par, const double* x,
                               unsigned dim);

        std::vector<double> cornerValues_;
    };


    /** The Student's-t copula */
    class TCopula : public AbsDistributionND
    {
    public:
        TCopula(const Matrix<double>& covmat, double nDegreesOfFreedom);
        virtual inline ~TCopula() {}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned bufLen, double* x) const;
        inline bool mappedByQuantiles() const {return false;}
        inline double nDegreesOfFreedom() const {return nDoF_;}

        virtual inline TCopula* clone() const {return new TCopula(*this);}
 
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::TCopula";}
        static inline unsigned version() {return 1;}
        static TCopula* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual inline bool isEqual(const AbsDistributionND& ri) const
        {
            const TCopula& o = static_cast<const TCopula&>(ri);
            return nDoF_ == o.nDoF_ && norm_ == o.norm_ && form_ == o.form_ &&
                   sqrCov_ == o.sqrCov_ && power_ == o.power_;
        }

    private:
        inline TCopula(unsigned nDim, double ndof)
            : AbsDistributionND(nDim), t_(0.0, 1.0, ndof) {}

        Matrix<double> form_;
        Matrix<double> sqrCov_;
        double nDoF_;
        double norm_;
        double power_;
        StudentsT1D t_;
        mutable std::vector<double> buf_;
    };


    // Archimedean copulas -- to be implemented
}

#endif // NPSTAT_COPULAS_HH_
