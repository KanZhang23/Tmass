#ifndef NPSTAT_GRIDINTERPOLATEDDISTRIBUTION_HH_
#define NPSTAT_GRIDINTERPOLATEDDISTRIBUTION_HH_

/*!
// \file GridInterpolatedDistribution.hh
//
// \brief Multivariate distribution obtained by nonparametric interpolation of
//        probability densities
//
// Author: I. Volobouev
//
// July 2010
*/

#include "npstat/nm/GridAxis.hh"
#include "npstat/nm/ArrayND.hh"

#include "npstat/stat/AbsInterpolationAlgoND.hh"
#include "npstat/stat/AbsGridInterpolatedDistribution.hh"

namespace npstat {
    /**
    // This class interpolates between probability distributions placed at
    // the points of a rectangular (not necessarily equidistant) grid. The
    // weights in the interpolations are determined in the multilinear
    // fashion. Multivariate quantile or copula interpolation method can be
    // used, depending on the types of interpolated densities.
    */
    class GridInterpolatedDistribution : public AbsGridInterpolatedDistribution
    {
    public:
        /** 
        // Constructor arguments "axes" and "distributionDimension" are,
        // respectively, the collection of parameter grid axes and the
        // dimensionality of the associated distributions.
        //
        // If "interpolateCopulas" is true, copula interpolation method
        // will be used (as implemented by class CopulaInterpolationND),
        // otherwise unit map interpolation will be used.
        //
        // If "useNearestGridCellOnly" is true, interpolation will be
        // disabled and only the distribution corresponding to the nearest
        // grid cell will be used.
        */
        GridInterpolatedDistribution(const std::vector<GridAxis>& axes,
                                     unsigned distributionDimension,
                                     bool interpolateCopulas,
                                     bool useNearestGridCellOnly = false);

        GridInterpolatedDistribution(const GridInterpolatedDistribution&);

        virtual ~GridInterpolatedDistribution();

        GridInterpolatedDistribution& operator=(
            const GridInterpolatedDistribution&);

        inline virtual GridInterpolatedDistribution* clone() const
            {return new GridInterpolatedDistribution(*this);}

        /**
        // Associate a distribution with the given parameter grid cell.
        //
        // This object will assume the ownership of the "distro" pointer.
        // All distributions must be set before density can be calculated
        // on all points in and outside the grid.
        */
        void setGridDistro(const unsigned* cell, unsigned lenCell,
                           AbsDistributionND* distro);

        /** 
        // Get the distribution associated with the given parameter grid cell.
        // Note that null pointer will be returned in case the cell has not
        // been set yet.
        */
        const AbsDistributionND* getGridDistro(const unsigned* cell,
                                               unsigned lenCell) const;
        /**
        // Associate a distribution with the given parameter grid cell
        // using a linear index for the cell
        */
        void setLinearDistro(unsigned long idx, AbsDistributionND* distro);

        /** 
        // Get the distribution associated with the given parameter grid cell
        // using a linear index for the cell. Note that null pointer will be
        // returned in case the cell has not been set yet.
        */
        const AbsDistributionND* getLinearDistro(unsigned long idx) const;

        /**
        // The following function must be called before calculating
        // the density. The length of the coordinates array must be
        // equal to the number of grid axes.
        */
        void setGridCoords(const double* coords, unsigned lenCoords);

        /** Switch between unit map interpolation and copula interpolation */
        void interpolateCopulas(bool b);

        /** Switch between normal interpolation and using just one cell */
        void useSingleCell(bool b);

        //@{
        /** Inspect object properties */
        inline unsigned nAxes() const {return nAxes_;}
        inline const GridAxis& getAxis(const unsigned i) const
            {return axes_.at(i);}
        inline unsigned long nDistros() const {return interpolated_.length();}
        inline ArrayShape gridShape() const {return interpolated_.shape();}
        inline bool interpolatingCopulas() const {return interpCopulas_;}
        inline bool usingSingleCell() const {return useNearestOnly_;}
        //@}

        //@{
        /** Method inherited from AbsDistributionND */
        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned len, double* x) const;
        bool mappedByQuantiles() const;
        //@}

        //@{
        /** 
         * This method works for copula interpolation only. Naturally,
         * all interpolated distributions in this case must be of type
         * (or must inherit from) CompositeDistributionND.
         */
        double marginalDensity(unsigned idim, double x) const;
        double marginalCdf(unsigned idim, double x) const;
        double marginalExceedance(unsigned idim, double x) const;
        double marginalQuantile(unsigned idim, double x) const;
        double copulaDensity(const double* x, const unsigned dim) const;
        double productOfTheMarginals(const double* x, const unsigned dim) const;
        //@}

        //@{
        // Method needed for "geners" I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;
        //@}

        // Class name for the I/O
        static inline const char* classname()
            {return "npstat::GridInterpolatedDistribution";}

        // Version number for the I/O
        static inline unsigned version() {return 3;}

        // Read method for the I/O
        static GridInterpolatedDistribution* read(
            const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistributionND& r) const;

    private:
        GridInterpolatedDistribution();

        void releaseDistros();
        void cloneDistros();
        void rebuildInterpolator();
        void setGridCoordsSingle(const double* coords);
        void setGridCoordsInterpolating(const double* coords);

        // We will own the interpolated distributions
        std::vector<GridAxis> axes_;
        std::vector<unsigned> cellBuf_;
        ArrayND<AbsDistributionND*> interpolated_;
        AbsInterpolationAlgoND* interpolator_;
        AbsDistributionND* single_;
        unsigned nAxes_;
        bool pointSet_;
        bool interpCopulas_;
        bool useNearestOnly_;
    };
}

#endif // NPSTAT_GRIDINTERPOLATEDDISTRIBUTION_HH_
