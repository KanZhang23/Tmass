#ifndef JETPTEFF_HH_
#define JETPTEFF_HH_

#include "geners/CPP11_shared_ptr.hh"
#include "geners/ClassId.hh"

#include "npstat/nm/LinInterpolatedTableND.hh"

class JetInfo;

//
// Single-parton efficiency model
//
class JetPtEff
{
public:
    //
    // The argument lookup table is 2-d, with MC jet Pt cut on the
    // first axis and parton Pt on the second
    //
    JetPtEff(CPP11_shared_ptr<const npstat::LinInterpolatedTableND<float> > table,
             double detectorPtCut, double sigmaAtPtCut);

    inline void setJetInfo(const JetInfo&) {}
    void setDeltaJES(double value);

    inline double getDetectorPtCut() const {return detectorJetPtCut_;}
    inline double getSigmaAtPtCut() const {return sigmaAtPtCut_;}
    inline double getDeltaJES() const {return deltaJES_;}
    inline double getJESAtCut() const {return jesAtCut_;}

    // operator() returns the efficiency for the given parton Pt
    // and previously set deltaJES
    double operator()(double partonPt) const;

    // Operators which compare objects for equality are very useful
    // for I/O testing
    bool operator==(const JetPtEff& r) const;
    inline bool operator!=(const JetPtEff& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    // I/O methods needed for reading
    static inline const char* classname() {return "JetPtEff";}
    static inline unsigned version() {return 1;}
    static JetPtEff* read(const gs::ClassId& id, std::istream& in);

private:
    JetPtEff();

    CPP11_shared_ptr<const npstat::LinInterpolatedTableND<float> > table_;
    double detectorJetPtCut_;
    double sigmaAtPtCut_;
    double deltaJES_;
    double jesAtCut_;
};

#endif // JETPTEFF_HH_
