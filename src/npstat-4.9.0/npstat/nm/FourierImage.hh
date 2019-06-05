#ifndef NPSTAT_FOURIERIMAGE_HH_
#define NPSTAT_FOURIERIMAGE_HH_

/*!
// \file FourierImage.hh
//
// \brief Memory manager for Fourier transforms produced by FFTW
//
// Author: I. Volobouev
//
// December 2011
*/

#include "fftw3.h"

namespace npstat {
    /**
    // Wrapper class for memory blocks allocated by FFTW function
    // fftw_malloc and deallocated by fftw_free. Intended for storing
    // results of Fourier transforms.
    */
    class FourierImage
    {
    public:
        FourierImage(const fftw_complex* image, unsigned long imageLen);
        FourierImage(const FourierImage&);
        FourierImage& operator=(const FourierImage&);

        ~FourierImage();

        inline const fftw_complex* image() const {return filterImage_;}
        inline unsigned long size() const {return imageLen_;}

    private:
        FourierImage();
        fftw_complex* filterImage_;
        unsigned long imageLen_;
    };

    /** Multiply two arrays of FFTW complex numbers */
    void multiplyTransforms(const fftw_complex* l, const fftw_complex* r,
                            fftw_complex* result, const unsigned long len);

    /** Divide two arrays of FFTW complex numbers */
    void divideTransforms(const fftw_complex* numer, const fftw_complex* denom,
                          fftw_complex* result, const unsigned long len);

    /** Multiply by a scalar. "l" and "result" can be the same. */
    void scaleTransform(const fftw_complex* l, double scale,
                        fftw_complex* result, const unsigned long len);
}

#endif // NPSTAT_FOURIERIMAGE_HH_
