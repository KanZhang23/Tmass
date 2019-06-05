#ifndef NPSTAT_ARCHIVEDNTUPLE_HH_
#define NPSTAT_ARCHIVEDNTUPLE_HH_

/*!
// \file ArchivedNtuple.hh
//
// \brief Large, archive-based homogeneous ntuple
//
// Author: I. Volobouev
//
// November 2010
*/

#include "npstat/stat/AbsNtuple.hh"
#include "npstat/stat/NtupleBuffer.hh"
#include "npstat/stat/NtupleRecordTypesFwd.hh"

#include "geners/AbsArchive.hh"

namespace npstat {
    template <typename T>
    class NtupleReference;

    /**
    // Ntuple which does not have to fit in memory, all rows at the same time.
    // It can grow very large -- the size is limited only by the capacity
    // of the underlying archive which usually means by the size of available
    // disk space.
    */
    template <typename T>
    class ArchivedNtuple : public AbsNtuple<T>
    {
    public:
        typedef T value_type;

        /**
        // Constructor arguments are as follows:
        //
        // columnNames   -- naturally, the names of the ntuple columns
        //
        // title         -- some title for the ntuple (arbitrary string)
        //
        // archive       -- Archive in which the ntuple data will be stored.
        //                  This archive must exist while the ntuple is
        //                  in use.
        //
        // name          -- ntuple name label in the archive
        //
        // category      -- ntuple category labels in the archive
        //
        // rowsPerBuffer -- Number of rows to keep in memory simultaneously.
        //                  This number essentially defines how large are
        //                  going to be the chunks of data which are written
        //                  to the archive or read back every single time
        //                  the underlying code talks to the archive. The
        //                  optimal buffer size will depend on the data 
        //                  access pattern. For example, if rows will be
        //                  accessed in random order, it makes little sense
        //                  to have large buffers. On the other hand,
        //                  a large buffer will minimize the number of times
        //                  we have to go to the archive in case the rows
        //                  are accessed sequentially.
        //
        // writeColumnWise -- If this argument is "true", the data in the
        //                  buffers will be written column after column.
        //                  This mode is useful (results in faster data
        //                  access) in case the ntuple data will be used
        //                  predominatly via the "columnContents" method.
        */
        ArchivedNtuple(const std::vector<std::string>& columnNames,
                       const char* title, gs::AbsArchive& archive,
                       const char* name, const char* category,
                       unsigned long rowsPerBuffer,
                       bool writeColumnWise=false);

        inline virtual ~ArchivedNtuple() {write();}

        //@{
        /** Basic inspector of ntuple properties */
        inline gs::AbsArchive& archive() const {return ar_;}
        inline const std::string& name() const {return name_;}
        inline const std::string& category() const {return category_;}
        inline unsigned long rowsPerBuffer() const
            {return fillBuffer_.maxrows();}
        inline bool writesByColumn() const
            {return fillBuffer_.writeByColumn();}
        inline bool isReadable() const {return readable_;}
        inline bool isWritable() const {return writable_;}
        //@}

        /**
        // Each object of this type created in one particular program run
        // will have its own unique number. This number in not persistent.
        */
        inline unsigned long objectNumber() const {return objectNumber_;}

        /**
        // Add data to the ntuple. Will throw std::runtime_error in case
        // the underlying archive is not writable. lenValues must be
        // divisible by the number of columns.
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

        /**
        // Number of rows. If the ntuple is always filled one row
        // at a time, this is also the number of fills.
        */
        inline unsigned long nRows() const {return fillBuffer_.nRows();}

        /** Access individual elements without bounds checking */
        inline T operator()(const unsigned long row,
                            const unsigned long column) const;

        /** Access individual elements with bounds checking */
        inline T at(const unsigned long row,
                    const unsigned long column) const;

        /**
        // Access one row at a time. The provided buffer should be
        // sufficiently large to contain the complete row.
        */
        void rowContents(const unsigned long row,
                         T* buffer, unsigned long lenBuffer) const;

        /**
        // Access one column at a time. The provided buffer should be
        // sufficiently large to contain the complete column.
        */
        void columnContents(const Column& c,
                            T* buffer, unsigned long lenBuffer) const;

        /**
        // Can't really clear the data, it is in the archive already.
        // So this is just a NOOP.
        */
        virtual void clear() {}

        //@{
        /** Method needed for "geners" I/O */
        virtual bool write();
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        //@}

        static const char* classname();
        static inline unsigned version() {return 1;}

    protected:
        virtual bool isEqual(const AbsNtuple<T>& r) const;

    private:
        typedef ArchivedNtuple<T> MyNtuple; 

        friend class Private::NtupleHeaderRecord<MyNtuple>;
        friend class Private::NtupleBufferRecord<MyNtuple>;
        friend class Private::NtupleFooterRecord<MyNtuple>;
        friend class Private::NtupleBufferReference<MyNtuple>;
        friend class Private::NtupleColumnReference<MyNtuple>;
        friend class NtupleReference<MyNtuple>;

        ArchivedNtuple();
        ArchivedNtuple(const ArchivedNtuple&);
        ArchivedNtuple& operator=(const ArchivedNtuple&);

        // The following function is used by NtupleReference
        static ArchivedNtuple* read(gs::AbsArchive& ar,
                                    std::istream& is,
                                    unsigned long long headId);

        static unsigned long nextObjectNumber();

        bool loadRowData(unsigned long rowNumber) const;
        bool loadColumnSection(unsigned long firstRow, unsigned long col,
                               T* buf, unsigned long lenBuf) const;
        void saveHeader();
        void saveFillBuffer();

        gs::AbsArchive& ar_;

        std::string name_;
        std::string category_;

        NtupleBuffer<T> fillBuffer_;
        mutable NtupleBuffer<T> readBuffer_;
        gs::ClassId bufferClass_;

        std::vector<unsigned long long> idlist_;
        std::vector<long long> columnOffsets_;
        unsigned long long headerSaved_;
        unsigned long ncols_;
        const unsigned long objectNumber_;
        bool readable_;
        bool writable_;
    };
}

#include "npstat/stat/ArchivedNtuple.icc"

#endif // NPSTAT_ARCHIVEDNTUPLE_HH_
