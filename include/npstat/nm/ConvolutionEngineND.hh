#ifndef NPSTAT_CONVOLUTIONENGINEND_HH_
#define NPSTAT_CONVOLUTIONENGINEND_HH_

/*!
// \file ConvolutionEngineND.hh
//
// \brief Fast multidimensional convolutions via Fourier 
//        transforms (FFTW interface)
//
// Author: I. Volobouev
//
// December 2011
*/

#include <map>
#include <vector>

#include "npstat/nm/FourierImage.hh"

namespace npstat {
    /**
    // Class for performing multivariate convolutions by FFT (using FFTW
    // library). Internally, transforms are calculated in double precision.
    */
    class ConvolutionEngineND
    {
    public:
        /**
        // Constructor takes the shape of the data and FFTW optimization
        // flag as parameters. Make sure the data dimensions are sufficient
        // to avoid the convolution wrap-around.
        */
        ConvolutionEngineND(const unsigned *shape, unsigned rank,
                            unsigned optimization = FFTW_ESTIMATE);
        ~ConvolutionEngineND();

        /** Provide the filter for subsequent convolutions */
        template <typename Real>
        void setFilter(const Real* data, const unsigned *dataShape,
                       unsigned dataRank);

        /**
        // Convolve provided data with the filter previously established
        // by setFilter
        */
        template <typename Real1, typename Real2>
        void convolveWithFilter(const Real1* in, Real2* out,
                                const unsigned *dataShape, unsigned dataRank);

        /**
        // Provide a filter for subsequent convolutions.
        //
        // "id" is the user id for the filter which can be later reused
        // with the method "convolveWithDeposit".
        */
        template <typename Real>
        void depositFilter(unsigned long id,
                           const Real* data, const unsigned *dataShape,
                           unsigned dataRank);

        /**
        // Convolve provided data with one of the filters previously
        // established by the depositFilter method. The "id" argument
        // is used to identify a particular filter.
        */
        template <typename Real1, typename Real2>
        void convolveWithDeposit(unsigned long id,
                                 const Real1* in, Real2* out,
                                 const unsigned *dataShape, unsigned dataRank);

        /**
        // Discard a filter previously deposited by depositFilter method.
        // This method returns "true" if the filter with the given id
        // was indeed discarded and "false" if such a filter was not found.
        */
        bool discardFilter(unsigned long id);

        /** Check if an array shape is compatible with this filter */
        bool isShapeCompatible(const unsigned *dataShape, unsigned rank) const;

        /** Expected data dimensionality */
        inline unsigned rank() const {return shape_.size();}

        /** Expected shape of the data */
        inline const std::vector<unsigned>& shape() const {return shape_;}

        /** Expected data length */
        inline unsigned long dataLength() const {return dataLen_;}

    private:
        typedef std::map<unsigned long,FourierImage*> FilterMap;

        ConvolutionEngineND();
        ConvolutionEngineND(const ConvolutionEngineND&);
        ConvolutionEngineND& operator=(const ConvolutionEngineND&);

        template <typename Real1, typename Real2>
        void convolveWithImage(const fftw_complex* image,
                               const Real1* in, Real2* out,
                               const unsigned *dataShape, unsigned dataRank);
        fftw_plan pf;
        fftw_plan pb;

        double* in;
        fftw_complex* out;
        fftw_complex* filterImage;

        std::vector<unsigned> shape_;
        unsigned long dataLen_;
        unsigned long cmplLen_;
        bool validFilter_;

        FilterMap filterMap_;

#ifdef SWIG
    public:
        inline void setFilter2(const float* data, const unsigned *dataShape,
                               unsigned dataRank)
            {setFilter(data, dataShape, dataRank);}

        inline void convolveWithFilter2(const float* in, float* out,
                                        const unsigned *dataShape,
                                        unsigned dataRank)
            {convolveWithFilter(in, out, dataShape, dataRank);}

        inline void setFilter2(const double* data, const unsigned *dataShape,
                               unsigned dataRank)
            {setFilter(data, dataShape, dataRank);}

        inline void convolveWithFilter2(const double* in, double* out,
                                        const unsigned *dataShape,
                                        unsigned dataRank)
            {convolveWithFilter(in, out, dataShape, dataRank);}        

        inline void depositFilter2(unsigned long id,
                                   const float* data,
                                   const unsigned *dataShape,
                                   unsigned dataRank)
            {depositFilter(id, data, dataShape, dataRank);}

        inline void convolveWithDeposit2(unsigned long id,
                                         const float* in, float* out,
                                         const unsigned *dataShape,
                                         unsigned dataRank)
            {convolveWithDeposit(id, in, out, dataShape, dataRank);}

        inline void depositFilter2(unsigned long id,
                                   const double* data,
                                   const unsigned *dataShape,
                                   unsigned dataRank)
            {depositFilter(id, data, dataShape, dataRank);}

        inline void convolveWithDeposit2(unsigned long id,
                                         const double* in, double* out,
                                         const unsigned *dataShape,
                                         unsigned dataRank)
            {convolveWithDeposit(id, in, out, dataShape, dataRank);}        
#endif
    };
}

#include "npstat/nm/ConvolutionEngineND.icc"

#endif // NPSTAT_CONVOLUTIONENGINEND_HH_
