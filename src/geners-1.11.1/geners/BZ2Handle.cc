#include <stdexcept>

#include "geners/BZ2Handle.hh"

namespace gs {
    BZ2InflateHandle::BZ2InflateHandle(bz_stream& strm)
        : strm_(&strm)
    {
        strm_->bzalloc = 0;
        strm_->bzfree = 0;
        strm_->opaque = 0;
        strm_->avail_in = 0;
        strm_->next_in = 0;
        if (BZ2_bzDecompressInit(strm_, 0, 0) != BZ_OK)
            throw std::runtime_error("In gs::BZ2InflateHandle constructor: "
                                     "bzip2 stream initialization failure");
    }

    BZ2InflateHandle::~BZ2InflateHandle()
    {
        BZ2_bzDecompressEnd(strm_);
    }

    BZ2DeflateHandle::BZ2DeflateHandle(bz_stream& strm)
        : strm_(&strm)
    {
        strm_->bzalloc = 0;
        strm_->bzfree = 0;
        strm_->opaque = 0;
        strm_->avail_in = 0;
        strm_->next_in = 0;
        if (BZ2_bzCompressInit(strm_, 9, 0, 0) != BZ_OK)
            throw std::runtime_error("In gs::BZ2DeflateHandle constructor: "
                                     "bzip2 stream initialization failure");
    }

    BZ2DeflateHandle::~BZ2DeflateHandle()
    {
        BZ2_bzCompressEnd(strm_);
    }
}
