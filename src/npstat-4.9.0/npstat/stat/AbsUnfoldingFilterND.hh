#ifndef NPSTAT_ABSUNFOLDINGFILTERND_HH_
#define NPSTAT_ABSUNFOLDINGFILTERND_HH_

/*!
// \file AbsUnfoldingFilterND.hh
//
// \brief Interface for smoothing filters used in multivariate unfolding problems
//
// Author: I. Volobouev
//
// June 2014
*/

#include <cassert>
#include <typeinfo>

#include "geners/ClassId.hh"
#include "geners/CPP11_auto_ptr.hh"

#include "npstat/nm/ArrayND.hh"
#include "npstat/nm/Matrix.hh"

#include "npstat/stat/AbsPolyFilterND.hh"

namespace npstat {
    class AbsUnfoldingFilterND : public AbsPolyFilterND
    {
    public:
        typedef Matrix<double> matrix_type;

        inline virtual ~AbsUnfoldingFilterND() {}

        virtual void filter(const ArrayND<double>& in,
                            ArrayND<double>* out) const = 0;

        virtual void convolve(const ArrayND<double>& in,
                              ArrayND<double>* out) const = 0;

        virtual Matrix<double> getFilterMatrix() const = 0;

        inline bool operator==(const AbsUnfoldingFilterND& r) const
            {return (typeid(*this) == typeid(r)) && this->isEqual(r);}

        inline bool operator!=(const AbsUnfoldingFilterND& r) const
            {return !(*this == r);}

        //@{
        /** Prototype needed for I/O */
        virtual gs::ClassId classId() const = 0;
        virtual bool write(std::ostream&) const = 0;
        //@}

        static inline const char* classname()
            {return "npstat::AbsUnfoldingFilterND";}
        static inline unsigned version() {return 1;}
        static AbsUnfoldingFilterND* read(const gs::ClassId& id, std::istream&);

    protected:
        virtual bool isEqual(const AbsUnfoldingFilterND&) const = 0;
    };


    template<class Impl>
    class UnfoldingFilterND : public AbsUnfoldingFilterND
    {
    public:
        inline UnfoldingFilterND(const Impl *filt, const bool assumeOwnership)
            : impl_(filt), owns_(assumeOwnership) {assert(impl_);}

        inline virtual ~UnfoldingFilterND() {if (owns_) delete impl_;}

        inline unsigned dim() const {return impl_->dim();}

        inline std::vector<unsigned> dataShape() const
            {return impl_->dataShape();}
        
        inline double selfContribution(
            const unsigned* index, const unsigned lenIndex) const
            {return impl_->selfContribution(index, lenIndex);}

        inline double linearSelfContribution(const unsigned long index) const
            {return impl_->linearSelfContribution(index);}

        inline void filter(const ArrayND<double>& in,
                           ArrayND<double>* out) const
            {impl_->filter(in, out);}

        inline void convolve(const ArrayND<double>& in,
                             ArrayND<double>* out) const
            {impl_->convolve(in, out);}

        inline Matrix<double> getFilterMatrix() const
            {return impl_->getFilterMatrix();}

        //@{
        /** Method related to "geners" I/O */
        inline virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        inline virtual bool write(std::ostream& of) const
            {return impl_->classId().write(of) && impl_->write(of);}
        //@}

        static inline const char* classname()
        {
            static const std::string name(
                gs::template_class_name<Impl>("npstat::UnfoldingFilterND"));
            return name.c_str();
        }

        static inline unsigned version() {return 1;}

        static inline UnfoldingFilterND* read(const gs::ClassId& id,
                                              std::istream& in)
        {
            static const gs::ClassId current(
                gs::ClassId::makeId<UnfoldingFilterND>());
            current.ensureSameId(id);
            gs::ClassId id1(in, 1);
            CPP11_auto_ptr<Impl> ptr(Impl::read(id1, in));
            UnfoldingFilterND* p = new UnfoldingFilterND(ptr.get(), true);
            ptr.release();
            return p;
        }

    protected:
        inline virtual bool isEqual(const AbsUnfoldingFilterND& r) const
            {return *impl_ == *(static_cast<const UnfoldingFilterND&>(r)).impl_;}

    private:
        UnfoldingFilterND();
        UnfoldingFilterND(const UnfoldingFilterND&);
        UnfoldingFilterND& operator=(const UnfoldingFilterND&);

        const Impl* impl_;
        bool owns_;
    };
}

#endif // NPSTAT_ABSUNFOLDINGFILTERND_HH_
