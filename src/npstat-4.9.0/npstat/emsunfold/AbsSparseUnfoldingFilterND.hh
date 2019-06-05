#ifndef EMSUNFOLD_ABSSPARSEUNFOLDINGFILTERND_HH_
#define EMSUNFOLD_ABSSPARSEUNFOLDINGFILTERND_HH_

/*!
// \file AbsSparseUnfoldingFilterND.hh
//
// \brief Interface for smoothing filters representable by sparse matrices
//
// Author: I. Volobouev
//
// July 2014
*/

#include <typeinfo>

#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"
#include "geners/AbsReader.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/stat/AbsPolyFilterND.hh"

#include "Eigen/SparseCore"

namespace emsunfold {
    class AbsSparseUnfoldingFilterND : public npstat::AbsPolyFilterND
    {
    public:
        typedef Eigen::Triplet<double,int> triplet_type;

        inline virtual ~AbsSparseUnfoldingFilterND() {}

        virtual void filter(const npstat::ArrayND<double>& in,
                            npstat::ArrayND<double>* out) const = 0;

        virtual void convolve(const npstat::ArrayND<double>& in,
                              npstat::ArrayND<double>* out) const = 0;

        virtual CPP11_auto_ptr<std::vector<triplet_type> >
        sparseFilterTriplets() const = 0;

        inline bool operator==(const AbsSparseUnfoldingFilterND& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}

        inline bool operator!=(const AbsSparseUnfoldingFilterND& r) const
            {return !(*this == r);}

        //@{
        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream&) const = 0;
        //@}

        static inline const char* classname()
            {return "emsunfold::AbsSparseUnfoldingFilterND";}
        static inline unsigned version() {return 1;}
        static AbsSparseUnfoldingFilterND* read(const gs::ClassId& id,
                                                std::istream&);
    protected:
        virtual bool isEqual(const AbsSparseUnfoldingFilterND&) const = 0;
    };


    template<class Impl>
    class SparseUnfoldingFilterND : public AbsSparseUnfoldingFilterND
    {
    public:
        inline SparseUnfoldingFilterND(const Impl *filt, const bool own)
            : impl_(filt), owns_(own) {assert(impl_);}

        inline virtual ~SparseUnfoldingFilterND() {if (owns_) delete impl_;}

        inline unsigned dim() const {return impl_->dim();}

        inline std::vector<unsigned> dataShape() const
            {return impl_->dataShape();}
        
        inline double selfContribution(
            const unsigned* index, const unsigned lenIndex) const
            {return impl_->selfContribution(index, lenIndex);}

        inline double linearSelfContribution(const unsigned long index) const
            {return impl_->linearSelfContribution(index);}

        inline void filter(const npstat::ArrayND<double>& in,
                           npstat::ArrayND<double>* out) const
            {impl_->filter(in, out);}

        inline void convolve(const npstat::ArrayND<double>& in,
                             npstat::ArrayND<double>* out) const
            {impl_->convolve(in, out);}

        inline CPP11_auto_ptr<std::vector<triplet_type> >
        sparseFilterTriplets() const
            {return impl_->template sparseFilterTriplets<triplet_type>();}

        //@{
        /** Method related to "geners" I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        inline virtual bool write(std::ostream& of) const
            {return impl_->classId().write(of) && impl_->write(of);}
        //@}

        static inline const char* classname()
        {
            static const std::string name(gs::template_class_name<Impl>(
                "emsunfold::SparseUnfoldingFilterND"));
            return name.c_str();
        }

        static inline unsigned version() {return 1;}

        static inline SparseUnfoldingFilterND* read(const gs::ClassId& id,
                                                    std::istream& in)
        {
            static const gs::ClassId current(
                gs::ClassId::makeId<SparseUnfoldingFilterND>());
            current.ensureSameId(id);
            gs::ClassId id1(in, 1);
            CPP11_auto_ptr<Impl> ptr(Impl::read(id1, in));
            SparseUnfoldingFilterND* p = 
                new SparseUnfoldingFilterND(ptr.get(), true);
            ptr.release();
            return p;
        }

    protected:
        inline virtual bool isEqual(const AbsSparseUnfoldingFilterND& r) const
            {return *impl_ == *(static_cast<const SparseUnfoldingFilterND&>(r)).impl_;}

    private:
        SparseUnfoldingFilterND();
        SparseUnfoldingFilterND(const SparseUnfoldingFilterND&);
        SparseUnfoldingFilterND& operator=(const SparseUnfoldingFilterND&);

        const Impl* impl_;
        bool owns_;
    };


    class SparseUnfoldingFilterNDReader : public gs::DefaultReader<AbsSparseUnfoldingFilterND>
    {
        typedef gs::DefaultReader<AbsSparseUnfoldingFilterND> Base;
        friend class gs::StaticReader<SparseUnfoldingFilterNDReader>;
        inline SparseUnfoldingFilterNDReader() {}
    };


    /** Factory for deserializing filters representable by sparse matrices */
    typedef gs::StaticReader<SparseUnfoldingFilterNDReader>
    StaticSparseUnfoldingFilterNDReader;


    inline AbsSparseUnfoldingFilterND* AbsSparseUnfoldingFilterND::read(
        const gs::ClassId& id, std::istream& in)
    {
        return StaticSparseUnfoldingFilterNDReader::instance().read(id, in);
    }
}

#endif // EMSUNFOLD_ABSSPARSEUNFOLDINGFILTERND_HH_
