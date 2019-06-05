#ifndef NPSTAT_HISTOUTILS_HH_
#define NPSTAT_HISTOUTILS_HH_

/*!
// \file histoUtils.hh
//
// \brief Various utility code related to histogram filling, etc
//
// Author: I. Volobouev
//
// April 2015
*/

#include <utility>

#include "npstat/stat/HistoND.hh"
#include "npstat/stat/AbsDistribution1D.hh"

namespace npstat {
    /**
    // Fill histogram with weighted values. Weights are calculated
    // on the basis of position of an entry in a sorted collection
    // (typically, std::vector). If the collection has length N, the
    // entry with number m is assigned the "coordinate" x = m/(N - 1).
    // Then the argument density is used to calculate a weight from
    // this "coordinate".
    //
    // The coordinate with which the histogram is filled is calculated
    // from each entry by a functor. This functor (represented by the
    // CoordExtractor template parameter) should take (a const reference
    // to) a collection element as its argument and return a double.
    //
    // It is assumed that the density has a finite bandwidth. The
    // collection scan starts near the "coordinate" obtained from
    // the density "location" function and proceeds up and down only
    // until the first point with 0 weight is encountered. Typical
    // densities useful with this function are TruncatedGauss1D
    // and SymmetricBeta1D.
    //
    // The function returns the Kish's effective sample size for the
    // histogram fills performed.
    */
    template<class Axis, class Collection, class CoordExtractor>
    double fill1DHistoWithCDFWeights(const Collection& coll,
                                   const AbsScalableDistribution1D& weightCalc,
                                   CoordExtractor extractor,
                                   HistoND<double,Axis>* h);

    /**
    // Similar function which fills the histogram with a product of
    // weights calculated by "weightCalc" and "coordWeightCalc".
    // "weightCalc" works just like in the previous function.
    // "coordWeightCalc" is a functor which is given two arguments:
    // the collection element and the coordinate of the bin center.
    // For each collection element, each bin of the histogram is filled
    // with the product of the weights returned by "weightCalc" and
    // "coordWeightCalc".
    //
    // The function returns the sum of the weights calculated by
    // "weightCalc" as the first element of the pair and the sum of
    // these weights squared as the second element.
    */
    template<class Axis, class Collection, class CoordWeightCalc>
    std::pair<double,double>
    multiFill1DHistoWithCDFWeights(const Collection& coll,
                                   const AbsScalableDistribution1D& weightCalc,
                                   CoordWeightCalc coordWeightCalc,
                                   HistoND<double,Axis>* h);
}

#include "npstat/stat/histoUtils.icc"

#endif // NPSTAT_HISTOUTILS_HH_
