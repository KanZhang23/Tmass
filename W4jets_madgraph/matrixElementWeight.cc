// Uncomment the following line to dump the kinematics and the calculated
// matrix element to the standard output
// #define ME_KINEM_DUMP

#include <cmath>

#ifdef ME_KINEM_DUMP
#include <iostream>
#endif

#include "matrixElementWeight.hh"
#include "W4JetsIntegrationMask.hh"
#include "w4jetsKinematics.hh"
#include "madgraph.h"

#include "npstat/nm/LinearMapper1d.hh"

using namespace geom3;
using namespace rk;
using namespace Pythia8;

double matrixElementWeight(Pythia * pythiaIn,
                           Parameters_sm_no_b_mass * parsIn,
                           const double eBeam, const double nominalWmass,
                           const rk::P4 partons[4],
                           const rk::P4& l, const int leptonCharge,
                           const rk::P4& nu) 
{
    // Calculate x1, x2. We will neglect the transverse momentum
    // of the system in this calculation.
    const rk::P4& w = l + nu;
    rk::P4 total(w);
    for (unsigned i=0; i<4; ++i)
        total += partons[i];
    const double invMass = total.m();
    const double pz = total.pz();
    const double eTotal = sqrt(invMass*invMass + pz*pz);
    const double x1 = 0.5*(eTotal + pz)/eBeam;
    const double x2 = 0.5*(eTotal - pz)/eBeam;

    assert(x1 > 0.0);
    assert(x2 > 0.0);
    if (x1 >= 1.0 || x2 >= 1.0)
        return 0.0;

    // Fill out the 4-momenta for Madgraph. The indices are
    //  0 - e
    //  1 - px
    //  2 - py
    //  3 - pz
    double PLAB[8][4];
    PLAB[0][1] = 0.0;
    PLAB[0][2] = 0.0;
    PLAB[0][3] = eBeam*x1;
    PLAB[0][0] = eBeam*x1;

    PLAB[1][1] = 0.0;
    PLAB[1][2] = 0.0;
    PLAB[1][3] = -eBeam*x2;
    PLAB[1][0] = eBeam*x2;

    // Boost all final state particles into the system with total pT = 0
    Vector3 velocity(total.velocity());
    velocity.set(geom3::Z, 0.0);
    const Boost tboost(velocity);

    for (int K=1; K<=4; ++K)
    {
        const rk::P4& pboosted = tboost*partons[K-1];
        PLAB[3+K][1] = pboosted.px();
        PLAB[3+K][2] = pboosted.py();
        PLAB[3+K][3] = pboosted.pz();
        PLAB[3+K][0] = pboosted.e();
    }

    // Lepton and anti-lepton
    const rk::P4& lep0 = leptonCharge < 0 ? l : nu;
    const rk::P4& lep = tboost*lep0;
    PLAB[2][1] = lep.px();
    PLAB[2][2] = lep.py();
    PLAB[2][3] = lep.pz();
    PLAB[2][0] = lep.e();

    const rk::P4& antilep0 = leptonCharge < 0 ? nu : l;
    const rk::P4& antilep = tboost*antilep0;
    PLAB[3][1] = antilep.px();
    PLAB[3][2] = antilep.py();
    PLAB[3][3] = antilep.pz();
    PLAB[3][0] = antilep.e();

    // Include the flux factor into the matrix element weight.
    // Note that the real flux is 1/8th of what is used here,
    // but we are not keeping up with the constant factors anyway...
    const double flux = 1.0/(x1*x2*eBeam*eBeam);

    //    const double mew = sfmew_(&flux);
    //    This should be global
    to_collider_.lpp[0] = 1;
    to_collider_.lpp[1] = 1;
    to_collider_.xbk[0] = x1;
    to_collider_.xbk[1] = x2;
    to_collider_.q2fact[0] = nominalWmass*nominalWmass;
    to_collider_.q2fact[1] = nominalWmass*nominalWmass;
    // Set the parameters which change event by event
    ParticleData * particleDataPtr = &(pythiaIn->particleData);
    SusyLesHouches * slhaPtr = &(pythiaIn->slhaInterface.slha);
    Couplings * cplPtr = pythiaIn->couplingsPtr;
    double alpS = pythiaIn->couplingsPtr->alphaS(nominalWmass*nominalWmass);
    parsIn->setDependentParameters(particleDataPtr, cplPtr, slhaPtr, alpS); 
    parsIn->setDependentCouplings();  
//    parsIn->printDependentParameters();
//    parsIn->printDependentCouplings();  
    strong_.g = parsIn->G;
    couplings_.gc_10 = parsIn->GC_10;
    couplings_.gc_11 = parsIn->GC_11;
    couplings_.gc_12 = parsIn->GC_12;

    double mew = 0.0, one = 1.0;
    if (leptonCharge > 0 ) {
       mew = p0_dsig_(PLAB, &one);
    } else {
       mew = p1_dsig_(PLAB, &one);
    }
    mew *= flux;

#ifdef ME_KINEM_DUMP
    {
        static unsigned callcount = 0;
        static const char* pname[8] = {
            "incoming 1",
            "incoming 2",
            "lepton",
            "antilepton",
            "parton 0",
            "parton 1",
            "parton 2",
            "parton 3"
        };

        P4 balance;
        std::cout.precision(10);
        std::cout << "\n==ME_KINEM " << callcount << std::endl;
        for (int i=1; i<=8; ++i)
        {
            P4 p4(PLAB[i-1][0], Vector3(PLAB[i-1][1], PLAB[i-1][2], PLAB[i-1][3]));
            balance += p4*(i <= 2 ? -1.0 : 1.0);

            std::cout << setw(10) << pname[i-1] << " :";
            for (int k=1; k<=4; ++k)
                std::cout << ' ' << PLAB[i-1][k-1];
            std::cout << std::endl;
        }
        std::cout << setw(10) << "P4 balance" << " : ";
        std::cout << balance.e() << ' ' << balance.px() << ' '
                  << balance.py() << ' ' << balance.pz() << std::endl;

        std::cout << "==ME_VALUE " << callcount++ << ' ' << mew << std::endl;
    }
#endif

    return mew;
}

double weightForKnownWMass(Pythia * pythiaIn,
                           Parameters_sm_no_b_mass * parsIn,
                           const unsigned mode, const double eBeam,
                           const double nominalWmass,
                           bool integrateOverNuPz,
                           const rk::P4 partons[4],
                           const geom3::Vector3& neutrinoPt,
                           const rk::P4& lepton, const int leptonCharge,
                           const double mwsq, const double wSamplingDensity)
{
    static const double oneOverSqrt3 = 1.0/sqrt(3.0);
    const bool calculateJacobians = mode & I_JACOBIAN_MASK;

    if (!calculateJacobians)
        integrateOverNuPz = false;

    // Solve the kinematics for nu Pz. Note that this function
    // should not be called at all when there are no solutions.
    double nuPz[2];
    const unsigned nSolutions = determineNuPz(neutrinoPt, lepton.momentum(),
                                              mwsq, nuPz);
    assert(nSolutions == 2);
    if (nuPz[0] > nuPz[1])
        std::swap(nuPz[0], nuPz[1]);
    const double nuPzSpan = nuPz[1] - nuPz[0];

    if (integrateOverNuPz)
    {
        // Choose good points on the nuPz interval. We will use points
        // which correspond to the 2-point Gauss-Legendre quadrature.
        const npstat::LinearMapper1d m(-1.0, nuPz[0], 1.0, nuPz[1]);
        nuPz[0] = m(-oneOverSqrt3);
        nuPz[1] = m(oneOverSqrt3);
    }

    // Cycle over the kinematic solutions and calculate the matrix
    // element squared, including parton distribution functions and other
    // kinematic terms.
    double matrel[2];
    double wJacobian[2] = {1.0, 1.0};
    for (unsigned iz=0; iz<2; ++iz)
    {
        // Create the neutrino 4-vector. It looks like the only remaining
        // weight unaccounted for by the phase space integration procedure
        // is the neutrino energy in the denominator. Take it into account here.
        const P4 nu(Vector3(neutrinoPt.x(), neutrinoPt.y(), nuPz[iz]), 0.0);
        matrel[iz] = matrixElementWeight(pythiaIn, parsIn, eBeam,
                                         nominalWmass, partons,
                                         lepton, leptonCharge, nu)/nu.e();
        assert(matrel[iz] >= 0.0);

        // The Jacobian here is just d mw^2/d nuPz. For the formula
        // see, for example, Appendix C in the CDF Note 8173.
        if (calculateJacobians)
            wJacobian[iz] = 2.0*(lepton.e()/nu.e()*nuPz[iz] - lepton.pz());
    }

    double w;
    if (integrateOverNuPz)
        // Average the kinematic solutions over the neutrino Pz interval
        w = (matrel[0] + matrel[1])/2.0*nuPzSpan;
    else
    {
        // Simply sum the neutrino Pz solutions with the corresponding
        // W mass jacobians. We also need to divide by the propagator as
        // it was already multiplied by in the matrix element weight.
        assert(wJacobian[0] && wJacobian[1]);
        w = (matrel[0]/fabs(wJacobian[0]) + matrel[1]/fabs(wJacobian[1]))/
            wSamplingDensity;
    }

    return w;
}
