#ifndef _JET_CORR_SYSTEMATICS_HH_
#define _JET_CORR_SYSTEMATICS_HH_
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Component: JetCorrSystematics.hh
// Purpose: calculate Jet corrections pt variations when varying one of the jet corrections by
//          one sigma. Translated from a Run I function by Lina Galtieri
//          
// Created: Jan 2, 2003:  Jean-Francois Arguin
// History: 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>
#include <iostream>
 
#ifndef __NO_CDFSOFT__
#ifndef __CINT__
#include "CLHEP/Vector/LorentzVector.h"
#else
class HepLorentzVector;
#endif
#endif //__NO_CDFSOFT__

//
// 1 sigma syst. uncertainties: Run I values documented in CDF-note 3983
//
// -- calor stability 
//
const float uncStability = 0.01;
//
// -- Multiple vertices correction (GeV)
//
const float uncMultVert = 0.1;
//
// -- Relative corrections in 5 eta bins: 
// 0-0.1, 0.1-1.0, 1.0-1.4, 1.4-2.2, 2.2-2.6
//
const float uncRelCorrEta[5] = {0.02, 0.002, 0.04, 0.002, 0.04};
//
// -- Fit parameter from "Behrends curve": 
// Parametrize absolute corr. uncert. and underlying event uncert. as
// a function of pt of raw jet
//
const float ssob1[2] = {0.28975E-01, -0.28182E-01};
const float ssob2[2] = {-0.81619E-04, 0.99435E-04};
const float ssob3[2] = {0.18338E-06, -0.21639E-06};
//
// -- Fit from soft gluon rad. (OOC) (%)
//
const float fitOocPar1 = 2.4666;
const float fitOocPar2 = -0.0736;
const float fitOocPar3 = 1.4379; 
//
// -- splash-out beyond R = 1.0 (GeV)
//
const float uncBeyond = 1.0;
//

#ifdef __NO_CDFSOFT__
void calcJetCorrSystematics(float ptJet,float emFraction,const float detectorEta, float dPtRaw[12], float dPtCorr[12], int coneSize, int nVertices, int runNumber);
#else
void calcJetCorrSystematics(HepLorentzVector fourVector,float emFraction,const float detectorEta, float dPtRaw[12], float dPtCorr[12], int coneSize, int nVertices, int runNumber);
#endif

#endif
