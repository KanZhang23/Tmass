//=========================================================================
// CatalogIO.hh
//
// The following functions provide an operational definition of
// the "standard" binary catalog file format
//
// I. Volobouev
// November 2010
//=========================================================================

#ifndef GENERS_CATALOGIO_HH_
#define GENERS_CATALOGIO_HH_

#include <iostream>

#include "geners/AbsCatalog.hh"

namespace gs {
    struct CatalogFormat1
    {
        enum {ID = 713265489};
    };

    // In the following, it is assumed that the catalog is stored
    // in memory and that it has a stream dedicated to it. The
    // function returns "true" on success.
    //
    // Programs which make new catalogs should set "mergeLevel" to 1.
    // Programs which combine existing catalogs should add up the
    // merge levels of those catalogs to come up with the "mergeLevel".
    //
    // "annotations" are arbitrary strings (which should be just combined
    // when catalogs are merged).
    bool writeBinaryCatalog(std::ostream& os, unsigned compressionCode,
                            unsigned mergeLevel,
                            const std::vector<std::string>& annotations,
                            const AbsCatalog& catalog,
                            unsigned formatId = CatalogFormat1::ID);

    // In the following, it is assumed that the Catalog class
    // has a "read" function which builds the catalog on the heap.
    // The "allowReadingByDifferentClass" parameter specifies
    // whether the catalog class is allowed to read something
    // written by another catalog class. This function returns NULL
    // pointer on failure. Note that the user must call "delete"
    // on the returned pointer at some point in the future.
    template <class Catalog>
    Catalog* readBinaryCatalog(std::istream& is, unsigned* compressionCode,
                               unsigned* mergeLevel,
                               std::vector<std::string>* annotations,
                               bool allowReadingByDifferentClass,
                               unsigned formatId = CatalogFormat1::ID);
}

#include "geners/CatalogIO.icc"

#endif // GENERS_CATALOGIO_HH_
