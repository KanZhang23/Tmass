#ifndef JETENERGYCORRECTION_HH
#define JETENERGYCORRECTION_HH
//--------------------------------------------------------------------------
// File and Version Information:
// Please send comments to bhatti@fnal.gov
//
// Environment:
//
// Author List:
//       Anwar A Bhatti The Rockefeller University (Sept 20,2001)
//
// Revisions:
// ----------
//   Sept 20,2002   AAB  Add  Relative Jet Corrections for Run II
//   Nov  20,2002   AAB  Add Pt Dependent Relative Jet Corrections for Run II
//------------------------------------------------------------------------
#include <iostream>
#include <cstdio>

#include "JetEnergyParam.hh"

#ifdef __NO_CDFSOFT__
#include <cmath>
#else
#ifndef __CINT__
#include "CLHEP/Vector/LorentzVector.h"
#else
  class HepLorentzVector;
#endif
#endif //__NO_CDFSOFT__

//------------------------------------
// Collaborating Class Declarations --

class JetEnergyCorrections {

private:

  int  _version, _conesize,_syscode,_nvtx,_level,_nrun,_imode;
  bool _PtDependentCorrectionsON;
  bool _CemCorON;
  bool _UseRun1CorForV5;
  bool _RelCorSysMinimal;
  bool _CentralCrackCorr;

  bool _sysRelativeCorrection;
  bool _sysCentralCalStability;
  bool _sysScaleCorrection;
  bool _sysUnderlyingEventCorrection;
  bool _sysMultipleInteractionCorrection;
  bool _sysAbsoluteCorrection;
  bool _sysOutOfConeCorrection;
  bool _sysSplashOut;
  bool _sysTotalUncert;
  bool _sysDataUncert;

  int _sysSign;

public:

  // Constructors

  JetEnergyCorrections(const char* const theName, const char* const theDescription);

  JetEnergyCorrections();

  JetEnergyCorrections(const char* const theName, const char* const theDescription,
		       int level,int nvtx,int conesize,int version,int syscode=0,int nrun=0,int imode=1);

  JetEnergyCorrections(int level,int nvtx,int conesize,int version,int syscode=0,int nrun=0,int imode=1);

   // Destructor

  virtual ~JetEnergyCorrections( );

  void  Init();             // to initialize quantities from the analysis code 
  void  CheckParameters();  // to check parameters before making corrections

  // added by Monica --> 09/03/2006: setting variables for null constructor 

  void setRunVerticies(int run, int numVerticies);
  void setLevel(int level); 
  void setNvtx(int nvtx); 
  void setConeSize(int conesize); 
  void setVersion(int version); 
  void setNrun(int nrun); 
  void setImode(int imode); 
  void setModeMC(bool mc);
  void setSyscode(int syscode); 

  inline int getLevel() const {return _level;}
  inline int getNvtx() const {return _nvtx;} // GVV added

  inline void setRunNumber(int runNumber){
    _nrun = runNumber; 
  }

  inline void printParameters() const {
    printf("level=%4d nvtx=%4d conesize=%4d version=%4d syscode=%4d nrun=%8d imode=%4d\n",
	   _level,_nvtx,_conesize,_version,_syscode,_nrun,_imode);
  }

#ifndef __NO_CDFSOFT__
  float doEnergyCorrections(HepLorentzVector &fourVector,float& emf,const float eta,int run, int nvtx);
  float doEnergyCorrections(HepLorentzVector &fourVector,float& emf,const float eta);
#endif
  float doEnergyCorrections(float ptin,float& emf,const float eta,int run, int nvtx);
  float doEnergyCorrections(float ptin,float& emf,const float eta);
  float totalJetSystematics(float ptin,float& emf,const float eta, int sysSign, int dataVersion, int mcVersion = 0, bool dataOnly = false);
  float totalJetSystematicsV5(float ptin,float& emf,const float eta, int sysSign);
  void relativeEnergyCorrections(float &pt,const double eta);
  void timeDependentCorrections(float &pt,const float eta,const float emf);
  double ptDependentRelativeCorrectionFactor(double pt,double eta);
  double ptDependentRelativeCorrectionFactorABC(double pt,const double eta,const int NPETA,
						 const double ETA[],const double P0[],const double P1[]);
  double ptDependentRelativeCorrectionFactorABCD(double pt,const double eta,const int NPETA,
						 const double ETA[],const double P0[],const double P1[],const double P2[],const double P3[],
						 const double Pa[],const double Pb[],const double Pc[]);

  void energyScaleCorrections(float &pt,const float eta,float &emFraction);
  
  double runDepCorrFactor(double etaJet,double emfJet);
  double emfJetCorrFactor(double etaJet,double emfJet);

  void relativeEnergyCorrections_96(float &pt,const float eta);
  void relativeEnergyCorrectionsCracks(float &pt,const float eta);

  double splint(const int NP,const double x,const double X[],const double Y[],const double Y2[]);
  void spline(int n, double x[],double y[],const double yp1, const double ypn,double y2[]);

  float absECorrGen5(int conesize, float pt);
  void absoluteEnergyCorrections(float &pt);
  void outOfConeEnergyCorrections(float &pt);
  void multipleInteractionEnergyCorrections(float &pt);
  void underlyingEnergyCorrections(float &pt);
  double  scaleRawMCtoRawDataUsingPhotonJet(double detectorEta);
  double  scaleRawMCtoRawDataUsingJetJet(double detectorEta);

  void invertAbsoluteEnergyCorrections(float &pt);
  void invertOutOfConeEnergyCorrections(float &pt);
  void invertMultipleInteractionEnergyCorrections(float &pt);
  void invertUnderlyingEnergyCorrections(float &pt);

  double Cal2HadError(double pt);

  inline float calculateEta(float p,float pt){
    return 0.5*log( (p+sqrt(p*p-pt*pt))/(p-sqrt(p*p-pt*pt)));
  }

  inline void setPtDepedendentCorrectionsON(bool PtDependentCorrectionsON=true){
    _PtDependentCorrectionsON = PtDependentCorrectionsON;
  }
  inline void setCemCorON(bool CemCorON=true){
    _CemCorON = CemCorON;
  }
  inline void setUseRun1CorForV5(bool UseRun1CorForV5=false){
    _UseRun1CorForV5 = UseRun1CorForV5;
  }
  inline void setRelCorSystematicsMinimal(bool RelCorSysMinimal=true){
   _RelCorSysMinimal=RelCorSysMinimal;
  }
  inline void setCentralCrackCorr(bool CentralCrackCorr=false){
   _CentralCrackCorr=CentralCrackCorr;
  }

  void setSysRelativeCorrection(int i); 
  void setSysCentralCalStability(int i);  
  void setSysScaleCorrection(int i); 
  void setSysUnderlyingEventCorrection(int i); 
  void setSysMultipleInteractionCorrection(int i); 
  void setSysAbsoluteCorrection(int i); 
  void setSysOutOfConeCorrection(int i);
  void setSysSplashOut(int i);
  void setTotalSysUncertainties(int i);
  void setDataSysUncertainties(int i);
  void setSysUncertaintiesOff();

  double sysRelativeCor(double eta,double pt); 
};

#endif
