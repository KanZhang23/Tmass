#ifndef NPSTAT_FILTER1DBUILDERS_HH_
#define NPSTAT_FILTER1DBUILDERS_HH_

/*!
// \file Filter1DBuilders.hh
//
// \brief Builders of local polynomial filters in one dimension
//
// The filters are intended for use with the "LocalPolyFilter1D" class.
// This particular file declares filter builders that inherit from
// the "AbsBoundaryFilter1DBuilder" class.
//
// Author: I. Volobouev
//
// June 2015
*/

#include "npstat/stat/AbsFilter1DBuilder.hh"

namespace npstat {
    /**
    // This class will simply truncate the kernel at the boundary
    */
    class TruncatingFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        inline TruncatingFilter1DBuilder(const AbsDistribution1D* distro,
                                         double stepSize,
                                         const unsigned char* exclusionMask=0,
                                         unsigned exclusionMaskLen = 0,
                                         bool excludeCentralPoint = false)
            : AbsBoundaryFilter1DBuilder(distro, stepSize,
                                         exclusionMask, exclusionMaskLen,
                                         excludeCentralPoint) {}

        inline virtual ~TruncatingFilter1DBuilder() {}

        inline virtual bool isFolding() const {return false;}

    private:
        inline virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const 
            {return 1.0;} 
    };

    /**
    // This class will mirror (or "fold") the kernel so that it fits
    // inside the boundaries. The kernel is assumed to be an even function.
    */
    class FoldingFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        inline FoldingFilter1DBuilder(const AbsDistribution1D* distro,
                                      double stepSize,
                                      const unsigned char* exclusionMask = 0,
                                      unsigned exclusionMaskLen = 0,
                                      bool excludeCentralPoint = false)
            : AbsBoundaryFilter1DBuilder(distro, stepSize,
                                         exclusionMask, exclusionMaskLen,
                                         excludeCentralPoint) {}

        inline virtual ~FoldingFilter1DBuilder() {}

        inline virtual bool isFolding() const {return true;}

    private:
        inline virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const 
            {return 1.0;} 
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched near the boundaries
    // by the factor Iw_max / Iw, where Iw is the integral of the
    // weight function inside the support boundary (Iw_max is the
    // same integral calculated far away from any boundary).
    */
    class StretchingFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        inline StretchingFilter1DBuilder(const AbsDistribution1D* distro,
                                         const double stepSize,
                                         const unsigned char* exclusionMask=0,
                                         unsigned exclusionMaskLen = 0,
                                         bool excludeCentralPoint = false)
            : AbsBoundaryFilter1DBuilder(distro, stepSize,
                                         exclusionMask, exclusionMaskLen,
                                         excludeCentralPoint) {}

        inline virtual ~StretchingFilter1DBuilder() {}

        inline virtual bool isFolding() const {return false;}

    private:
        virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const;
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched near the boundaries
    // so that the integral of the weight function squared is
    // preserved when the weight is normalized.
    */
    class ConstSqFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        inline ConstSqFilter1DBuilder(const AbsDistribution1D* distro,
                                      double centralStepSize,
                                      const unsigned char* exclusionMask = 0,
                                      unsigned exclusionMaskLen = 0,
                                      bool excludeCentralPoint = false)
            : AbsBoundaryFilter1DBuilder(distro, centralStepSize,
                                         exclusionMask, exclusionMaskLen,
                                         excludeCentralPoint) {}

        inline virtual ~ConstSqFilter1DBuilder() {}

        inline virtual bool isFolding() const {return false;}

    private:
        virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const;
    };

    /**
    // This class will both fold and stretch the kernel to preserve its
    // squared integral. The kernel is assumed to be an even function.
    */
    class FoldingSqFilter1DBuilder : public ConstSqFilter1DBuilder
    {
    public:
        inline FoldingSqFilter1DBuilder(const AbsDistribution1D* distro,
                                        double stepSize,
                                        const unsigned char* exclusionMask = 0,
                                        unsigned exclusionMaskLen = 0,
                                        bool excludeCentralPoint = false)
            : ConstSqFilter1DBuilder(distro, stepSize,
                                     exclusionMask, exclusionMaskLen,
                                     excludeCentralPoint) {}

        inline virtual ~FoldingSqFilter1DBuilder() {}

        inline virtual bool isFolding() const {return true;}
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched near the boundaries
    // so that the weight function variance is preserved when
    // the weight is normalized.
    */
    class ConstVarFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        inline ConstVarFilter1DBuilder(const AbsDistribution1D* distro,
                                       double centralStepSize,
                                       const unsigned char* exclusionMask = 0,
                                       unsigned exclusionMaskLen = 0,
                                       bool excludeCentralPoint = false)
            : AbsBoundaryFilter1DBuilder(distro, centralStepSize,
                                         exclusionMask, exclusionMaskLen,
                                         excludeCentralPoint) {}

        inline virtual ~ConstVarFilter1DBuilder() {}

        inline virtual bool isFolding() const {return false;}

    private:
        virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const;
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched and folded near the
    // boundaries so that the weight function variance is preserved.
    */
    class FoldingVarFilter1DBuilder : public ConstVarFilter1DBuilder
    {
    public:
        inline FoldingVarFilter1DBuilder(const AbsDistribution1D* distro,
                                         double centralStepSize,
                                         const unsigned char* exclusionMask=0,
                                         unsigned exclusionMaskLen = 0,
                                         bool excludeCentralPoint = false)
            : ConstVarFilter1DBuilder(distro, centralStepSize,
                                      exclusionMask, exclusionMaskLen,
                                      excludeCentralPoint) {}

        inline virtual ~FoldingVarFilter1DBuilder() {}

        inline virtual bool isFolding() const {return true;}
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched near the boundaries
    // so that the ratio of the squared kernel integral to the
    // squared bias is preserved. This corresponds to constant
    // AMISE bandwidth.
    //
    // "filterDegree" is the degree of the filter. filterDegree = 0
    // corresponds to the second order kernel.
    //
    // "typicalDistributionWidth" is the parameter which connects
    // typical values of density derivatives at the boundary.
    */
    class ConstBwFilter1DBuilder : public AbsBoundaryFilter1DBuilder
    {
    public:
        ConstBwFilter1DBuilder(double filterDegree,
                               double typicalDistributionWidth,
                               const AbsDistribution1D* distro,
                               double centralStepSize,
                               const unsigned char* exclusionMask = 0,
                               unsigned exclusionMaskLen = 0,
                               bool excludeCentralPoint = false);

        inline virtual ~ConstBwFilter1DBuilder() {}

        inline virtual bool isFolding() const {return false;}

    private:
        double filterDeg_;
        double typicalWidth_;
        std::vector<double> taperVec_;

        virtual double calculateCriterion(
            const AbsDistribution1D*, double, int, int, double, double*) const;
    };

    /**
    // This class will construct a local polynomial filter out of
    // an AbsDistribution1D weight function assumed to be even.
    //
    // The weight function will be stretched and folded near the
    // boundaries so that the ratio of the squared kernel integral
    // to the squared bias is preserved. This corresponds to constant
    // AMISE bandwidth.
    */
    class FoldBwFilter1DBuilder : public ConstBwFilter1DBuilder
    {
    public:
        inline FoldBwFilter1DBuilder(const double filterDegree,
                                     const double typicalDistributionWidth,
                                     const AbsDistribution1D* distro,
                                     const double centralStepSize,
                                     const unsigned char* exclusionMask = 0,
                                     unsigned exclusionMaskLen = 0,
                                     bool excludeCentralPoint = false)
            : ConstBwFilter1DBuilder(filterDegree, typicalDistributionWidth,
                                     distro, centralStepSize, exclusionMask,
                                     exclusionMaskLen, excludeCentralPoint) {}

        inline virtual ~FoldBwFilter1DBuilder() {}

        inline virtual bool isFolding() const {return true;}
    };
}

#endif // NPSTAT_FILTER1DBUILDERS_HH_
