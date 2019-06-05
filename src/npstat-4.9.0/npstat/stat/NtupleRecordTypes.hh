#ifndef NPSTAT_NTUPLERECORDTYPES_HH_
#define NPSTAT_NTUPLERECORDTYPES_HH_

//=========================================================================
// NtupleRecordTypes.hh
//
// Special record types for storing ArchivedNtuple contents in the archives.
// Applications should never use this header directly.
//
// Author: I. Volobouev
//
// November 2010
//=========================================================================

#include <stdexcept>

#include "geners/AbsRecord.hh"
#include "geners/AbsReference.hh"
#include "geners/binaryIO.hh"

#include "npstat/stat/NtupleRecordTypesFwd.hh"

namespace npstat {
namespace Private {
    template<class Ntuple>
    class NtupleHeaderRecord : public gs::AbsRecord
    {
    public:
        inline explicit NtupleHeaderRecord(const Ntuple& obj)
            : gs::AbsRecord(obj.classId(), "npstat::NtupleHeader",
                            obj.name_.c_str(), obj.category_.c_str()),
              obj_(obj) {}

        inline virtual ~NtupleHeaderRecord() {}

        inline bool writeData(std::ostream& os) const
        {
            obj_.classId().write(os);
            obj_.fillBuffer_.classId().write(os);
            gs::write_pod_vector(os, obj_.columnNames());
            gs::write_pod(os, obj_.title());
            const unsigned long rowsPerBuffer = obj_.rowsPerBuffer();
            gs::write_pod(os, rowsPerBuffer);
            gs::write_pod(os, obj_.writesByColumn());

            return !os.fail();
        }

    private:
        NtupleHeaderRecord();
        const Ntuple& obj_;
    };


    template<class Ntuple>
    class NtupleFooterRecord : public gs::AbsRecord
    {
    public:
        inline explicit NtupleFooterRecord(const Ntuple& obj)
            : gs::AbsRecord(obj.classId(), "npstat::NtupleFooter",
                            obj.name_.c_str(), obj.category_.c_str()),
              obj_(obj) {}

        inline virtual ~NtupleFooterRecord() {}

        inline bool writeData(std::ostream& os) const
        {
            unsigned long nrows = obj_.nRows();
            gs::write_pod(os, nrows);
            gs::write_pod(os, obj_.headerSaved_);
            gs::write_pod_vector(os, obj_.idlist_);
            const unsigned char writeColumnWise = obj_.writesByColumn();
            gs::write_pod(os, writeColumnWise);
            if (writeColumnWise)
                gs::write_pod_vector(os, obj_.columnOffsets_);

            return !os.fail();
        }

    private:
        NtupleFooterRecord();
        const Ntuple& obj_;
    };


    template<class Ntuple>
    class NtupleBufferRecord : public gs::AbsRecord
    {
    public:
        inline explicit NtupleBufferRecord(const Ntuple& obj)
            : gs::AbsRecord(obj.fillBuffer_.classId(), "npstat::NtupleBuffer",
                            obj.name_.c_str(), obj.category_.c_str()),
              obj_(obj) {}

        inline virtual ~NtupleBufferRecord() {}

        inline bool writeData(std::ostream& os) const
            {return obj_.fillBuffer_.write(os);}

    private:
        NtupleBufferRecord();
        const Ntuple& obj_;
    };


    template<class Ntuple>
    class NtupleBufferReference : public gs::AbsReference
    {
    public:
        typedef NtupleBuffer<typename Ntuple::value_type> Buffer;

        inline NtupleBufferReference(const Ntuple& obj,
                                     const unsigned long long itemId)
            : gs::AbsReference(obj.ar_, obj.bufferClass_, 
                               "npstat::NtupleBuffer", itemId), obj_(obj) {}

        inline virtual ~NtupleBufferReference() {}

        inline void restore(const unsigned long number, Buffer* buf) const
        {
            const unsigned long long itemId = id(number);
            if (itemId == 0ULL) throw std::out_of_range(
                "In npstat::NtupleBufferReference::restore: "
                "buffer number out of range");
            Buffer::restore(obj_.bufferClass_,
                            this->positionInputStream(itemId), buf);
        }

    private:
        const Ntuple& obj_;
    };


    template<class Ntuple>
    class NtupleColumnReference : public gs::AbsReference
    {
    public:
        inline NtupleColumnReference(const Ntuple& obj,
                                     const unsigned long long itemId,
                                     const unsigned long column,
                                     const long long offset)
            : gs::AbsReference(obj.ar_, obj.bufferClass_,
                               "npstat::NtupleBuffer", itemId),
              obj_(obj), offset_(offset), col_(column) {}

        inline virtual ~NtupleColumnReference() {}

        inline bool fillItems(typename Ntuple::value_type* buf,
                              const unsigned long lenBuf) const
        {
            const unsigned long long itemId = id(0);
            if (itemId == 0)
                return false;
            else
            {
                NtupleBuffer<typename Ntuple::value_type>::readColumn(
                    obj_.bufferClass_, this->positionInputStream(itemId),
                    col_, offset_, buf, lenBuf);
                return true;
            }
        }

    private:
        const Ntuple& obj_;
        long long offset_;
        unsigned long col_;
    };


    // We want to see the ntuple footer reference before reading
    // the ntuple, so do not use an ntuple object to construct
    // this reference.
    struct NtupleFooterReference : public gs::AbsReference
    {
        inline NtupleFooterReference(
            gs::AbsArchive& ar, const gs::ClassId& classId,
            const char* name, const char* category)
            : gs::AbsReference(ar, classId, "npstat::NtupleFooter",
                               name, category) {}

        inline virtual ~NtupleFooterReference() {}

        inline bool fillItems(unsigned long* nrows,
                              unsigned long long* headerId,
                              std::vector<unsigned long long>* idlist,
                              std::vector<long long>* columnOffsets,
                              unsigned long long* recordOffset,
                              const unsigned long number) const
        {
            const unsigned long long itemId = id(number);
            if (itemId == 0)
                return false;

            std::istream& s = this->positionInputStream(itemId);
            *recordOffset = archive().catalogEntry(itemId)->offset();
            gs::read_pod(s, nrows);
            gs::read_pod(s, headerId);
            gs::read_pod_vector(s, idlist);
            unsigned char writeColumnWise = false;
            gs::read_pod(s, &writeColumnWise);
            if (writeColumnWise)
                gs::read_pod_vector(s, columnOffsets);
            else
                columnOffsets->clear();

            return !s.fail();
        }
    };
}
}

#endif // NPSTAT_NTUPLERECORDTYPES_HH_
