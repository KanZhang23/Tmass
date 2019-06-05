#ifndef NPSTAT_CONVERTTOSPHERICALRANDOM_HH_
#define NPSTAT_CONVERTTOSPHERICALRANDOM_HH_

/*!
// \file convertToSphericalRandom.hh
//
// \brief Random numbers on the surface of N-dimensional unit sphere
//
// Author: I. Volobouev
//
// March 2010
*/

namespace npstat {
    /**
    // This function converts an N-dimensional random number set from
    // a unit hypercube into a random direction in the N-dim space and
    // a random number between 0 and 1 which can be later used to generate
    // the distance from the origin.
    //
    // The method implementation depends crucially on the numerical
    // accuracies of the "inverseGaussCdf" and "incompleteGamma" special
    // functions (it is likely that both can be improved, especially the
    // latter).
    //
    // The function returns the "remaining" random number.
    // The "direction" array which must have at least "dim"
    // elements is filled with the random direction vector
    // of unit length.
    //
    // If "getRadialRandom" argument is set to "false", the
    // radial random number is not generated and -1 is returned.
    // Use this to increase the code speed if only the random
    // direction itself is needed.
    */
    double convertToSphericalRandom(const double* rnd, unsigned dim,
                                    double* direction,
                                    bool getRadialRandom=true);
}

#endif // NPSTAT_CONVERTTOSPHERICALRANDOM_HH_
