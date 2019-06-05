#ifndef NPSTAT_ABSGRIDINTERPOLATEDDISTRIBUTION_HH_
#define NPSTAT_ABSGRIDINTERPOLATEDDISTRIBUTION_HH_

/*!
// \file AbsGridInterpolatedDistribution.hh
//
// \brief Interface class for interpolating probability distributions on a grid
//        of parameters
//
// Author: I. Volobouev
//
// July 2010
*/

#include <vector>

#include "npstat/stat/AbsDistributionND.hh"

namespace npstat {
    class GridAxis;

    /**
    // Interface class for interpolating between probability distributions
    // placed at the points of a rectangular (not necessarily equidistant)
    // parameter grid
    */
    struct AbsGridInterpolatedDistribution : public AbsDistributionND
    {
        inline explicit AbsGridInterpolatedDistribution(const unsigned dim) 
            : AbsDistributionND(dim) {}

        inline virtual ~AbsGridInterpolatedDistribution() {}

        /** "Virtual copy constructor" */
        virtual AbsGridInterpolatedDistribution* clone() const = 0;

        /**
        // Set the distribution corresponding to the given parameter grid
        // point. The derived class should assume the ownership of the
        // "distro" pointer.
        */
        virtual void setGridDistro(const unsigned* cell, unsigned lenCell,
                                   AbsDistributionND* distro) = 0;

        /** Same as "setGridDistro" but using a linear index for the cell */
        virtual void setLinearDistro(unsigned long idx, AbsDistributionND* pd)=0;

        //@{
        /** 
        // Lookup the distribution at the given grid cell. This method should
        // return null pointer in case the distribution has not been set for
        // that cell.
        */
        virtual const AbsDistributionND* getGridDistro(const unsigned* cell,
                                                       unsigned lenCell) const=0;
        virtual const AbsDistributionND* getLinearDistro(unsigned long idx) const=0;
        //@}

        /**
        // This method will be called before calculating the density
        // (it will perform various preparations related to distribution
        // interpolation). The length of the "coords" array must be
        // equal to the number of grid axes.
        */
        virtual void setGridCoords(const double* coords, unsigned nCoords) = 0;

        /** Number of gris axes */
        virtual unsigned nAxes() const = 0;

        /** Number of gris points (and associated distributions) */
        virtual unsigned long nDistros() const = 0;

        /** The shape of the rectangular grid of parameters */
        virtual std::vector<unsigned> gridShape() const = 0;

        /** The grid axis for the given coordinate */
        virtual const GridAxis& getAxis(unsigned i) const = 0;
    };
}

#endif // NPSTAT_ABSGRIDINTERPOLATEDDISTRIBUTION_HH_
