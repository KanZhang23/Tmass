// A persistent tuple. Allows for type-safe access to its arguments.
// Optimized for column access.

#ifndef GENERS_COLUMNPACKER_HH_
#define GENERS_COLUMNPACKER_HH_

#include "geners/CPP11_config.hh"
#ifdef CPP11_STD_AVAILABLE

#include "geners/ColumnPackerHelper.hh"
#include "geners/CPHeaderRecord.hh"
#include "geners/CPFooterRecord.hh"
#include "geners/CPReference.hh"
#include "geners/tupleIO.hh"

namespace gs {
    template<typename Pack>
    class ColumnPacker
    {
        template<typename Pack2> friend class ColumnPacker;

        // Each column will have its own write (and possibly read) buffer,
        // so the default buffer size should not be too large
        static const unsigned defaultBufferSize = 65536U;

    public:
        typedef Pack value_type;

        ColumnPacker(const std::vector<std::string>& columnNames,
                     const char* title, AbsArchive& archive,
                     const char* name, const char* category,
                     unsigned bufferSize = defaultBufferSize);

        // The following constructor will work only if every
        // element of the "protoPack" has a function with the
        // signature "const std::string& name() const" (think
        // gs::IOProxy).
        //
        ColumnPacker(const char* title, AbsArchive& archive,
                     const char* name, const char* category,
                     const Pack& protoPack,
                     unsigned bufferSize = defaultBufferSize);

        // A minimalistic constructor which can be used if you
        // do not care about things like column names and table
        // title. Default values will be assigned instead: all
        // columns will be named "c0", "c1", ..., and the title
        // will be an empty string.
        ColumnPacker(AbsArchive& archive,
                     const char* name, const char* category,
                     unsigned bufferSize = defaultBufferSize);

        ~ColumnPacker();

        // Various simple inspectors
        inline AbsArchive& archive() const {return ar_;}
        inline const std::string& name() const {return name_;}
        inline const std::string& category() const {return category_;}
        inline unsigned long bufferSize() const {return bufferSize_;}
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

        // The following method will return "true" if all column names
        // and types of this tuple correspond to column names and types
        // of the archived tuple (and come in the same order).
        inline bool isOriginal() const {return isOriginalTuple_;}

        // Info about columns of the original tuple saved in the archive
        unsigned long nOriginalColumns() const;
        const std::string& originalColumnName(unsigned long i) const;
        const std::vector<std::string>& originalColumnNames() const;
        unsigned long originalColumnNumber(const char* columnName) const;

        // A faster way to calculate the original column number
        // which corresponds to the current column number. Returns
        // nOriginalColumns() for columns which were not mapped or
        // if the argument column number is out of range.
        unsigned long originalColumn(unsigned long currentNumber) const;

        // Number of columns which will be read when "rowContents"
        // function is called
        unsigned long nColumnsReadBack() const;

        // The following function returns the mask of tuple elements
        // which will be filled on readback (1 for filled, 0 for not
        // filled).
        inline const std::vector<unsigned char>& readbackMask() const
            {return readbackMask_;}

        // Fill one tuple. This method returns "true" is everything is fine,
        // "false" if there was a problem.
        bool fill(const Pack& tuple);

        // Read the row contents back. This method will throw
        // std::out_of_range in case the row number is out of range.
        void rowContents(unsigned long row, Pack* tuple) const;

        // Fetch just one item at the given row and column. "StoragePtr"
        // should be an appropriate pointer type: IOPtr, IOProxy, bare
        // pointer, or CPP11_shared_ptr, depending on how the data was
        // actually packed. The signature of this method is kind of ugly,
        // so it may be more convenient to use column iterators.
        //
        // This method will throw std::out_of_range in case the row number
        // is out of range and std::invalid_argument if the column was
        // disabled when the packer was read back from the archive.
        //
        template <unsigned long Column, typename StoragePtr>
        void fetchItem(unsigned long row, StoragePtr ptr) const;

        // Compare packed contents
        template<typename Pack2>
        bool operator==(const ColumnPacker<Pack2>& r) const;

        template<typename Pack2>
        inline bool operator!=(const ColumnPacker<Pack2>& r) const
            {return !(*this == r);}

        // Methods needed for I/O
        bool write();
        inline ClassId classId() const {return ClassId(*this);}

        static const char* classname();
        static inline unsigned version() {return 1;}

    private:
        template<typename Pack2, unsigned long N>
        friend struct Private::ColumnPackerHelper;
        friend class Private::CPHeaderRecord<ColumnPacker<Pack> >;
        friend class Private::CPFooterRecord<ColumnPacker<Pack> >;
        friend class CPReference<ColumnPacker<Pack> >;

        ColumnPacker();
        ColumnPacker(const ColumnPacker&);
        ColumnPacker& operator=(const ColumnPacker&);

        // The following function is used by CPReference
        static ColumnPacker* read(
            AbsArchive& ar, std::istream& is,
            unsigned long long headId,
            const std::vector<std::string>& colNames,
            bool namesProvided);

        static unsigned long nextObjectNumber();

        AbsArchive& ar_;
        std::vector<std::string> colNames_;

        std::string name_;
        std::string category_;
        std::string title_;
        unsigned long long headerSaved_;
        unsigned long bufferSize_;
        unsigned long fillCount_;
        const unsigned long objectNumber_;
        bool readable_;
        bool writable_;
        mutable bool firstUnpack_;

        // The following flag may be set to "false" by the "read"
        // function
        bool isOriginalTuple_;

        std::vector<Private::ColumnBuffer*> fillBuffers_;
        mutable std::vector<Private::ColumnBuffer*> readBuffers_;
        mutable std::vector<std::vector<ClassId> > iostack_;
        mutable unsigned long currentReadRow_;
        std::vector<unsigned char> readbackMask_;

        // Id list for the buffers. The fisrt index is the column number.
        // The internal vector contains the pairs of the starting row
        // number and the buffer id in the archive.
        std::vector<std::vector<std::pair<
            unsigned long,unsigned long long> > > bufIds_;

        ClassId thisClass_;
        ClassId bufferClass_;
        ClassId cbClass_;

        // The vector of original column names. May or may not
        // be present, depending on how ntuple was created.
        std::vector<std::string>* originalColNames_;

        // The vector of original column numbers. May or may not
        // be present, depending on how ntuple was created.
        std::vector<unsigned long>* originalColNumber_;

        void initialize();
        void saveHeader();
        void saveFillBuffer(unsigned long col);
        void prepareToUnpack() const;

        inline bool dumpColumnClassIds(std::ostream& os) const
        {
            return Private::TupleClassIdCycler<
                Pack,std::tuple_size<Pack>::value>::dumpClassIds(os);
        }

        std::istream* getRowStream(
            unsigned long row, unsigned long col, unsigned long* len=0) const;

        std::ostream& columnOstream(unsigned long columnNumber);

        std::istream* columnIstream(unsigned long columnNumber,
                                    std::vector<ClassId>** iostack) const;

        static const std::vector<std::string>& defaultColumnNames();
    };
}

#include "geners/ColumnPacker.icc"

#endif // CPP11_STD_AVAILABLE
#endif // GENERS_COLUMNPACKER_HH_
