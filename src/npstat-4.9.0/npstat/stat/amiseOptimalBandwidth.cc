#include <cmath>
#include <cassert>
#include <sstream>
#include <stdexcept>

#include "npstat/nm/MathUtils.hh"
#include "npstat/nm/GaussLegendreQuadrature.hh"

#include "npstat/stat/amiseOptimalBandwidth.hh"
#include "npstat/stat/Distributions1D.hh"

#define MAX_SUPPORTED_DEGREE 42U
#define SQRTPI 1.7724538509055160273

// The function below calculates the integral of squared k^{th} order
// derivative of the Gaussian distribution
static double gaussDerivativeIntegral(const unsigned k, const double sigma)
{
    const double coeffs[] = {
        0.28209479177387814347,0.14104739588693907174,0.21157109383040860761,
        0.52892773457602151901,1.8512470710160753165,8.3306118195723389245,
        45.818365007647864085,297.81937254971111655,2233.6452941228333741,
        18985.98500004408368,180366.85750041879496,1.8938520037543973471e6,
        2.1779298043175569491e7,2.7224122553969461864e8,3.6752565447858773517e9,
        5.3291219899395221599e10,8.2601390844062593479e11,
        1.3629229489270327924e13,2.3851151606223073867e14,
        4.4124630471512686654e15,8.6043029419449738976e16,1.763882103098719649e18,
        3.7923465216622472453e19,8.532779673740056302e20,2.005203223328913231e22,
        4.9127478971558374159e23,1.2527507137747385411e25,
        3.3197893915030571338e26,9.1294208266334071179e27,
        2.6018849355905210286e29,7.6755605599920370344e30,
        2.3410459707975712955e32,7.3742948080123495808e33,
        2.3966458126040136138e35,8.0287634722234456061e36,
        2.7699233979170887341e38,9.8332280626056650061e39,
        3.5891282428510677272e41,1.3459230910691503977e43,
        5.1818039006162290312e44,2.0468125407434104673e46,
        8.2895907900108123926e47,3.4401801778544871429e49,
        1.4620765755881570357e51,6.3600331038084831055e52,
        2.8302147311947749819e54,1.2877477026936226168e56,5.988026817525345168e57,
        2.8443127383245389548e59,1.3794916780874013931e61,
        6.8284838065326368958e62};
    if (k >= sizeof(coeffs)/sizeof(coeffs[0])) 
    {
        std::ostringstream os;
        os << "In npstat static function gaussDerivativeIntegral: "
           << "derivative order can not exceed " << sizeof(coeffs)/sizeof(coeffs[0]) - 1;
        throw std::out_of_range(os.str());
    }
    if (sigma <= 0.0) throw std::invalid_argument(
        "In npstat static function gaussDerivativeIntegral: "
        "standard deviation must be positive");
    return coeffs[k]/pow(sigma, 2U*k + 1U);
}


namespace npstat {
    namespace Private {
        static double amiseGaussCoeff(const unsigned filterDegree)
        {
            static const double table[] = {
                0.7763883564090196412, 1.1601801382564782543, 
                1.4450467187749709199, 1.6818440790937138085,
                1.8888920025960289659, 2.0752051460454913437,
                2.2460003158587854920, 2.4046162324871727761, 
                2.5533421315344606073, 2.6938285908812696361,
                2.8273122259820905536, 2.9547479503774093633,
                3.0768914093658725973, 3.1943527695228836267, 
                3.3076331635645612683, 3.4171501727043358113,
                3.5232561262600069321, 3.6262515483723832173,
                3.7263952379652137353, 3.8239119583959906923, 
                3.9189983952072462532, 4.0118278362036896802
            };
            if (filterDegree % 2U) throw std::invalid_argument(
                "In npstat::Private::amiseGaussCoeff: filter degree must be even");
            const unsigned idx = filterDegree/2U;
            if (!(idx < sizeof(table)/sizeof(table[0]))) throw std::out_of_range(
                "In npstat::Private::amiseGaussCoeff: filter degree is out of range");
            return table[idx];
        }

        static double amiseSymbetaCoeff(const unsigned power,
                                        const unsigned filterDegree)
        {
            static const double table[11][MAX_SUPPORTED_DEGREE/2U+1U] = {
                {1.3509600385206134134,2.8128927750924412583,
                 4.2807363497512651953,5.7503075204264349083,7.220619187974499472,
                 8.6913162099697717756,10.162239300301520014,11.633306345949534227,
                 13.104470711834314553,14.575703945541352035,16.046987701893851712,
                 17.51830962061967504,18.989661069596238279,20.461035837705797374,
                 21.932429341646925168,23.403838125427853571,24.875259533838693209,
                 26.346691493208303412,27.81813236045492942,29.289580816835867871,
                 30.761035791679745085,32.232496406673688899},
                {1.718771927587478777,3.243131560314586551,4.735382257906651823,
                 6.2179346556483784759,7.6962984130124134883,9.1724766934095020998,
                 10.647371521983958866,12.121448727771521553,13.594973100207188522,
                 15.068106257177596421,16.540952424602170584,18.013581830681437433,
                 19.486043521065895766,20.958372784387275274,22.430595659142600402,
                 23.902731777911109456,25.374796223011391902,26.846800772477520522,
                 28.318754757903866106,29.790665668245201961,31.262539583211400306,
                 32.734381489840815369},
                {2.0361680046403980174,3.6333454537033738294,
                 5.1581093447616179746,6.6592142928654252407,8.149600040615102008,
                 9.6342099167592681997,11.115348319444509145,12.59423619534386538,
                 14.071581420491009663,15.547822906878457785,17.023247357304608275,
                 18.498050023689243232,19.972368497703548947,21.446302542716282862,
                 22.919926263745361278,24.393295861312565771,25.866454732753472657,
                 27.339436922858774033,28.812269515274917799,30.2849743256452104,
                 31.757569123373205925,33.23006852835858082},
                {2.3121667641823967173,3.986645837631487039,5.5493966040169609515,
                 7.0734333753976839124,8.5792450101718384946,10.074966821035415539,
                 11.564500524681122469,13.049959082191523019,14.532589008974119771,
                 16.013174211052051063,17.492232801603862429,18.970121149188925153,
                 20.447092525126709769,21.923331924095656566,23.3989776480863177,
                 24.874135180418768147,26.348886385385754848,27.823295774783848371,
                 29.297414878177747133,30.771285354700648445,32.244941250103465171,
                 33.718410661149308735},
                {2.559067851900808541,4.311295850616169863,
                 5.914839739651167265,7.4645506861827802062,8.988143372986744187,
                 10.496953150056513984,11.99654135522515482,13.489974419753920307,
                 14.979088027118519613,16.465050710926218552,17.948642757924012689,
                 19.430405591983674236,20.910726934385653406,22.38989186938427824,
                 23.868114775731099386,25.345560012155444764,26.822355729277677467,
                 28.298603336942251401,29.774384144201625783,31.249764111645350066,
                 32.724797314595472247,34.199528507965255794},
                {2.7843579286436103242,4.6131849934271338527,6.2588631073916434005,
                 7.835949919885565104,9.378945485751997157,10.902290917200370828,
                 12.413202260794515047,13.915717939281426635,15.412285493464744776,
                 16.904479218702277174,18.393359727506925858,19.879668630993739833,
                 21.363940583644596513,22.846571039098963963,24.327858938252169019,
                 25.808034550389976202,27.287278176701696333,28.765733041637098424,
                 30.243514380020151099,31.720715970865597969,33.197414919001079434,
                 34.673675210193897119},
                {2.9928316208559228916,4.8964850848042573044,
                 6.5848184206353634584,8.1903240540743610073,9.7538435322038233628,
                 11.292791316016904889,12.816000726485763419,14.328477094741725924,
                 15.833285885133145877,17.332416557196432003,18.827219819729475513,
                 20.318646522444974972,21.807386254942522616,23.293951766700847181,
                 24.778732500697802895,26.262029713934465898,27.744080197745312139,
                 29.225072707035546784,30.705159593225692828,32.184465203102483447,
                 33.663092048739269821,35.14112541099603344},
                {3.1877522539787426765,5.1642471164949763729,6.8952817031469796455,
                 8.5298210452640453472,10.114637708150789624,11.669977347355443264,
                 13.206238308169932527,14.729374981534406103,16.243066948610543155,
                 17.749720814802611109,19.250981599660020841,20.748014260265054442,
                 22.241668145795934745,23.732577788834420423,25.221227181793608222,
                 26.707992162641153731,28.193169177267208189,29.676995287392541871,
                 31.159662394093128736,32.641327544989304851,34.122120532017626353,
                 35.60214957834600859},
                {3.3714531470506309977,5.4187743571205263495,
                 7.1922746021584484388,8.8561771899879808093,10.462817827639741612,
                 12.035133965511475276,13.585031045236078926,15.119388033635221984,
                 16.642489021247414559,18.157155054393717378,19.665325988456909387,
                 21.168382871849311407,22.667337378789846481,24.162948534066394879,
                 25.655797517540982838,27.146337215900911928,28.634925988209321698,
                 30.121851244814498905,31.607346269426349574,33.091602449866661248,
                 34.574778321656023077,36.057006356666797332},
                {3.5456688701088532046,5.6618537154880833602,7.4774155190856732729,
                 9.1708167199185372531,10.799630155968415853,12.38935366855644876,
                 13.953340936426132028,15.499367898954250113,17.032310238069719913,
                 18.555397802524719012,20.070863362124839923,21.58030404729859775,
                 23.084894811771408706,24.585520580803162319,26.082861306553424638,
                 27.577448535734500187,29.069704098995860656,30.559967221370085268,
                 32.048513925981267847,33.535571184078352465,35.021327407077651122,
                 36.505940343184437159},
                {3.7117296436351696251,5.8949036859206256396,
                 7.7520229981863904286,9.4749240193725258311,11.126128555220909468,
                 12.733573287539513661,14.312002777841265851,15.870061248909978319,
                 17.413201287374083271,18.945054110660448875,20.46814187205830422,
                 21.984276399829687569,23.494795731492187972,25.000711147010469241,
                 26.50280216622313513,28.001679953279965944,29.497830825747343406,
                 30.991646836664857316,32.483447726328241767,33.973496973897519322,
                 35.462013729720519676,36.949181817441108573}
            };
            if (filterDegree % 2U) throw std::invalid_argument(
                "In npstat::Private::amiseSymbetaCoeff: filter degree must be even");
            if (!(power < 11)) throw std::out_of_range(
                "In npstat::Private::amiseSymbetaCoeff: power parameter out of range");
            const unsigned idx = filterDegree/2U;
            if (!(idx <= MAX_SUPPORTED_DEGREE/2U)) throw std::out_of_range(
                "In npstat::Private::amiseSymbetaCoeff: filter degree is out of range");
            return table[power][idx];
        }

        static double squaredKernelIntegralGauss(const unsigned filterDegree)
        {
            static const double table[MAX_SUPPORTED_DEGREE/2U+1U] = {
                0.28209479177387814347,0.47603496111841936711,0.62396943688265038571,
                0.74785078617543927989,0.85648001106224300562,0.9543599225349779124,
                1.0441468825848104863,1.1275597369416904188,1.205785159778723121,
                1.2796826724101757126,1.3498982646254293645,1.4169318951985559054,
                1.4811798466573655826,1.5429625024119446397,1.6025432401677446403,
                1.6601416824434331664,1.7159432351662706572,1.7701061103815678241,
                1.822766598991656468,1.8740430983972030403,1.9240392363913578946,
                1.972846327333157967
            };
            if (filterDegree % 2U) throw std::invalid_argument(
                "In npstat::Private::squaredKernelIntegralGauss: "
                "filter degree must be even");
            const unsigned idx = filterDegree/2U;
            if (!(idx < sizeof(table)/sizeof(table[0]))) throw std::out_of_range(
                "In npstat::Private::squaredKernelIntegralGauss: "
                "filter degree is out of range");
            return table[idx];
        }

        static double squaredKernelIntegralSymbeta(const unsigned power,
                                                   const unsigned filterDegree)
        {
            static const double table[11][MAX_SUPPORTED_DEGREE/2U+1U] = {
                {0.5,1.125,1.7578125,2.392578125,3.028106689453125,
                 3.66400909423828125,4.3001217842102050781,4.936364293098449707,
                 5.5726925027556717396,6.2090802268357947469,6.8455109500864637084,
                 7.4819737450325192185,8.1184610948703550548,8.7549676600007231286,
                 9.3914895434446532541,10.028023834722568641,10.66456831641882544,
                 11.301121269561471595,11.937681341072264362,12.574247451364898958,
                 13.210818728590246967,13.847394460976965217},
                {0.6,1.25,1.8930288461538461538,2.5333180147058823529,
                 3.17230224609375,3.8105694580078125,4.4484018457346949084,
                 5.0859510898590087891,5.723305813640960165,6.3605212079781312041,
                 6.9976334156439406797,7.6346670867678767536,8.2716396060943240181,
                 8.9085635838603849379,9.5454483884191557664,10.182301124487531236,
                 10.819127277526344649,11.455931149966423261,12.092716163683592471,
                 12.729485074221255735,13.366240125397191049,14.002983162785695163},
                {0.71428571428571428571,1.4073426573426573427,2.0711962669683257919,
                 2.7237930534055727554,3.3709159519361413043,4.0149678197400323276,
                 4.6571245129832436431,5.2980293592430910088,5.9380640055224220849,
                 6.5774692181727341289,7.2164044125724529372,7.8549793445436423425,
                 8.4932720548700571018,9.1313395556712864784,9.7692244678546231543,
                 10.406959293360065994,11.04456925050972208,11.682074206433292915,
                 12.319490025168516849,12.956829527708410692,13.594103188364055213,
                 14.231319648247638438},
                {0.81585081585081585082,1.5549156725627313863,
                 2.2435029173612764944,2.9116952988289137165,3.5695930721162856072,
                 4.2215497416834677419,4.8697590513280690714,5.5154497433274532512,
                 6.159364830691435519,6.8019805351609869761,7.443616397990792334,
                 8.0844949299813694309,8.7247758995453298923,9.3645770218360918935,
                 10.003986958068669114,10.643073768434691797,11.281890573247947513,
                 11.920479442844050497,12.558874131251983378,13.19710203596189209,
                 13.835185628054392588,14.473143512593626999},
                {0.90703414232825997532,1.6916527744701119314,2.4064119497913581909,
                 3.0918658378337920823,3.7620828621806158413,4.4233018997660747772,
                 5.0787409140166553489,5.7302372545436550299,6.3789198674030520123,
                 7.0255221664398672765,7.6705419758146194388,8.314329422951472743,
                 8.9571381012532941601,9.5991562873634285289,10.240526751255685075,
                 10.881359753012366127,11.521741817115727543,12.161741805308183355,
                 12.801415212562839684,13.440807265426803616,14.079955195500803255,
                 14.718889933700979562},
                {0.99023577042152893546,1.8190200565351998923,
                 2.560330467506184988,3.2638796197720324897,3.947335014097391556,
                 4.6187024476456083579,5.2821873027846428842,5.94022804278842402,
                 6.594342781695299776,7.2455289582910219489,7.8944702784182463988,
                 8.5416517440181060984,9.1874273028229665176,9.8320615084924549173,
                 10.47575617925501356,11.118668020697401465,11.760920605374545892,
                 12.402612716462393617,13.043824284227953587,13.684620690292795329,
                 14.325055941463792095,14.965175045682894873},
                {1.0671584331673172702,1.9384959223174790314,2.7062427773804099648,
                 3.4282633401313728489,4.1254983756528672271,4.80759606629582356,
                 5.4796983791124889608,6.1448243978575915532,6.8048742487648000003,
                 7.4611082697066147713,8.1143976165321945031,8.7653648474511038244,
                 9.4144672981329232358,10.062048820718049699,10.70837312492960067,
                 11.353645960487947087,11.998030285636618241,12.641656889525300784,
                 13.284631988101747251,13.927042757263179449,14.568961430512805725,
                 15.210448378858605533},
                {1.1390032537910611258,2.0512914171097812724,
                 2.8451205412986619158,3.5857248188395170225,4.2970430153807687049,
                 4.9902499433393948527,5.6713710089286920895,6.34398057576749203,
                 7.010348764758553766,7.6719934891472101961,8.3299714825812924201,
                 8.9850427765519089505,9.6377689200702239042,10.288574320872264031,
                 10.937786008625204636,11.58566023538549326,12.232400757901277766,
                 12.878171701147434063,13.523106797345143703,14.167316143666471248,
                 14.810891225940427111,15.453908708137397191},
                {1.206642010064577219,2.1583673071920883499,2.9778096963875325186,
                 3.7369531709256573912,4.4625038577123284801,5.1670600428730534556,
                 5.8574797768415309126,6.5378663366783739767,7.2108458184010769575,
                 7.8781862742504097691,8.5411262176787716599,9.2005614117630437017,
                 9.857157050883151277,10.511418197716957504,11.163735672018186148,
                 11.81441689319678678,12.463707171423867589,13.111804749856461214,
                 13.758871650147664191,14.40504163389954297,15.050426141360807386,
                 15.695118785429905728},
                {1.270727072632571496,2.260487568323776105,
                 3.1050237397320711989,3.8825672063161872098,4.6223974797043616832,
                 5.3384457441936852207,6.0383559216052082049,6.72673458052560403,
                 7.4065492625992765421,8.0798097833084455153,8.7479316124960026056,
                 9.4119435306394051221,10.072612939697720506,10.730524900269333053,
                 11.386133843840150242,12.039798469928583833,12.691805927970154732,
                 13.342388963311395195,13.991738321468914066,14.640011882965414491,
                 15.28734149788280743,15.933838172560295489},
                {1.3317601177721471626,2.3582665510443712353,3.2273609639877450891,
                 4.023108023338764013,4.7771961624102559167,5.5048102163552732759,
                 6.2143376132920252308,6.9108641066649783682,7.597684414855023474,
                 8.2770414646080457501,8.9505223737004482371,9.6192856839853839631,
                 10.284199037797977215,10.945926348980947352,11.604985046501037469,
                 12.261784839386083069,12.916654667995013918,13.569861873866457775,
                 14.221626109620142267,14.872129611666731737,15.52152490680990572,
                 16.169940675699852882}
            };
            if (filterDegree % 2U) throw std::invalid_argument(
                "In npstat::Private::squaredKernelIntegralSymbeta: "
                "filter degree must be even");
            if (!(power < 11)) throw std::out_of_range(
                "In npstat::Private::squaredKernelIntegralSymbeta: "
                "power parameter out of range");
            const unsigned idx = filterDegree/2U;
            if (!(idx <= MAX_SUPPORTED_DEGREE/2U)) throw std::out_of_range(
                "In npstat::Private::squaredKernelIntegralSymbeta: "
                "filter degree is out of range");
            return table[power][idx];
        }

        double amiseBwGauss(const unsigned filterDegree, const double npoints,
                            const double rfk, /* squared derivative integral */
                            double* expectedAmise)
        {
            const double c = amiseGaussCoeff(filterDegree);
            const double bw = c*pow(rfk*npoints, -1.0/(2*filterDegree + 5));
            if (expectedAmise)
            {
                const double rk = squaredKernelIntegralGauss(filterDegree);
                *expectedAmise = rk/bw/npoints*(2*filterDegree + 5)/
                    (2.0*filterDegree + 4);
            }
            return bw;
        }

        double amiseSymbeta(const unsigned power, const unsigned filterDegree,
                            const double rfk, /* squared derivative integral */
                            const double npoints, double* expectedAmise)
        {
            const double c = amiseSymbetaCoeff(power, filterDegree);
            const double bw = c*pow(rfk*npoints, -1.0/(2*filterDegree + 5));
            if (expectedAmise)
            {
                const double rk = squaredKernelIntegralSymbeta(
                    power, filterDegree);
                *expectedAmise = rk/bw/npoints*(2*filterDegree + 5)/
                    (2.0*filterDegree + 4);
            }
            return bw;
        }
    }

    double amisePluginBwGauss(const unsigned filterDegree,
                              const double npoints, const double sigma,
                              double* expAmise)
    {
        const double rfk = gaussDerivativeIntegral(filterDegree+2, sigma);
        return Private::amiseBwGauss(filterDegree, npoints, rfk, expAmise);
    }

    double approxAmisePluginBwGauss(const double filterDegree,
                                    const double npoints,
                                    const double sampleSigma)
    {
        return 1.06*sampleSigma*pow(npoints, -1.0/(2*filterDegree + 5));
    }

    double amisePluginBwSymbeta(const unsigned power,
                                const unsigned filterDegree,
                                const double npoints, const double sigma,
                                double* expectedAmise)
    {
        const double rfk = gaussDerivativeIntegral(filterDegree+2, sigma);
        return Private::amiseSymbeta(power, filterDegree, rfk,
                                     npoints, expectedAmise);
    }

    double symbetaBandwidthRatio(const int power, const unsigned filterDeg)
    {
        if (power < 0)
            return 1.0;
        else
        {
            const unsigned filterDegree = (filterDeg / 2) * 2;
            const double c1 = Private::amiseSymbetaCoeff(power, filterDegree);
            const double c2 = Private::amiseGaussCoeff(filterDegree);
            return c1/c2;
        }
    }

    double approxSymbetaBandwidthRatio(const int power, const double filterDeg)
    {
        if (power < 0)
            return 1.0;
        else
        {
            const unsigned degree = 5;
            static const double coeffs[11][degree+1U] = {
                {1.755825639, 0.3524142206, -0.01551067829, 0.0005740813795, -1.127153246e-05, 8.712527233e-08},
                {2.222903013, 0.301379174, -0.01134911738, 0.0003940133902, -7.496571016e-06, 5.691626015e-08},
                {2.627974987, 0.2644844055, -0.008572408929, 0.0002783103555, -5.119840807e-06, 3.812982996e-08},
                {2.981479645, 0.2379725128, -0.006757468451, 0.000206080862, -3.672682396e-06, 2.686218359e-08},
                {3.298346519, 0.2178665549, -0.005496900994, 0.0001580553508, -2.732946768e-06, 1.964840557e-08},
                {3.587818861, 0.2019804418, -0.004578707274, 0.0001244868763, -2.090584303e-06, 1.478231937e-08},
                {3.85588026, 0.1890378147, -0.003885307582, 0.0001001162454, -1.634095952e-06, 1.136781691e-08},
                {4.106634617, 0.1782407314, -0.003346590558, 8.188421634e-05, -1.299525252e-06, 8.89525964e-09},
                {4.34303236, 0.1690629423, -0.002918424085, 6.791410124e-05, -1.048247782e-06, 7.060071194e-09},
                {4.567273617, 0.1611413211, -0.002571604913, 5.699019675e-05, -8.555169302e-07, 5.668228997e-09},
                {4.781053066, 0.1542170048, -0.002286247443, 4.830721809e-05, -7.052356068e-07, 4.595182013e-09}
            };

            if (!(power < 11)) throw std::out_of_range(
                "In npstat::approxSymbetaBandwidthRatio: "
                "power parameter out of range");       

            const double maxdeg = MAX_SUPPORTED_DEGREE;
            if (filterDeg < 0.0 || filterDeg > maxdeg)
                throw std::invalid_argument(
                    "In npstat::approxSymbetaBandwidthRatio: "
                    "filter degree is out of range");

            return polySeriesSum(&coeffs[power][0], degree, filterDeg);
        }
    }

    unsigned amisePluginDegreeGauss(const double npoints)
    {
        double amise = 0.0;
        amisePluginBwGauss(0U, npoints, 1.0, &amise);
        for (unsigned deg = 2U; deg <= MAX_SUPPORTED_DEGREE; deg += 2U)
        {
            double nextAmise = 0.0;
            amisePluginBwGauss(deg, npoints, 1.0, &nextAmise);
            if (nextAmise >= amise)
                return deg - 2U;
            amise = nextAmise;
        }
        return MAX_SUPPORTED_DEGREE;
    }

    unsigned amisePluginDegreeSymbeta(const unsigned power, const double npoints)
    {
        double amise = 0.0;
        amisePluginBwSymbeta(power, 0U, npoints, 1.0, &amise);
        for (unsigned deg = 2U; deg <= MAX_SUPPORTED_DEGREE; deg += 2U)
        {
            double nextAmise = 0.0;
            amisePluginBwSymbeta(power, deg, npoints, 1.0, &nextAmise);
            if (nextAmise >= amise)
                return deg - 2U;
            amise = nextAmise;
        }
        return MAX_SUPPORTED_DEGREE;
    }

    unsigned maxFilterDegreeSupported()
    {
        return MAX_SUPPORTED_DEGREE;
    }

    double integralOfSymmetricBetaSquared(const int power)
    {
        if (power < 0)
            return 0.5/SQRTPI;
        else
            return Private::squaredKernelIntegralSymbeta(power, 0);
    }

    double integralOfSymmetricBetaSquared(const int power,
                                          double a, double b)
    {
        if (a == b)
            return 0.0;
        if (power < 0)
            return (erfl(b) - erfl(a))/4.0/SQRTPI;
        else
        {
            if (a < -1.0)
                a = -1.0;
            if (a > 1.0)
                a = 1.0;
            if (b < -1.0)
                b = -1.0;
            if (b > 1.0)
                b = 1.0;
            if (a == b)
                return 0.0;
            if (power == 0)
                return (b - a)/4.0;
            if (!(power < 11)) throw std::out_of_range(
                "In npstat::integralOfSymmetricBetaSquared: "
                "power parameter out of range");
            GaussLegendreQuadrature glq(16);
            SymmetricBeta1D sb(0.0, 1.0, power);
            DensitySquaredFunctor1D df(sb);
            return glq.integrate(df, a, b);
        }
    }
}
