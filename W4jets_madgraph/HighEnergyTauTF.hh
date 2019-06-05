#ifndef HIGHENERGYTAUTF_HH_
#define HIGHENERGYTAUTF_HH_

#include "npstat/stat/AbsDistribution1D.hh"

/* The class "HighEnergyTauTF" implements the formula given in the paper
 * "Measurement of the polarization of a high energy muon beam" by the
 * Spin Muon Collaboration (SMC), B. Adeva et. al., Nuclear Instruments
 * and Methods in Physics Research A 343 (1994) 363-373.
 */
class HighEnergyTauTF : public npstat::AbsDistribution1D
{
public:
    inline explicit HighEnergyTauTF(const double tauPolarization)
        : polar_(tauPolarization) {}

    inline virtual HighEnergyTauTF* clone() const
        {return new HighEnergyTauTF(*this);}

    inline virtual ~HighEnergyTauTF() {}

    inline double polarization() const {return polar_;}

    virtual double density(double x) const;
    virtual double cdf(double x) const;
    virtual double exceedance(double x) const;
    virtual double quantile(double x) const;

    // Methods needed for I/O
    virtual gs::ClassId classId() const {return gs::ClassId(*this);}
    virtual bool write(std::ostream& os) const;

    static inline const char* classname() {return "HighEnergyTauTF";}
    static inline unsigned version() {return 1;}
    static HighEnergyTauTF* read(const gs::ClassId& id, std::istream& in);

protected:
    inline virtual bool isEqual(const AbsDistribution1D& other) const
    {
        const HighEnergyTauTF& r = static_cast<const HighEnergyTauTF&>(other);
        return polar_ == r.polar_;
    }

private:
    HighEnergyTauTF();

    double polar_;
};

#endif // HIGHENERGYTAUTF_HH_
