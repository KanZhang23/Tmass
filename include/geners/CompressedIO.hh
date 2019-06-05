// This is a simple high-level driver to write items into streams
// in a compressed form. Useful if, due to some reason, we do not
// want to write directly into a compressed archive.
//
// Note that this code is rather slow, and is not recommended for
// saving a lot of small objects (use a compressed archive instead).

#ifndef GENERS_COMPRESSEDIO_HH_
#define GENERS_COMPRESSEDIO_HH_

#include "geners/GenericIO.hh"
#include "geners/CStringStream.hh"

namespace gs {
    // The following function returns "true" on success, "false" on failure
    template <class Item>
    bool write_compressed_item(std::ostream& os, const Item& item,
             CStringStream::CompressionMode m = CStringStream::ZLIB,
             int compressionLevel = -1, unsigned minSizeToCompress = 1024U,
             unsigned bufSize = 1048576U);

    template <class Item>
    void restore_compressed_item(std::istream& in, Item* item);

    template <class Item>
    CPP11_auto_ptr<Item> read_compressed_item(std::istream& in);
}

#include "geners/CompressedIO.icc"

#endif // GENERS_COMPRESSEDIO_HH_
