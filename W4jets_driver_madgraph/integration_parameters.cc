#include <climits>
#include <fstream>
#include <stdexcept>

#include "parameter_parser.hh"

#include "WJetsTFSet.hh"
#include "RandomNumberSequence.hh"
#include "misc_utils.hh"

#include "npstat/stat/InMemoryNtuple.hh"

#include "npstat/rng/HOSobolGenerator.hh"
#include "npstat/rng/MersenneTwister.hh"

#include "geners/CPP11_auto_ptr.hh"

/* Scan parameters */
static int maxevents;
static int skipevents;
static int script_period;
static std::string category;
static std::string periodic_script;
static std::string rng_dump_file;
static std::string fcn_dump_file;
static std::string qmc_input_file;

static std::string tf_config_file;

static unsigned seed;
static unsigned nIntegrationPoints;
static unsigned interlacingFactor;
static double minDeltaJES;
static double maxDeltaJES;
static unsigned deltaJesSteps;
static double nominalWmass;
static double nominalWwidth;
static double maxWmassForScan;
static unsigned nWmassScanPoints;
static double eBeam;
static double minPartonPtFactor;
static double jetPtCut;
static double maxPartonPt;
static double priorPowerParam;
static double priorBandwidth;

static bool useQuasiMC;
static bool useLogScaleForTFpt;
static bool allowTau;
static bool allowDirectDecay;
static bool useDetectorEtaBins;
static bool saveJetPriors;

static unsigned maxSeconds;
static unsigned integrationMode;
static unsigned reportInterval;
static unsigned firstPowerOfTwoToStore;

static double reltol;
static double per_requirement;

static unsigned qmcuncert_m_min;
static unsigned qmcuncert_l_star;
static unsigned qmcuncert_r_lag;

/* Objects which will persist between events */
static npstat::InMemoryNtuple<double>* qmc = 0;
static WJetsTFSet* tfset = 0;
static npstat::InMemoryNtuple<double>* rawFcn = 0;
static std::ofstream* rawFcnStream = 0;
static npstat::LocalPolyFilter1D* priorFilter = 0;

#define check_parameter_range(name, minval, maxval) do {\
    if (name < minval || name > maxval)\
    {\
        Tcl_AppendResult(interp,\
                         "parameter \"" #name "\" out of range",\
                         NULL);\
        return TCL_ERROR;\
    }\
} while(0);

#define required_parameter(parser_type, name) do {\
    if (get_ ## parser_type ## _parameter(interp, some_string,\
                                          #name, &name) != TCL_OK)\
        return TCL_ERROR;\
} while(0);

#define optional_parameter(parser_type, name, default_value) do {\
    if (get_ ## parser_type ## _parameter(interp, some_string,\
                                          #name, &name) != TCL_OK)\
    {\
        Tcl_ResetResult(interp);\
        name = default_value;\
    }\
} while(0);

#define setup_required_parameter(parser_type, name, minval, maxval) do {\
    required_parameter(parser_type, name);\
    check_parameter_range(name, minval, maxval);\
} while(0);

#define setup_optional_parameter(parser_type, name, default_value, minval, maxval) do {\
    optional_parameter(parser_type, name, default_value);\
    check_parameter_range(name, minval, maxval);\
} while(0);

static CPP11_auto_ptr<npstat::AbsRandomGenerator> build_qmc_generator()
{
    npstat::AbsRandomGenerator* rng = 0;
    if (useQuasiMC)
        rng = new npstat::HOSobolGenerator(N_RSEQ_VARS, interlacingFactor, 20U);
    else
    {
        if (seed)
            rng = new npstat::MersenneTwister(seed);
        else
            rng = new npstat::MersenneTwister();
    }
    return CPP11_auto_ptr<npstat::AbsRandomGenerator>(rng);
}

static int setup_integration_parameters(Tcl_Interp *interp, const char *some_string)
{
    const unsigned minIntegPoints = 32;

    setup_optional_parameter(int,      maxevents,       INT_MAX, 0,        INT_MAX);
    setup_optional_parameter(int,      skipevents,      0,       0,        INT_MAX);
    setup_optional_parameter(int,      script_period,   1,       0,        INT_MAX);
    optional_parameter(string, category,        "Work");
    optional_parameter(string, periodic_script, "");
    optional_parameter(string, rng_dump_file,   "");
    optional_parameter(string, fcn_dump_file,   "");
    optional_parameter(string, qmc_input_file,  "");

    required_parameter(string, tf_config_file);

    optional_parameter(unsigned, seed, 0);

    // Sync the "nIntegrationPoints" setting with the "maxPowerOfTwo"
    // constructor parameter of HOSobolGenerator
    setup_optional_parameter(unsigned, interlacingFactor,  2,    1,        3);
    setup_optional_parameter(double,   minDeltaJES,    -3.05,   -1.e100,   1.e100);
    setup_optional_parameter(double,   maxDeltaJES,     3.05,   -1.e100,   1.e100);
    setup_optional_parameter(unsigned, deltaJesSteps,   61,      1,        INT_MAX);
    setup_optional_parameter(double,   nominalWmass,    80.385,  2.0,      1.e100);
    setup_optional_parameter(double,   nominalWwidth,   2.085,   1.0e-100, 1.e100);
    setup_optional_parameter(double, maxWmassForScan, 2.0*nominalWmass, 2.0, 1.e100);
    setup_optional_parameter(unsigned, nWmassScanPoints, 40,     3,        INT_MAX);
    setup_optional_parameter(double,   eBeam,           980.0,   1.0,      1.e100);
    setup_optional_parameter(double,   minPartonPtFactor, 0.35,  0.0,      1.0);
    setup_optional_parameter(double,   jetPtCut,        15.0,    0.0,      1.e100);
    setup_optional_parameter(double,   priorPowerParam, 0.0,     0.0,      1.e100);
    setup_optional_parameter(double,   priorBandwidth,  0.04,    0.001,    1.0);
    setup_optional_parameter(double,   maxPartonPt,     eBeam*0.6, 0.0,    eBeam);

    optional_parameter(bool,     useQuasiMC,         true);
    optional_parameter(bool,     useLogScaleForTFpt, false);
    optional_parameter(bool,     allowTau,           false);
    optional_parameter(bool,     allowDirectDecay,   true);
    optional_parameter(bool,     useDetectorEtaBins, true);
    optional_parameter(bool,     saveJetPriors,      false);

    const unsigned maxIntegPoints = useQuasiMC ? 1048576 : INT_MAX;
    setup_optional_parameter(unsigned, nIntegrationPoints, 2048, minIntegPoints, maxIntegPoints);

    optional_parameter(unsigned, maxSeconds,             14400);
    optional_parameter(unsigned, integrationMode,        0xffffffff);
    optional_parameter(unsigned, reportInterval,         0);
    optional_parameter(unsigned, firstPowerOfTwoToStore, 6);

    setup_optional_parameter(double,   reltol,           0.05, 0.0, 1.e100);
    setup_optional_parameter(double,   per_requirement,  0.5,  0.0, 1.0);

    optional_parameter(unsigned, qmcuncert_m_min, 8);
    optional_parameter(unsigned, qmcuncert_l_star, 4);
    optional_parameter(unsigned, qmcuncert_r_lag, 4);

    if (!is_power_of_two(nIntegrationPoints))
    {
        Tcl_AppendResult(interp, "number of QMC integration points "
                         "must be a power of 2", NULL);
        return TCL_ERROR;
    }

    // Read the transfer function data
    assert(!tfset);
    tfset = new WJetsTFSet(tf_config_file.c_str(), useLogScaleForTFpt, useDetectorEtaBins);

    // Quasi-MC (or pseudo-MC) vectors for integration
    assert(!qmc);
    qmc = new npstat::InMemoryNtuple<double>(
        npstat::simpleColumnNames(N_RSEQ_VARS));
    if (qmc_input_file.empty())
    {
        CPP11_auto_ptr<npstat::AbsRandomGenerator> gen = build_qmc_generator();
        double buf[minIntegPoints*N_RSEQ_VARS];
        for (unsigned i=0; i<nIntegrationPoints/minIntegPoints; ++i)
        {
            gen->run(buf, minIntegPoints*N_RSEQ_VARS, minIntegPoints);
            qmc->fill(buf, minIntegPoints*N_RSEQ_VARS);
        }
    }
    else
    {
        std::ifstream is(qmc_input_file.c_str());
        if (!is.is_open())
        {
            std::string ermess("Error: failed to open file \"");
            ermess += qmc_input_file;
            ermess += '"';
            throw std::invalid_argument(ermess);
        }
        fillNtupleFromText(is, qmc, false);
    }

    // Save the integration vectors
    if (!rng_dump_file.empty())
    {
        std::ofstream of(rng_dump_file.c_str());
        if (!of.is_open())
        {
            std::string ermess("Error: failed to open file \"");
            ermess += rng_dump_file;
            ermess += '"';
            throw std::invalid_argument(ermess);
        }
        of.precision(12);
        npstat::dumpNtupleAsText(*qmc, of);
        std::cout << "Wrote file " << rng_dump_file << std::endl;
    }

    if (!fcn_dump_file.empty())
    {
        assert(!rawFcnStream);
        rawFcnStream = new std::ofstream(fcn_dump_file.c_str());
        if (!rawFcnStream->is_open())
        {
            delete rawFcnStream; rawFcnStream = 0;
            std::string ermess("Error: failed to open file \"");
            ermess += fcn_dump_file;
            ermess += '"';
            throw std::invalid_argument(ermess);
        }
        *rawFcnStream << "# " << deltaJesSteps << " columns\n";
        rawFcnStream->precision(12);

        assert(!rawFcn);
        rawFcn = new npstat::InMemoryNtuple<double>(
            npstat::simpleColumnNames(deltaJesSteps));
    }

    return TCL_OK;
}

static void cleanup_integration_parameters(void)
{
    if (rawFcnStream)
        std::cout << "Wrote file " << fcn_dump_file << std::endl;
    delete rawFcnStream; rawFcnStream = 0;
    delete rawFcn; rawFcn = 0;
    delete tfset; tfset = 0;
    delete qmc; qmc = 0;
    delete priorFilter; priorFilter = 0;
}
