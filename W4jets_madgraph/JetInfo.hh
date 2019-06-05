#ifndef JETINFO_HH_
#define JETINFO_HH_

#include "rk/rk.hh"
#include "geners/ClassId.hh"

class JetInfo
{
public:
    // Default constructor will create a dummy jet with zero 4-vector
    inline JetInfo() : etaDet_(0.0), sysErr_(-1.0), derrDpt_(0.0) {}

    inline JetInfo(const rk::P4& i_p4, const double etaDet,
                   const double i_sysErr, const double derrDpt)
        : p4_(i_p4), etaDet_(etaDet), sysErr_(i_sysErr), derrDpt_(derrDpt) {}

    inline double pt() const {return p4_.pt();}
    inline double eta() const {return p4_.eta();}
    inline double e() const {return p4_.e();}
    inline const rk::P4& p4() const {return p4_;}

    inline double detectorEta() const {return etaDet_;}
    inline double sysErr() const {return sysErr_;}
    inline double dErrDPt() const {return derrDpt_;}

    // Operators which compare objects for equality are very useful
    // for I/O testing
    bool operator==(const JetInfo& r) const;
    inline bool operator!=(const JetInfo& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    // I/O methods needed for reading
    static inline const char* classname() {return "JetInfo";}
    static inline unsigned version() {return 1;}
    static void restore(const gs::ClassId& id, std::istream& in,
                        JetInfo* ptr);
private:
    rk::P4 p4_;       // Jet 4-momentum
    double etaDet_;   // Jet detector eta
    double sysErr_;   // JES systematic uncertainty
    double derrDpt_;  // Derivative of the JES systematic uncertainty
                      // over the observed jet Pt
};

#endif // JETINFO_HH_
