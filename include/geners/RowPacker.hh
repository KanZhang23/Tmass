// A persistent tuple. Allows for type-safe access to its arguments.
// Optimized for a single row access (one tuple at a time).

#ifndef GENERS_ROWPACKER_HH_
#define GENERS_ROWPACKER_HH_

#include "geners/CPP11_config.hh"
#ifdef CPP11_STD_AVAILABLE

#include <utility>

#include "geners/AbsArchive.hh"
#include "geners/ClassId.hh"
#include "geners/CharBuffer.hh"
#include "geners/RPHeaderRecord.hh"
#include "geners/RPBufferRecord.hh"
#include "geners/RPFooterRecord.hh"
#include "geners/RPBufferReference.hh"
#include "geners/RPReference.hh"

namespace gs {
    template<typename Pack>
    class RowPacker
    {
        template<typename Pack2> friend class RowPacker;
        static const unsigned defaultBufferSize = 800000U;

    public:
        typedef Pack value_type;

        // Comment on choosing the buffer size: this size should
        // be appropriate for the compression library used with
        // the archive, as each buffer is compressed independently.
        // For example, the bzip2 compression is done in chunks
        // which are slightly less than 900 kB (because in that
        // library 1 kB == 1000 bytes and because a small part of
        // the buffer is used for housekeeping). At the same time,
        // guaranteeing exact limit on the buffer size would slow
        // the code down: as the serialized size of each object
        // is not known in advance, each row would have to be
        // serialized into a separate buffer first instead of using
        // the common buffer. The code below utilizes a different
        // approach: the buffer can grow slightly above the limit,
        // and the buffer is dumped into the archive as soon as
        // its size exceeds the limit. Thus the actual size of the
        // archived buffer can be the value of "bufferSize"
        // parameter plus size of one serialized table row.
        // In principle, row size can be arbitrary, and this is why
        // the choice is left to the user.
        //
        // Naturally, setting the buffer size to 0 will result in
        // every row serialized and dumped to the archive as
        // a separate entity. This setting can have its own good
        // use if the expected row access pattern is random.
        //
        RowPacker(const std::vector<std::string>& columnNames,
                  const char* title, AbsArchive& archive,
                  const char* name, const char* category,
                  unsigned bufferSize = defaultBufferSize);

        // The following constructor will work only if every
        // element of the "protoPack" has a function with the
        // signature "const std::string& name() const" (think
        // gs::IOProxy).
        //
        RowPacker(const char* title, AbsArchive& archive,
                  const char* name, const char* category,
                  const Pack& protoPack,
                  unsigned bufferSize = defaultBufferSize);

        // A minimalistic constructor which can be used if you
        // do not care about things like column names and table
        // title. Default values will be assigned instead: all
        // columns will be named "c0", "c1", ..., and the title
        // will be an empty string.
        RowPacker(AbsArchive& archive,
                  const char* name, const char* category,
                  unsigned bufferSize = defaultBufferSize);

        inline ~RowPacker() {write();}

        // Various simple inspectors
        inline AbsArchive& archive() const {return ar_;}
        inline const std::string& name() const {return name_;}
        inline const std::string& category() const {return category_;}
        inline unsigned bufferSize() const {return bufferSize_;}
        inline bool isReadable() const {return readable_;}
        inline bool isWritable() const {return writable_;}
        inline const std::string& title() const {return title_;}

        // Each object of this type created in one particular program
        // will have its own unique number. This number in not persistent.
        inline unsigned long objectNumber() const {return objectNumber_;}

        // Simple modifiers
        inline void setTitle(const char* newtitle)
            {title_ = newtitle ? newtitle : "";}
        inline void setBufferSize(const unsigned newsize)
            {bufferSize_ = newsize;}

        // Dealing with rows and columns
        inline unsigned long nRows() const {return fillCount_;}

        inline unsigned long nColumns() const 
            {return std::tuple_size<Pack>::value;}

        inline const std::string& columnName(const unsigned long i) const
            {return colNames_.at(i);}

        inline const std::vector<std::string>& columnNames() const
            {return colNames_;}

        unsigned long columnNumber(const char* columnName) const;

        // The code will refuse to set the column name (and false will be
        // returned in this case) if the provided name duplicates an existing
        // column name. False will also be returned if the column index
        // is out of range.
        bool setColumnName(unsigned long i, const char* newname);

        // Fill one tuple. This method will return "true" on success and
        // "false" on failure.
        bool fill(const Pack& tuple);

        // Read the row contents back. This method will throw
        // std::out_of_range in case the row number is out of range.
        void rowContents(unsigned long row, Pack* tuple) const;

        template<typename Pack2>
        bool operator==(const RowPacker<Pack2>& r) const;

        template<typename Pack2>
        inline bool operator!=(const RowPacker<Pack2>& r) const
            {return !(*this == r);}

        // Methods needed for I/O
        bool write();
        inline ClassId classId() const {return ClassId(*this);}

        static const char* classname();
        static inline unsigned version() {return 1;}

    private:
        friend class Private::RPHeaderRecord<RowPacker<Pack> >;
        friend class Private::RPBufferRecord<RowPacker<Pack> >;
        friend class Private::RPFooterRecord<RowPacker<Pack> >;
        friend class Private::RPBufferReference<RowPacker<Pack> >;
        friend class RPReference<RowPacker<Pack> >;

        RowPacker();
        RowPacker(const RowPacker&);
        RowPacker& operator=(const RowPacker&);

        // The following function is used by RPReference
        static RowPacker* read(AbsArchive& ar,
                               std::istream& is,
                               unsigned long long headId);

        static unsigned long nextObjectNumber();

        void prepareToUnpack() const;
        bool unpackTuple(std::istream& is, Pack* tuple) const;
        std::istream& getRowStream(unsigned long row,
                                   unsigned long* len = 0) const;

        bool loadRowData(unsigned long rowNumber) const;
        void saveHeader();
        void saveFillBuffer();

        mutable CharBuffer fillBuffer_;
        unsigned long firstFillBufferRow_;
        std::vector<std::streampos> fillBufferOffsets_;

        mutable CharBuffer readBuffer_;
        mutable unsigned long firstReadBufferRow_;
        mutable std::vector<std::streampos> readBufferOffsets_;
        ClassId bufferClass_;
        ClassId thisClass_;

        AbsArchive& ar_;
        std::vector<std::string> colNames_;

        // The first member of the pair is the first row in the
        // buffer, the second is the id of the buffer in the archive
        std::vector<std::pair<unsigned long,unsigned long long> > idlist_;

        std::string name_;
        std::string category_;
        std::string title_;
        unsigned long long headerSaved_;
        unsigned long bufferSize_;
        unsigned long fillCount_;
        unsigned long objectNumber_;
        bool readable_;
        bool writable_;
        mutable bool firstUnpack_;
        bool unused_;
        mutable std::vector<std::vector<ClassId> > iostack_;

        static const std::vector<std::string>& defaultColumnNames();
    };
}

#include "geners/RowPacker.icc"

#endif // CPP11_STD_AVAILABLE
#endif // GENERS_ROWPACKER_HH_
