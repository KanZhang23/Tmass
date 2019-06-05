#ifndef NPSTAT_KDE1DHOSYMBETAKERNEL_HH_
#define NPSTAT_KDE1DHOSYMBETAKERNEL_HH_

/*!
// \file KDE1DHOSymbetaKernel.hh
//
// \brief High order Gaussian or symmetric beta KDE kernels
//
// Author: I. Volobouev
//
// November 2018
*/

#include <vector>

#include "npstat/stat/AbsKDE1DKernel.hh"
#include "npstat/nm/AbsClassicalOrthoPoly1D.hh"

namespace npstat {
    class KDE1DHOSymbetaKernel : public AbsKDE1DKernel
    {
    public:
        // Use power < 0 for Gaussian. Kernel order will be
        // "filterDegree" +  2 if "filterDegree" is even,
        // and "filterDegree" + 1 if "filterDegree" is odd.
        KDE1DHOSymbetaKernel(int power, double filterDegree);

        KDE1DHOSymbetaKernel(const KDE1DHOSymbetaKernel& r);
        KDE1DHOSymbetaKernel& operator=(const KDE1DHOSymbetaKernel& r);

        inline virtual KDE1DHOSymbetaKernel* clone() const
            {return new KDE1DHOSymbetaKernel(*this);}

        inline virtual ~KDE1DHOSymbetaKernel() {delete poly_;}

        inline int power() const {return power_;}
        inline double filterDegree() const {return filterDegree_;}
        inline double weight(const double x) const {return poly_->weight(x);}

        inline double xmin() const {return xmin_;}
        inline double xmax() const {return xmax_;}
        double operator()(const double x) const;

    private:
        void copyInternals(const KDE1DHOSymbetaKernel& r);

        int power_;
        unsigned maxdeg_;
        double filterDegree_;
        double xmin_;
        double xmax_;
        double lastShrinkage_;
        AbsClassicalOrthoPoly1D* poly_;
        mutable std::vector<long double> polyValues_;
    };
}

#endif // NPSTAT_KDE1DHOSYMBETAKERNEL_HH_
