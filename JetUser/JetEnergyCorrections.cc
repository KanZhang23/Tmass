//////////////////////////////////////////////////////////////////////////
//
//    Please send bug reports to bhatti@fnal.gov
//
// Environment:
//
// Author List:
//             Anwar Ahmad Bhatti  Sept 20, 2001.
//             Rockefeller University
//
// Revisions: 
// ---------- 
//   Sept 20,2002   AAB  Add  Relative Jet Corrections for Run II
//   Nov  20,2002   AAB  Add Pt Dependent Relative Jet Corrections for Run II
//   NOv  25,2002   AAB  Add time dependent corrections from C. Currant
//                       Created 11/14/02 by Charles Currat, LBNL -- CACurrat@lbl.gov
//   Dec  13,2002   J.-F. Arguin  Move time dependent corrections out of relative corrections.
//                                Add negative level to apply single corrections 
//   Jan  15,2003   Anwar  Fix typo for out-of-range Eta values
//                         Fix very high Pt jet corrections
//
//   Jan  20,2003   J.-F. Arguin: Create version 2 of Run II jet corrections:
//
//                                1) Underlying event + multiple interactions corrections 
//                                   updated for Run II
//                                2) Enable energyScaleCorrections:
//                                       - Contain adhoc factor from gamma-jet
//                                       - Contain CEM scale corrections after run 152400
//                                         (before that run, corr. is online)
//                                3) Relative corrections updated up to 2003 January shutdown 
//
//   Feb  01, 2003  Anwar         1) Add Monte Carlo Corrections
//                                2) New Relative Corrections using data upto Jan 13, 2003
//                                3) Use data UE for the Monte Carlo Corrections
//                          
//   Feb  08, 2003  Anwar         Add systematic uncertainties        
//   Feb  24, 2003  Anwar         Add functions to scale Raw Monte Carlo to collider data
//   June 11, 2003  Anwar         Corrections for 4.10.4 data from Feb, 2002 to May 2003.(Version=3)
//                                This version does not have any scale corrections and time dependent 
//                                corrections.
//   July 8, 2003   J.-F. Arguin  Add new syst. uncertainties for version 0, 3, 4:
//                                1) New relative corr. syst. for MC and data
//                                2) Change MC E-scale syst. from 5% to 4%
//   Sept 11, 2003  J.-F. Arguin  Add function totalJetSystematics to calculate
//                                total jet syst.
//   Feb  17, 2004  K. Hatakeyama Version 5 of Run II jet corrections
//                                1) Relative corrections from 5.3.0 stntuples
//                                   up to 2003 September shutdown
//                                2) pt-dependent relative corrections use pt=10GeV for pt<10GeV.
//   Apr  6,  2004  J.-F. Arguin  Modify totalJetSystematics to apply new prescription to 
//                                calculate jet systematics to lower level jets
//   Aug 18,  2004  K. Hatakeyama Update version 5 corrections 
//                                1) Relative corrections for data from the complete 5.3.1 stntuples
//                                   up to Feb 13, 2004 
//                                 - different corrections before/after the Sep 2003 shutdown
//                                   (for central |eta|<0.823, use the same corrections)
//                                 - corrections for MidPoint(MP) algorithm are also included
//                                 - pt-dependent relative corrections use pt=8GeV for pt<8GeV.
//                                1') Relative corrections for MC from 5.3.2 Pythia dijet samples.
//                                 - pt-dependent relative corrections are from data
//                                 - for central |eta|<0.823, use the corrections from data
//                                1'') Relative corrections systematics are not updated yet.  
//                                2) add another flag for MC(imode=0)/data(imode=1)
//                                3) adhoc factors and its errors are updated for version 5
//                                4) change the mult. interaction corr. error from 30% to 15%
//   Sept 7, 2004   T. Spreitzer   - Include Multiple Interactions correction in MC systematics
//                                    calculation.
//                                 - Include a patch for systematics between version 4 and 5, so
//                                    users can still use version 4, if they wish.
//   Sept 7, 2004   K. Hatakeyama  - Change the level-3 MC syst. uncertainty from 4% to 0%. 
//   Sept 22, 2004  T. Spreitzer   - In doEnergyCorrections only give warning message if 
//                                   version > 0 && version < 4.
//                                 - Include a message to user in JetEnergyCorrections
//                                   when version is set to 0 when imode = 0
//                                   and version <= 4 and version >=1.
//   Nov  8,  2004  K. Hatakeyama Update version 5 corrections
//                                1) Relative corrections Pt-dependence & systematics updated
//                                2) Absolute & out-of-cone corrections from the Run2 simulation
//                                3) Absolute & out-of-cone corrections systematics updated
//                                4) Cancel level-3 energy scale corrections for version=5
//                                5) Use setUseRun1CorForV5 to go back to the Run1 absolute
//                                   & out-of-cone corrections.
//   Nov  9,  2004  K. Hatakeyama totalJetSystemaiticsV5 is created for version 5
//   Dec 17,  2004  K. Hatakeyama Update version 5 corrections
//                                1) updated absolute and out-of-cone corrections
//                                2) relative corrections for MC and data up to Aug 2004
//   Feb  5,  2005  K. Hatakeyama Update version 5 corrections
//                                1) Additinal factors in relative corrections to correct for
//                                   bias due to the missing ET significance cut
//                                2) Two sets of uncertainties for relative corrections,
//                                   minimal(default) & maximal versions.
//                                3) Phi crack uncertainties (0.5%) added to the absolute
//                                   correction uncertainties
//   Feb 18,  2005  K. Hatakeyama Update version 5 corrections
//                                1) Updated relative correction uncertainties
//                                2) Updated OOC corrections and uncertainties
//                                3) Updated splash-out uncertainties (0.25GeV)
//                                4) Corrections for the central crack added (off by default)
//   Feb 22,  2005  K. Hatakeyama Update version 5 corrections (bug fixes)
//                                1) Relative correction systematics for |eta|>2.6 fixed.
//                                2) 2nd constructor updated (_imode etc were missing)
//                                3) SplashOut is added to totalJetSystematicsV5
//   Feb 24,  2005  K. Hatakeyama Update version 5 corrections
//                                1) Change the sign of MI and UE correction uncertainties
//                                2) UE is subtrated at level 6, but added back at level 7
//   Mar 23,  2005  K. Hatakeyama Modify totalJetSystematicsV5 (and doEnergyCorrections) to apply
//                                new prescription to calculate jet systematics to lower 
//                                level jets. (many thanks to Erik, Un-ki and Jean-Francois)
//   Dec 30,  2005  K. Hatakeyama Update relative corrections for the Dec 2004 - Sep 2005 data
//                                Central calorimeter stability uncertainty 0.3%->0.5%
//   Jan 30,  2006  K. Hatakeyama Update relative corrections for the Dec 2004 - Sep 2005 data
//                                (additional correction of ~1% for eta=0.6-0.9 only at high Pt)
//                                MI correction systematics from 15% to 30%.
//   Jun 19,  2006  K. Hatakeyama Update relative corrections for 203819 - 207513 (05 Sep 05 - 21 Nov 05)
//                                (WHA gain drop was not well accounted for in the calo calibration)
//   Jun 27,  2006  K. Hatakeyama Make the code independent of cdfsoft2 & CLHEP
//                                (Thanks to Ulrich Husemann & Charles Plager)
//   Apr 12,  2007  K. Hatakeyama Update relative corrections for period 9 (222529-228596)
//                                Takes care of the WHA miscalibration and response drop in east plug
//   Jun 23,  2007  M. D'Onofrio  Update relative corrections for period 10 (228664-233111)
//                                Takes care of the WHA/PHA response drop around eta 1 
//   Jul 23,  2007  M. D'Onofrio  Update relative corrections for period 11 (233133-237795)
//                                Takes care of the WHA/PHA response drop around eta 1 
//   Sept 26,  2007  M. D'Onofrio  Update relative corrections for period 12 (237845-241657)
//                                Takes care of the WHA/PHA response drop around eta 1 
//   May   9,  208  F. Margaroli  Update relative corrections for period 14/15 (252836-256824)  
//                                Takes care of the WHA/PHA response drop in the region 1<|eta|<2

//////////////////////////////////////////////////////////////////////////
//------------------------
// String Header First  --
//------------------------

#include <string>
#include <cassert>
#include <ctime>
#include <iostream>

#ifndef __NO_CDFSOFT__
#include "ErrorLogger_i/gERRLOG.hh"
#else
#include <algorithm>
#endif

//-----------------------
// This Class's Header --
//-----------------------

#include "JetEnergyParam.hh"
#include "JetEnergyCorrections.hh"
#include "splint00.hh"
#include "splint01.hh"
#include "splint02.hh"
#include "splintPt00.hh"
#include "splintPt01.hh"
#include "splintPt02.hh"

#include "splint00MC.hh"

#include "splint00V2.hh"
#include "splint01V2.hh"
#include "splint02V2.hh"
#include "splintPt00V2.hh"
#include "splintPt01V2.hh"
#include "splintPt02V2.hh"
#include "phojet_data_mc.hh"


#include "splint00V3.hh"
#include "splint01V3.hh"
#include "splint02V3.hh"
#include "splint03V3.hh"
#include "splint05V3.hh"

#include "splintPt00V3.hh"
#include "splintPt01V3.hh"
#include "splintPt02V3.hh"
#include "splintPt03V3.hh"

#include "splint00V4.hh"
#include "splintPt00V4.hh"

#include "splint00V5.hh"
#include "splint01V5.hh"
#include "splint02V5.hh"
#include "splint03V5.hh"
#include "splint04V5.hh"
#include "splint05V5.hh"
#include "splint06V5.hh"
#include "splint07V5.hh"
#include "splint08V5.hh"
#include "splint09V5.hh"
#include "splint00MCV5.hh"
#include "splintPt00V5.hh"
#include "splintPt01V5.hh"
#include "splintPt03V5.hh"
#include "splintPt00MCV5.hh"
#include "splintPt00METV5.hh"
#include "splintPt00METMCV5.hh"

JetEnergyCorrections::JetEnergyCorrections(const char* const theName, const char* const theDescription){

  Init();

}

JetEnergyCorrections::JetEnergyCorrections(){

  Init();

}

JetEnergyCorrections::JetEnergyCorrections(const char* const theName, const char* const theDescription,
					   int level,int nvtx,int conesize,int version,int syscode,int nrun,
					   int imode){

  Init();

  _level=level;
  _nvtx=nvtx;  
  _conesize=conesize;
  _version=version;
  _nrun=nrun;
  _imode=imode;
  setSyscode(syscode);

  CheckParameters();

}

JetEnergyCorrections::JetEnergyCorrections(int level,int nvtx,int conesize,int version,int syscode,int nrun,
					   int imode){

  Init();

  _level=level;
  _nvtx=nvtx;  
  _conesize=conesize;
  _version=version;
  _nrun=nrun;
  _imode=imode;
  setSyscode(syscode);

  CheckParameters();

}

JetEnergyCorrections::~JetEnergyCorrections(){
   //delete _manager;
}

// add by Monica 09/03/2006: Initialization 
void JetEnergyCorrections::Init(){
 
  _PtDependentCorrectionsON=true;
  _CemCorON=true;
  _UseRun1CorForV5=false;
  _RelCorSysMinimal=true;
  _CentralCrackCorr=false;

  _sysRelativeCorrection=false;
  _sysCentralCalStability=false;
  _sysScaleCorrection=false;
  _sysMultipleInteractionCorrection=false;
  _sysAbsoluteCorrection=false;
  _sysUnderlyingEventCorrection=false;
  _sysOutOfConeCorrection=false;
  _sysSplashOut=false;
  _sysTotalUncert=false;
  _sysDataUncert=false;
  _sysSign=0;

}

void JetEnergyCorrections::CheckParameters(){

  // if MC, _version=0.
  if ( _imode == 0 && _version >= 1 && _version <= 4 ){
    _version=0;
    std::cout << "*** WARNING : JetEnergyCorrections " << std::endl;
    std::cout << " Version has been set to 0, MC corrections being done. " << std::endl;
  }
  // if _version=0, _imode=1.
  if ( _version == 0 && _imode == 1 ){
    std::cout << std::endl;
    std::cout << "*** WARNING *** WARNING *** WARNING : JetEnergyCorrections " << std::endl;
    std::cout << "You set _version=0(which is for MC) but _imode=1(which is for data)." << std::endl;
    std::cout << "_imode is set to 0(which is for MC)." << std::endl;
    std::cout << std::endl;
    _imode=0;
  }
  // if the specified version is not available yet, use the latest version.
  if ( _version > NVersion-1 ){
    std::cout << std::endl;
    std::cout << "*** WARNING *** WARNING *** WARNING : JetEnergyCorrections " << std::endl;
    std::cout << "The specified version is not available yet." << std::endl;
    std::cout << "The version is now set to " << NVersion-1 << "." << std::endl;
    std::cout << std::endl;
    _version=NVersion-1;
  }
  // use the JetClu corrections for MidPoint, Dec 27, 2005
  if (_conesize >= 3 && _conesize<=5 ) _conesize=_conesize-3;
  // _conesize should be between 0 and 5.
  if (_conesize<0 || _conesize>5 ){
    std::cout << std::endl;
    std::cout << "*** WARNING *** WARNING *** WARNING : JetEnergyCorrections " << std::endl;
    std::cout << "The conesize should be between 0 and 5." << std::endl;
    std::cout << std::endl;    
    return;
  }
 
}

// Set run number and vertices in a separate function 
void JetEnergyCorrections::setRunVerticies(int run, int nvtx){
  _nrun=run;
  _nvtx=nvtx;  
}

void JetEnergyCorrections::setLevel(int level){
  _level=level;
}
void JetEnergyCorrections::setNvtx(int nvtx){
  _nvtx=nvtx;
}
void JetEnergyCorrections::setConeSize(int conesize){
  _conesize=conesize;
}
void JetEnergyCorrections::setVersion(int version){
  _version=version;
}
void JetEnergyCorrections::setNrun(int nrun){
  _nrun=nrun;
}
void JetEnergyCorrections::setImode(int imode){
  _imode=imode;
}
void JetEnergyCorrections::setModeMC(bool mc){
  _imode=1;
  if (mc) _imode=0;
}
void JetEnergyCorrections::setSyscode(int syscode){
  _syscode=syscode;

  if(abs(_syscode) == 1) 
    _sysRelativeCorrection=true;
  else
    _sysRelativeCorrection=false;
  if(abs(_syscode) == 2) 
    _sysCentralCalStability=true;
  else
    _sysCentralCalStability=false;
  if(abs(_syscode) == 3) 
    _sysScaleCorrection=true;
  else
    _sysScaleCorrection=false;
  if(abs(_syscode) == 4) 
    _sysMultipleInteractionCorrection=true;
  else
    _sysMultipleInteractionCorrection=false;
  if(abs(_syscode) == 5) 
    _sysAbsoluteCorrection=true;
  else
    _sysAbsoluteCorrection=false;
  if(abs(_syscode) == 6) 
    _sysUnderlyingEventCorrection=true;
  else
    _sysUnderlyingEventCorrection=false;
  if(abs(_syscode) == 7) 
    _sysOutOfConeCorrection=true;
  else
    _sysOutOfConeCorrection=false;
  if(abs(_syscode) == 8) 
    _sysSplashOut=true;
  else
    _sysSplashOut=false;
  if(abs(_syscode) == 100)
    _sysTotalUncert=true;
  else
    _sysTotalUncert=false;
  if(abs(_syscode) == 101)
    _sysDataUncert=true;
  else
    _sysDataUncert=false;

  if(_sysUnderlyingEventCorrection && (_version<=4 || (_UseRun1CorForV5 && _version==5) ))
    std::cout << "WARNING: the syst. uncert. for the underlying event corr. will be computed. Be aware that the underlying event corr. uncertainty is already included in the Run1 absolute corr. uncert., so don't count it twice!" << std::endl;

  _sysSign=0;
  if(_syscode > 0)
    _sysSign=1;
  if(_syscode < 0)
    _sysSign=-1;

}


#ifndef __NO_CDFSOFT__
float JetEnergyCorrections::doEnergyCorrections(HepLorentzVector &fourVector,float& emf,const float eta,int run, int nvtx){

  setRunVerticies(run,nvtx);
  float ptin = fourVector.perp();
  return doEnergyCorrections(ptin, emf, eta);

}
float JetEnergyCorrections::doEnergyCorrections(HepLorentzVector &fourVector,float& emf,const float eta){

  float ptin = fourVector.perp();
  return doEnergyCorrections(ptin, emf, eta);

}
#endif

float JetEnergyCorrections::doEnergyCorrections(float ptin, float& emf,const float eta,int run, int nvtx){

  setRunVerticies(run,nvtx);
  return doEnergyCorrections(ptin, emf, eta);

}
float JetEnergyCorrections::doEnergyCorrections(float ptin, float& emf,
						const float eta) {
  // Midified by Erik Brubaker, March 15, 2005
  if (ptin>800. || ptin<=0.0) {
    //    std::cout << "JetEnergyCorrections: Jet with Pt = " << ptin;
    //    std::cout << " can not be corrected " << std::endl;
    return 1.0;
  }
  
  float ptsave =ptin;
  float &pt =ptin;
    
  // If you are using version 5 and evaluating systematics
  if(_sysSign!=0 && _version==5 && !_UseRun1CorForV5){
  if(_level>=1){
     relativeEnergyCorrections(pt,eta);
     timeDependentCorrections(pt,eta,emf);
     energyScaleCorrections(pt,eta,emf);
     multipleInteractionEnergyCorrections(pt);
     absoluteEnergyCorrections(pt);
     underlyingEnergyCorrections(pt);
     outOfConeEnergyCorrections(pt);
     if (_level <= 6) invertOutOfConeEnergyCorrections(pt);
     if (_level <= 5) invertUnderlyingEnergyCorrections(pt);
     if (_level <= 4) invertAbsoluteEnergyCorrections(pt);
     if (_level <= 3) invertMultipleInteractionEnergyCorrections(pt);
  }  //  _level>=1
  }  //  _sysSign!=0 && _version==5 && !_UseRun1CorForV5
  else {
    if(_level>=1) relativeEnergyCorrections(pt,eta);
    if(_level>=2) timeDependentCorrections(pt,eta,emf);
    if(_level>=3) energyScaleCorrections(pt,eta,emf);
    if(_level>=4) multipleInteractionEnergyCorrections(pt);
    if(_level>=5) absoluteEnergyCorrections(pt);
    if(_level>=6) underlyingEnergyCorrections(pt);
    if(_level>=7) outOfConeEnergyCorrections(pt);
  }

  // Negative level: apply just the individual corrections:
  if(_level==-1) relativeEnergyCorrections(pt,eta);
  if(_level==-2) timeDependentCorrections(pt,eta,emf);
  if(_level==-3) energyScaleCorrections(pt,eta,emf);
  if(_level==-4) multipleInteractionEnergyCorrections(pt);
  if(_level==-5) absoluteEnergyCorrections(pt);
  if(_level==-6) underlyingEnergyCorrections(pt);
  if(_level==-7) outOfConeEnergyCorrections(pt);

  // calculate total jet systematics
  int dataVersion;
  int mcVersion;
  // The following is a small patch to bridge the gap between version 4 and
  // version 5 syst. Version 5 now uses imode, making this patch necessary.
  // TS, Sept 04
  if(_version < 4 && _version > 0){
    std::cerr << " Error: Jet Systematics only valid for version 4 and "
	      << std::endl;
    std::cerr << " higher, use at least version 4! " << std::endl;
    assert(0);
  } else if (_version > 4) {
    dataVersion = _version;
    mcVersion = _version;
  } else if (_version == 4 || _version == 0){
    dataVersion = 4;
    mcVersion = 0;
  } 
  
  if (_version <= 4 || _UseRun1CorForV5) {
    if (_sysTotalUncert)
      pt *= 1 + _sysSign*totalJetSystematics(ptsave,emf,eta,_sysSign,
					     dataVersion,mcVersion);
    if (_sysDataUncert)
      pt *= 1 + _sysSign*totalJetSystematics(ptsave,emf,eta,_sysSign,
					     dataVersion,mcVersion, true);
  } else {
    // version 5
    if (_sysTotalUncert)
      pt *= 1 + _sysSign*totalJetSystematicsV5(ptsave,emf,eta,_sysSign);
  }
  
  float return_value;
  if(_version == 1)
    return_value = adhocfactorVersion1[_conesize]*pt/ptsave;
  else
    return_value = pt/ptsave;
  
  return return_value; 

}

void JetEnergyCorrections::relativeEnergyCorrections(float &pt,
						     const double eta) {
  
  double correction_factor=1.0;

  // Generic relative corrections for all Pt/full Eta range
  if(_version==0) {
    if (fabs(eta)>1) {
      correction_factor=splint(NP00MC[_conesize],eta,X00MC[_conesize],Y00MC[_conesize],Y200MC[_conesize]);
    } else {
      // BH: use data correction factors for central region (MC statistics was too low to get
      // proper correction out)
      correction_factor=splint(NP00V4[_conesize],eta,X00V4[_conesize],Y00V4[_conesize],Y200V4[_conesize]);
    }
  }
  else if(_version==1) {
    if( _nrun>=BEGIN_RUN00 && _nrun <= END_RUN00){
      correction_factor=splint(NP00[_conesize],eta,X00[_conesize],Y00[_conesize],Y200[_conesize]);
    }
    else if( _nrun>BEGIN_RUN01 && _nrun <= END_RUN01){
      correction_factor=splint(NP01[_conesize],eta,X01[_conesize],Y01[_conesize],Y201[_conesize]);
    }
    else if( _nrun>BEGIN_RUN02 && _nrun <= END_RUN02){
      correction_factor=splint(NP02[_conesize],eta,X02[_conesize],Y02[_conesize],Y202[_conesize]);
    }
  }
  else if(_version==2) {
    if( _nrun>=BEGIN_RUN00 && _nrun <= END_RUN00){
      correction_factor=splint(NP00V2[_conesize],eta,X00V2[_conesize],Y00V2[_conesize],Y200V2[_conesize]);
    }
    else if( _nrun>BEGIN_RUN01 && _nrun <= END_RUN01){
      correction_factor=splint(NP01V2[_conesize],eta,X01V2[_conesize],Y01V2[_conesize],Y201V2[_conesize]);
    }
    else if( _nrun>BEGIN_RUN02 && _nrun <= END_RUN02){
      correction_factor=splint(NP02V2[_conesize],eta,X02V2[_conesize],Y02V2[_conesize],Y202V2[_conesize]);
    }
  }
  else if(_version==3) {
    if( _nrun>=BEG_RUN00V3 && _nrun <= END_RUN00V3){
      correction_factor=splint(NP00V3[_conesize],eta,X00V3[_conesize],Y00V3[_conesize],Y200V3[_conesize]);
    }
    else if( _nrun>BEG_RUN01V3 && _nrun <= END_RUN01V3){
      correction_factor=splint(NP01V3[_conesize],eta,X01V3[_conesize],Y01V3[_conesize],Y201V3[_conesize]);
    }
    else if( _nrun>BEG_RUN02V3 && _nrun <= END_RUN02V3){
      correction_factor=splint(NP02V3[_conesize],eta,X02V3[_conesize],Y02V3[_conesize],Y202V3[_conesize]);
    }
    else if( _nrun>BEG_RUN03V3 && _nrun <= END_RUN03V3){
      correction_factor=splint(NP03V3[_conesize],eta,X03V3[_conesize],Y03V3[_conesize],Y203V3[_conesize]);
    }
  }
  else if(_version==4) {
    if( _nrun>=BEG_RUN00V4 && _nrun <= END_RUN00V4){
      correction_factor=splint(NP00V4[_conesize],eta,X00V4[_conesize],Y00V4[_conesize],Y200V4[_conesize]);
     }
    else if( _nrun>END_RUN00V4 && _nrun < BEG_RUN05V3){
      //   These are the same post-shutwdown corrections as in version =3
      correction_factor=splint(NP03V3[_conesize],eta,X03V3[_conesize],Y03V3[_conesize],Y203V3[_conesize]);
    }
    else if( _nrun>BEG_RUN05V3 && _nrun <= 999999){
      //   These are the same post-shutwdown corrections as in version =3
      correction_factor=splint(NP05V3[_conesize],eta,X05V3[_conesize],Y05V3[_conesize],Y205V3[_conesize]);
    }
  }  
  else if(_version>=5) {   // new corrections from 5.3.1 stntuples.
    if (_imode == 0){
      //MC - use correction factors from Pythia
      correction_factor=splint(NP00MCV5[_conesize],eta,X00MCV5[_conesize],Y00MCV5[_conesize],Y200MCV5[_conesize]);
      //MC - use correction factors from data before Sep 2003
      //correction_factor=splint(NP00V5[_conesize],eta,X00V5[_conesize],Y00V5[_conesize],Y200V5[_conesize]);
    } else {
      // Data before Sep 2003
      if( _nrun>=BEG_RUN00V5 && _nrun<=END_RUN00V5 ){
	correction_factor=splint(NP00V5[_conesize],eta,X00V5[_conesize],Y00V5[_conesize],Y200V5[_conesize]);
      }
      // Data between Nov 2003 - Feb 13, 2004
      else if( _nrun>=BEG_RUN01V5 && _nrun<=END_RUN01V5 ){
	if (fabs(eta)>0.823) {
	  correction_factor=splint(NP01V5[_conesize],eta,X01V5[_conesize],Y01V5[_conesize],Y201V5[_conesize]);
	} else {
	  // use correction factors from data before Sep 2003 for central region
	  correction_factor=splint(NP00V5[_conesize],eta,X00V5[_conesize],Y00V5[_conesize],Y200V5[_conesize]);
	}
      }
      // Data between Feb 13, 2004 - Aug 22, 2004
      else if ( _nrun>=BEG_RUN02V5 && _nrun<=END_RUN02V5 ) {
	correction_factor=splint(NP02V5[_conesize],eta,X02V5[_conesize],Y02V5[_conesize],Y202V5[_conesize]);
      }
      // Data after September, 2004
      else if ( _nrun>END_RUN02V5 && _nrun<BEG_RUN04V5) {
	correction_factor=splint(NP03V5[_conesize],eta,X03V5[_conesize],Y03V5[_conesize],Y203V5[_conesize]);
      }
      // Data between 05 Sep 05 - 21 Nov 05
      else if ( _nrun>=BEG_RUN04V5 && _nrun<=END_RUN04V5) {
	correction_factor=splint(NP04V5[_conesize],eta,X04V5[_conesize],Y04V5[_conesize],Y204V5[_conesize]);
      }
      // Data between November 2005 and September 1, 2006
      else if ( _nrun>END_RUN04V5  && _nrun<BEG_RUN05V5 ) {
	correction_factor=splint(NP03V5[_conesize],eta,X03V5[_conesize],Y03V5[_conesize],Y203V5[_conesize]);
      }
      // Data between 01 Sep 06 and 22 Nov 06 [period 9]
      else if ( _nrun>=BEG_RUN05V5 && _nrun<=END_RUN05V5 ) {
	correction_factor=splint(NP05V5[_conesize],eta,X05V5[_conesize],Y05V5[_conesize],Y205V5[_conesize]);
      }
      // Data between Nov 06 and 30 January 2007 [period 10]
      else if ( _nrun>=BEG_RUN06V5 && _nrun<=END_RUN06V5 ) {
	correction_factor=splint(NP06V5[_conesize],eta,X06V5[_conesize],Y06V5[_conesize],Y206V5[_conesize]);
      }
      // Data between Jan. 2007 and March 30th, 2007 [period 11]
      else if ( _nrun>=BEG_RUN07V5 && _nrun<=END_RUN07V5 ) {
	correction_factor=splint(NP07V5[_conesize],eta,X07V5[_conesize],Y07V5[_conesize],Y207V5[_conesize]);
      }
      // Data between March. 2007 and May 13th, 2007 [period 12]
      else if ( _nrun>=BEG_RUN08V5 && _nrun<=END_RUN08V5 ) {
	correction_factor=splint(NP08V5[_conesize],eta,X08V5[_conesize],Y08V5[_conesize],Y208V5[_conesize]);
      }
      // Data between 13 May, 2007 and 04 Aug, 2007 [period 13]
      else if ( _nrun>END_RUN08V5 && _nrun<=246231 ) {
	correction_factor=splint(NP03V5[_conesize],eta,X03V5[_conesize],Y03V5[_conesize],Y203V5[_conesize]);
      }
      //Data between 28 Oct, 2007 and 27th Jan 2008 [period 14+15]
      else if ( _nrun>252836 && _nrun<=256824 ) {
	correction_factor=splint(NP09V5[_conesize],eta,X09V5[_conesize],Y09V5[_conesize],Y209V5[_conesize]);
      }
    }
  }

  pt /=correction_factor;

  if(_PtDependentCorrectionsON) {

    correction_factor=ptDependentRelativeCorrectionFactor(pt,eta);
    pt /= correction_factor;
    // std::cout << " Correction factor 2 " <<  correction_factor<< std::endl;
  }

  if (_CentralCrackCorr){
    correction_factor=1.0;
    if (fabs(eta)<0.16) correction_factor=0.95+(fabs(eta)*5./16.);
    pt /= correction_factor;
  }

  if(_sysRelativeCorrection) {
    pt *=(1+_sysSign*sysRelativeCor(eta,pt));
  }

}

void JetEnergyCorrections::timeDependentCorrections(float &pt, const float eta, const float emFraction) {

  double correction_factor=1.0;

  if(_nrun>0) {
    if(  _version == 1 || _version ==2 ) {
      correction_factor=runDepCorrFactor(eta,emFraction);
      pt *=correction_factor;
    }
  }

  return;
}

double  JetEnergyCorrections::ptDependentRelativeCorrectionFactor(double pt,const double eta){

  double corr=1.0;

  double pttmp;
  double ptmax =250.;
  //
  double ptmax_data_v5 = 260.;
  double ptmax_mc_v5   = 420.;
  double ptmin_v5  =   8.;
  double ptmin2_v5 =  25.;     // for MC, 0.065<|eta|<1.930

  if(_version==0) {
    pttmp=std::min(pt,150.);
    corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA00[_conesize],
        SPLINTETA00[_conesize],SPLINTP000[_conesize],SPLINTP100[_conesize]);
  }
  else if(_version==1) {
    pttmp=std::min(pt,150.);
    if( _nrun>=BEGIN_RUN00 && _nrun <= END_RUN00){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA00[_conesize],
        SPLINTETA00[_conesize],SPLINTP000[_conesize],SPLINTP100[_conesize]);
    }
    else if( _nrun>BEGIN_RUN01 && _nrun <= END_RUN01){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA01[_conesize],
      SPLINTETA01[_conesize],SPLINTP001[_conesize],SPLINTP101[_conesize]);
    }
    else if( _nrun>BEGIN_RUN02 && _nrun <= END_RUN02){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA02[_conesize],
        SPLINTETA02[_conesize],SPLINTP002[_conesize],SPLINTP102[_conesize]);
    }
  }
  else if(_version==2) {
    pttmp=std::min(pt,ptmax);
    if( _nrun>=BEGIN_RUN00 && _nrun <= END_RUN00){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA00V2[_conesize],
        SPLINTETA00V2[_conesize],SPLINTP000V2[_conesize],SPLINTP100V2[_conesize]);
    }
    else if( _nrun>BEGIN_RUN01 && _nrun <= END_RUN01){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA01V2[_conesize],
      SPLINTETA01V2[_conesize],SPLINTP001V2[_conesize],SPLINTP101V2[_conesize]);
    }
    else if( _nrun>BEGIN_RUN02 && _nrun <= END_RUN02){
      corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA02V2[_conesize],
        SPLINTETA02V2[_conesize],SPLINTP002V2[_conesize],SPLINTP102V2[_conesize]);
    }
  }
  else if(_version==3 || _version==4) {  // new corrections Anwar 6/15/2003 same for version
                           // 3 and 4
    pttmp=std::min(pt,ptmax);
    // same correction for pre- and post-shutdown data
    corr= ptDependentRelativeCorrectionFactorABC(pttmp,eta,NPETA00V4[_conesize],
	SPLINTETA00V4[_conesize],SPLINTP000V4[_conesize],SPLINTP100V4[_conesize]);
  }
  else if(_version>=5) {  // new corrections from 5.3.1 stntuples.
    if (_imode == 0){
      // MC
      pttmp=std::min(pt,ptmax_mc_v5);
      pttmp=std::max(pttmp,ptmin_v5);
      if (fabs(eta)>0.065 && fabs(eta)<1.93) {
	pttmp=std::max(pttmp,ptmin2_v5);
      }
      corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA00MCV5[_conesize],SPLINTETA00MCV5[_conesize],
	      SPLINTP000MCV5[_conesize],SPLINTP100MCV5[_conesize],SPLINTP200MCV5[_conesize],SPLINTP300MCV5[_conesize],
	      SPLINTP000METMCV5[_conesize],SPLINTP100METMCV5[_conesize],SPLINTP200METMCV5[_conesize]);
    } else {
      // Data
      pttmp=std::min(pt,ptmax_data_v5);
      pttmp=std::max(pttmp,ptmin_v5);     
      if( _nrun>=BEG_RUN00V5 && _nrun <= END_RUN00V5){
	corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA00V5[_conesize],SPLINTETA00V5[_conesize],
 	      SPLINTP000V5[_conesize],SPLINTP100V5[_conesize],SPLINTP200V5[_conesize],SPLINTP300V5[_conesize],
              SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
      }
      // Data between Nov 2003 - Feb 13, 2004
      else if( _nrun>=BEG_RUN01V5 && _nrun <= END_RUN01V5){
	if (fabs(eta)>0.823) {
	  corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA01V5[_conesize],SPLINTETA01V5[_conesize],
                SPLINTP001V5[_conesize],SPLINTP101V5[_conesize],SPLINTP201V5[_conesize],SPLINTP301V5[_conesize],
		SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
	} else {
	  corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA00V5[_conesize],SPLINTETA00V5[_conesize],
		SPLINTP000V5[_conesize],SPLINTP100V5[_conesize],SPLINTP200V5[_conesize],SPLINTP300V5[_conesize],
		SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
	}
      }
      // Data after Feb 13, 2004
      // (Same corrections for
      //  data between Feb 13, 2004 and Aug 22, 2004, and
      //  data after September, 2004)
      else if ( _nrun>=BEG_RUN02V5 && _nrun<=END_RUN02V5 ) {
	corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA00V5[_conesize],SPLINTETA00V5[_conesize],
	      SPLINTP000V5[_conesize],SPLINTP100V5[_conesize],SPLINTP200V5[_conesize],SPLINTP300V5[_conesize],
	      SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
      }
      // Data after September, 2004
      else if ( _nrun>END_RUN02V5 ) {
	corr= ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA03V5[_conesize],SPLINTETA03V5[_conesize],
 	      SPLINTP003V5[_conesize],SPLINTP103V5[_conesize],SPLINTP203V5[_conesize],SPLINTP303V5[_conesize],
              SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
	//
	// At eta=0.616-0.916, the first step relative correction is the same as used for the earlier data;
        // however, the pt-dependent relative correction is modified to compensate the 1% drop at high Pt.
        // Below, we make sure that the additional correction doesn't reduce jet Pt at low Pt (with corr2) & 
        // is no more than than ~+1% (with corr3).
	//
	if (eta>0.60 && eta<0.95) {
	  double 
	    corr2=ptDependentRelativeCorrectionFactorABCD(pttmp,eta,NPETA00V5[_conesize],SPLINTETA00V5[_conesize],
	      SPLINTP000V5[_conesize],SPLINTP100V5[_conesize],SPLINTP200V5[_conesize],SPLINTP300V5[_conesize],
	      SPLINTP000METV5[_conesize],SPLINTP100METV5[_conesize],SPLINTP200METV5[_conesize]);
	  double additionalCorrection[6]={0.9890,0.9917,0.9947,0.9890,0.9917,0.9947};
	  double corr3=corr2*additionalCorrection[_conesize];
	  if (corr>corr2){
	    corr=corr2;
	    //printf("corr>corr2, pttmp=%8.3f\n",pttmp);
	  }
	  else if (corr<corr3){
	    corr=corr3;
	    //printf("corr<corr3, pttmp=%8.3f\n",pttmp);
	  }
	}
	//
      }
    }    
  }

  return corr;
  //  std::cout << " pt " << pt << " eta "<< eta << std::endl;
}


double  JetEnergyCorrections::ptDependentRelativeCorrectionFactorABC(double pt,const double eta,
  const int NPETA, const double ETA[],const double P0[],const double P1[]) {

  const double PMAXII=980.0;

  if(pt>=0.99*PMAXII) {
    // std::cout << " Invalid Jet " << pt << std::endl; 
    return 1.0;
  }

  double etaMax=calculateEta(PMAXII,pt);
  double etaMin=-1.0*etaMax;
  // std::cout << " Eta Max " << etaMax << std::endl;

  int ietamin=-1;
  if(etaMin < ETA[0]){
    ietamin=0;
  }
  else {
    int ieta=0;
    while (ietamin<0 && ieta < NPETA){
      if((etaMin >= ETA[ieta]) && (etaMin <  ETA[ieta+1])) {ietamin=ieta;}
      ieta++;
    }
  }

  //std::cout << " ietamin  " << ietamin << std::endl;

  int ietamax=-1;
  if(etaMax > ETA[NPETA-1]){
    ietamax=NPETA;
  }
  else {
    int ieta=NPETA-1;
    while (ietamax<0 && ieta >=0 ){
      if(( etaMax >= ETA[ieta-1]) && (etaMax<ETA[ieta])) {ietamax=ieta+1;}
      ieta--;
    }
  }

  //  std::cout << " ietamax  " << ietamax << std::endl;

  double deta1=std::max(eta,ETA[0]);
  deta1=std::min(deta1,ETA[NPETA-1]);

  // std::cout << " Eta Max " << etaMax << " IETAMIN " << ietamin;
  // std::cout << " IETAMAX " << ietamax;  
  // std::cout	<<  " NPETA  " << NPETA00 <<std::endl;

  double  deta[100],y[100],y2[100];
  int n=0;

  for(int ieta=ietamin;ieta<ietamax;ieta++){
    deta[n]= ETA[ieta];
    y[n]   = P0[ieta]+pt*P1[ieta];
    n++;
  }

  //  std::cout << " n " << n << " eta[0] " << deta[0] << " eta[n-1] " << deta[n-1];
  //  std::cout << " y[0] " << y[0] << " y[n-1] " << y[n-1];
  // std::cout << " eta " << eta <<  " pt " << pt<< " deta1 " << deta1 << std::endl;  

  // Get spline parameters and correction factor
   
  double beg=0.;
  double end=0.; 
  spline(n,deta, y,beg,end,y2);
  double cf = splint(n,deta1,deta,y,y2);
  // std::cout << "Pt dependent correctiobs " << cf << std::endl;

  return cf;
}

//
//  To use p1+p2*x+(p3+p4*x)*log(x) for MC
//  === start
double  JetEnergyCorrections::ptDependentRelativeCorrectionFactorABCD(double pt,const double eta,
  const int NPETA, const double ETA[],const double P0[],const double P1[],const double P2[],const double P3[],
  const double Pa[],const double Pb[],const double Pc[]) {

  const double PMAXII=980.0;

  if(pt>=0.99*PMAXII) {
    return 1.0;
  }

  double etaMax=calculateEta(PMAXII,pt);
  double etaMin=-1.0*etaMax;

  int ietamin=-1;
  if(etaMin < ETA[0]){
    ietamin=0;
  }
  else {
    int ieta=0;
    while (ietamin<0 && ieta < NPETA){
      if((etaMin >= ETA[ieta]) && (etaMin <  ETA[ieta+1])) {ietamin=ieta;}
      ieta++;
    }
  }

  int ietamax=-1;
  if(etaMax > ETA[NPETA-1]){
    ietamax=NPETA;
  }
  else {
    int ieta=NPETA-1;
    while (ietamax<0 && ieta >=0 ){
      if(( etaMax >= ETA[ieta-1]) && (etaMax<ETA[ieta])) {ietamax=ieta+1;}
      ieta--;
    }
  }

  double deta1=std::max(eta,ETA[0]);
  deta1=std::min(deta1,ETA[NPETA-1]);

  double  deta[100],y[100],y2[100];
  int n=0;
  double fix;

  for(int ieta=ietamin;ieta<ietamax;ieta++){
    deta[n]= ETA[ieta];
    fix    = Pa[ieta]+(pt*Pb[ieta]/100)+(pt*pt*Pc[ieta]/10000.);   // 
    if (pt<100. && fix>1.) fix=1.;
    y[n]   = P0[ieta]+pt*P1[ieta]+(P2[ieta]+pt*P3[ieta])*log(pt);  //
    y[n]   = y[n]*fix;                                             //
    n++;
  }

  // Get spline parameters and correction factor
  double beg=0.;
  double end=0.;
  spline(n,deta, y,beg,end,y2);
  double cf = splint(n,deta1,deta,y,y2);

  return cf;
}
//  === end


void JetEnergyCorrections::relativeEnergyCorrections_96(float &pt, const float eta) {
 
  double  sply[37], sply2[37], etaval2[37];

  double PMAXI=900.0;

  double etamax;  (pt<0.999*PMAXI)?  etamax = calculateEta(PMAXI,pt) : etamax = 0.0;
  
  int nspl =0;
  while(fabs(ETAVAL[nspl+NSPLMID])<etamax && nspl < NSPLMAX){
    nspl++;
  }
  int  nspllow =NSPLMID-nspl;
  int  nsplhigh=NSPLMID+nspl;

/*
C  Determine the spline parameters from the linear fit,
C  and also re-order the ETAVAL array for input to QDSPLN,QDSPLT
C  Option to change relative scale OUTSIDE THE CENTRAL (0-->0.8 in eta)
C
C  The factor of 2*PT corrects for the bug in the original version of 
C  JTC96X.  The relative corrections are measured in bins of Sum dijet pt, 
C  so 2*Pt is more accurate than Pt.  Robert Harris, 11/21/95
C 
*/

  int jspl=0;
  for(int ispl=nspllow;ispl<=nsplhigh;ispl++){
    etaval2[jspl] = ETAVAL[ispl];   
    if(fabs(ETAVAL[ispl])<= 0.8 ) {  // central : no error 
//        std::cout << " P0 " <<  P0[_version][_conesize][ispl] << " P1 " <<P1[_version][_conesize][ispl] << std::endl;
      sply[jspl] = P0[_version][_conesize][ispl]+2*pt*P1[_version][_conesize][ispl]/100.;
    }
    else {   //  outside central 
      if(_syscode ==0 ) {  // nominal
//        std::cout << " P0 " <<  P0[_version][_conesize][ispl] << " P1 " <<P1[_version][_conesize][ispl] << std::endl;
	sply[jspl] = P0[_version][_conesize][ispl]+2*pt*P1[_version][_conesize][ispl]/100.;
      }
      else if(_syscode>0){ // positive sys error
	sply[jspl] = P0P[_version][_conesize][ispl]+2*pt*P1P[_version][_conesize][ispl]/100.;
      }
      else if(_syscode<0){ // negative sys error
	sply[jspl] = P0M[_version][_conesize][ispl]+2*pt*P1M[_version][_conesize][ispl]/100.;
      }
    }
    jspl++;
  }
  int totspln =jspl;

  //C
  //C  Compute the spline.  Unfortunately, since the spline parameters must
  //C  be interpolated for each jet Pt, the spline has to be re-computed
  //C  for every jet.
  //C

  float yp1=0.0;
  float ypn=0.0;
  //  qdspln_(etaval2,sply,&totspln,&yp1,&ypn,sply2);
  spline(totspln,etaval2,sply,yp1,ypn,sply2);


  //C
  //C  Extrapolate the correction as a flat line beyond the measured limits
  //C

  float factor;

  float etamn=etaval2[0];
  float etamx=etaval2[totspln-1];
  if(eta < etamn){
    //  qdsplt_(etaval2,sply,sply2,&totspln,&etamn,&factor);
    factor=splint(totspln,etamn,etaval2,sply,sply2);
  }
  else if (etamn <= eta && eta <=  etamx){
    // float etatmp=eta;
    //    qdsplt_(etaval2,sply,sply2,&totspln,&etatmp,&factor);
    factor=splint(totspln,eta,etaval2,sply,sply2);
  }
  else {
    //    qdsplt_(etaval2,sply,sply2,&totspln,&etamx,&factor);
    factor=splint(totspln,etamx,etaval2,sply,sply2);
  }
  //  std::cout << "Relative Corrections: " << " Ptin " << pt << " Eta  " << eta << " Scale Factor " << factor << std::endl;
  pt *=factor;
}
       
void JetEnergyCorrections::relativeEnergyCorrectionsCracks(float &pt, const float eta) {
  //C
  //C  Correct the eta=0 and eta=+/-1.1 correction for higher pts
  //C  Only implemented for new 1A and 1B
  //C
	
  if(fabs(eta)<=0.05){
    if(pt>= 40.0 && pt<= 55.0){
      pt *= 1.0+(pt-40.0)*(P0_ETA_0[_version][_conesize]-1.0)/15;
    }
    else if(pt>55.0){
      pt *= (P0_ETA_0[_version][_conesize]-(pt-55.0)*P1_ETA_0[_version][_conesize]);
    }
  }
  
  if(fabs(eta) <= 1.15 && fabs(eta) >= 1.05) {
    if (pt> 40.0 && pt <= 55.0){
      pt *= 1.0+(pt-40.0)*(P0_ETA_1[_version][_conesize]-1.0)/15;
    }
    else if(pt>55.0){
      pt *= P0_ETA_1[_version][_conesize]-(pt-55.0)*P1_ETA_1[_version][_conesize];
    }
  }
}

double JetEnergyCorrections::splint(const int NP,const double x,const double X[],const double Y[],const double Y2[]){

  int k;
  int klo=0;
  int khi=NP-1;

  if(x< X[0]) {
   return Y[0];
  }
  else if(x> X[NP-1]) {
    return Y[NP-1];
  }
  else {

    do{ 
      k=(khi+klo)/2;
      X[k] > x  ? khi=k : klo=k;
    }  while ((khi-klo)>1);

    double h = X[khi]-X[klo];
 
    if(h != 0.) {
      double a= (X[khi]-x)/h;
      double b= (x-X[klo])/h;

      double y= a*Y[klo]+b*Y[khi]+((a*a*a-a)*Y2[klo]+(b*b*b-b)*Y2[khi])*(h*h)/6.0;
      return y;
    }
    else {
      return Y[klo];
    }
  }
}


void JetEnergyCorrections::spline(int n, double x[],double y[],const double yp1, const double ypn,
	    double y2[]){

  int i,k;

  double p,qn,un,sig;

  double u[200];

  if(yp1>0.99e30) {
    y2[0]=0.0;
    u[0]=0.0;
  }
  else {
    y2[0]=-0.5;
    u[0] = (3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
  }

  for (i=1;i<n-1;i++) {
    sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
    p= sig*y2[i-1]+2.0;
    y2[i]=(sig-1.0)/p;
    u[i]= (y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
    u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
  }

  if(ypn>0.99e30) {
    qn=0.0;
    un=0.0;
  }
  else {
    qn=-0.5;
    un = (3.0/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
  }

  y2[n-1]= (un-qn*u[n-2])/(qn-y2[n-2]+1.0);

  for (k=n-2;k>=0;k--){
    y2[k]= y2[k]*y2[k+1]+u[k];
  } 
}

/////////////////////////////////////////////////////////////////////////
//
// Time dependence jet correction function in the plugs
// (applies to Feb'02-Jun'02 data, runs 138425--145200)
// 
// t0 is Feb/04/02, run 138425
//
// Assessed on JET_20 data samples using dijet balance technique
// Derived from source data (Jun'02/Nov'01)
//
// Created 11/14/02 by Charles Currat, LBNL -- CACurrat@lbl.gov
//
/////////////////////////////////////////////////////////////////////////

double JetEnergyCorrections::runDepCorrFactor(double etaJet,double emfJet) {

#include "run2lum.hh"

  double tfac=-999.,rfac=0.;

  double lrco_flum = 1.;


  //run-to-integrated live luminosity correspondence

  if (_nrun>=FIRSTRUN && _nrun<=LASTRUN){
    tfac = run2lum[_nrun-FIRSTRUN][0];
    if (tfac <= 0.) {
      //      std::cout<< "### JET CORRECTION, time dependence: run " << _nrun << 
      //           " is not listed. Probably not a PHYSICS run!!!"<<std::endl;
    }
    else {
      rfac = emfJetCorrFactor(etaJet,emfJet);     //eta dependence
      lrco_flum= (1. + rfac*(tfac-10007.35)/lumtot);         //linear time dependence
      // std::cout << " nrun " <<_nrun << "   "<< rfac <<" "<< tfac;
      //  std::cout << " Lum Fraction " << lrco_flum<<" "<<tfac/lumtot<<" "<<lumtot<<std::endl;
    }
  }
  else {
   lrco_flum=1.;
  }

  return(lrco_flum); //multiplicative factor to apply to jet Et
}
/////////////////////////////////////////////////////////////////////////
//
double JetEnergyCorrections::emfJetCorrFactor(double etaJet,double emfJet) {

  double rfac=0.;
  double lrco_jeta_em=1.;
  double lrco_jeta_ha=1.;

  //Here's a fit to the source data

  double aetaJet = fabs(etaJet);

  if(aetaJet>1.2){
    if (etaJet<=-1.2){
      lrco_jeta_em = 1.081 -0.07011*aetaJet +0.006028*(aetaJet*aetaJet);
      lrco_jeta_ha = 1.017 -0.02057*aetaJet +0.003311*(aetaJet*aetaJet);
    } else if (etaJet>=1.2) {
      lrco_jeta_em = 1.072 -0.07360*aetaJet +0.008442*(aetaJet*aetaJet);
      lrco_jeta_ha = 1.092 -0.07484*aetaJet +0.003146*(aetaJet*aetaJet);
    }

    rfac = (emfJet*lrco_jeta_em+(1-emfJet)*lrco_jeta_ha)-1;
  }
  return(rfac);
}
 

void JetEnergyCorrections::energyScaleCorrections(float &pt,const float eta,float &emFraction){
  
  // overall factor (not implemented)
  if(fabs(eta)<CENTRAL_ETA){
    pt *= 1+emCorrectionFactor[_version]*emFraction;
    emFraction *= 1+emCorrectionFactor[_version];
  }
  
  if(_version == 2){
    
    // Implement CEM scale dependence after run 152400. We can apply this corrections
    // to plug jets because it goes after relative corrections
    if(_CemCorON) {
      if(_nrun > 152400 && _nrun < 159000){
 
      // From Larry's function:
        float cemFactor = 1.0/(1.014  - .0000022  * (_nrun-153600) );
        pt = ( (1 - emFraction) * pt ) + (emFraction * cemFactor * pt); // HAD + EM
      }
    }
      
    pt *= adhocfactorVersion2[_conesize];

    if(_sysScaleCorrection) {
      // Adhoc-factor
      pt *=1.0+_sysSign* adhocfactorErrVersion2;
    }

    if(_sysCentralCalStability) {
      // Central Calorimeter stability
      pt *=1.0+_sysSign*0.01;
    }

  } // version 2

  if(_version >= 3 && _version <= 4){
    
    pt *= adhocfactorVersion3[_conesize];

    if(_sysScaleCorrection) {
      // Adhoc-factor
      pt *=1.0+_sysSign* adhocfactorErrVersion3;
    }

    if(_sysCentralCalStability) {
      // Central Calorimeter stability
      pt *=1.0+_sysSign*0.01;
    }

  } // version 3-4


  if(_version >= 5 && _UseRun1CorForV5){
    
    if (_imode == 0){
    // MC
      if(_sysScaleCorrection) {
	double mcScaleError = 0.00;
	pt *= 1.0+_sysSign* mcScaleError;
      }
    } else {
    // Data

      pt *= adhocfactorVersion5[_conesize];

      if(_sysScaleCorrection) {
	// Adhoc-factor
	if (_sysSign>0){
	  pt *=1.0+_sysSign* adhocfactorErrVersion5;
	} else if (_sysSign<0){
	  pt *=1.0+_sysSign* adhocfactorErrVersion5;
	} 
      }

      if(_sysCentralCalStability) {
	// Central Calorimeter stability
	pt *=1.0+_sysSign*0.01;
      }

    }  // Data

  } // version 5-


  // For MC, have 5% scale uncertainty, but no adhoc factor applied
  if(_version == 0){

    if(_sysScaleCorrection) {
      // July 8 2003 (J.F. Arguin): change error on MC scale from 5% to 4%
      double mcScaleError = 0.04;
      pt *= 1.0+_sysSign* mcScaleError;
    }
  } // version 0

} // energyScaleCorrections


void JetEnergyCorrections::setSysRelativeCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysRelativeCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysRelativeCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysCentralCalStability(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysCentralCalStability=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysCentralCalStability=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysScaleCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysScaleCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysScaleCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysUnderlyingEventCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysUnderlyingEventCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysUnderlyingEventCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysMultipleInteractionCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysMultipleInteractionCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysMultipleInteractionCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysAbsoluteCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysAbsoluteCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysAbsoluteCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysOutOfConeCorrection(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysOutOfConeCorrection=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysOutOfConeCorrection=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setSysSplashOut(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysSplashOut=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysSplashOut=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setTotalSysUncertainties(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysTotalUncert=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysTotalUncert=false;
    _sysSign=0;
  }
}
void JetEnergyCorrections::setDataSysUncertainties(int i=0){
  if(i!=0) {
    setSysUncertaintiesOff();
    _sysDataUncert=true;
    (i<0) ? _sysSign=-1 : _sysSign=+1;
  }
  else {
    _sysDataUncert=false;
    _sysSign=0;
  }
}

void JetEnergyCorrections::setSysUncertaintiesOff(){

  _sysRelativeCorrection=false;
  _sysCentralCalStability=false;
  _sysScaleCorrection=false;
  _sysUnderlyingEventCorrection=false;
  _sysMultipleInteractionCorrection=false;
  _sysAbsoluteCorrection=false;
  _sysOutOfConeCorrection=false;
  _sysSplashOut=false;
  _sysTotalUncert=false;
  _sysDataUncert=false;
  _sysSign=0;

}

double JetEnergyCorrections::sysRelativeCor(double eta, double pt){

  double DETA_V2[5]={0.1, 0.8,  1.4, 2.0, 9999.};
  double DETA[6]={0.2, 0.6,  1.0, 1.4, 2.0, 9999.};
  double RELERR_V2[5];
  double RELERR[6];
  double DETA_V5[8]={0.0, 0.2, 0.6, 0.9, 1.4, 2.0, 2.6, 9999.};
  double PT_V5[6]={0.0, 12.0, 25.0, 55.0, 75.0, 9999.};
  double RELERR_V5[7][5];
  
  // Different syst. uncertainty for data and MC
  if(_version == 2){
    RELERR_V2[0]=0.02; RELERR_V2[1]=0.002; RELERR_V2[2]=0.04; RELERR_V2[3]=0.04; RELERR_V2[4]=0.07;
  }
  if(_version == 3 || _version == 4){
    RELERR[0]=0.03; RELERR[1]=0.005; RELERR[2]=0.02; RELERR[3]=0.04; RELERR[4]=0.02; RELERR[5]=0.07;
  }
  if(_version >= 5){
    //     0.0-0.2                0.2-0.6                0.6-0.9                0.9-1.4                1.4-2.0               2.0-2.6               2.6-
    //     "Minimal"
    if (_RelCorSysMinimal) {
    RELERR_V5[0][0]=0.015; RELERR_V5[1][0]=0.005; RELERR_V5[2][0]=0.015; RELERR_V5[3][0]=0.025; RELERR_V5[4][0]=0.015; RELERR_V5[5][0]=0.050; RELERR_V5[6][0]=0.075; //     PT<12 GeV
    RELERR_V5[0][1]=0.015; RELERR_V5[1][1]=0.005; RELERR_V5[2][1]=0.015; RELERR_V5[3][1]=0.015; RELERR_V5[4][1]=0.015; RELERR_V5[5][1]=0.030; RELERR_V5[6][1]=0.060; // 12<=PT<25 GeV
    RELERR_V5[0][2]=0.010; RELERR_V5[1][2]=0.005; RELERR_V5[2][2]=0.010; RELERR_V5[3][2]=0.010; RELERR_V5[4][2]=0.005; RELERR_V5[5][2]=0.015; RELERR_V5[6][2]=0.060; // 25<=PT<55 GeV
    RELERR_V5[0][3]=0.005; RELERR_V5[1][3]=0.005; RELERR_V5[2][3]=0.005; RELERR_V5[3][3]=0.005; RELERR_V5[4][3]=0.005; RELERR_V5[5][3]=0.015; RELERR_V5[6][3]=0.060; // 55<=PT<75 GeV
    RELERR_V5[0][4]=0.005; RELERR_V5[1][4]=0.005; RELERR_V5[2][4]=0.005; RELERR_V5[3][4]=0.005; RELERR_V5[4][4]=0.005; RELERR_V5[5][4]=0.015; RELERR_V5[6][4]=0.060; // 75<=PT    GeV
    //     "Maximal"
    } else {
    RELERR_V5[0][0]=0.030; RELERR_V5[1][0]=0.005; RELERR_V5[2][0]=0.025; RELERR_V5[3][0]=0.085; RELERR_V5[4][0]=0.095; RELERR_V5[5][0]=0.120; RELERR_V5[6][0]=0.130; //     PT<12 GeV
    RELERR_V5[0][1]=0.030; RELERR_V5[1][1]=0.005; RELERR_V5[2][1]=0.025; RELERR_V5[3][1]=0.080; RELERR_V5[4][1]=0.095; RELERR_V5[5][1]=0.110; RELERR_V5[6][1]=0.120; // 12<=PT<25 GeV
    RELERR_V5[0][2]=0.020; RELERR_V5[1][2]=0.005; RELERR_V5[2][2]=0.020; RELERR_V5[3][2]=0.030; RELERR_V5[4][2]=0.050; RELERR_V5[5][2]=0.070; RELERR_V5[6][2]=0.085; // 25<=PT<55 GeV
    RELERR_V5[0][3]=0.005; RELERR_V5[1][3]=0.005; RELERR_V5[2][3]=0.005; RELERR_V5[3][3]=0.010; RELERR_V5[4][3]=0.015; RELERR_V5[5][3]=0.030; RELERR_V5[6][3]=0.085; // 55<=PT<75 GeV
    RELERR_V5[0][4]=0.005; RELERR_V5[1][4]=0.005; RELERR_V5[2][4]=0.005; RELERR_V5[3][4]=0.010; RELERR_V5[4][4]=0.010; RELERR_V5[5][4]=0.020; RELERR_V5[6][4]=0.085; // 75<=PT    GeV
    }
  }
  if(_version == 0){
    RELERR[0]=0.01; RELERR[1]=0.01; RELERR[2]=0.01; RELERR[3]=0.07; RELERR[4]=0.06; RELERR[5]=0.07;
  }

  double err=0.0;

  if(_version == 2){
    err=0.02;
    for(int i=0;i<5;i++){
      if(fabs(eta)>DETA_V2[i] && fabs(eta)<=DETA_V2[i+1]) {
        err= RELERR_V2[i+1];
      }
    }
  }

  if(_version == 0 || _version == 3 || _version == 4){
    err = 0.03;
    for(int i=0;i<6;i++){
      if(fabs(eta)>DETA[i] && fabs(eta)<=DETA[i+1]) {
        err= RELERR[i+1];
      }
    }
  }

  if(_version == 5){
    for(int i=0;i<7;i++){
      for(int j=0;j<5;j++){
	if(fabs(eta)>DETA_V5[i] && fabs(eta)<=DETA_V5[i+1] && pt>=PT_V5[j] && pt<PT_V5[j+1]) {
	  err= RELERR_V5[i][j];
	}
      }
    }
  }

  return err;
}

// This function gives the Gen5 absolute correction (L5/L4) for a given
// consize and L4 jet pt.
float JetEnergyCorrections::absECorrGen5(int conesize, float pt) {
  return exp(p0Cal2Had[conesize]+p1Cal2Had[conesize]*pt) +
    exp(p2Cal2Had[conesize]+p3Cal2Had[conesize]*pt) +
    exp(p4Cal2Had[conesize]+p5Cal2Had[conesize]*pt) +
    (p6Cal2Had[conesize]+p7Cal2Had[conesize]*pt);
}

void JetEnergyCorrections::absoluteEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  
  if (_version<=4 || _UseRun1CorForV5) {
    int jpt=0;
    (pt < CROSSPOINT[conesize]) ? jpt = 0 : jpt =1;
    pt = QUAP0[jpt][conesize] + QUAP1[jpt][conesize]*pt +
      QUAP2[jpt][conesize]*pt*pt;
  } else {
    // version 5
    pt *= absECorrGen5(conesize,pt);
  }
  
  if (_sysAbsoluteCorrection) {
    if (_version<=4 || _UseRun1CorForV5) {
      // Declare Behrends curve parameters
      float ssob[3];
      if (_sysSign > 0) {
	ssob[0] = 0.28975E-01;
	ssob[1] = -0.81619E-04;
	ssob[2] = 0.18338E-06;
      }
      if (_sysSign < 0) {
	ssob[0] = -0.28182E-01;
	ssob[1] = 0.99435E-04;
	ssob[2] = -0.21639E-06;
      }
      
      float df = ssob[0] + ssob[1]*pt + ssob[2]*(pt*pt);
      
      // Multiply df by 1.05 to get similar uncertainties than Run I
      pt = (1.0 + (df*1.05))*pt;
    } else {
      // version 5
      pt = (1.0 + _sysSign*Cal2HadError(pt))*pt;
    } 
  } 
}

// This function returns the L4 jet pt that would produce the given L5
// jet pt under default corrections (i.e. no systematics).
void JetEnergyCorrections::invertAbsoluteEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  
  if (_version<=4 || _UseRun1CorForV5) {
    // Invert a quadratic equation. (See absoluteEnergyCorrections above.)
    int jpt=0;
    (pt < CROSSPOINT[conesize]) ? jpt = 0 : jpt =1;
    double a = QUAP2[jpt][conesize];
    double b = QUAP1[jpt][conesize];
    double c = QUAP0[jpt][conesize] - pt;
    // We're always in the regime where you add the sqrt part.
    pt = (-b + sqrt(pow(b,2)-4.*a*c))/2./a;
  } else {
    // OK, this is too complicated. Do it numerically.
    // The function of which we want zeros is F = f(ptl4)*ptl4-ptl5.
    // First bracket a solution.
    float ptlo = pt*.9, pthi = pt*1.1;
    float flo,fhi;
    for (int i = 0 ; i < 10 ; i++) {
      flo = absECorrGen5(conesize,ptlo)*ptlo - pt;
      fhi = absECorrGen5(conesize,pthi)*pthi - pt;
      if (flo*fhi < 0.) break;
      ptlo *= 0.9;
      pthi *= 1.1;
    }
    assert(flo*fhi < 0.);
    // Now bisect the interval until a solution is found within 1 MeV.
    float ptmid, fmid;
    for (int i = 0 ; i < 40 ; i++) {
      assert(flo*fhi < 0.);
      ptmid = (ptlo+pthi)/2.;
      fmid = absECorrGen5(conesize,ptmid)*ptmid - pt;
      if (fabs(pthi-ptlo) < .001 || fmid == 0.) break;
      if (flo*fmid < 0.) {
	pthi = ptmid;
	fhi = fmid;
      } else {
	ptlo = ptmid;
	flo = fmid;
      }
    }
    pt = ptmid;
  }
  return;
}

double JetEnergyCorrections::Cal2HadError(double pt) {

// This is fractional error and same for all cones 
// EOP errors were determine for each cone but found to be almost
// identical for three cones.

//    1  p0           1.01137e+00   1.11686e-05   3.97466e-07   1.11060e+04
//    2  p1          -2.07009e-02   7.73900e-04  -3.44721e-05   3.64041e+06
//    3  p2           2.09145e-02   7.73870e-04   3.44679e-05   3.60932e+06
//    4  p3           9.98575e-01   5.34147e-05   2.39274e-06   4.52197e+05
  
  if(pt<5.0)   pt=5.0;
  if(pt>500.0) pt=500.0;
  
  double P0=1.01137e+00; 
  double P1=-2.07009e-02; 
  double P2=2.09145e-02;  
  double P3=9.98575e-01; 
  
  // E/P error
  double EOP= P0+P1*pt+P2*pow(pt,P3)-1.0;; 
  
  // Fragmentation error 1% 
  double Frag(0.01);
  
  // Central calorimeter stability
  //double Stability(0.003);
  double Stability(0.005);  // Dec 29, 2005.
  
  // Phi crack uncertainty (data/test beam difference)
  double PhiCrack(0.005);
  
  // Add in quadrature
  double Error = sqrt(pow(EOP,2)+pow(Frag,2)+pow(Stability,2)+pow(PhiCrack,2));
  
  return Error;
  
}

void JetEnergyCorrections::outOfConeEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  
  if (_version<=4 || _UseRun1CorForV5){
    pt += OUT1[conesize]*(1.0-OUT2[conesize]*exp(-OUT3[conesize]*pt));
  } else {
    // version 5
    // Undo UE correction by the default value
    pt += UD1EVT[_version][conesize];
    // Apply OOC correction to the level5-corrected jet Pt
    pt = OOC1[conesize]+OOC2[conesize] * pt;
  }
  
  if(_sysOutOfConeCorrection) {
    
    if (_version<=4 || _UseRun1CorForV5){
      
      // Calculate OOC uncertainty (CDF-note 3253)
      float FitOOCParams[3] = {2.4666, -0.073, 1.4379};
      float OOCSYS = 0.01 * (exp(FitOOCParams[0] + FitOOCParams[1] * pt) + FitOOCParams[2]);
      
      pt *= (1.0+_sysSign*OOCSYS);
      
    } else {
      // version 5
      
      double OOCSYS;
      if (pt<crossOocSys) OOCSYS=scaleOocSys*exp(p0OocSys+p1OocSys*pt);
      else OOCSYS=scaleOocSys*p2OocSys;
      pt *= (1.0+_sysSign*OOCSYS);
      
    }
    
  }
  
  if (_version<=4 || _UseRun1CorForV5){
    if(_sysSplashOut){
      // plus splash-out corrections: 1 GeV uncertainty beyond R=1.0
      pt += _sysSign*1.0;
    }
  } else {
    // version 5
    if(_sysSplashOut){
      // plus splash-out corrections: 0.25 GeV uncertainty beyond R=1.3
      pt += _sysSign*0.25;
    }
  }

}

// This function returns the L6 jet pt that would produce the given L7
// jet pt under default corrections (i.e. no systematics).
void JetEnergyCorrections::invertOutOfConeEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  
  if (_version<=4 || _UseRun1CorForV5){
    // pt += OUT1[conesize]*(1.0-OUT2[conesize]*exp(-OUT3[conesize]*pt));
    // Don't do this inversion now since it's complicated (need numerical?)
    // and because it should go back to L6, not L5 as in version 5.
    assert(0);
  } else {
    pt = (pt - OOC1[conesize])/OOC2[conesize] - UD1EVT[_version][conesize];
  }
}

void JetEnergyCorrections::multipleInteractionEnergyCorrections(float &pt) {
  //double MISYS=0.30;
  double MISYS;
  if (_version>=5){
    MISYS=MISYSV5;
  } else {
    MISYS=MISYSV1;
  }

  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;

  if(_nvtx < 0){   // average correction
    pt -= UNDPT1[_version][conesize];
  }
  else if(_nvtx>1){
    if(!_sysMultipleInteractionCorrection) {
      pt -= (_nvtx-1)*UDEVT[_version][conesize];
    }
    else {
//   The sign of the MI correction uncertainty is changed so that +1sigma increases JES
//    pt -=(1.0+_sysSign*MISYS)*(_nvtx-1)*UDEVT[_version][conesize];
      pt -=(1.0-_sysSign*MISYS)*(_nvtx-1)*UDEVT[_version][conesize];
    } 
  }

}

void JetEnergyCorrections::invertMultipleInteractionEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  if(_nvtx < 0){   // average correction
    pt += UNDPT1[_version][conesize];
  }
  else if(_nvtx>1){
    pt += (_nvtx-1)*UDEVT[_version][conesize];
  }
}

void JetEnergyCorrections::underlyingEnergyCorrections(float &pt) {
  
  double UESYS=0.30;  //30\% uncertainty on the UE
  
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  
  if(!_sysUnderlyingEventCorrection){
    pt -= UD1EVT[_version][conesize];
  } else {
    //   The sign of the UE correction uncertainty is changed so that
    //   +1sigma increases JES
    //   pt -= (1.0+_sysSign*UESYS)*UD1EVT[_version][conesize];
    pt -= (1.0-_sysSign*UESYS)*UD1EVT[_version][conesize];
  }
}

void JetEnergyCorrections::invertUnderlyingEnergyCorrections(float &pt) {
  // For MidPoint jets,
  int conesize=_conesize;
  if (conesize>=3) conesize=_conesize-3;
  pt += UD1EVT[_version][conesize];
}

double JetEnergyCorrections::scaleRawMCtoRawDataUsingPhotonJet(double detectorEta) {
   
  // Return the ratio  Data-correction/MC-correction
  // PJKBRD[_conesize][ieta]  Correction factor from Photon-Jet  RAW data  
  // PJKBRM[_conesize][ieta]  Correction factor from Photon-Jet  RAW Monte Carlo

  int NP=47;
  if(detectorEta <=  PJDET[_conesize][0]) {
    return (PJKBRD[_conesize][0]/ PJKBRM[_conesize][0]);
  }
  else  if(detectorEta >=  PJDET[_conesize][NP-1]) {
    return (PJKBRD[_conesize][NP-1]/ PJKBRM[_conesize][NP-1]);
  }
  else {
    for(int ieta=0;ieta<NP-1;ieta++) {
      if(detectorEta >= PJDET[_conesize][ieta] && detectorEta <= PJDET[_conesize][ieta+1]){
        double x0= PJDET[_conesize][ieta]; 
        double y0= PJKBRD[_conesize][ieta]/PJKBRM[_conesize][ieta];

        double x1=  PJDET[_conesize][ieta+1]; 
        double y1= PJKBRD[_conesize][ieta+1]/ PJKBRM[_conesize][ieta+1];

	double slope=(y1-y0)/(x1-x0);
        return y0+slope*(detectorEta-x0);
      }
    }
  }
  return 1.0;
}
double JetEnergyCorrections::scaleRawMCtoRawDataUsingJetJet(double detectorEta){
   
  // Return the ratio  Data-correction/MC-correction using dijet data.

  int NP= NP00V2[_conesize];

  if(detectorEta <=  X00MC[_conesize][0]) {
    return (Y00V2[_conesize][0]/ Y00MC[_conesize][0]);
  }
  else  if(detectorEta >=  X00MC[_conesize][NP-1]) {
    return (Y00V2[_conesize][NP-1]/ Y00MC[_conesize][NP-1]);
  }
  else {
    for(int ieta=0;ieta<NP-1;ieta++) {
      if(detectorEta >= X00MC[_conesize][ieta] && detectorEta <= X00MC[_conesize][ieta+1]){
        double x0= X00MC[_conesize][ieta]; 
        double y0= Y00V2[_conesize][ieta]/Y00MC[_conesize][ieta];

        double x1= X00MC[_conesize][ieta+1]; 
        double y1= Y00V2[_conesize][ieta+1]/Y00MC[_conesize][ieta+1];

	double slope=(y1-y0)/(x1-x0);
        return y0+slope*(detectorEta-x0);
      }
    }
  }
  return 1.0;
}
// Return sum in quadrature of all individual jet systematic uncertainties: CURRENTLY USES
// VERSION 4 FOR DATA AND VERSION 0 FOR MONTE CARLO CORRECTIONS
float JetEnergyCorrections::totalJetSystematics(float inputRawPt, float& emFraction, const float JDetEta, int sysSign, int dataVersion, int mcVersion, bool dataOnly){

  // Declare relevant variables
  float sysF[6]={0,0,0,0,0,0};
  float scaleFactorMCInitLevel=1;
  float ptMCInitLevel= 0.0;
  float scaleFactorDataInitLevel=1;
  float ptDataInitLevel= 0.0;

  // Keep in memory parameters of JetEnergyCorrections object
  int initVersion = _version;
  int initmode = _imode;  
  int initLevel = _level;
  int initSyscode = _syscode;
  if(_version!=0) _version=mcVersion;
  if(_imode!=0) _imode=0;
  if(_level!=7) _level=7;
  if(_syscode!=0) _syscode=0;

  setSysUncertaintiesOff();
  // Get default MC scale
  _level = 7;
  float scaleFactorMCLevel7 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
  _level = initLevel;
  scaleFactorMCInitLevel = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
  ptMCInitLevel = scaleFactorMCInitLevel*inputRawPt;

  //
  // First, get MC uncertainties
  //

  // get error on relative corrections in MC
  if(initLevel >= 1){
    setSysRelativeCorrection(sysSign);
    sysF[0] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 1;
    setSysUncertaintiesOff();
    float scaleFactorMCLev1 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysRelativeCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev1)*inputRawPt;
    sysF[0] = delta_pt/ptMCInitLevel;
  }

  // get error on energy scale in MC
  if(initLevel >= 3){
    setSysScaleCorrection(sysSign);
    sysF[1] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 3;
    setSysUncertaintiesOff();
    float scaleFactorMCLev3 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysScaleCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev3)*inputRawPt;
    sysF[1] = delta_pt/ptMCInitLevel;
  }

  // get multiple interaction error in MC
  if(initLevel >= 4){
    setSysMultipleInteractionCorrection(sysSign);
    sysF[2] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  } 
  else{
    _level = 4;
    setSysUncertaintiesOff();
    float scaleFactorMCLev4 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysMultipleInteractionCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev4)*inputRawPt;
    sysF[2] = delta_pt/ptMCInitLevel;
  } 

  // get error on absolute corrections in MC (include underlying event uncertainties)
  if(initLevel >= 5){
    setSysAbsoluteCorrection(sysSign);
    sysF[3] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 5;
    setSysUncertaintiesOff();
    float scaleFactorMCLev5 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysAbsoluteCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev5)*inputRawPt;
    sysF[3] = delta_pt/ptMCInitLevel;
  }

  // get error on out of cone corrections in MC
  if(initLevel >= 7){
    setSysOutOfConeCorrection(sysSign);
    sysF[4] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 7;
    setSysUncertaintiesOff();
    float scaleFactorMCLev7 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysOutOfConeCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev7)*inputRawPt;
    sysF[4] = delta_pt/ptMCInitLevel;
  }

  // Uncertainties from splash-out
  if(initLevel >= 7){
    setSysSplashOut(sysSign);
    sysF[5] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 7;
    setSysUncertaintiesOff();
    float scaleFactorMCLev7 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysSplashOut(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev7)*inputRawPt;
    sysF[5] = delta_pt/ptMCInitLevel;
  }


  float jetSysMC=sqrt(sysF[0]*sysF[0]+sysF[1]*sysF[1]+sysF[2]*sysF[2]+sysF[3]*sysF[3]+sysF[4]*sysF[4]+sysF[5]*sysF[5]);


  //
  // Now, DATA uncertainties: apply data corrections to MC 
  // and find the difference given by each systematic error
  // 
  _version=dataVersion;
  _imode=1;

  // default data scale
  setSysUncertaintiesOff();
  _level = 7;
  float scaleFactorDataLevel7 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
  _level = initLevel;
  scaleFactorDataInitLevel = doEnergyCorrections(inputRawPt,emFraction,JDetEta);  
  ptDataInitLevel = scaleFactorDataInitLevel*inputRawPt;

  // get error on relative corrections in data
  
  if(initLevel >= 1){
    setSysRelativeCorrection(sysSign);
    sysF[0] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorDataInitLevel - 1.0;
  }  
  else{
    _level = 1;
    setSysUncertaintiesOff();
    float scaleFactorDataLev1 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysRelativeCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorDataLev1)*inputRawPt;
    sysF[0] = delta_pt/ptDataInitLevel;
  }

  // get error on central scale stability in data
  if(initLevel >= 2){
    setSysCentralCalStability(sysSign);
    sysF[1] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorDataInitLevel - 1.0;
  }
  else{
    _level = 2;
    setSysUncertaintiesOff();
    float scaleFactorDataLev2 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysCentralCalStability(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorDataLev2)*inputRawPt;
    sysF[1] = delta_pt/ptDataInitLevel;
  }
  
  // get error on energy scale in data
  if(initLevel >= 3){
    setSysScaleCorrection(sysSign);
    sysF[2] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorDataInitLevel - 1.0;
  } 
  else{
    _level = 3;
    setSysUncertaintiesOff();
    float scaleFactorDataLev3 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysScaleCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorDataLev3)*inputRawPt;
    sysF[2] = delta_pt/ptDataInitLevel;
  } 
  
  float jetSysData=sqrt(sysF[0]*sysF[0]+sysF[1]*sysF[1]+sysF[2]*sysF[2]);
  
  // Add in quadrature data + MC uncertainties
  float totalJetSyst = sqrt(jetSysData*jetSysData+jetSysMC*jetSysMC);
  if(dataOnly)
    totalJetSyst = sqrt(jetSysData*jetSysData);
  
  // Cleanup: reset version and level to original
  setTotalSysUncertainties(sysSign);
  if(dataOnly)
    setDataSysUncertainties(sysSign);
  _version = initVersion;
  _imode = initmode;
  _level = initLevel;
  _syscode = initSyscode;

  return totalJetSyst;
}

// Return sum in quadrature of all individual jet systematic uncertainties: for VERSION 5
float JetEnergyCorrections::totalJetSystematicsV5(float inputRawPt, float& emFraction, const float JDetEta, int sysSign){

  // Declare relevant variables
  float sysF[6]={0,0,0,0,0,0};
  float scaleFactorMCInitLevel=1;
  float ptMCInitLevel= 0.0;
  //float scaleFactorDataInitLevel=1;
  //float ptDataInitLevel= 0.0;

  // This works only for version 5 and only when _UseRun1CorForV5 is false
  if (_version != 5 || _UseRun1CorForV5){
    std::cout << "This works only for version 5 and only when _UseRun1CorForV5 is false." << std::endl;
    return 0.0;
  }

  // Keep in memory parameters of JetEnergyCorrections object
  int initmode    = _imode;  
  int initLevel   = _level;
  int initSyscode = _syscode;
  if(_imode!=0)     _imode=0;
  if(_syscode!=0) _syscode=0;
//if(_level!=7)     _level=7;

  setSysUncertaintiesOff();
  // Get default MC scale
  // _level = 7;
  // float scaleFactorMCLevel7 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
  _level = initLevel;
  scaleFactorMCInitLevel = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
  ptMCInitLevel = scaleFactorMCInitLevel*inputRawPt;

  //
  // First, get uncertainties for each level
  //

  // get error on relative corrections
  if(initLevel >= 1){
    setSysRelativeCorrection(sysSign);
    sysF[0] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;
  }
  else{
    _level = 1;
    setSysUncertaintiesOff();
    float scaleFactorMCLev1 = doEnergyCorrections(inputRawPt,emFraction,JDetEta);
    setSysRelativeCorrection(sysSign);
    float delta_pt = (doEnergyCorrections(inputRawPt,emFraction,JDetEta) - scaleFactorMCLev1)*inputRawPt;
    sysF[0] = delta_pt/ptMCInitLevel;
  }

  // get multiple interaction error
  //
  // Starting at this level, corrections inversion is possible:
  // we don't need to put the level higher
  //
  setSysMultipleInteractionCorrection(sysSign);
  sysF[1] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;

  // get error on absolute corrections
  setSysAbsoluteCorrection(sysSign);
  sysF[2] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;

  // get error on underlying event corrections
  setSysUnderlyingEventCorrection(sysSign);
  sysF[3] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;

  // get error on out of cone corrections
  setSysOutOfConeCorrection(sysSign);
  sysF[4] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;

  // get error on out of cone corrections "due to splash-out effect"
  setSysSplashOut(sysSign);
  sysF[5] = doEnergyCorrections(inputRawPt,emFraction,JDetEta)/scaleFactorMCInitLevel - 1.0;

  float totalJetSyst=sqrt(sysF[0]*sysF[0]+
			  sysF[1]*sysF[1]+
			  sysF[2]*sysF[2]+
			  sysF[3]*sysF[3]+
			  sysF[4]*sysF[4]+
			  sysF[5]*sysF[5]);

  // Cleanup: reset level to original
  setTotalSysUncertainties(sysSign);
  _imode = initmode;
  _level = initLevel;
  _syscode = initSyscode;

  return totalJetSyst;
}
