#ifndef NPSTAT_RATIOOFNORMALS_HH_
#define NPSTAT_RATIOOFNORMALS_HH_

/*!
// \file RatioOfNormals.hh
//
// \brief Density of the ratio of two correlated normal random variables
//
// Author: I. Volobouev
//
// November 2013
*/

#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // Formulae used in the implementation of this class come from
    // D.V. Hinkley, "On the ratio of two correlated normal random variables",
    // Biometrika, v. 56, p. 635 (1969).
    */
    class RatioOfNormals : public AbsDistribution1D
    {
    public:
        /**
        // The constructor arguments are as follows:
        //
        // "mu1" and "s1" are the mean and the standard deviation of the
        // normal random variable in the numerator.
        //
        // "mu2" and "s2" are the mean and the standard deviation of the
        // normal random variable in the denominator.
        //
        // "rho" is the correlation coefficient.
        */
        RatioOfNormals(double mu1, double s1, double mu2, double s2, double rho);

        inline virtual ~RatioOfNormals() {}

        inline virtual RatioOfNormals* clone() const
            {return new RatioOfNormals(*this);}

        virtual double density(double x) const;
        virtual double cdf(double x) const;
        virtual double exceedance(double x) const;
        virtual double quantile(double x) const;
        virtual unsigned random(AbsRandomGenerator& g,
                                double* generatedRandom) const;

        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;

        static inline const char* classname() {return "npstat::RatioOfNormals";}
        static inline unsigned version() {return 1;}
        static RatioOfNormals* read(const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const;

    private:
        RatioOfNormals();

        double mu1_;
        double s1_;
        double mu2_;
        double s2_;
        double rho_;
        double support_;
    };
}

#endif // NPSTAT_RATIOOFNORMALS_HH_
