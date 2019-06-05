#ifndef LOADRESULTSFROMHS_HH_
#define LOADRESULTSFROMHS_HH_

#include <vector>
#include "JesIntegResult.hh"

// Load W + 4 jets ME integration results from a file created by the
// distributed computations framework. The arguments of this function
// are as follows:
//
//  filename   -- The name of the file (typically, with extension .hs)
//                in which the integration results are stored.
//
//  category   -- Histo-Scope category in which the results are stored
//                (if you don't know the category, run "hsdir" on the
//                input file).
//
//  results    -- The vector of loaded integration results which will
//                be filled by this function.
//
// The vector of results will not be reset by this function.
// Instead, new items will be added to it (so that this function
// can be used to combine items stored in multiple files).
//
void loadResultsFromHS(const char* filename, const char* category,
                       std::vector<JesIntegResult>* results);

#endif // LOADRESULTSFROMHS_HH_
