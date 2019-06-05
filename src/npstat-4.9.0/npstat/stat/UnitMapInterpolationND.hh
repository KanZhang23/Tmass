#ifndef NPSTAT_UNITMAPINTERPOLATIONND_HH_
#define NPSTAT_UNITMAPINTERPOLATIONND_HH_

/*!
// \file UnitMapInterpolationND.hh
//
// \brief Interpolation of multivariate statistical distributions by
//        conditional quantiles
//
// The interpolation will be linear (or multilinear), with user-defined
// weights.
//
// Author: I. Volobouev
//
// March 2010
*/

#include "npstat/stat/AbsInterpolationAlgoND.hh"

namespace npstat {
    /**
    // This class interpolates multivariate statistical distributions
    // by conditional quantiles (which is a particular mapping from the
    // unit cube into the density support region). Interpolation weights
    // are to be calculated outside of this class.
    */
    class UnitMapInterpolationND : public AbsInterpolationAlgoND
    {
    public:
        /**
        // The first constructor argument is the dimensionality of
        // the density support, the second is the number of distributions
        // which will be used in the interpolation. That number, in
        // general, will depend on the dimensionality of the parameter
        // space. After the object is constructed, the distributions
        // should be added with the "add" method. It is possible to add
        // more than "nInterpolated" distributions (which will result
        // in a slightly less efficient code), however the user must add
        // at least "nInterpolated" distributions before calling the
        // "density" method. All interpolated distributions must have
        // their unit cube mapping implemented by conditional quantiles
        // (so that their "mappedByQuantiles" method returns "true").
        //
        // This class does not make copies of the interpolated distributions
        // and works with references and pointers (making internal copies
        // would result in a significant performance hit in a variety of
        // typical situations). This means that all added distributions
        // must still exist while this object is in use. It is the
        // responsibility of the user of this class to make sure that
        // this is indeed the case.
        */
        UnitMapInterpolationND(unsigned dim, unsigned nInterpolated);

        inline virtual ~UnitMapInterpolationND() {}

        inline virtual UnitMapInterpolationND* clone() const
            {return new UnitMapInterpolationND(*this);}

        inline bool mappedByQuantiles() const {return true;}

        //@{
        /**
        // In this method, "d" must refer to a distribution for which
        // "mappedByQuantiles" method returns "true".
        */
        void add(const AbsDistributionND& d, double weight);
        void replace(unsigned i, const AbsDistributionND& d, double weight);
        //@}

        /** Set the weight for the given term in the weighted sum */
        void setWeight(unsigned i, double weight);

        /** Clear all the terms in the weighted sum */
        void clear();

        /**
        // This method should be called to disable
        // (and later enable) automatic weight normalization
        // if you want to use the "setWeight" or "replace" methods
        // many times and, especially, if at some point in this process
        // the sum of the weights becomes zero. The "density" and
        // "unitMap" methods can not be called if normalization
        // is not enabled.
        */
        void normalizeAutomatically(bool allow);

        /** The number of terms in the weighted sum */
        inline unsigned size() const {return distros_.size();}

        double density(const double* x, unsigned dim) const;
        void unitMap(const double* rnd, unsigned dim, double* x) const;

        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}

        // Class name for the I/O
        static inline const char* classname()
            {return "npstat::UnitMapInterpolationND";}

        // Version number for the I/O
        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsDistributionND&) const;

    private:
        void normalize();

        std::vector<WeightedND> distros_;
        mutable std::vector<double> work_;
        mutable std::vector<double> point_;
        double wsum_;
        unsigned nInterpolated_;
        bool autoNormalize_;
    };
}

#endif // NPSTAT_UNITMAPINTERPOLATIONND_HH_
