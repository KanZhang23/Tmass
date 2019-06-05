#ifndef NPSTAT_NTUPLEBUFFER_HH_
#define NPSTAT_NTUPLEBUFFER_HH_

//=========================================================================
// NtupleBuffer.hh
//
// Buffer for data exchange between ArchivedNtuple and underlying archive.
// Application code should never use this header directly.
//
// Author: I. Volobouev
//
// November 2010
//=========================================================================

#include <vector>
#include <cassert>
#include <stdexcept>

#include "geners/ClassId.hh"
#include "geners/Int2Type.hh"

namespace npstat {
    template <typename T>
    class NtupleBuffer
    {
    public:
        typedef T value_type;

        inline NtupleBuffer()
            : firstRow_(0),  maxrows_(0), ncols_(0),
              writeColumnByColumn_(false) {}

        inline NtupleBuffer(const unsigned long maxrows,
                            const unsigned long ncols,
                            const bool writeColumnByColumn)
            : firstRow_(0),
              maxrows_(maxrows),
              ncols_(ncols),
              writeColumnByColumn_(writeColumnByColumn)
        {
            if (maxrows_ == 0)
                ++maxrows_;
            if (!ncols_) throw std::invalid_argument(
                "In npstat::NtupleBuffer constructor: "
                "at least one column is required");
            data_.reserve(maxrows_*ncols_);
            if (writeColumnByColumn)
            {
                columnOffsets_.reserve(ncols_);
                for (unsigned long i=0; i<ncols_; ++i)
                    columnOffsets_.push_back(0LL);
            }
        }

        inline unsigned long nColumns() const {return ncols_;}
        inline unsigned long maxrows() const {return maxrows_;}
        inline unsigned long firstRow() const {return firstRow_;}
        inline unsigned long itemsBuffered() const {return data_.size();}
        inline bool writeByColumn() const {return writeColumnByColumn_;}
        inline bool isFull() const {return data_.size() >= maxrows_*ncols_;}
        inline const std::vector<long long>& columnOffsets() const
            {return columnOffsets_;}

        inline unsigned long nRows() const
            {return firstRow_ + data_.size()/ncols_;}
        inline bool rowInRange(const unsigned long row) const
            {return row >= firstRow_ && row - firstRow_ < data_.size()/ncols_;}

        inline T operator()(const unsigned long row,
                            const unsigned long c) const
        {
            return data_[(row-firstRow_)*ncols_ + c];
        }
        inline T at(const unsigned long row, const unsigned long c) const
        {
            if (row < firstRow_)
                throw std::out_of_range("In npstat::NtupleBuffer::at: "
                                        "row number is out of range");
            if (c >= ncols_)
                throw std::out_of_range("In npstat::NtupleBuffer::at: "
                                        "column number is out of range");
            return data_.at((row-firstRow_)*ncols_ + c);
        }

        inline bool fill(const T* values, const unsigned long lenValues)
        {
            if (lenValues != ncols_)
                throw std::invalid_argument("In npstat::NtupleBuffer::fill: "
                                            "incompatible data size");
            assert(values);
            for (unsigned long i=0; i<lenValues; ++i)
                data_.push_back(values[i]);
            return data_.size() < maxrows_*ncols_;
        }

        inline void clear()
        {
            firstRow_ += (data_.size()/ncols_);
            data_.clear();
        }

        inline bool rowContents(const unsigned long absRow, T* buf,
                                const unsigned long lenBuf) const
        {
            if (absRow < firstRow_)
                return false;
            const unsigned long row = absRow - firstRow_;
            if (row >= data_.size()/ncols_)
                return false;
            if (lenBuf < ncols_)
                throw std::invalid_argument(
                    "In npstat::NtupleBuffer::rowContents:"
                    " provided buffer is too small");
            assert(buf);
            const T* local = &data_[0] + row*ncols_;
            for (unsigned long i=0; i<ncols_; ++i)
                buf[i] = local[i];
            return true;
        }

        inline bool columnContents(const unsigned long col, T* buf,
                                   const unsigned long lenBuf) const
        {
            if (col >= ncols_)
                throw std::out_of_range(
                    "In npstat::NtupleBuffer::columnContents:"
                    " column number is out of range");
            const unsigned long localRows = data_.size()/ncols_;
            if (localRows)
            {
                if (lenBuf < localRows)
                    throw std::invalid_argument(
                        "In npstat::NtupleBuffer::columnContents:"
                        " provided buffer is too small");
                assert(buf);
                const T* local = &data_[0] + col;
                for (unsigned long i=0; i<localRows; ++i)
                    buf[i] = local[ncols_*i];
            }
            return true;
        }

        inline bool operator==(const NtupleBuffer& r) const
        {
            return firstRow_ == r.firstRow_ &&
                   maxrows_ == r.maxrows_ &&
                   ncols_ == r.ncols_ &&
                   writeColumnByColumn_ == r.writeColumnByColumn_ &&
                   data_ == r.data_;
        }
        inline bool operator!=(const NtupleBuffer& r) const
            {return !(*this == r);}

        inline void setFirstRow(const unsigned long off) {firstRow_ = off;}

        // Methods needed for I/O
        gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream&) const;

        static const char* classname();
        static inline unsigned version() {return 1;}
        static void restore(const gs::ClassId& id, std::istream& in, 
                            NtupleBuffer* buf);
        static void readColumn(const gs::ClassId& id, std::istream& in,
                               unsigned long column, long long offset,
                               T* buffer, unsigned long bufferLength);
    private:
        bool writeTransposed(std::ostream&, gs::Int2Type<true>) const;
        bool writeTransposed(std::ostream&, gs::Int2Type<false>) const;
        void readTransposed(std::istream&, unsigned long, gs::Int2Type<false>);
        void readTransposed(std::istream&, unsigned long, gs::Int2Type<true>);

        static bool readColumnWOffset(
            std::istream& in, long long offset,
            T* buffer, unsigned long nrows, gs::Int2Type<false>);

        static bool readColumnWOffset(
            std::istream& in, long long offset,
            T* buffer, unsigned long nrows, gs::Int2Type<true>);

        std::vector<T> data_;
        mutable std::vector<long long> columnOffsets_;
        unsigned long firstRow_;
        unsigned long maxrows_;
        unsigned long ncols_;
        bool writeColumnByColumn_;
    };
}

#include "npstat/stat/NtupleBuffer.icc"

#endif // NPSTAT_NTUPLEBUFFER_HH_
