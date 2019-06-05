#ifndef NPSTAT_LOGLIKELIHOODPEAK_HH_
#define NPSTAT_LOGLIKELIHOODPEAK_HH_

/*!
// \file logLikelihoodPeak.hh
//
// \brief Summarize properties of one-dimensional log-likelihood curves
//
// Author: I. Volobouev
//
// May 2010
*/

#ifdef SWIG
#include <vector>
#endif

namespace npstat {
    /** Coordinate of a point on a likelihood curve, with assigned status */
    class LikelihoodPoint
    {
    public:
        enum Status {
            OK = 0,
            NOT_FOUND,
            ON_THE_EDGE,
            INVALID
        };

        inline LikelihoodPoint() : coord_(0.0), status_(INVALID) {}
        inline LikelihoodPoint(const double coord, const Status s)
            : coord_(coord), status_(s) {}

        inline double coord() const {return coord_;}
        inline Status status() const {return status_;}

    private:
        double coord_;
        Status status_;
    };

    /**
    // Class for storing the location of the likelihood peak and the points
    // where the log-likelihood curve descends down by a certain magnitude
    */
    class LikelihoodSummary
    {
    public:
        inline LikelihoodSummary() : down_(0.0) {}
        inline LikelihoodSummary(const LikelihoodPoint& peak,
                                 const LikelihoodPoint& left,
                                 const LikelihoodPoint& right,
                                 const double down)
            : peak_(peak), left_(left), right_(right), down_(down) {}

        inline const LikelihoodPoint& peak() const {return peak_;}
        inline const LikelihoodPoint& left() const {return left_;}
        inline const LikelihoodPoint& right() const {return right_;}
        inline double down() const {return down_;}

        inline double peakCoord() const
            {return peak_.coord();}
        inline double leftError() const
            {return peak_.coord() - left().coord();}
        inline double rightError() const
            {return right().coord() - peak_.coord();}
        inline double averageError() const
            {return (right().coord() - left().coord())/2.0;}

        inline bool isOK() const {return down_ > 0.0 &&
                peak_.status() == LikelihoodPoint::OK &&
                left_.status() == LikelihoodPoint::OK &&
                right_.status() == LikelihoodPoint::OK;}
    private:
        LikelihoodPoint peak_;
        LikelihoodPoint left_;
        LikelihoodPoint right_;
        double down_;
    };

    /**
    // Function which summarizes properties of one-dimensional
    // log-likelihoods.
    //
    // The number of points in the curve must be at least 3.
    // Typical value of "down" is 0.5 for 1 sigma, 2.0 for 2 sigmas,
    // 4.5 for 3 sigmas, etc.
    //
    // leftPointCoordinate and rightPointCoordinate are the leftmost
    // and rightmost coordinates which correspont to the first and the
    // last value of the "curve" array, respectively. It is assumed that
    // all other point coordinates are equidistantly spaced in between.
    */
    LikelihoodSummary logLikelihoodPeak(
        const double* curve, unsigned npoints, double down,
        double leftPointCoordinate, double rightPointCoordinate);

#ifdef SWIG
    inline LikelihoodSummary logLikelihoodPeak2(
        const std::vector<double>& curve, const double down,
        const double leftPointCoordinate, const double rightPointCoordinate)
    {
        const unsigned n = curve.size();
        const double* data = n ? &curve[0] : NULL;
        return logLikelihoodPeak(data, n, down, leftPointCoordinate,
                                 rightPointCoordinate);
    }
#endif
}

#endif // NPSTAT_LOGLIKELIHOODPEAK_HH_
