#ifndef NPSTAT_STORABLEINTERPOLATIONFUNCTOR_HH_
#define NPSTAT_STORABLEINTERPOLATIONFUNCTOR_HH_

/*!
// \file StorableInterpolationFunctor.hh
//
// \brief Storable multivariate functor represented by a multilinear
//        interpolation/extrapolation table
//
// Author: I. Volobouev
//
// July 2012
*/

#include "npstat/stat/StorableMultivariateFunctor.hh"
#include "npstat/nm/LinInterpolatedTableND.hh"
#include "npstat/nm/SimpleFunctors.hh"

namespace npstat {
    /**
    // This class adapts LinInterpolatedTableND template to the
    // StorableMultivariateFunctor interface
    */
    template
    <
        class Numeric,
        class Axis = UniformAxis,
        class Converter = Same<Numeric>
    >
    class StorableInterpolationFunctor : public StorableMultivariateFunctor
    {
        template <typename Num2, typename Axis2, typename Conv2>
        friend class StorableInterpolationFunctor;

    public:
        typedef LinInterpolatedTableND<Numeric,Axis> Table;

        //@{
        /** Constructor from a pre-existing table */
        template <class Num2>
        inline StorableInterpolationFunctor(
            const LinInterpolatedTableND<Num2,Axis>& table)
            : StorableMultivariateFunctor(), table_(table) {}

        template <class Num2>
        inline StorableInterpolationFunctor(
            const LinInterpolatedTableND<Num2,Axis>& table,
            const std::string& descr)
            : StorableMultivariateFunctor(descr), table_(table) {}
        //@}

        /** Converting copy constructor */
        template <class Num2, class Conv2>
        inline StorableInterpolationFunctor(
            const StorableInterpolationFunctor<Num2,Axis,Conv2>& tab)
            : StorableMultivariateFunctor(tab.description()),
              table_(tab.table_) {}

        //@{
        /**
        // Constructor which builds the table in place.
        // It basically passses its arguments to the
        // corresponding constructor of LinInterpolatedTableND.
        */
        inline StorableInterpolationFunctor(
            const std::vector<Axis>& axes,
            const std::vector<std::pair<bool,bool> >& interpolationType,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(axes, interpolationType, functionLabel) {}

        inline StorableInterpolationFunctor(
            const Axis& xAxis, bool leftX, bool rightX,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(xAxis, leftX, rightX, functionLabel) {}

        inline StorableInterpolationFunctor(
            const Axis& xAxis, bool leftX, bool rightX,
            const Axis& yAxis, bool leftY, bool rightY,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(xAxis, leftX, rightX,
                     yAxis, leftY, rightY, functionLabel) {}

        inline StorableInterpolationFunctor(
            const Axis& xAxis, bool leftX, bool rightX,
            const Axis& yAxis, bool leftY, bool rightY,
            const Axis& zAxis, bool leftZ, bool rightZ,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(xAxis, leftX, rightX,
                     yAxis, leftY, rightY,
                     zAxis, leftZ, rightZ, functionLabel) {}

        inline StorableInterpolationFunctor(
            const Axis& xAxis, bool leftX, bool rightX,
            const Axis& yAxis, bool leftY, bool rightY,
            const Axis& zAxis, bool leftZ, bool rightZ,
            const Axis& tAxis, bool leftT, bool rightT,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(xAxis, leftX, rightX,
                     yAxis, leftY, rightY,
                     zAxis, leftZ, rightZ,
                     tAxis, leftT, rightT, functionLabel) {}

        inline StorableInterpolationFunctor(
            const Axis& xAxis, bool leftX, bool rightX,
            const Axis& yAxis, bool leftY, bool rightY,
            const Axis& zAxis, bool leftZ, bool rightZ,
            const Axis& tAxis, bool leftT, bool rightT,
            const Axis& vAxis, bool leftV, bool rightV,
            const char* functionLabel=0)
            : StorableMultivariateFunctor(),
              table_(xAxis, leftX, rightX,
                     yAxis, leftY, rightY,
                     zAxis, leftZ, rightZ,
                     tAxis, leftT, rightT,
                     vAxis, leftV, rightV, functionLabel) {}
        //@}

        virtual ~StorableInterpolationFunctor() {}

        virtual unsigned minDim() const {return table_.dim();};

        virtual double operator()(const double* point, unsigned dim) const
            {return conv_(table_(point, dim));}

        //@{
        /** Retrieve the underlying LinInterpolatedTableND object */
        inline Table& interpolator() {return table_;}
        inline const Table& interpolator() const {return table_;}
        //@}

        //@{
        /** Retrieve the tabulated data */
        inline ArrayND<Numeric>& table() {return table_.table();}
        inline const ArrayND<Numeric>& table() const {return table_.table();}
        //@}

        /** Change the coordinate converter */
        inline void setConverter(const Converter& conv) {conv_ = conv;}

        //@{
        // Method related to "geners" I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& of) const;
        //@}

        // I/O methods needed for reading
        static inline const char* classname();
        static inline unsigned version() {return 1;}
        static StorableInterpolationFunctor* read(
            const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const StorableMultivariateFunctor& other) const
        {
            // Note the use of static_cast rather than dynamic_cast below.
            // static_cast works faster and it is guaranteed to succeed here.
            const StorableInterpolationFunctor& r = 
                static_cast<const StorableInterpolationFunctor&>(other);
            return table_ == r.table_ &&
                   this->description() == other.description();
        }

    private:
        StorableInterpolationFunctor();

        Table table_;
        Converter conv_;
    };
}

#include "npstat/stat/StorableInterpolationFunctor.icc"

#endif // NPSTAT_STORABLEINTERPOLATIONFUNCTOR_HH_
