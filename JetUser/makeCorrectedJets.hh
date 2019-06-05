#ifndef _MAKE_CORRECTED_JETS_HH_
#define _MAKE_CORRECTED_JETS_HH_
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Component: makeCorrectedJets.hh
// Purpose: Make corrected jets using JetEnergyCorrections. Plan to use it as the jet correction
//               framework for Run II.
//          
// Created: 12/13/01  Anwar Ahmad Bhatti, Jean-Francois Arguin
// History: 
//           August 6, 2002: J.-F. Arguin - Added function to correct jets from CdfJetView
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iosfwd>
#include <iostream>
#include "ErrorLogger_i/gERRLOG.hh"
#include "Edm/GenericConstHandle.hh"
#include "JetObjects/CdfJetColl.hh"
#include "JetObjects/CdfJetView.hh"

void correctedJetList(JetList &jets, int level, int nvtx, int conesize, int version, int syscode, bool order = false, int nrun = 0, int imode = 1);

void correctedJetList(CdfJetView::collection_type& jets, int level, int nvtx, int conesize, int version, int syscode, bool order = false, int nrun = 0, int imode = 1);

#endif 
