#ifndef NPSTAT_CONVOLUTIONENGINE1D_HH_
#define NPSTAT_CONVOLUTIONENGINE1D_HH_

/*!
// \file ConvolutionEngine1D.hh
//
// \brief Fast one-dimensional convolutions via Fourier
//        transforms (FFTW interface)
//
// Author: I. Volobouev
//
// November 2009
*/

#include <map>

#include "npstat/nm/FourierImage.hh"

namespace npstat {
    /**
    // Class for performing convolutions by FFT using FFTW library.
    // Internally, transforms are calculated in double precision.
    */
    class ConvolutionEngine1D
    {
    public:
        /**
        // Constructor takes data length and FFTW optimization flag
        // as parameters. Make sure the data length is sufficient
        // to avoid the convolution wrap-around.
        */
        explicit ConvolutionEngine1D(unsigned dataLen,
                                     unsigned optimization = FFTW_ESTIMATE);
        ~ConvolutionEngine1D();

        /**
        // Provide the filter for subsequent convolutions.
        //
        // The "shift" argument allows you to rotate the filter data
        // buffer so that the element with index "shift" becomes new
        // element with index 0 (essentially, the buffer is rotated
        // left by "shift" elements).
        */
        template <typename Real>
        void setFilter(const Real* data, unsigned dataLen, unsigned shift=0);

        /**
        // Convolve provided data with the filter previously established
        // by setFilter.
        //
        // The "shift" argument allows you to rotate the input data
        // buffer so that the element with index "shift" becomes new
        // element with index 0 (essentially, the buffer is rotated
        // left by "shift" elements).
        */
        template <typename Real1, typename Real2>
        void convolveWithFilter(const Real1* in, Real2* out, unsigned dataLen,
                                unsigned shift=0);

        /** Deconvolve the input using the previously established filter */
        template <typename Real1, typename Real2>
        void deconvolveFilter(const Real1* in, Real2* out, unsigned dataLen,
                              unsigned shift=0);

        /**
        // Provide a filter for subsequent convolutions.
        //
        // The "shift" argument allows you to rotate the filter data
        // buffer so that the element with index "shift" becomes new
        // element with index 0 (essentially, the buffer is rotated
        // left by "shift" elements).
        //
        // "id" is the user id for the filter which can be later reused
        // with the method "convolveWithDeposit".
        */
        template <typename Real>
        void depositFilter(unsigned long id,
                           const Real* data, unsigned dataLen,
                           unsigned shift=0);

        /**
        // Convolve provided data with one of the filters previously
        // established by the depositFilter method. The "id" argument
        // is used to identify a particular filter.
        //
        // The "shift" argument allows you to rotate the input data
        // buffer so that the element with index "shift" becomes new
        // element with index 0 (essentially, the buffer is rotated
        // left by "shift" elements).
        */
        template <typename Real1, typename Real2>
        void convolveWithDeposit(unsigned long id,
                                 const Real1* in, Real2* out, unsigned dataLen,
                                 unsigned shift=0);

        /** Deconvolve the data with a previously deposited filter */
        template <typename Real1, typename Real2>
        void deconvolveDeposit(unsigned long id,
                               const Real1* in, Real2* out, unsigned dataLen,
                               unsigned shift=0);

        /**
        // Discard a filter previously deposited by depositFilter method.
        // This method returns "true" if the filter with the given id
        // was indeed discarded and "false" if such a filter was not found.
        */
        bool discardFilter(unsigned long id);

        /** Expected data length */
        inline unsigned dataLength() const {return dataLen_;}

    private:
        typedef std::map<unsigned long,FourierImage*> FilterMap;

        ConvolutionEngine1D();
        ConvolutionEngine1D(const ConvolutionEngine1D&);
        ConvolutionEngine1D& operator=(const ConvolutionEngine1D&);

        template <typename Real1, typename Real2>
        void convolveWithImage(const fftw_complex* image,
                               const Real1* in, Real2* out, unsigned dataLen,
                               unsigned shift);

        template <typename Real1, typename Real2>
        void deconvolveImage(const fftw_complex* image,
                             const Real1* in, Real2* out, unsigned dataLen,
                             unsigned shift);
        fftw_plan pf;
        fftw_plan pb;

        double* in;
        fftw_complex* out;
        fftw_complex* filterImage;

        unsigned dataLen_;
        bool validFilter_;

        FilterMap filterMap_;

#ifdef SWIG
    public:
        inline void setFilter2(const double* data,
                               unsigned dataLen, unsigned shift=0)
            {setFilter(data, dataLen, shift);}

        inline void setFilter2(const float* data,
                               unsigned dataLen, unsigned shift=0)
            {setFilter(data, dataLen, shift);}

        inline void convolveWithFilter2(const double* in, double* out,
                                        unsigned dataLen, unsigned shift=0)
            {convolveWithFilter(in, out, dataLen, shift);}

        inline void convolveWithFilter2(const float* in, float* out,
                                        unsigned dataLen, unsigned shift=0)
            {convolveWithFilter(in, out, dataLen, shift);}

        inline void depositFilter2(unsigned long id, const double* data,
                                   unsigned dataLen, unsigned shift=0)
            {depositFilter(id, data, dataLen, shift);}

        inline void depositFilter2(unsigned long id, const float* data,
                                   unsigned dataLen, unsigned shift=0)
            {depositFilter(id, data, dataLen, shift);}

        inline void convolveWithDeposit2(unsigned long id,
                                         const double* in, double* out,
                                         unsigned dataLen, unsigned shift=0)
            {convolveWithDeposit(id, in, out, dataLen, shift);}

        inline void convolveWithDeposit2(unsigned long id,
                                         const float* in, float* out,
                                         unsigned dataLen, unsigned shift=0)
            {convolveWithDeposit(id, in, out, dataLen, shift);}
#endif // SWIG
    };
}

#include "npstat/nm/ConvolutionEngine1D.icc"

#endif // NPSTAT_CONVOLUTIONENGINE1D_HH_
