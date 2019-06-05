#ifndef NPSTAT_PRODUCTRESPONSEMATRIX_HH_
#define NPSTAT_PRODUCTRESPONSEMATRIX_HH_

/*!
// \file productResponseMatrix.hh
//
// \brief Function for building sparse response matrices for multivariate
//        unfolding problems using product distributions
//
// Author: I. Volobouev
//
// June 2014
*/

#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/BoxND.hh"
#include "npstat/nm/AbsMultivariateFunctor.hh"

#include "npstat/stat/DistributionsND.hh"

namespace npstat {
    /**
    // Function arguments are as follows:
    //
    // unfoldedBox         -- boundaries for the "unfolded" (that is,
    //                        physical) space.
    //
    // unfoldedShape       -- number of subdivisions for each dimension of
    //                        the unfolded space. Uniform binning is used.
    //
    // observedBox         -- boundaries for the "observed" space.
    //
    // observedShape       -- number of subdivisions for each dimension of
    //                        the observed space. Uniform binning is used.
    //
    // distro              -- distribution to use in order to build the
    //                        response matrix. The method "isScalable"
    //                        of this distribution must return "true".
    //                        Locations and scales of "distro" components
    //                        will be modified while this function runs
    //                        but will be restored to their initial values
    //                        upon exit.
    //
    // shifts              -- these functors should calculate shifts in the
    //                        physical space, as a function of point location
    //                        in that space. If the function pointer is NULL,
    //                        the shift is set to 0. In order to calculate
    //                        the response in the obseved space, locations
    //                        of the marginals will be set to the coordinate
    //                        of the physical space point plus this shift.
    //
    // widthFactors        -- these functors should calculate width factors
    //                        used to mutiply the original scales, as
    //                        a function of point location in the physical
    //                        space. If the function pointer is NULL, the
    //                        factor is set to 1.
    //
    // It is expected that some or all of the "distro" marginals will
    // have finite support, otherwise the returned collection of triplets
    // will not be sparse. The first index of each triplet will correspond
    // to the cell number in the observed space, and the second index
    // will correspond to the cell number in the physical space.
    */
    template<class Triplet>
    CPP11_auto_ptr<std::vector<Triplet> > productResponseMatrix(
        const npstat::BoxND<double>& unfoldedBox,
        const std::vector<unsigned>& unfoldedShape,
        const npstat::BoxND<double>& observedBox,
        const std::vector<unsigned>& observedShape,
        npstat::ProductDistributionND& distro,
        const std::vector<npstat::AbsMultivariateFunctor*>& shifts,
        const std::vector<npstat::AbsMultivariateFunctor*>& widthFactors);
}

#include "npstat/stat/productResponseMatrix.icc"

#endif // NPSTAT_PRODUCTRESPONSEMATRIX_HH_
