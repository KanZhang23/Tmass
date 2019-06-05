#ifndef NPSTAT_FILLARRAYCENTERSPRESERVINGAREAS_HH_
#define NPSTAT_FILLARRAYCENTERSPRESERVINGAREAS_HH_

/*!
// \file fillArrayCentersPreservingAreas.hh
//
// \brief Special copy of array data appropriate for reducing cell size
//        in smoothing scenarios
//
// Author: I. Volobouev
//
// April 2015
*/

#include "npstat/nm/ArrayND.hh"

namespace npstat {
    /**
    // Fill the second array from the first one in a special way.
    // It is assumed that, in each dimension, the number of elements
    // in the second array is a multiple of the number of elements
    // in the first one. Only the "central" element in the target
    // array will be filled, with the value increased in the proportion
    // of the number of elements factor. Other elements of the target
    // array will be filled with zeros. This function is useful in
    // certain rebinning and smoothing scenarios.
    */
    template <typename Num1, unsigned StackLen1, unsigned StackDim1,
              typename Num2, unsigned StackLen2, unsigned StackDim2>
    void fillArrayCentersPreservingAreas(
        const ArrayND<Num1,StackLen1,StackDim1>& from,
        ArrayND<Num2,StackLen2,StackDim2>* to)
    {
        assert(to);
        if (!(from.isShapeKnown() && to->isShapeKnown()))
            throw std::invalid_argument(
                "In npstat::fillArrayCentersPreservingAreas:"
                " uninitialized array");
        const unsigned dim = from.rank();
        if (dim != to->rank()) throw std::invalid_argument(
            "In npstat::fillArrayCentersPreservingAreas:"
            " incompatible array ranks");
        const unsigned* toShape = to->shapeData();
        const unsigned* fromShape = from.shapeData();
        unsigned factors[CHAR_BIT*sizeof(unsigned long)];
        double areaFactor = 1.0;
        for (unsigned idim=0; idim<dim; ++idim)
        {
            factors[idim] = toShape[idim]/fromShape[idim];
            if (toShape[idim] % fromShape[idim]) throw std::invalid_argument(
                "In npstat::fillArrayCentersPreservingAreas:"
                " array dimensions are not exact multiples");
            if (factors[idim] % 2U)
                areaFactor *= factors[idim];
            else
                areaFactor *= (factors[idim]/2.0);
        }
        const Num2 zero = Num2();
        const unsigned long toLen = to->length();
        unsigned toIndex[CHAR_BIT*sizeof(unsigned long)];
        unsigned fromIndex[CHAR_BIT*sizeof(unsigned long)];
        for (unsigned long i=0; i<toLen; ++i)
        {
            to->convertLinearIndex(i, toIndex, dim);

            // Is this particular index in the center of the cell?
            bool central = true;
            for (unsigned idim=0; idim<dim && central; ++idim)
            {
                fromIndex[idim] = toIndex[idim]/factors[idim];
                const unsigned rem = toIndex[idim] % factors[idim];
                const unsigned half = factors[idim]/2U;
                if (factors[idim] % 2U)
                    central = rem == half;
                else
                    central = rem == half || rem == half - 1U;
            }

            if (central)
                to->linearValue(i) = areaFactor*from.value(fromIndex, dim);
            else
                to->linearValue(i) = zero;
        }
    }

    /**
    // Check whether the shapes of two arrays are compatible for
    // running the "fillArrayCentersPreservingAreas" function on them
    */
    template <typename Num1, unsigned StackLen1, unsigned StackDim1,
              typename Num2, unsigned StackLen2, unsigned StackDim2>
    bool canFillArrayCentersPreservingAreas(
        const ArrayND<Num1,StackLen1,StackDim1>& from,
        const ArrayND<Num2,StackLen2,StackDim2>& to)
    {
        if (!(from.isShapeKnown() && to.isShapeKnown()))
            return false;
        const unsigned dim = from.rank();
        if (dim != to.rank())
            return false;
        const unsigned* toShape = to.shapeData();
        const unsigned* fromShape = from.shapeData();
        for (unsigned idim=0; idim<dim; ++idim)
            if (toShape[idim] % fromShape[idim])
                return false;
        return true;
    }
}

#endif // NPSTAT_FILLARRAYCENTERSPRESERVINGAREAS_HH_
