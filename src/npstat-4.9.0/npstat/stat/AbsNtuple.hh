#ifndef NPSTAT_ABSNTUPLE_HH_
#define NPSTAT_ABSNTUPLE_HH_

/*!
// \file AbsNtuple.hh
//
// \brief Interface definition for homogeneous ntuples (point clouds)
//
// Author: I. Volobouev
//
// November 2010
*/

#include <vector>
#include <climits>
#include <typeinfo>
#include <iterator>
#include <stdexcept>

#include "geners/ClassId.hh"
#include "geners/binaryIO.hh"
#include "geners/allUnique.hh"

#include "npstat/stat/Column.hh"

#ifdef SWIG
#include "npstat/stat/NtRectangularCut.hh"
#endif

namespace npstat {
    /**
    // Interface class for ntuples. Here, ntuples are homogeneous 2-d tables
    // in which the number of columns is fixed while the number of rows can
    // grow dynamically.
    */
    template <typename T>
    class AbsNtuple
    {
    public:
        typedef T value_type;

        /** 
        // The constructor arguments are a vector of column names
        // and an ntuple title c-string (which can be NULL)
        */ 
        inline AbsNtuple(const std::vector<std::string>& columnNames,
                         const char* ntTitle)
            : colNames_(columnNames), title_(ntTitle ? ntTitle : "")
        {
            if (columnNames.empty())
                throw std::invalid_argument("In npstat::AbsNtuple constructor:"
                                            " no column labels provided");
            if (!gs::allUnique(columnNames))
                throw std::invalid_argument("In npstat::AbsNtuple constructor:"
                                            " column labels are not unique");
        }

        inline virtual ~AbsNtuple() {}

        /** Retrieve the ntuple title */
        inline const std::string& title() const {return title_;}

        /** Set the ntuple title */
        inline virtual void setTitle(const char* newtitle)
            {title_ = newtitle ? newtitle : "";}

        /** Retrieve the number of columns */
        inline unsigned long nColumns() const 
            {return colNames_.size();}

        /** Retrieve the name for the given column */
        inline const std::string& columnName(const unsigned long i) const
            {return colNames_.at(i);}

        /** Retrieve all column names */
        inline const std::vector<std::string>& columnNames() const
            {return colNames_;}

        /**
        // The code will refuse to set the column name (and false will be
        // returned in this case) if the provided name duplicates an existing
        // column name. False will also be returned if the column index
        // is out of range. Derived classes can also refuse to change the
        // column name if their implementation relies in some way on the
        // permanence of these names.
        */
        virtual bool setColumnName(unsigned long i, const char* newname);

        /**
        // This method returns nColumns() in case the
        // given column name is not valid
        */
        unsigned long columnNumber(const char* columnName) const;

        /**
        // This method works just like columnNumber but
        // generates a dynamic fault in case the given
        // column name is invalid
        */
        unsigned long validColumn(const char* columnName) const;

        /** Retrieve the number of rows */
        virtual unsigned long nRows() const = 0;

        /** Retrieve the total number of ntuple elements */
        inline unsigned long length() const {return nRows()*colNames_.size();}

        /**
        // The number of values provided, "lenValues", should be
        // divisible by the number of columns. If it is not, the
        // function should throw "std::invalid_argument" exception.
        */
        virtual void fill(const T* values, unsigned long lenValues) = 0;

        //@{
        /**
        // Convenience method which works if the number of arguments equals
        // the number if colums (otherwise an exception will be thrown)
        */
        virtual void fill(const T& v0) = 0;
        virtual void fill(const T& v0, const T& v1) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3)=0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4, const T& v5) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4, const T& v5, const T& v6) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4, const T& v5, const T& v6, const T& v7)=0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4, const T& v5, const T& v6, const T& v7,
                          const T& v8) = 0;
        virtual void fill(const T& v0, const T& v1, const T& v2, const T& v3,
                          const T& v4, const T& v5, const T& v6, const T& v7,
                          const T& v8, const T& v9) = 0;
        //@}

        /**
        // Append contents of another ntuple. That ntuple must have
        // the same number of columns. The copy constructor must exist
        // which builds elements of this ntuple from the elements of
        // another ntuple.
        */
        template <typename T2>
        void append(const AbsNtuple<T2>& another);

        /**
        // Access individual elements (no bounds checking).
        // Note that access is by value, so it involves a fair
        // amount of copying. The other alternative, return by
        // reference, could lead to subtle bugs with references
        // invalidated by fill operations and buffer swapping.
        */
        virtual T operator()(unsigned long r, unsigned long c) const=0;

        /** Access individual elements with bounds checking */
        virtual T at(unsigned long r, unsigned long c) const = 0;

        /**
        // Access with flexible column argument. The "Column" class has
        // converting constructors from unsigned long, const char*, and
        // std::string&. All these can now be used as the second argument.
        // Note that this type of access will be slower than access by
        // simple indices, so don't use this method in tight loops.
        */
        T element(unsigned long r, const Column& c) const;

        /** Similar method with bounds checking */
        T elementAt(unsigned long r, const Column& c) const;

        //@{
        /**
        // Fetch copies of rows/columns. The buffer should be at least
        // as large as the number of elements expected in return.
        */
        virtual void rowContents(unsigned long row, T* buf,
                                 unsigned long lenBuf) const = 0;
        virtual void columnContents(const Column& c, T* buf,
                                    unsigned long lenBuf) const = 0;
        //@}

        /**
        // Clear the data (if possible). Note that certain disk-based
        // implementations may ignore this. Check the number of rows
        // to make sure that the data was indeed cleared.
        */
        virtual void clear() = 0;

        /**
        // Iteration over column contents (cycles row numbers).
        // Note that rowContents/columnContents methods to access
        // the data are going to work faster than these iterators.
        */
        class column_iterator
        {
        public:
            typedef T value_type;
            typedef std::forward_iterator_tag iterator_category;

            column_iterator();

            T operator*() const;
            column_iterator& operator++();
            column_iterator operator++(int);
            bool operator==(const column_iterator&) const;
            bool operator!=(const column_iterator&) const;
            bool operator<(const column_iterator&) const;

        private:
            friend class AbsNtuple;
            const AbsNtuple<T>* nt_;
            unsigned long column_;
            unsigned long row_;
        };

        /** Iteration over row contents (cycles column numbers) */
        class row_iterator
        {
        public:
            typedef T value_type;
            typedef std::forward_iterator_tag iterator_category;

            row_iterator();

            T operator*() const;
            row_iterator& operator++();
            row_iterator operator++(int);
            bool operator==(const row_iterator&) const;
            bool operator!=(const row_iterator&) const;
            bool operator<(const row_iterator&) const;

        private:
            friend class AbsNtuple;
            const AbsNtuple<T>* nt_;
            unsigned long column_;
            unsigned long row_;
        };

        //@{
        /**
        // Methods which return begin/end positions for
        // the row/column iterators
        */
        row_iterator row_begin(unsigned long rowNumber) const;
        row_iterator row_end() const;
        column_iterator column_begin(const Column& column) const;
        column_iterator column_end() const;
        //@}

        /**
        // Function for cycling over all rows. The accumulator class
        // must implement the following function:
        //
        // void accumulate(T* rowContents, unsigned long nColumns)
        */
        template <class Accumulator>
        void cycleOverRows(Accumulator& acc) const;

        /**
        // Same as cycleOverRows, but the accumulator will be called
        // only if the filter returns "true". The filter must be
        // a functor which implements
        //
        // bool operator()(const T* rowContents, unsigned long nColumns) const
        //
        // The function returns the number of rows passing the filter.
        */
        template <class Filter, class Accumulator>
        unsigned long conditionalCycleOverRows(
            const Filter& f, Accumulator& acc) const;

        /** Cycle over rows counting how many rows pass the filter */
        template <class Filter>
        unsigned long conditionalRowCount(const Filter& f) const;

        /**
        // Another row cycling function which also use a functor that
        // calculates a weight. The weight functor must implement
        //
        // double operator()(const T* rowContents, unsigned long nCols) const
        //
        // The returned weights must be non-negative. The accumulator must
        // implement
        //
        // void accumulate(T* rowContents, unsigned long nColumns, double w)
        //
        // The option "skipZeroWeights" allows the user not to call the
        // accumulator if the calculated weight is 0.
        */
        template <class Accumulator, class WeightCalc>
        void weightedCycleOverRows(Accumulator& acc, const WeightCalc& wcalc,
                                   bool skipZeroWeights = false) const;

        /**
        // This method returns the sum of weights for rows
        // that pass the filter and call the accumulator as appropriate
        */
        template <class Filter, class Accumulator, class WeightCalc>
        double weightedConditionalCycleOverRows(
            const Filter& f, Accumulator& acc, const WeightCalc& wcalc,
            bool skipZeroWeights = false) const;

        /**
        // This method just returns the sum of weights for rows
        // that pass the filter
        */
        template <class Filter, class WeightCalc>
        double weightedConditionalRowCount(
            const Filter& f, const WeightCalc& wcalc) const;

        //@{
        /**
        // Convenience method which returns a collection of column
        // indices using a set of inhomogeneous column descriptions as
        // an argument. Exception will be thrown if the column
        // does not exist.
        */
        std::vector<unsigned long> columnIndices(const Column& c0) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4,
                                                 const Column& c5) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4,
                                                 const Column& c5,
                                                 const Column& c6) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4,
                                                 const Column& c5,
                                                 const Column& c6,
                                                 const Column& c7) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4,
                                                 const Column& c5,
                                                 const Column& c6,
                                                 const Column& c7,
                                                 const Column& c8) const;
        std::vector<unsigned long> columnIndices(const Column& c0,
                                                 const Column& c1,
                                                 const Column& c2,
                                                 const Column& c3,
                                                 const Column& c4,
                                                 const Column& c5,
                                                 const Column& c6,
                                                 const Column& c7,
                                                 const Column& c8,
                                                 const Column& c9) const;
        //@}

        /**
        // Convenience method which returns a collection of column
        // indices using a vector of column names as an argument.
        */
        std::vector<unsigned long> columnIndices(
            const std::vector<std::string>& colNames) const;

        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;

        /** 
        // Comparison for equality. Do not override in derived classes
        // (override "isEqual" method instead)
        */
        inline bool operator==(const AbsNtuple& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}

        /** Logical negation of operator== */
        inline bool operator!=(const AbsNtuple& r) const
            {return !(*this == r);}

    protected:
        /**
        // Comparison for equality to be overriden by the derived
        // classes. Don't forget to call "isEqual" method of the
        // base class.
        */
        virtual bool isEqual(const AbsNtuple& r) const;

    private:
        std::vector<std::string> colNames_;
        std::string title_;

        // Before using "AppendNTuple", check compatibility
        // of the number of columns
        template<int, class T2>
        struct AppendNTuple
        {
            static inline void append(AbsNtuple* nt,
                                      const AbsNtuple<T2>& other)
            {
                const unsigned long nRows = other.nRows();
                if (nRows)
                {
                    // We need to create buffers carefully, so that this code
                    // can work for objects without default constructors
                    const unsigned long nCols = other.nColumns();
                    std::vector<T2> bufVec;
                    bufVec.reserve(nCols);
                    std::vector<T> myBufVec;
                    myBufVec.reserve(nCols);
                    for (unsigned long col=0; col<nCols; ++col)
                        bufVec.push_back(other(0UL, col));
                    for (unsigned long col=0; col<nCols; ++col)
                        myBufVec.push_back(T(bufVec[col]));
                    T2* buf = &bufVec[0];
                    T* myBuf = &myBufVec[0];
                    nt->fill(myBuf, nCols);
                    for (unsigned long row=1UL; row<nRows; ++row)
                    {
                        other.rowContents(row, buf, nCols);
                        for (unsigned long col=0; col<nCols; ++col)
                            myBuf[col] = T(buf[col]);
                        nt->fill(myBuf, nCols);
                    }
                }
            }
        };

        // Faster version of "AppendNTuple" which will be called
        // when T and T2 types are the same
        template<class T2>
        struct AppendNTuple<1, T2>
        {
            static inline void append(AbsNtuple* nt,
                                      const AbsNtuple<T2>& other)
            {
                const unsigned long nRows = other.nRows();
                if (nRows)
                {
                    const unsigned long nCols = other.nColumns();
                    std::vector<T2> bufVec;
                    bufVec.reserve(nCols);
                    for (unsigned long col=0; col<nCols; ++col)
                        bufVec.push_back(other(0UL, col));
                    T2* buf = &bufVec[0];
                    nt->fill(buf, nCols);
                    for (unsigned long row=1UL; row<nRows; ++row)
                    {
                        other.rowContents(row, buf, nCols);
                        nt->fill(buf, nCols);
                    }
                }
            }
        };

#ifdef SWIG
    public:
        template <class Accumulator>
        inline unsigned long cutCycleOverRows(
            const NtRectangularCut<T>& f, Accumulator& acc) const
        {
            return conditionalCycleOverRows(f, acc);
        }

        inline void append2(const AbsNtuple& other)
        {
            append(other);
        }
#endif
    };

    /**
    // Function for dumping ntuples into text files, one row per line.
    // By default, column values will be separated by a single white
    // space. If "insertCommasBetweenValues" is "true" then column
    // values will be separated by ", ". Only the data is dumped, not
    // the info about the ntuple structure.
    //
    // This function will only work with T objects that have default
    // constructors. "true" is returned on success, "false" on failure.
    */
    template <typename T>    
    bool dumpNtupleAsText(const AbsNtuple<T>& ntuple,
                          std::ostream& asciiStream,
                          bool insertCommasBetweenValues=false,
                          unsigned long firstRowToDump=0,
                          unsigned long maxRowsToDump=ULONG_MAX);

    /**
    // Function for filling ntuples from text files, one row per line.
    // Will work with T objects that have default constructors.
    // "true" is returned on success, "false" on failure.
    //
    // There may be more columns (but not less) in the file than
    // in the ntuple. In this case extra columns are ignored.
    //
    // Empty lines, lines which consist of pure white space, and lines
    // which start with an arbitrary amount of white space (including
    // none) followed by '#' are ignored (considered comments).
    */
    template <typename T>    
    bool fillNtupleFromText(std::istream& asciiStream,
                            AbsNtuple<T>* ntuple,
                            bool hasCommasBetweenValues=false,
                            unsigned long maxRowsToFill=ULONG_MAX);

    //@{
    /**
    // Convenience function for creating vectors of std::string
    // using variable number of arguments (from 1 to 10 here)
    */
    std::vector<std::string> ntupleColumns(const char* v0);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7,
                                           const char* v8);
    std::vector<std::string> ntupleColumns(const char* v0, const char* v1,
                                           const char* v2, const char* v3,
                                           const char* v4, const char* v5,
                                           const char* v6, const char* v7,
                                           const char* v8, const char* v9);
    std::vector<std::string> ntupleColumns(const char** names, unsigned len);
    //@}

    /** Generate column names "c0", "c1", ..., "cM", where M = ncols - 1 */
    std::vector<std::string> simpleColumnNames(unsigned ncols);
}

#include "npstat/stat/Column.icc"
#include "npstat/stat/AbsNtuple.icc"

#endif // NPSTAT_ABSNTUPLE_HH_
