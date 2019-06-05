#ifndef MUONTAUTF_HH_
#define MUONTAUTF_HH_

#include "npstat/stat/AbsDistribution1D.hh"

/* 
 * This works similar to "HighEnergyTauTF" but includes an empirical
 * turn-on term which is supposed to take into account the finite mass
 * of the muon
 */
class MuonTauTF : public npstat::AbsDistribution1D
{
public:
    MuonTauTF(double tauPolarization, double s);

    inline virtual MuonTauTF* clone() const
        {return new MuonTauTF(*this);}

    inline virtual ~MuonTauTF() {}

    inline double polarization() const {return polar_;}
    inline double turnOnWidth() const {return s_;}

    virtual double density(double x) const;
    virtual double cdf(double x) const;
    virtual double exceedance(double x) const;
    virtual double quantile(double x) const;

    // Methods needed for I/O
    virtual gs::ClassId classId() const {return gs::ClassId(*this);}
    virtual bool write(std::ostream& os) const;

    static inline const char* classname() {return "MuonTauTF";}
    static inline unsigned version() {return 1;}
    static MuonTauTF* read(const gs::ClassId& id, std::istream& in);

protected:
    inline virtual bool isEqual(const AbsDistribution1D& other) const
    {
        const MuonTauTF& r = static_cast<const MuonTauTF&>(other);
        return polar_ == r.polar_ && s_ == r.s_;
    }

private:
    MuonTauTF();

    double polar_;
    double s_;
    double normfactor_;
};

#endif // MUONTAUTF_HH_
