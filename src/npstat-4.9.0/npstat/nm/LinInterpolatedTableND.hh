#ifndef NPSTAT_LININTERPOLATEDTABLEND_HH_
#define NPSTAT_LININTERPOLATEDTABLEND_HH_

/**
// \file LinInterpolatedTableND.hh
//
// \brief Multilinear interpolation/extrapolation on rectangular grids
//
// Author: I. Volobouev
//
// June 2012
*/

#include <climits>
#include <vector>
#include <utility>

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/UniformAxis.hh"

namespace npstat {
    /** 
    // Template for multilinear interpolation/extrapolation of values provided
    // on a rectangular coordinate grid. "Numeric" is the type stored in
    // the table. "Axis" should be one of GridAxis, UniformAxis, or DualAxis
    // classes or a user-provided class with a similar set of methods.
    */
    template <class Numeric, class Axis=UniformAxis>
    class LinInterpolatedTableND
    {
        template <typename Num2, typename Axis2>
        friend class LinInterpolatedTableND;

    public:
        typedef Numeric value_type;
        typedef Axis axis_type;

        /** 
        // Main constructor for arbitrary-dimensional interpolators.
        //
        // "axes" are the axes of the rectangular coordinate grid.
        //
        // In each pair provided by the "extrapolationType" argument,
        // the first element of the pair specifies whether the extrapolation
        // to negative infinity should be linear (if "true") or constant
        // (if "false"). The second element of the pair specifies whether
        // to extrapolate linearly to positive infinity.
        //
        // "functionLabel" is an arbitrary string which can potentially
        // be used by plotting programs and such.
        */
        LinInterpolatedTableND(
            const std::vector<Axis>& axes,
            const std::vector<std::pair<bool,bool> >& extrapolationType,
            const char* functionLabel=0);

        /** Convenience constructor for 1-d interpolators */
        LinInterpolatedTableND(const Axis& xAxis, bool leftX, bool rightX,
                               const char* functionLabel=0);

        /** Convenience constructor for 2-d interpolators */
        LinInterpolatedTableND(const Axis& xAxis, bool leftX, bool rightX,
                               const Axis& yAxis, bool leftY, bool rightY,
                               const char* functionLabel=0);

        /** Convenience constructor for 3-d interpolators */
        LinInterpolatedTableND(const Axis& xAxis, bool leftX, bool rightX,
                               const Axis& yAxis, bool leftY, bool rightY,
                               const Axis& zAxis, bool leftZ, bool rightZ,
                               const char* functionLabel=0);

        /** Convenience constructor for 4-d interpolators */
        LinInterpolatedTableND(const Axis& xAxis, bool leftX, bool rightX,
                               const Axis& yAxis, bool leftY, bool rightY,
                               const Axis& zAxis, bool leftZ, bool rightZ,
                               const Axis& tAxis, bool leftT, bool rightT,
                               const char* functionLabel=0);

        /** Convenience constructor for 5-d interpolators */
        LinInterpolatedTableND(const Axis& xAxis, bool leftX, bool rightX,
                               const Axis& yAxis, bool leftY, bool rightY,
                               const Axis& zAxis, bool leftZ, bool rightZ,
                               const Axis& tAxis, bool leftT, bool rightT,
                               const Axis& vAxis, bool leftV, bool rightV,
                               const char* functionLabel=0);

        /**
        // Converting copy constructor from interpolator
        // with another storage type
        */
        template <class Num2>
        LinInterpolatedTableND(const LinInterpolatedTableND<Num2,Axis>&);

        /**
        // Basic interpolation result. Argument point dimensionality must be
        // compatible with the interpolator dimensionality.
        */
        Numeric operator()(const double* point, unsigned dim) const;

        //@{
        /** Convenience function for low-dimensional interpolators */
        Numeric operator()(const double& x0) const;
        Numeric operator()(const double& x0, const double& x1) const;
        Numeric operator()(const double& x0, const double& x1,
                           const double& x2) const;
        Numeric operator()(const double& x0, const double& x1,
                           const double& x2, const double& x3) const;
        Numeric operator()(const double& x0, const double& x1,
                           const double& x2, const double& x3,
                           const double& x4) const;
        //@}

        //@{
        /** Examine interpolator contents */
        inline unsigned dim() const {return dim_;}
        inline const std::vector<Axis>& axes() const {return axes_;}
        inline const Axis& axis(const unsigned i) const
            {return axes_.at(i);}
        inline unsigned long length() const {return data_.length();}
        bool leftInterpolationLinear(unsigned i) const;
        bool rightInterpolationLinear(unsigned i) const;
        std::vector<std::pair<bool,bool> > interpolationType() const;
        inline const std::string& functionLabel() const
            {return functionLabel_;}
        //@}

        //@{
        /** Access the interpolator table data */
        inline const ArrayND<Numeric>& table() const {return data_;}
        inline ArrayND<Numeric>& table() {return data_;}
        //@}

        /** Convenience function for getting coordinates of the grid points */
        void getCoords(unsigned long linearIndex,
                       double* coords, unsigned coordsBufferSize) const;

        /** 
        // This method returns "true" if the method isUniform()
        // of each interpolator axis returns "true" 
        */
        bool isUniformlyBinned() const;

        /**
        // This method will return "true" if the point
        // is inside the grid limits or on the boundary
        */
        bool isWithinLimits(const double* point, unsigned dim) const;

        /** Modifier for the function label */
        inline void setFunctionLabel(const char* newlabel)
            {functionLabel_ = newlabel ? newlabel : "";}

        /**
        // Invert the function w.r.t. the variable represented by one of
        // the axes. The function values must change monotonously along
        // the chosen axis. Note that this operation is meaningful only
        // in case "Numeric" type is either float or double.
        */
        template <typename ConvertibleToUnsigned>
        CPP11_auto_ptr<LinInterpolatedTableND> invertWRTAxis(
            ConvertibleToUnsigned axisNumber, const Axis& replacementAxis,
            bool newAxisLeftLinear, bool newAxisRightLinear,
            const char* functionLabel=0) const;

        /**
        // This method inverts the ratio response.
        // That is, we assume that the table encodes r(x) for
        // some function f(x) = x*r(x). We also assume that the
        // original axis does not represent x directly -- instead,
        // axis coordinates are given by g(x) (in practice, g is
        // often the natural log). We will also assume that the new
        // axis is not going to represent f(x) directly -- it
        // will be h(f(x)) instead. The functors "invg" and "invh"
        // below must do the inverse: taking the axes coordinates
        // to the actual values of x and f(x). Both "invg" and "invh"
        // must be monotonously increasing functions. The code assumes
        // that x*r(x) -> 0 when x->0 (that is, r(x) is bounded at 0)
        // and it also assumes (but does not check explicitly)
        // that x*r(x) is monotonously increasing with x.
        //
        // The returned interpolation table encodes the values
        // of x/f(x). Of course, they are just 1/r(x), but the trick
        // is to be able to look them up quickly as a function of
        // h(f(x)). Naturally, this whole operation is meaningful
        // only in case "Numeric" type is either float or double.
        */
        template <class Functor1, class Functor2>
        CPP11_auto_ptr<LinInterpolatedTableND> invertRatioResponse(
            unsigned axisNumber, const Axis& replacementAxis,
            bool newAxisLeftLinear, bool newAxisRightLinear,
            Functor1 invg, Functor2 invh,
            const char* functionLabel=0) const;

        /** Comparison for equality */
        bool operator==(const LinInterpolatedTableND&) const;

        /** Logical negation of operator== */
        inline bool operator!=(const LinInterpolatedTableND& r) const
            {return !(*this == r);}

        //@{
        // Method related to "geners" I/O
        inline gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream& of) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static LinInterpolatedTableND* read(
            const gs::ClassId& id, std::istream& in);

    private:
        LinInterpolatedTableND();

        LinInterpolatedTableND(
            const ArrayND<Numeric>& data,
            const std::vector<Axis>& axes,
            const char* leftInterpolation,
            const char* rightInterpolation,
            const std::string& label);

        bool allConstInterpolated() const;

        ArrayND<Numeric> data_;
        std::vector<Axis> axes_;
        std::string functionLabel_;
        char leftInterpolationLinear_[CHAR_BIT*sizeof(unsigned long)];
        char rightInterpolationLinear_[CHAR_BIT*sizeof(unsigned long)];
        unsigned dim_;
        bool allConstInterpolated_;

        template <class Functor1>
        static double solveForRatioArg(double xmin, double xmax,
                                       double rmin, double rmax,
                                       double fval, Functor1 invg);

        template <class Functor1>
        static void invert1DResponse(const ArrayND<Numeric>& fromSlice,
                                     const Axis& fromAxis, const Axis& toAxis,
                                     bool newLeftLinear, bool newRightLinear,
                                     Functor1 invg,
                                     const double* rawx, const double* rawf,
                                     double* workspace,
                                     ArrayND<Numeric>* toSlice);
    };
}

#include "npstat/nm/LinInterpolatedTableND.icc"

#endif // NPSTAT_LININTERPOLATEDTABLEND_HH_
