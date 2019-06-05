#include <cassert>
#include <cstring>
#include <stdexcept>

#include "npstat/nm/FourierImage.hh"

namespace npstat {
    FourierImage::FourierImage(const fftw_complex* iimage,
                               const unsigned long imageLen)
        : imageLen_(imageLen)
    {
        filterImage_ = static_cast<fftw_complex *>(fftw_malloc(
            imageLen_ * sizeof(fftw_complex)));
        assert(filterImage_);
        memcpy(filterImage_, iimage, imageLen_*sizeof(fftw_complex));
    }

    FourierImage::~FourierImage()
    {
        fftw_free(filterImage_);
    }

    FourierImage::FourierImage(const FourierImage& r)
        : imageLen_(r.imageLen_)
    {
        filterImage_ = static_cast<fftw_complex *>(fftw_malloc(
            imageLen_ * sizeof(fftw_complex)));
        assert(filterImage_);
        memcpy(filterImage_, r.filterImage_, imageLen_*sizeof(fftw_complex));
    }

    FourierImage& FourierImage::operator=(const FourierImage& r)
    {
        if (this != &r)
        {
            if (imageLen_ != r.imageLen_)
            {
                fftw_free(filterImage_);
                imageLen_ = r.imageLen_;
                filterImage_ = static_cast<fftw_complex *>(fftw_malloc(
                                    imageLen_ * sizeof(fftw_complex)));
                assert(filterImage_);
            }
            memcpy(filterImage_, r.filterImage_, imageLen_*sizeof(fftw_complex));
        }
        return *this;
    }

    void multiplyTransforms(const fftw_complex* l, const fftw_complex* r,
                            fftw_complex* res, const unsigned long len)
    {
        if (len)
        {
            assert(l);
            assert(r);
            assert(res);
            for (unsigned long i=0; i<len; ++i)
            {
                // Need a temporary here in case "res" coincides
                // with either "l" or "r"
                const double tmp = l[i][0]*r[i][1] + l[i][1]*r[i][0];
                res[i][0] = l[i][0]*r[i][0] - l[i][1]*r[i][1];
                res[i][1] = tmp;
            }
        }
    }

    void divideTransforms(const fftw_complex* l, const fftw_complex* r,
                          fftw_complex* res, const unsigned long len)
    {
        if (len)
        {
            assert(l);
            assert(r);
            assert(res);
            for (unsigned long i=0; i<len; ++i)
            {
                const double denom = r[i][0]*r[i][0] + r[i][1]*r[i][1];
                if (denom)
                {
                    // Need a temporary here in case "res" coincides
                    // with either "l" or "r"
                    const double tmp = l[i][1]*r[i][0] - l[i][0]*r[i][1];
                    res[i][0] = (l[i][0]*r[i][0] + l[i][1]*r[i][1])/denom;
                    res[i][1] = tmp/denom;
                }
                else
                {
                    if (l[i][0] || l[i][1]) throw std::invalid_argument(
                        "In npstat::divideTransforms: division by complex zero");
                    res[i][0] = 0.0;
                    res[i][1] = 0.0;
                }
            }
        }
    }

    void scaleTransform(const fftw_complex* l, const double scale,
                        fftw_complex* res, const unsigned long len)
    {
        if (len)
        {
            assert(l);
            assert(res);
            for (unsigned long i=0; i<len; ++i)
            {
                res[i][0] = l[i][0]*scale;
                res[i][1] = l[i][1]*scale;
            }
        }
    }
}
