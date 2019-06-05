#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "level0_pt.c"

#define L5_CORRECTOR 0

using namespace std;

static int parse_double(const char *c, double *result)
{
    assert(c);
    assert(result);
    char *endptr;
    errno = 0;
    *result = strtod(c, &endptr);
    const int local_errno = errno;
    if (local_errno || *endptr != '\0')
    {
        cerr << "Expected a double, got \"" << c << '"' << endl;
        if (local_errno)
            cerr << ", " << strerror(local_errno) << endl;
        return 1;
    }
    return 0;
}

static const char *parse_progname(const char *c)
{
    assert(c);
    const char * progname = strrchr(c, '/');
    if (progname)
        progname++;
    else
        progname = c;
    return progname;
}

// This function will calculate the MC jet Pt given the level 5 "detector" Pt
static float getMcJetPt(const float deltaJES, const float detPt,
                        const float detectorEta)
{
    float emf = 0.3f;
    const float pt0 = level0_pt(L5_CORRECTOR, detPt, emf, detectorEta);
    const float sigma = generic_correction_uncertainty(
        L5_CORRECTOR, pt0, detectorEta, 0);
    const float jes = 1.f + deltaJES*sigma;
    return jes*detPt;
}

static float getDetJetPt(const float deltaJES, const float mcPt,
                         const float detectorEta)
{
    const float stepFactor = 1.3;
    const float tol = 1.0e-6f;

    if (deltaJES == 0.f)
        return mcPt;

    // Bound the root
    float ptmin, ptmax;
    if (deltaJES > 0.f)
    {
        // jes will be > 1. detPt will be < mcPt.
        ptmax = mcPt;
        assert(getMcJetPt(deltaJES, ptmax, detectorEta) > mcPt);
        ptmin = mcPt/stepFactor;
        while (getMcJetPt(deltaJES, ptmin, detectorEta) > mcPt)
            ptmin /= stepFactor;
    }
    else
    {
        // jes will be < 1. detPt will be > mcPt.
        ptmin = mcPt;
        assert(getMcJetPt(deltaJES, ptmin, detectorEta) < mcPt);
        ptmax = mcPt*stepFactor;
        while (getMcJetPt(deltaJES, ptmax, detectorEta) < mcPt)
            ptmax *= stepFactor;
    }

    // Solve the equation by the simple interval division method
    while (fabsf(ptmax - ptmin)/(fabsf(ptmax) + fabsf(ptmin) + 1.f) > tol)
    {
        const float ptnext = (ptmax + ptmin)/2.f;
        if (getMcJetPt(deltaJES, ptnext, detectorEta) > mcPt)
            ptmax = ptnext;
        else
            ptmin = ptnext;
    }
    return (ptmax + ptmin)/2.f;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << parse_progname(argv[0])
             << " ptJetMC detectorEta deltaJES" << endl;
        return 1;
    }

    double ptJet, detectorEta, deltaJES;
    if (parse_double(argv[1], &ptJet))
        return 1;
    if (parse_double(argv[2], &detectorEta))
        return 1;
    if (parse_double(argv[3], &deltaJES))
        return 1;

    if (ptJet <= 0.0)
    {
        cerr << "Jet Pt can not be negative" << endl;
        return 1;
    }

    // Initialize jet corrections in some way.
    // In the production code this should be done for each event,
    // changing the run number, the number of primary vertices, etc.
    const int nvtx = 1;
    const int conesize = 0;
    const int version = 5; 
    const int syscode = 0;
    const int nrun = 195000;
    const int mode = 0; /* Use 1 for data, 0 for MC */
    init_generic_corrections(L5_CORRECTOR, 5, nvtx, conesize,
                             version, syscode, nrun, mode);

    cout << getDetJetPt(deltaJES, ptJet, detectorEta) << endl;
    return 0;
}
