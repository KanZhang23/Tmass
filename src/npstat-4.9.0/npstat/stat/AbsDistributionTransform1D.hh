#ifndef NPSTAT_ABSDISTRIBUTIONTRANSFORM1D_HH_
#define NPSTAT_ABSDISTRIBUTIONTRANSFORM1D_HH_

/*!
// \file AbsDistributionTransform1D.hh
//
// \brief Interface definition for 1-d coordinate transformations used
//        to build statistical distributions
//
// Author: I. Volobouev
//
// April 2015
*/

#include <cassert>
#include <typeinfo>
#include <stdexcept>

#include "geners/ClassId.hh"

namespace npstat {
    /**
    // It will be assumed that transforms implementing this interface
    // are monotonous
    */
    struct AbsDistributionTransform1D
    {
        inline explicit AbsDistributionTransform1D(const unsigned nParams)
            : npara_(nParams) {}

        inline virtual ~AbsDistributionTransform1D() {}

        /** "Virtual copy constructor" */
        virtual AbsDistributionTransform1D* clone() const = 0;

        inline unsigned nParameters() const {return npara_;}

        inline void setParameter(const unsigned which, const double value)
        {
            if (which >= npara_) throw std::invalid_argument(
                "In npstat::AbsDistributionTransform1D::setParameter: "
                "parameter number out of range");
            this->setParameterChecked(which, value);
        }

        inline void setAllParameters(const double* p, const unsigned len)
        {
            if (len != npara_) throw std::invalid_argument(
                "In npstat::AbsDistributionTransform1D::setAllParameters: "
                "wrong number of parameters");
            if (len)
                assert(p);
            this->setAllParametersChecked(p);
        }

        inline double getParameter(const unsigned which) const
        {
            if (which >= npara_) throw std::invalid_argument(
                "In npstat::AbsDistributionTransform1D::getParameter: "
                "parameter number out of range");
            return this->getParameterChecked(which);
        }

        virtual double transformForward(double x, double* dydx) const = 0;
        virtual double transformBack(double y) const = 0;

        /** Is y increasing or decreasing as a function of x? */
        virtual bool isIncreasing() const = 0;

        /**
        // Derived classes should not implement "operator==", implement
        // "isEqual" instead
        */
        inline bool operator==(const AbsDistributionTransform1D& r) const
        {
            return (typeid(*this) == typeid(r)) &&
                   npara_ == r.npara_ && this->isEqual(r);
        }

        /** Logical negation of operator== */
        inline bool operator!=(const AbsDistributionTransform1D& r) const
            {return !(*this == r);}

        //@{
        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream&) const {return false;}
        //@}

        static inline const char* classname()
            {return "npstat::AbsDistributionTransform1D";}
        static inline unsigned version() {return 1;}
        static AbsDistributionTransform1D* read(
            const gs::ClassId& id, std::istream& is);

    protected:
        /** Comparison for equality. To be implemented by derived classes. */
        virtual bool isEqual(const AbsDistributionTransform1D&) const = 0;

    private:
        virtual void setParameterChecked(unsigned which, double value) = 0;
        virtual void setAllParametersChecked(const double* p) = 0;
        virtual double getParameterChecked(unsigned which) const = 0;

        unsigned npara_;
    };
}

#endif // NPSTAT_ABSDISTRIBUTIONTRANSFORM1D_HH_
