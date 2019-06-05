#include <stdexcept>

#include "zlib.h"

#include "geners/ZlibHandle.hh"

namespace gs {
    ZlibInflateHandle::ZlibInflateHandle()
    {
        strm_ = new z_stream_s();
        strm_->zalloc = Z_NULL;
        strm_->zfree = Z_NULL;
        strm_->opaque = Z_NULL;
        strm_->avail_in = 0;
        strm_->next_in = Z_NULL;
        if (inflateInit(strm_) != Z_OK)
            throw std::runtime_error("In gs::ZlibInflateHandle constructor: "
                                     "zlib stream initialization failure");
    }

    ZlibInflateHandle::~ZlibInflateHandle()
    {
        inflateEnd(strm_);
        delete strm_;
    }

    ZlibDeflateHandle::ZlibDeflateHandle(const int lev)
        : level_(lev)
    {
        strm_ = new z_stream_s();
        strm_->zalloc = Z_NULL;
        strm_->zfree = Z_NULL;
        strm_->opaque = Z_NULL;
        strm_->avail_in = 0;
        strm_->next_in = Z_NULL;
        if (deflateInit(strm_, lev) != Z_OK)
            throw std::runtime_error("In gs::ZlibDeflateHandle constructor: "
                                     "zlib stream initialization failure");
    }

    ZlibDeflateHandle::~ZlibDeflateHandle()
    {
        deflateEnd(strm_);
        delete strm_;
    }
}
