#ifndef NPSTAT_UGAUSSCONVOLUTION1D_HH_
#define NPSTAT_UGAUSSCONVOLUTION1D_HH_

/*!
// \file UGaussConvolution1D.hh
//
// \brief Convolution of 1-d uniform and Gaussian distributions
//
// Author: I. Volobouev
//
// June 2014
*/

#include "npstat/stat/Distribution1DFactory.hh"

namespace npstat {
    class UGaussConvolution1D : public AbsScalableDistribution1D
    {
    public:
        /**
        // Parameters "leftEdge" and "uniformWidth" specify the
        // location of the left edge and the width of the uniform
        // distribution in the coordinate system for which the
        // Gaussian has mean 0 and width 1.
        */
        UGaussConvolution1D(double location, double scale,
                            double leftEdge, double uniformWidth);

        inline virtual UGaussConvolution1D* clone() const
            {return new UGaussConvolution1D(*this);}

        inline virtual ~UGaussConvolution1D() {}

        inline double leftEdge() const {return leftEdge_;}
        inline double uniformWidth() const {return width_;}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname()
            {return "npstat::UGaussConvolution1D";}
        static inline unsigned version() {return 1;}
        static UGaussConvolution1D* read(const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsDistribution1D&) const;

    private:
        friend class ScalableDistribution1DFactory<UGaussConvolution1D>;

        UGaussConvolution1D(double location, double scale,
                            const std::vector<double>& params);
        inline static int nParameters() {return 2;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;
        double unscaledExceedance(double x) const;

        double leftEdge_;
        double width_;
        double rightEdge_;
    };
}

#endif // NPSTAT_UGAUSSCONVOLUTION1D_HH_
