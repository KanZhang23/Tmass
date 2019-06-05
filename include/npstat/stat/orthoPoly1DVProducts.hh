#ifndef NPSTAT_ORTHOPOLY1DVPRODUCTS_HH_
#define NPSTAT_ORTHOPOLY1DVPRODUCTS_HH_

/*!
// \file orthoPoly1DVProducts.hh
//
// \brief Utility functions for calculating statistical properties of
//        1-d orthogonal polynomials
//
// Author: I. Volobouev
//
// September 2017
*/

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/AbsClassicalOrthoPoly1D.hh"
#include "npstat/nm/matrixIndexPairs.hh"

namespace npstat {
    // See the "Empirical Chaos Polynomials" write-up for the
    // definition of v_{ab} and various related formulae for
    // expectations and covariances
    template <class Quadrature>
    double expectedVProduct(const AbsClassicalOrthoPoly1D& poly,
                            const Quadrature& quad, unsigned long nPoints,
                            UUPair ab, UUPair cd);

    template <class Quadrature>
    double expectedVProduct(const AbsClassicalOrthoPoly1D& poly,
                            const Quadrature& quad, unsigned long nPoints,
                            UUPair ab, UUPair cd, UUPair ef);

    template <class Quadrature>
    double expectedVProduct(const AbsClassicalOrthoPoly1D& poly,
                            const Quadrature& quad, unsigned long nPoints,
                            UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Covariance between v_{ab} and v_{cd}
    template <class Quadrature>
    double expectedVCovariance(const AbsClassicalOrthoPoly1D& poly,
                               const Quadrature& quad, unsigned long nPoints,
                               UUPair ab, UUPair cd);

    // Covariance between v_{ab}*v_{cd} and v_{ef}
    template <class Quadrature>
    double expectedVCovariance(const AbsClassicalOrthoPoly1D& poly,
                               const Quadrature& quad, unsigned long nPoints,
                               UUPair ab, UUPair cd, UUPair ef);

    // Covariance between v_{ab}*v_{cd} and v_{ef}*v_{gh}
    template <class Quadrature>
    double expectedVCovariance(const AbsClassicalOrthoPoly1D& poly,
                               const Quadrature& quad, unsigned long nPoints,
                               UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Covariance of sample covariance between v_{ab} and v_{cd}
    template <class Quadrature>
    double expectedVCovCov(const AbsClassicalOrthoPoly1D& poly,
                           const Quadrature& quad, unsigned long nPoints,
                           UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // v_{ab} calculated for a given sample of unweighted points.
    // For samples generated using the polynomial weight function
    // as the density, the expectation value of v_{ab} is 0.
    template <typename Numeric>
    double sampleVProduct(const AbsClassicalOrthoPoly1D& poly,
                          const Numeric* coords, unsigned long nCoords,
                          UUPair ab);

    // v_{ab}*v_{cd} calculated for a given sample of unweighted points
    template <typename Numeric>
    double sampleVProduct(const AbsClassicalOrthoPoly1D& poly,
                          const Numeric* coords, unsigned long nCoords,
                          UUPair ab, UUPair cd);

    // v_{ab}*v_{cd}*v_{ef} calculated for a given sample
    template <typename Numeric>
    double sampleVProduct(const AbsClassicalOrthoPoly1D& poly,
                          const Numeric* coords, unsigned long nCoords,
                          UUPair ab, UUPair cd, UUPair ef);

    // v_{ab}*v_{cd}*v_{ef}*v_{gh} calculated for a given sample
    template <typename Numeric>
    double sampleVProduct(const AbsClassicalOrthoPoly1D& poly,
                          const Numeric* coords, unsigned long nCoords,
                          UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Expectation of v_{ab}*v_{cd} using sample averages of oracle polys
    template <typename Numeric>
    double sampleVProductExp(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd);

    // Expectation of v_{ab}*v_{cd}*v_{ef} using sample averages
    // of oracle polys
    template <typename Numeric>
    double sampleVProductExp(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd, UUPair ef);

    // Expectation of v_{ab}*v_{cd}*v_{ef}*v_{gh} using sample averages
    // of oracle polys
    template <typename Numeric>
    double sampleVProductExp(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Covariance between v_{ab} and v_{cd} estimated using
    // polynomial averages taken from the given sample
    template <typename Numeric>
    double sampleVCovariance(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd);

    // Fill a 4-d array with "v_{mn}" covariances estimated from the sample.
    // The code knows that the covariances are symmetric with respect to
    // permuting some indices, and does not recalculate them when permuting
    // indices is sufficient. The array span will be maxdeg+1 in each
    // dimension.
    template <typename Numeric>
    ArrayND<double> sampleVCovarianceArray(
        const AbsClassicalOrthoPoly1D& poly, const Numeric* coords,
        unsigned long nCoords, unsigned maxdeg);

    // Covariance between v_{ab}*v_{cd} and v_{ef} estimated using
    // polynomial averages taken from the given sample
    template <typename Numeric>
    double sampleVCovariance(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd, UUPair ef);

    // Covariance between v_{ab}*v_{cd} and v_{ef}*v_{gh} estimated
    // using polynomial averages taken from the given sample
    template <typename Numeric>
    double sampleVCovariance(const AbsClassicalOrthoPoly1D& poly,
                             const Numeric* coords, unsigned long nCoords,
                             UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Covariance of sample covariance between v_{ab} and v_{cd} estimated
    // using polynomial averages taken from the given sample
    template <typename Numeric>
    double sampleVCovCov(const AbsClassicalOrthoPoly1D& poly,
                         const Numeric* coords, unsigned long nCoords,
                         UUPair ab, UUPair cd, UUPair ef, UUPair gh);

    // Oracle counterpart of the "epsExpectation" method of the
    // ContOrthoPoly1D class. If "highOrder" parameter is "true",
    // the calculations will be performed to O(N^{-3/2}). If "highOrder"
    // is "false", the calculations will be to O(N^{-1}).
    template <class Quadrature>
    double oracleEpsExpectation(const AbsClassicalOrthoPoly1D& poly,
                                const Quadrature& quad, unsigned long nPoints,
                                unsigned m, unsigned n, bool highOrder);

    // Oracle counterpart of the "epsCovariance" method of the
    // ContOrthoPoly1D class
    template <class Quadrature>
    double oracleEpsCovariance(const AbsClassicalOrthoPoly1D& poly,
                               const Quadrature& quad, unsigned long nPoints,
                               unsigned m1, unsigned n1,
                               unsigned m2, unsigned n2, bool highOrder);

    // Fill a 4-d array with oracle "eps_{mn}" covariances. The code knows 
    // that the covariances are symmetric with respect to permuting some
    // indices, and does not recalculate them when permuting indices is
    // sufficient. The array span will be maxdeg+1 in each dimension.
    template <class Quadrature>
    ArrayND<double> oracleEpsCovarianceArray(
        const AbsClassicalOrthoPoly1D& poly, const Quadrature& quad,
        unsigned long nPoints, unsigned maxdeg, bool highOrder);

    // Calculate the "eps_{mn}" approximation for a given sample
    // of unweighted points (up to O(N^{-1}) or O(N^{-3/2})). In
    // this approximation, all v_{ij} values are calculated using
    // sample averages of oracle polynomials. Note that the result
    // is not the exact "eps_{mn}" for the sample but just this
    // particular special approximation.
    template <typename Numeric>
    double sampleEpsValue(const AbsClassicalOrthoPoly1D& poly,
                          const Numeric* coords, unsigned long nCoords,
                          unsigned m, unsigned n, bool highOrder);

    // Counterpart of the "epsExpectation" method of the ContOrthoPoly1D
    // class in which chaos polys are replaced by oracle polys. It is useful
    // for checking the effect of replacing the empirical polys with the
    // oracle ones in the "eps_{mn}" approximation.
    template <typename Numeric>
    double sampleEpsExpectation(const AbsClassicalOrthoPoly1D& poly,
                                const Numeric* coords, unsigned long nCoords,
                                unsigned m, unsigned n, bool highOrder);

    // eps_{mn} covariance approximation using polynomial product
    // averages from the given sample of points
    template <typename Numeric>
    double sampleEpsCovariance(const AbsClassicalOrthoPoly1D& poly,
                               const Numeric* coords, unsigned long nCoords,
                               unsigned m1, unsigned n1,
                               unsigned m2, unsigned n2, bool highOrder);

    // Fill a 4-d array with "eps_{mn}" covariances estimated from the sample.
    // The code knows that the covariances are symmetric with respect to
    // permuting some indices, and does not recalculate them when permuting
    // indices is sufficient. The array span will be maxdeg+1 in each
    // dimension.
    template <typename Numeric>
    ArrayND<double> sampleEpsCovarianceArray(
        const AbsClassicalOrthoPoly1D& poly, const Numeric* coords,
        unsigned long nCoords, unsigned maxdeg, bool highOrder);
}

#include "npstat/stat/orthoPoly1DVProducts.icc"

#endif // NPSTAT_ORTHOPOLY1DVPRODUCTS_HH_
