#ifndef NPSTAT_PTRBUFFERHANDLE_HH_
#define NPSTAT_PTRBUFFERHANDLE_HH_

//======================================================================
// PtrBufferHandle.hh
//
// This is an internal header which is subject to change without notice.
// Application code should never use the classes declared/defined in
// this header directly.
//
// Author: I. Volobouev
//
// June 2015
//======================================================================

#include <cassert>

namespace npstat {
    namespace Private {
        template <class T>
        class PtrBufferHandle
        {
        public:
            inline PtrBufferHandle(T** buf, const unsigned len)
                : buf_(buf), len_(len) {if (len_) assert(buf_);}

            inline ~PtrBufferHandle()
            {
                for (unsigned i=0; i<len_; ++i)
                    delete buf_[i];
            }

            inline T** release()
            {
                T** b = buf_;
                buf_ = 0;
                len_ = 0U;
                return b;
            }

        private:
            PtrBufferHandle();
            PtrBufferHandle(const PtrBufferHandle&);
            PtrBufferHandle& operator=(const PtrBufferHandle&);

            T** buf_;
            unsigned len_;
        };
    }
}

#endif // NPSTAT_PTRBUFFERHANDLE_HH_
