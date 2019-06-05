#ifndef NPSTAT_RESPONSEMATRIX_HH_
#define NPSTAT_RESPONSEMATRIX_HH_

/*!
// \file ResponseMatrix.hh
//
// \brief Efficient response matrix representation for multidimensional unfolding
//
// Author: I. Volobouev
//
// June 2014
*/

#include <vector>
#include <utility>

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/Matrix.hh"

namespace npstat {
    class ResponseMatrix :
    public ArrayND<std::pair<std::vector<unsigned long>, std::vector<double> > >
    {
    public:
        ResponseMatrix(const ArrayShape& unfoldedShape,
                       const ArrayShape& observedShape);

        ResponseMatrix(const ArrayShape& unfoldedShape,
                       const ArrayShape& observedShape,
                       const Matrix<double>& denseMatrix);

        ResponseMatrix(const unsigned* unfoldedShape, unsigned unfoldedDim,
                       const unsigned* observedShape, unsigned observedDim);

        ResponseMatrix(const unsigned* unfoldedShape, unsigned unfoldedDim,
                       const unsigned* observedShape, unsigned observedDim,
                       const Matrix<double>& denseMatrix);

        void shrinkToFit() const;

        inline const ArrayShape& observedShape() const {return toShape_;}
        
        unsigned long observedLength() const;

        bool isValid() const;

        void timesVector(const ArrayND<double>& a,
                         ArrayND<double>* result) const;

        void rowMultiply(const ArrayND<double>& a,
                         ArrayND<double>* result) const;

        ResponseMatrix T() const;

        bool operator==(const ResponseMatrix& r) const;

        inline bool operator!=(const ResponseMatrix& r) const
            {return !(*this == r);}

        Matrix<double> denseMatrix() const;

        /** Efficiency is calculated without bounds checking */
        double linearEfficiency(unsigned long index) const;

    private:
        void sparsify(const Matrix<double>& dense);

        ArrayShape toShape_;
    };
}

#endif // NPSTAT_RESPONSEMATRIX_HH_
