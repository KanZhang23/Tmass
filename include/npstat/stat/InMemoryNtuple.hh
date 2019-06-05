#ifndef NPSTAT_INMEMORYNTUPLE_HH_
#define NPSTAT_INMEMORYNTUPLE_HH_

/*!
// \file InMemoryNtuple.hh
//
// \brief Homogeneous ntuple which must fit in computer memory
//
// Fast access to any ntuple row.
//
// Author: I. Volobouev
//
// November 2010
*/

#include <stdexcept>
#include "npstat/stat/AbsNtuple.hh"

namespace npstat {
    /**
    // Homogeneous ntuple which must fit in memory (so its size is limited).
    // See ArchivedNtuple class if you need to create really large ntuples.
    */
    template <typename T>
    class InMemoryNtuple : public AbsNtuple<T>
    {
    public:
        /**
        // Constructor arguments are as follows:
        //
        // colNames   -- naturally, the names of the ntuple columns
        //
        // ntTitle    -- some title for the ntuple (arbitrary string)
        */
        inline explicit InMemoryNtuple(const std::vector<std::string>& colNames,
                                       const char* ntTitle = 0)
            : AbsNtuple<T>(colNames, ntTitle), ncols_(colNames.size()) {}

        inline virtual ~InMemoryNtuple() {}

        /**
        // Number of rows. If the ntuple is always filled one row
        // at a time, this is also the number of fills.
        */
        inline unsigned long nRows() const {return data_.size()/ncols_;}

        /**
        // Add data to the ntuple. Will throw std::invalid_argument
        // in case lenValues is not divisible by the number of columns.
        */
        void fill(const T* values, unsigned long lenValues);

        //@{
        /**
        // Convenience method which works if the number of arguments equals
        // the number if colums (otherwise an exception will be thrown)
        */
        void fill(const T& v0);
        void fill(const T& v0, const T& v1);
        void fill(const T& v0, const T& v1, const T& v2);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4, const T& v5);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4, const T& v5, const T& v6);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4, const T& v5, const T& v6, const T& v7);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4, const T& v5, const T& v6, const T& v7,
                  const T& v8);
        void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                  const T& v4, const T& v5, const T& v6, const T& v7,
                  const T& v8, const T& v9);
        //@}

        /** Access individual elements without bounds checking */
        inline T operator()(const unsigned long r, const unsigned long c) const
            {return data_[r*ncols_ + c];}

        /** Access individual elements with bounds checking */
        inline T at(const unsigned long r, const unsigned long c) const
        {
            if (c >= ncols_)
                throw std::out_of_range("In npstat::InMemoryNtuple::at: "
                                        "column number is out of range");
            return data_.at(r*ncols_ + c);
        }

        /** Clear all ntuple contents */
        inline void clear() {data_.clear();}

        /**
        // Access one row at a time. The provided buffer should be
        // sufficiently large to contain the complete row.
        */
        void rowContents(unsigned long row,
                         T* buf, unsigned long lenBuf) const;

        /**
        // Access one column at a time. The provided buffer should be
        // sufficiently large to contain the complete column.
        */
        void columnContents(const Column& c,
                            T* buf, unsigned long lenBuf) const;

        //@{
        /** Method needed for "geners" I/O */
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        bool write(std::ostream&) const;
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}
        static InMemoryNtuple* read(const gs::ClassId& id, std::istream& in);

    protected:
        inline virtual bool isEqual(const AbsNtuple<T>& other) const
        {
            return this->columnNames() == other.columnNames() &&
                   this->title() == other.title() &&
                   data_ == static_cast<const InMemoryNtuple&>(other).data_;
        }

    private:
        std::vector<T> data_;
        unsigned long ncols_;
    };
}

#include "npstat/stat/InMemoryNtuple.icc"

#endif // NPSTAT_INMEMORYNTUPLE_HH_
