#ifndef JETPTTF_HH_
#define JETPTTF_HH_

#include "geners/CPP11_shared_ptr.hh"

#include "npstat/nm/GridAxis.hh"
#include "npstat/stat/AbsDistribution1D.hh"

#include "JetInfo.hh"

//
// Two typical uses of this class envisioned:
//
//  1) Parton pT scan
//
//  2) Importance sampling of parton Pt (for use
//     with "importanceSamplingDistro" function).
//
class JetPtTF
{
public:
    // The constructor arguments are as follows:
    //
    // partonPtAxis    -- Definition of the grid in the parton Pt predictor
    //                    variable on which jet pt densities are made.
    //
    // vd              -- Collection of jet pt densities which correspond
    //                    to parton pt values defined by the "partonPtAxis"
    //                    argument.
    //
    // detectorPtCut   -- Cut value for jet Pt in data.
    //
    // sigmaAtPtCut    -- The value of jet systematic uncertainty (sigma_JES)
    //                    at jet Pt = detectorPtCut.
    //
    JetPtTF(const npstat::GridAxis& partonPtAxis,
            const std::vector<CPP11_shared_ptr<const npstat::AbsDistribution1D> >& vd,
            double detectorPtCut, double sigmaAtPtCut);

    void setJetInfo(const JetInfo& info);
    void setDeltaJES(double value);

    inline double getDetectorPtCut() const {return detectorJetPtCut_;}
    inline double getSigmaAtPtCut() const {return sigmaAtPtCut_;}
    inline const JetInfo& getJetInfo() const {return jetInfo_;}
    inline double getJetPt() const {return jetPt_;}
    inline double getDeltaJES() const {return deltaJES_;}

    // JES will be reasonable only after both "setJetInfo" and
    // "setDeltaJES" have been called
    inline double getJES() const {return jes_;}

    // JES at cut, on the other hand, needs only "setDeltaJES"
    inline double getJESAtCut() const {return jesAtCut_;}

    // Sample MC jet Pt for the given parton Pt. To convert this
    // MC Pt into detector jet Pt, one needs to divide by JES.
    // But JES itself depends on the detector jet Pt, so this can
    // not be done without knowing how to calculate Pt-dependent
    // systematic uncertainty. This class does not possess such
    // knowledge -- see "convertMCJetPtToData.hh" header instead.
    //
    // Note that "setDeltaJES" should be called prior to calling
    // this function, otherwise default deltaJES = 0 will be used.
    //
    double sampleMCJetPt(double partonPt, double rnd) const;

    // Jet Pt probability density for the given parton Pt.
    // "setJetInfo" must be called before calling this and
    // subsequent functions.
    double density(double partonPt) const;

    // Jet Pt cumulative probability density for the given parton Pt
    double cdf(double partonPt) const;

    // Operators which compare objects for equality are very useful
    // for I/O testing
    bool operator==(const JetPtTF& r) const;
    inline bool operator!=(const JetPtTF& r) const
        {return !(*this == r);}

    // I/O methods needed for writing
    inline gs::ClassId classId() const {return gs::ClassId(*this);}
    bool write(std::ostream& of) const;

    // I/O methods needed for reading
    static inline const char* classname() {return "JetPtTF";}
    static inline unsigned version() {return 1;}
    static JetPtTF* read(const gs::ClassId& id, std::istream& in);

private:
    JetPtTF();

    void updateJES();

    // Variables set by the constructor
    npstat::GridAxis partonPtAxis_;
    std::vector<CPP11_shared_ptr<const npstat::AbsDistribution1D> > rho0_;
    double detectorJetPtCut_;
    double sigmaAtPtCut_;

    // Variables set inside "setJetInfo"
    JetInfo jetInfo_;
    double jetPt_;

    // Variables set inside "setDeltaJES"
    double deltaJES_;
    double jesAtCut_;

    // Variables set inside "updateJES"
    double jes_;
    double scaleFactor_;
};

#endif // JETPTTF_HH_
