#ifndef NPSTAT_LININTERPOLATEDTABLE1D_HH_
#define NPSTAT_LININTERPOLATEDTABLE1D_HH_

/*!
// \file LinInterpolatedTable1D.hh
//
// \brief One-dimensional linear interpolation/extrapolation table
//
// LinInterpolatedTableND template provides a more flexible tool
// in multiple dimensions (including one). It can use arbitrary
// internal data representation, non-uniform point spacing, etc.
//
// For tabular interpolation of statistical densities, see classes
// "Tabulated1D" and "BinnedDensity1D" in the "Distributions1D.hh" header.
//
// If linear interpolation beyond the grid limits is not required,
// the facilities built into the ArrayND class can do the job as well.
//
// Author: I. Volobouev
//
// April 2011
*/

#include <utility>
#include <vector>

#include "geners/CPP11_auto_ptr.hh"
#include "geners/ClassId.hh"

namespace npstat {
    /**
    // This class can be used to interpolate a table of values linearly
    // in one dimension. Regular coordinate spacing only, double precision
    // numbers are used to represent the data internally. Extrapolation
    // beyond the leftmost or rightmost data points could be either
    // linear or constant.
    */
    class LinInterpolatedTable1D
    {
    public:
        /**
        // Constructor from a regularly spaced data. Extrapolation
        // from the edge to infinity can be either linear or constant,
        // as defined by the parameters "leftExtrapolationLinear" and
        // "rightExtrapolationLinear". "npoints" (size of the data array)
        // must be larger than 1.
        */
        template <typename Real>
        LinInterpolatedTable1D(const Real* data, unsigned npoints,
                               double x_min, double x_max,
                               bool leftExtrapolationLinear,
                               bool rightExtrapolationLinear);

        /**
        // Constructor from a list of points, not necessarily regularly
        // spaced (but must be sorted in the increasing order). The first
        // member of the pair is the x coordinate, the second is the
        // tabulated function value. The input list will be interpolated
        // to "npoints" internal points linearly.
        */
        template <typename Real>
        LinInterpolatedTable1D(const std::vector<std::pair<Real,Real> >& v,
                               unsigned npoints,
                               bool leftExtrapolationLinear,
                               bool rightExtrapolationLinear);

        /**
        // Constructor which builds a function returning the given constant
        // for every argument value
        */
        explicit LinInterpolatedTable1D(double c);

        /** Default constructor builds a functor which always returns 0.0 */
        LinInterpolatedTable1D();

        ~LinInterpolatedTable1D();

        /** Interpolate to the given coordinate */
        double operator()(const double& x) const;

        //@{
        /** Comparison for equality (useful for I/O testing) */
        bool operator==(const LinInterpolatedTable1D& r) const;
        inline bool operator!=(const LinInterpolatedTable1D& r) const
            {return !(*this == r);}
        //@}
        
        //@{
        /** A simple inspector */
        inline double xmin() const {return xmin_;}
        inline double xmax() const {return xmax_;}
        inline unsigned npoints() const {return npoints_;}
        inline bool leftExtrapolationLinear() const
            {return leftExtrapolationLinear_;}
        inline bool rightExtrapolationLinear() const
            {return rightExtrapolationLinear_;}
        inline const double* data() const {return &data_[0];}
        //@}

        /**
        // This method checks whether the table is monotonous
        // (and, therefore, can be inverted). Possible flat regions
        // at the edges are not taken into account.
        */
        bool isMonotonous() const;

        /**
        // Generate the inverse lookup table. Note that it is only
        // possible if the original table is monotonous (not taking
        // into account possible flat regions at the edges). If the
        // inversion is not possible, NULL pointer will be returned.
        //
        // The new table will have "npoints" points. The parameters
        // "leftExtrapolationLinear" and "rightExtrapolationLinear"
        // refer to the inverted table. Note that left and right will
        // exchange places if the original table is decreasing.
        */
        CPP11_auto_ptr<LinInterpolatedTable1D> inverse(
            unsigned npoints, bool leftExtrapolationLinear,
            bool rightExtrapolationLinear) const;

        //@{
        /** Method related to "geners" I/O */
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static inline const char* classname()
            {return "npstat::LinInterpolatedTable1D";}
        static inline unsigned version() {return 1;}
        static LinInterpolatedTable1D* read(const gs::ClassId& id, std::istream&);

    private:
        static inline double interpolateSimple(
            const double x0, const double x1,
            const double y0, const double y1,
            const double x)
        {
            return y0 + (y1 - y0)*((x - x0)/(x1 - x0));
        }

        std::vector<double> data_;
        double xmin_;
        double xmax_;
        double binwidth_;
        unsigned npoints_;
        bool leftExtrapolationLinear_;
        bool rightExtrapolationLinear_;
        mutable bool monotonous_;
        mutable bool monotonicityKnown_;

#ifdef SWIG
    public:
        inline LinInterpolatedTable1D* inverse2(
            unsigned npoints, bool leftExtrapolationLinear,
            bool rightExtrapolationLinear) const
        {
            CPP11_auto_ptr<LinInterpolatedTable1D> ptr = inverse(
                npoints, leftExtrapolationLinear, rightExtrapolationLinear);
            return ptr.release();
        }
#endif
    };
}

#include "npstat/nm/LinInterpolatedTable1D.icc"

#endif // NPSTAT_LININTERPOLATEDTABLE1D_HH_
