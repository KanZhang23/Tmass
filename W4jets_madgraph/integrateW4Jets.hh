#ifndef INTEGRATEW4JETS_HH_
#define INTEGRATEW4JETS_HH_

#include "npstat/stat/AbsDistribution1D.hh"
#include "npstat/stat/AbsDistributionND.hh"
#include "npstat/stat/LocalPolyFilter1D.hh"
#include "npstat/stat/HistoAxis.hh"
#include "npstat/stat/InMemoryNtuple.hh"

#include "JetPtTF.hh"
#include "JetPtEff.hh"
#include "BinnedJetPtFcn.hh"
#include "JesIntegResult.hh"
#include "W4JetsIntegrationMask.hh"
#include "Parameters_sm_no_b_mass.h"
#include "Pythia8/Pythia.h"

// This is the main matrix element integration procedure for the
// W + 4 jets channel. The arguments are as follows:
//
// mode                  -- Integration mode. This is a mask used
//                          according to the W4JetsIntegrationMask enum.
//
// reportInterval        -- Print a time-stamped message to the standard
//                          output every "reportInterval" QMC points.
//                          Value of 0 suppresses the printouts.
//
// allowTau              -- Include W -> tau nu channel, with subsequent
//                          tau -> l nu nu decay.
//
// allowDirectDecay      -- Include W -> l nu channel, with l = e or mu.
//                          Note that at least one of the "allowTau" and
//                          "allowDirectDecay" arguments must be "true".
//
// eBeam                 -- Proton beam energy (assumed to be the same for
//                          antiproton).
//
// nominalWmass          -- Location of the W mass peak (around 80.4 --
//                          check with tables).
//
// nominalWwidth         -- Width of the W (around 2.1 -- check with tables).
//
// maxWmassForScan       -- Maximum value of the W mass to use in the
//                          integration procedure. Should probably be
//                          around M top.
//
// nWmassScanPoints      -- Number of points to use in scanning the W
//                          propagator (which is used as the importance
//                          sampling function).
//
// jets                  -- Jets measured in the detector.
//
// lepton                -- Lepton momentum measured in the detector.
//                          The mass should be set to 0.
//
// leptonCharge          -- +1 or -1 for positively or negatively charged
//                          leptons, respectively.
//
// isMuon                -- "true" in case the measured lepton was identified
//                          as a muon, "false" for electron.
//
// centralValueOfTotalPt -- Average transverse momentum of the W + 4 jets
//                          system. It is not clear if it would be best
//                          to calculate this from the observed recoil
//                          or just set it to 0.
//
// minPartonPtFactor     -- Minimum fraction of parton/jet momentum to scan
//                          when the importance sampling functions are
//                          created for the parton momenta. This argument
//                          should be about 1/3 or 1/2.
//
// tfs                   -- Jet transfer functions. The code assumes that
//                          all of these pointers point to distinct objects
//                          so that jet momenta have to be set only once
//                          for each object.
//
// effs                  -- Jet efficiencies. Must be distinct objects, just
//                          as the transfer functions.
//
// minDeltaJes           -- These parameters specify how to scan the
// maxDeltaJes              delta JES values. The scan will be performed
// nDeltaJes                as if these parameters specify a histogram axis.
//
// partonPtPrior         -- Parton Pt prior to include in the generation
//                          of parton Pt importance samplers for each jet.
//
// jetPriorFilter        -- Filter for the parton Pt importance sampler.
//
// jetPriorSamplingGrid  -- Grid for creating the parton Pt importance sampler
//                          (in the log(partonPt) space).
//
// tauTransferFunction   -- Probability density of y = p_lepton/p_tau in
//                          the decay of energetic tau.
//
// systemPtPrior         -- Probability density of Pt for the whole W + 4
//                          jets system. The code will generate random Pt
//                          according to this density and will add
//                          "centralValueOfTotalPt" in order to determine
//                          W + 4 jets Pt for each phase space point.
//
// randomNumbers         -- Collection of pseudo- or quasi- random numbers
//                          to use in the integration procedure. Each ntuple
//                          column will be mapped to the corresponding
//                          integration variable according to the
//                          "RandomNumberSequence" enum defined in the
//                          header file "RandomNumberSequence.hh".
//
// convergenceChecker    -- An instance of AbsConvergenceChecker. The "check"
//                          method will be called each time 2^n points
//                          are sampled (see the next argument for the first
//                          value of n checked). The function will exit in
//                          case the "check" method returns anything other
//                          than CS_CONTINUE.
//
// firstPowerOfTwoToStoreInHistory -- The code will accumulate the history
//                          of integral values calculated for 2^n phase space
//                          integration points. This argument represents the
//                          first value of n for which the history should
//                          be stored. For example, if this argument is 8,
//                          the history will be stored for 256, 512, 1024, ...
//                          phase space points sampled. Of course, the
//                          final result will be stored as well, no matter
//                          what the integration termination reason is.
//
// result                -- Upon exit, will contain the history of integral
//                          values, wall clock time used in seconds, the
//                          reason for terminating the integration cycle, etc.
//
// functionValues        -- The actual function values calculated for each
//                          quasi-MC (or pseudo-MC) point. If this ntuple
//                          is provided, the number of its columns must be
//                          equal "nDeltaJes".
//
// saveJetPriors         -- If "true", Histo-Scope histograms will be generated
//                          representing parton Pt importance samplers for each
//                          jet. This setting is intended for testing purposes
//                          only.
//
// Note that one can try different combinations of "allowTau" and
// "allowDirectDecay" (the only forbidden combination is when both of these
// arguments are "false"). It is not obvious a priori whether combining
// W -> tau nu and W -> l nu (l = e or mu) in the same call would be a good
// idea because one expects significantly faster convergence of the
// integration procedure for W -> l nu. If this is, indeed, the case then
// running W -> l nu integration until W -> tau nu also converges would be
// a waste of time.
//
void integrateW4Jets_m(Pythia8::Pythia* pythiaIn,
                     Pythia8::Parameters_sm_no_b_mass* parsIn,
                     unsigned mode, unsigned reportInterval, bool allowTau,
                     bool allowDirectDecay, double eBeam,
                     double nominalWmass, double nominalWwidth,
                     double maxWmassForScan, unsigned nWmassScanPoints,
                     const JetInfo jets[4], rk::P4 lepton,
                     int leptonCharge, bool isMuon,
                     geom3::Vector3 centralValueOfTotalPt,
                     double minPartonPtFactor,
                     BinnedJetPtFcn<JetPtTF>* tfs[4],
                     BinnedJetPtFcn<JetPtEff>* effs[4],
                     double minDeltaJes, double maxDeltaJes,
                     unsigned nDeltaJes,
                     const npstat::AbsDistribution1D& partonPtPrior,
                     const npstat::LocalPolyFilter1D& jetPriorFilter,
                     const npstat::HistoAxis& jetPriorSamplingGrid,
                     const npstat::AbsDistribution1D& tauTransferFunction,
                     const npstat::AbsDistributionND& systemPtPrior,
                     const npstat::InMemoryNtuple<double>& randomNumbers,
                     AbsConvergenceChecker& convergenceChecker,
                     unsigned firstPowerOfTwoToStoreInHistory,
                     JesIntegResult* result,
                     npstat::InMemoryNtuple<double>* functionValues = 0,
                     bool saveJetPriors = false);

#endif // INTEGRATEW4JETS_HH_
