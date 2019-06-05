///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Component: JetCorrSystematics.cc
// Purpose: calculate Jet corrections pt uncertainty when varying one of the jet corrections by
//          one sigma. Translated from a Run I function by Lina Galtieri. Only for JetClu jets
//          for now. Originally written with Run I systematics.
//
//          Fills an array of 12 float containing the dPtCorr variation for a +-1 sigma variation
//          from each one of 6 uncertainties:
//
//            + 1 sigma 1.) cal.stab.,  2.) UE from Multiv, 3.) f_rel,
//                      4.) f_abs    ,  5.) softglu,        6.) Beyond R=1
//            - 1 sigma 7.) cal.stab.,  8.) UE from Multiv, 9.) f_rel,
//                     10.) f_abs    , 12.) softglu,       12.) Beyond R=1
//
// Created: Jan 2, 2003:  Jean-Francois Arguin
//
// Revisions:
// ----------
//   Dec. 28,2005   K.Hatakeyama, change coneSize from float to int (pointed out by Yen-Chu Chen)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


#include "JetCorrSystematics.hh"
#include "JetEnergyCorrections.hh"

#ifdef __NO_CDFSOFT__
void calcJetCorrSystematics(float ptJet,float emFraction,const float detectorEta, float dPtRaw[12], float dPtCorr[12], int coneSize, int nVertices, int runNumber){
#else
void calcJetCorrSystematics(HepLorentzVector fourVector,float emFraction,const float detectorEta, float dPtRaw[12], float dPtCorr[12], int coneSize, int nVertices, int runNumber){
#endif

#ifndef __NO_CDFSOFT__
  float ptJet = fourVector.perp();
#endif

  int version = 1; // data
  int syscode = 0;


  /////////////////////////////////////////////////////////////
  //
  // Get correction factor for different level of corrections
  //
  /////////////////////////////////////////////////////////////


  // Only relative corrections
  int relCorrLevel = 3;
  JetEnergyCorrections relativeCorrections=JetEnergyCorrections("JetCorrections","JetCorrections", relCorrLevel, nVertices, coneSize, version, syscode, runNumber);
#ifdef __NO_CDFSOFT__
  float f_rel = relativeCorrections.doEnergyCorrections(ptJet, emFraction, detectorEta);
#else
  float f_rel = relativeCorrections.doEnergyCorrections(fourVector, emFraction, detectorEta);
#endif

  // absolute correction factor (but we have to apply previous corrections to do that properly
  // That means applying all corrections up to mult. interactions, get the pt there, then 
  // apply absolute corrections from this pt)
  int multVertLevel = 4;
  JetEnergyCorrections multVertCorrections=JetEnergyCorrections("JetCorrections","JetCorrections", multVertLevel, nVertices, coneSize, version, syscode, runNumber);
#ifdef __NO_CDFSOFT__
  float f_multvert = multVertCorrections.doEnergyCorrections(ptJet, emFraction, detectorEta);
  float inPt = f_multvert * ptJet;
#else
  float f_multvert = multVertCorrections.doEnergyCorrections(fourVector, emFraction, detectorEta);
  float inPt = (f_multvert * fourVector).perp();
#endif
  float outPt = inPt;
  multVertCorrections.absoluteEnergyCorrections(outPt);
  float f_abs = outPt/inPt;

  // all corrections except out-of-cone corrections
  int coneCorrLevel = 6;
  JetEnergyCorrections coneCorrections=JetEnergyCorrections("JetCorrections","JetCorrections", coneCorrLevel, nVertices, coneSize, version, syscode, runNumber);
#ifdef __NO_CDFSOFT__
  float f_cone = coneCorrections.doEnergyCorrections(ptJet, emFraction, detectorEta);
#else
  float f_cone = coneCorrections.doEnergyCorrections(fourVector, emFraction, detectorEta);
#endif

  // All corrections
  int allCorrLevel = 7;
  JetEnergyCorrections allCorrections=JetEnergyCorrections("JetCorrections","JetCorrections", allCorrLevel, nVertices, coneSize, version, syscode, runNumber);
#ifdef __NO_CDFSOFT__
  float f_tot = allCorrections.doEnergyCorrections(ptJet, emFraction, detectorEta);
#else
  float f_tot = allCorrections.doEnergyCorrections(fourVector, emFraction, detectorEta);
#endif

  // rawet corrected for single out-of-cone correction
  //float rawPtOocc = ptJet * (f_tot - f_cone);
  

  /////////////////////////////////////////////////////////////////
  //
  // Get Pt variation from 1 sigma uncertainty of each correction
  //

  /////////////////////////////////
  //
  // 1) Calor. stability
  //
  /////////////////////////////////

  dPtRaw[0] = ptJet * uncStability;
  dPtRaw[6] = -dPtRaw[0];
  dPtCorr[0] = ptJet * uncStability * f_rel * f_abs;
  dPtCorr[6] = -dPtCorr[0];
  
  /////////////////////////////////
  //
  // 2) Multiple interactions
  //
  /////////////////////////////////

  dPtRaw[1] = 0.;
  dPtRaw[7] = 0.;
  int nAddVertices = nVertices - 1;
  dPtCorr[1] = f_abs * nAddVertices * uncMultVert;
  dPtCorr[7] = -dPtCorr[1];

  /////////////////////////////////
  //
  // 3) Relative corrections
  //
  /////////////////////////////////

  dPtRaw[2] = 0.;
  dPtRaw[8] = 0.;
  
  // uncertainty is function of eta
  float uncertRelCorr=0.f;
  if( fabs(detectorEta) < 0.1) uncertRelCorr = uncRelCorrEta[0];
  if( fabs(detectorEta) >= 0.1 && fabs(detectorEta) < 1.0 ) uncertRelCorr = uncRelCorrEta[1];
  if( fabs(detectorEta) >= 1.0 && fabs(detectorEta) < 1.4 ) uncertRelCorr = uncRelCorrEta[2];
  if( fabs(detectorEta) >= 1.4 && fabs(detectorEta) < 2.2 ) uncertRelCorr = uncRelCorrEta[3];
  if( fabs(detectorEta) >= 2.2 ) uncertRelCorr = uncRelCorrEta[4];

  dPtCorr[2] = ptJet * f_abs * f_rel * uncertRelCorr;
  dPtCorr[8] = -dPtCorr[2];

  ///////////////////////////////////////////////
  //
  // 4) Absolute + underlying event corrections
  //
  ///////////////////////////////////////////////

  float coneCorrEt = ptJet * f_cone;
  // Use S. Behrends' curve to get uncertainty: + 1 sigma
  float df_plus = ssob1[0] + ssob2[0]*coneCorrEt + ssob3[0]*(coneCorrEt*coneCorrEt);

  dPtRaw[3] = ptJet * f_cone * df_plus;
#ifdef __NO_CDFSOFT__
  float ptRaw_plus = ptJet * (1 + dPtRaw[3]/ptJet);
  float f_cone_plus = coneCorrections.doEnergyCorrections(ptRaw_plus, emFraction, detectorEta);
#else
  HepLorentzVector fourVecRaw_plus = fourVector * (1 + dPtRaw[3]/ptJet);
  float f_cone_plus = coneCorrections.doEnergyCorrections(fourVecRaw_plus, emFraction, detectorEta);
#endif
  dPtCorr[3] = ptJet * (f_cone_plus - f_cone);

  // Use S. Behrends' curve to get uncertainty: -1 sigma
  float df_minus = ssob1[1] + ssob2[1]*coneCorrEt + ssob3[1]*(coneCorrEt*coneCorrEt);

  dPtRaw[9] = ptJet * f_cone * df_minus;
#ifdef __NO_CDFSOFT__
  float ptRaw_minus = ptJet * (1 + dPtRaw[9]/ptJet);
  float f_cone_minus = coneCorrections.doEnergyCorrections(ptRaw_minus, emFraction, detectorEta);
#else
  HepLorentzVector fourVecRaw_minus = fourVector * (1 + dPtRaw[9]/ptJet);
  float f_cone_minus = coneCorrections.doEnergyCorrections(fourVecRaw_minus, emFraction, detectorEta);
#endif
  dPtCorr[9] = ptJet * (f_cone_minus - f_cone);


  ///////////////////////////////////////////////
  //
  // 5) Out-of-cone corrections
  //    The uncertainty on this correction is dependent on pt
  //    and has been fit to an exponential plus constant.
  //    See CDF-note 3253.
  //
  ///////////////////////////////////////////////

  dPtRaw[4] = 0.;
  dPtRaw[10] = 0.;
  
  float ptTot = ptJet * f_tot;
  float dSoft = 0.01 * (exp(fitOocPar1 + fitOocPar2 * ptTot) + fitOocPar3);
  
  dPtCorr[4] = ptTot * dSoft;
  dPtCorr[10] = -dPtCorr[4];
  
  ////////////////////
  //
  // 5) Splash out
  //
  ////////////////////
  
  dPtRaw[5] = 0.;
  dPtRaw[11] = 0.;
  dPtCorr[5] = 1.0;
  dPtCorr[11] = -1.0;

  return;
}
