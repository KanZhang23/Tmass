#ifndef NPSTAT_ABSSYMBETAFILTERPROVIDER_HH_
#define NPSTAT_ABSSYMBETAFILTERPROVIDER_HH_

/*!
// \file AbsSymbetaFilterProvider.hh
//
// \brief Interface definition for classes which build symmetric beta LOrPE
//        filters
//
// Author: I. Volobouev
//
// October 2013
*/

#include <vector>

#include "geners/CPP11_shared_ptr.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"

namespace npstat {
    /**
    // Interface definition for classes which build LOrPE filters
    // using symmetric beta kernels (or a Gaussian kernel) as weights.
    // The derived classes are supposed to override the "provideFilter"
    // member function.
    */
    class AbsSymbetaFilterProvider
    {
    public:
        inline AbsSymbetaFilterProvider() : memoize_(false) {}

        inline virtual ~AbsSymbetaFilterProvider() {}
        
        /**
        // Parameters are:
        //
        //  symbetaPower -- This parameter defines which kernel to use.
        //                  Negative value means use Gaussian.
        //
        //  bandwidth    -- Kernel bandwidth.
        //
        //  degree       -- Polynomial degree.
        //
        //  nbins        -- Number of bins in the discretized dataset.
        //
        //  binwidth     -- Bin width.
        //
        //  bm           -- Boundary handling method.
        //
        //  excludedBin  -- Bin number to exclude. Numbers equal to
        //                  nbins and larger are to be ignored.
        //
        //  excludeCentralPoint  -- Exclude central bin from the
        //                          weight function.
        //
        // The function should return an instance of LocalPolyFilter1D
        // wrapped into a smart pointer.
        */
        virtual CPP11_shared_ptr<const LocalPolyFilter1D> provideFilter(
            int symbetaPower, double bandwidth, double degree,
            unsigned nbins, double binwidth, const BoundaryHandling& bm,
            unsigned excludedBin, bool excludeCentralPoint) = 0;

        //@{
        /**
        // This function can be used to advise the filter provider
        // which filters should be stored for subsequent reuse.
        // The filter provider is free to heed or to ignore this advice.
        */
        inline virtual void startMemoizing() {memoize_ = true;}
        inline virtual void stopMemoizing() {memoize_ = false;}
        //@}

        /** Is the filter provider memoizing filters currently? */
        inline virtual bool isMemoizing() const {return memoize_;}

    private:
        bool memoize_;
    };


    /** Simple non-memoizing implementation of AbsSymbetaFilterProvider */
    class SimpleSymbetaFilterProvider : public AbsSymbetaFilterProvider
    {
    public:
        inline virtual ~SimpleSymbetaFilterProvider() {}

        inline virtual CPP11_shared_ptr<const LocalPolyFilter1D> provideFilter(
            const int symbetaPower, const double bandwidth, const double degree,
            const unsigned nbins, const double binwidth,
            const BoundaryHandling& bm,
            const unsigned excludedBin, const bool excludeCentralPoint)
        {
            CPP11_auto_ptr<LocalPolyFilter1D> p;
            if (excludedBin < nbins)
            {
                std::vector<unsigned char> mask(nbins, 0);
                mask[excludedBin] = 1;
                p = symbetaLOrPEFilter1D(symbetaPower, bandwidth, degree,
                                         nbins, 0.0, nbins*binwidth,
                                         bm, &mask[0], excludeCentralPoint);
            }
            else
                p = symbetaLOrPEFilter1D(symbetaPower, bandwidth, degree,
                                         nbins, 0.0, nbins*binwidth,
                                         bm, (unsigned char*)0,
                                         excludeCentralPoint);
            return CPP11_shared_ptr<const LocalPolyFilter1D>(p.release());
        }
    };
}

#endif // NPSTAT_ABSSYMBETAFILTERPROVIDER_HH_
