#ifndef NPSTAT_INTERPOLATEDDISTRO1DNP_HH_
#define NPSTAT_INTERPOLATEDDISTRO1DNP_HH_

/*!
// \file InterpolatedDistro1DNP.hh
//
// \brief Interpolation of 1-d distributions as a function of multiple 
//        parameters
//
// Author: I. Volobouev
//
// July 2015
*/

#include <stdexcept>

#include "npstat/nm/GridAxis.hh"
#include "npstat/nm/ArrayND.hh"

#include "npstat/stat/AbsInterpolatedDistribution1D.hh"

namespace npstat {
    class InterpolatedDistro1DNP : public AbsDistribution1D
    {
    public:
        explicit InterpolatedDistro1DNP(const std::vector<GridAxis>& axes,
                                        bool interpolateVertically = false);

        virtual ~InterpolatedDistro1DNP();

        InterpolatedDistro1DNP(const InterpolatedDistro1DNP&);
        InterpolatedDistro1DNP& operator=(const InterpolatedDistro1DNP&);

        /** "Virtual copy constructor" */
        inline virtual InterpolatedDistro1DNP* clone() const
            {return new InterpolatedDistro1DNP(*this);}

        void interpolateVertically(bool b);
        inline bool interpolatingVertically() const {return vertical_;}

        /**
        // Set the distribution corresponding to the given parameter grid
        // point. This class will assume the ownership of the "distro" pointer.
        */
        void setGridDistro(const unsigned* cell, unsigned lenCell,
                           AbsDistribution1D* distro);

        /** Same as "setGridDistro" but using a linear index for the cell */
        void setLinearDistro(unsigned long idx, AbsDistribution1D* pd);

        //@{
        /** 
        // Lookup the distribution at the given grid cell. This method
        // returns null pointer in case the distribution has not been
        // set for that cell.
        */
        const AbsDistribution1D* getGridDistro(const unsigned* cell,
                                               unsigned lenCell) const;
        const AbsDistribution1D* getLinearDistro(unsigned long idx) const;
        //@}

        /**
        // This method must be called before calculating the density
        // (it will perform various preparations related to distribution
        // interpolation). The length of the "coords" array must be
        // equal to the number of grid axes.
        */
        void setGridCoords(const double* coords, unsigned nCoords);

        /** Number of gris axes */
        inline unsigned nAxes() const {return nAxes_;}

        /** Number of gris points (and associated distributions) */
        inline unsigned long nDistros() const {return interpolated_.length();}

        /** The shape of the rectangular grid of parameters */
        inline ArrayShape gridShape() const {return interpolated_.shape();}

        /** The grid axis for the given coordinate */
        inline const GridAxis& getAxis(const unsigned i) const
            {return axes_.at(i);}

        /** Probability density */
        inline double density(const double x) const
        {
            if (!pointSet_) throw std::logic_error(
                "In npstat::InterpolatedDistro1DNP::density: "
                "grid coordinates not set");
            return interpolator_->density(x);
        }

        /** Cumulative distribution function */
        inline double cdf(const double x) const
        {
            if (!pointSet_) throw std::logic_error(
                "In npstat::InterpolatedDistro1DNP::cdf: "
                "grid coordinates not set");
            return interpolator_->cdf(x);
        }

        /** 1 - cdf, avoiding subtractive cancellation */
        inline double exceedance(const double x) const
        {
            if (!pointSet_) throw std::logic_error(
                "In npstat::InterpolatedDistro1DNP::exceedance: "
                "grid coordinates not set");
            return interpolator_->exceedance(x);
        }

        /** The quantile function */
        inline double quantile(const double x) const
        {
            if (!pointSet_) throw std::logic_error(
                "In npstat::InterpolatedDistro1DNP::quantile: "
                "grid coordinates not set");
            return interpolator_->quantile(x);
        }

        //@{
        /** Prototype needed for I/O */
        virtual inline gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream&) const;
        //@}

        static inline const char* classname()
            {return "npstat::InterpolatedDistro1DNP";}
        static inline unsigned version() {return 1;}
        static InterpolatedDistro1DNP* read(const gs::ClassId& id,
                                            std::istream& in);
    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        InterpolatedDistro1DNP();

        void releaseDistros();
        void cloneDistros();

        std::vector<GridAxis> axes_;
        std::vector<unsigned> cellBuf_;
        ArrayND<AbsDistribution1D*> interpolated_;
        AbsInterpolatedDistribution1D* interpolator_;
        unsigned nAxes_;
        bool vertical_;
        bool pointSet_;
    };
}

#endif // NPSTAT_INTERPOLATEDDISTRO1DNP_HH_
