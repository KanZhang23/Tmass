#ifndef NPSTAT_STORABLEPOLYSERIES1D_HH_
#define NPSTAT_STORABLEPOLYSERIES1D_HH_

/*!
// \file StorablePolySeries1D.hh
//
// \brief Storable functor for orthogonal polynomial series
//
// Author: I. Volobouev
//
// November 2018
*/

#include <vector>
#include <typeinfo>

#include "geners/ClassId.hh"

#include "npstat/nm/SimpleFunctors.hh"
#include "npstat/nm/AbsClassicalOrthoPoly1D.hh"

namespace npstat {
    class StorablePolySeries1D : public Functor1<double,double>,
                                 private AbsClassicalOrthoPoly1D
    {
    public:
        /**
        // The best way to create an object of this class is to call
        // "makeStorablePolySeries" method of either ContOrthoPoly1D
        // or AbsClassicalOrthoPoly1D class
        */
        StorablePolySeries1D(
            const std::vector<std::pair<long double,long double> >& rCoeffs,
            double xmin, double xmax, double shift = 0.0,
            const double *coeffs = 0, unsigned maxdeg = 0);

        inline virtual ~StorablePolySeries1D() {}

        inline virtual double operator()(const double& x) const
        {
            return AbsClassicalOrthoPoly1D::series(
                &coeffs_[0], coeffs_.size()-1U, x-shift_);
        }

        //@{
        /** A simple inspector of object properties */
        inline double xmin() const {return xmin_;}
        inline double xmax() const {return xmax_;}
        inline unsigned maxDegCoeffs() const {return coeffs_.size()-1U;}
        inline double getCoefficient(const unsigned i) {return coeffs_.at(i);}
        inline unsigned maxDegPoly() const {return this->maxDegree();}
        inline double getShift() const {return shift_;}
        //@}

        inline double poly(const unsigned deg, const double x) const
            {return AbsClassicalOrthoPoly1D::poly(deg, x-shift_);}

        inline double series(const double* coeffs, const unsigned maxdeg,
                             const double x) const
            {return AbsClassicalOrthoPoly1D::series(coeffs, maxdeg, x-shift_);}

        inline double integratePoly(const unsigned degree,
                                    const unsigned power) const
            {return integratePoly(degree, power, xmin_, xmax_);}

        double integratePoly(unsigned degree, unsigned power,
                             double xmin, double xmax) const;

        /** Set the series coefficients */
        void setCoeffs(const double *coeffs, unsigned maxdeg);

        inline bool operator==(const StorablePolySeries1D& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}
        inline bool operator!=(const StorablePolySeries1D& r) const
            {return !(*this == r);}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::StorablePolySeries1D";}
        static inline unsigned version() {return 1;}
        static StorablePolySeries1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const StorablePolySeries1D& other) const;

    private:
        inline StorablePolySeries1D() {}

        inline virtual StorablePolySeries1D* clone() const
            {return new StorablePolySeries1D(*this);}

        inline virtual std::pair<long double,long double>
        recurrenceCoeffs(const unsigned deg) const {return recur_.at(deg);}

        inline virtual unsigned maxDegree() const {return recur_.size() - 1U;}

        virtual long double weight(long double x) const;

        std::vector<double> coeffs_;
        std::vector<std::pair<long double,long double> > recur_;
        double xmin_;
        double xmax_;
        double shift_;
    };
}

#endif // NPSTAT_STORABLEPOLYSERIES1D_HH_
