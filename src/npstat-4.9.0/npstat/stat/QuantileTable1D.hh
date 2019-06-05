#ifndef NPSTAT_QUANTILETABLE1D_HH_
#define NPSTAT_QUANTILETABLE1D_HH_

/*!
// \file QuantileTable1D.hh
//
// \brief 1-d statistical distribution defined by its quantile table
//
// Author: I. Volobouev
//
// March 2013
*/

#include <vector>
#include <stdexcept>

#include "npstat/stat/Distribution1DFactory.hh"
#include "npstat/nm/isMonotonous.hh"

namespace npstat {
    class QuantileTable1D : public AbsScalableDistribution1D
    {
    public:
        /**
        // The "data" array must provide quantile values at
        // equidistant intervals. It is assumed that the first
        // value corresponds to the 0.5/dataLen quantile and
        // the last to the 1.0 - 0.5/dataLen quantile. All data
        // values must be between 0 and 1 (the unscaled quantile
        // function value will be set to 0 at 0 and to 1 at 1).
        //
        // The represented density looks essentially like
        // a variable-bin histogram. Calculation of the quantile
        // function will be very fast, but calculation of the density
        // and cdf will be O(log(N)), where N is the number of entries
        // in the table.
        */
        template <typename Real>
        inline QuantileTable1D(const double location, const double scale,
                               const Real* data, const unsigned dataLen)
            : AbsScalableDistribution1D(location, scale),
              qtable_(data, data+dataLen)
        {
            if (!(data && dataLen))
                throw std::invalid_argument(
                    "In npstat::QuantileTable1D constructor: no data");
            if (dataLen > 1U && !isNonDecreasing(qtable_.begin(), qtable_.end()))
                throw std::invalid_argument(
                    "In npstat::QuantileTable1D constructor: invalid table");
            for (unsigned i=0; i<dataLen; ++i)
                if (qtable_[i] < 0.0 || qtable_[i] > 1.0)
                    throw std::invalid_argument(
                        "In npstat::QuantileTable1D constructor: "
                        "out of range table value");
        }

        inline virtual QuantileTable1D* clone() const
            {return new QuantileTable1D(*this);}

        inline virtual ~QuantileTable1D() {}

        // Methods needed for I/O
        virtual gs::ClassId classId() const {return gs::ClassId(*this);}
        virtual bool write(std::ostream& os) const;

        static inline const char* classname() {return "npstat::QuantileTable1D";}
        static inline unsigned version() {return 1;}
        static QuantileTable1D* read(const gs::ClassId& id, std::istream& in);

    protected:
        virtual bool isEqual(const AbsDistribution1D& r) const;

    private:
        friend class ScalableDistribution1DFactory<QuantileTable1D>;

        QuantileTable1D(double location, double scale,
                        const std::vector<double>& params);
        inline static int nParameters() {return -1;}

        double unscaledDensity(double x) const;
        double unscaledCdf(double x) const;
        double unscaledQuantile(double x) const;

        inline double unscaledExceedance(const double x) const
            {return 1.0 - unscaledCdf(x);}

        std::vector<double> qtable_;
    };
}

#endif // NPSTAT_QUANTILETABLE1D_HH_
