#include <math.h>

#include "p0_interpolator.h"
#include "linear_interpolator_1d.h"

/* The interpolator uses five different eta bins:
   eta < 0.2, 0.2 <= eta < 0.6, 0.6 <= eta <= 0.9,
   0.9 < eta <= 1.4, eta > 1.4.
   These are derived from 0.10, 0.40, 0.70, 1.20,
   and 1.60, respectively. */

#ifdef __cplusplus
extern "C" {
#endif

static const float p0_interpolator_bin1_data_data[101] = {
    10.0245275497,
    10.4081249237,
    10.7617149353,
    11.0925273895,
    11.4048109055,
    11.7015037537,
    11.9852581024,
    12.2578029633,
    12.5204086304,
    12.774353981,
    13.0204896927,
    13.2595891953,
    13.4922943115,
    13.7192277908,
    13.9405078888,
    14.1570968628,
    14.3689432144,
    14.5765342712,
    14.7800540924,
    14.9799203873,
    15.17598629,
    15.3688936234,
    15.5584430695,
    15.7451734543,
    15.9288053513,
    16.1098613739,
    16.2882671356,
    16.4642848969,
    16.6377162933,
    16.8090229034,
    16.9781074524,
    17.1450595856,
    17.3100204468,
    17.4730110168,
    17.6341209412,
    17.7934112549,
    17.9509658813,
    18.1068115234,
    18.2610187531,
    18.413640976,
    18.5647201538,
    18.7143135071,
    18.862449646,
    19.0091838837,
    19.1545963287,
    19.2985496521,
    19.4412498474,
    19.5827445984,
    19.7230148315,
    19.8620986938,
    20.0,
    20.1367664337,
    20.2724380493,
    20.4070281982,
    20.5405597687,
    20.6730155945,
    20.8044395447,
    20.9348297119,
    21.0642414093,
    21.1929130554,
    21.3205451965,
    21.4472694397,
    21.573102951,
    21.6980724335,
    21.8221893311,
    21.9454650879,
    22.0679206848,
    22.1895809174,
    22.3104496002,
    22.4305362701,
    22.549861908,
    22.6684570312,
    22.7863121033,
    22.9034442902,
    23.0198726654,
    23.1355705261,
    23.2506713867,
    23.3650684357,
    23.478767395,
    23.5918598175,
    23.7043113708,
    23.8160953522,
    23.9272613525,
    24.0378093719,
    24.1478843689,
    24.2570896149,
    24.3659973145,
    24.4743022919,
    24.5819358826,
    24.688999176,
    24.7955989838,
    24.9017276764,
    25.0071983337,
    25.1119308472,
    25.2166576385,
    25.3205280304,
    25.4239902496,
    25.5269527435,
    25.6294212341,
    25.7314395905,
    25.8329353333
};

static const Interpolator_data_1d p0_interpolator_bin1_data = {
    101,
    -5.00000018885,
    5.00000018885,
    p0_interpolator_bin1_data_data
};

static double p0_interpolator_bin1(double xvariable)
{
    return linear_interpolate_1d(&p0_interpolator_bin1_data, (float)xvariable);
}

static const float p0_interpolator_bin2_data_data[101] = {
    10.1861743927,
    10.5580148697,
    10.9025535583,
    11.2256574631,
    11.5311222076,
    11.8221635818,
    12.1007175446,
    12.3686208725,
    12.6269378662,
    12.8768377304,
    13.1192111969,
    13.3547210693,
    13.5840291977,
    13.8076324463,
    14.0259799957,
    14.2394628525,
    14.4483804703,
    14.6531095505,
    14.8540391922,
    15.0509843826,
    15.24447155,
    15.4346961975,
    15.6217861176,
    15.8059206009,
    15.9871721268,
    16.1658916473,
    16.3417072296,
    16.5154285431,
    16.6865634918,
    16.8554916382,
    17.0221729279,
    17.1869277954,
    17.3495502472,
    17.5102767944,
    17.6691150665,
    17.8261737823,
    17.9814891815,
    18.1351108551,
    18.2871131897,
    18.4375362396,
    18.5864162445,
    18.7338161469,
    18.8797721863,
    19.0243263245,
    19.1675167084,
    19.3094520569,
    19.4499664307,
    19.5892543793,
    19.7273483276,
    19.8642578125,
    20.0,
    20.1345806122,
    20.2680091858,
    20.4002971649,
    20.5318450928,
    20.6621627808,
    20.7914676666,
    20.9197845459,
    21.0471305847,
    21.173532486,
    21.2990093231,
    21.4235782623,
    21.5472564697,
    21.6700553894,
    21.7920036316,
    21.9131278992,
    22.0334014893,
    22.1528892517,
    22.2715950012,
    22.3895206451,
    22.50664711,
    22.6230564117,
    22.738779068,
    22.8536624908,
    22.9679336548,
    23.0814170837,
    23.1943817139,
    23.3065357208,
    23.4180221558,
    23.5290431976,
    23.6391773224,
    23.7488899231,
    23.8577461243,
    23.9659614563,
    24.0737457275,
    24.180978775,
    24.2876358032,
    24.393573761,
    24.4988899231,
    24.6037425995,
    24.7079563141,
    24.8118572235,
    24.9151039124,
    25.0178089142,
    25.119884491,
    25.2216873169,
    25.3228473663,
    25.4236412048,
    25.5236797333,
    25.6233882904,
    25.7226600647
};

static const Interpolator_data_1d p0_interpolator_bin2_data = {
    101,
    -5.00000018885,
    5.00000018885,
    p0_interpolator_bin2_data_data
};

static double p0_interpolator_bin2(double xvariable)
{
    return linear_interpolate_1d(&p0_interpolator_bin2_data, (float)xvariable);
}

static const float p0_interpolator_bin3_data_data[101] = {
    10.0244836807,
    10.4081115723,
    10.7618713379,
    11.0927534103,
    11.4047670364,
    11.7015972137,
    11.9853076935,
    12.2578239441,
    12.5204229355,
    12.7743616104,
    13.0204868317,
    13.2595443726,
    13.4922847748,
    13.7191267014,
    13.9405956268,
    14.1570920944,
    14.368979454,
    14.5765190125,
    14.7800474167,
    14.9797973633,
    15.175992012,
    15.3688488007,
    15.5584459305,
    15.7450809479,
    15.928981781,
    16.1099891663,
    16.2883892059,
    16.4643115997,
    16.6377868652,
    16.8090610504,
    16.978105545,
    17.1450996399,
    17.3100090027,
    17.4730205536,
    17.6341171265,
    17.7933979034,
    17.9509601593,
    18.1068077087,
    18.2610168457,
    18.413640976,
    18.5647239685,
    18.7143115997,
    18.862449646,
    19.0091819763,
    19.1545467377,
    19.2985801697,
    19.4414520264,
    19.5828495026,
    19.7230644226,
    19.8621101379,
    20.0,
    20.136756897,
    20.2723999023,
    20.4069499969,
    20.5404109955,
    20.6727924347,
    20.8043861389,
    20.9348545074,
    21.0643558502,
    21.1929111481,
    21.3205451965,
    21.4472675323,
    21.5731048584,
    21.6980724335,
    21.8221797943,
    21.9454574585,
    22.0679149628,
    22.189573288,
    22.3104324341,
    22.4305343628,
    22.5498638153,
    22.6684513092,
    22.786315918,
    22.9034385681,
    23.0198307037,
    23.1355934143,
    23.2506504059,
    23.3650436401,
    23.4787025452,
    23.5918292999,
    23.7042427063,
    23.8161182404,
    23.9272727966,
    24.037820816,
    24.1478443146,
    24.2571983337,
    24.3660469055,
    24.4743385315,
    24.5817451477,
    24.6888942719,
    24.7955284119,
    24.9016342163,
    25.0070724487,
    25.1119689941,
    25.2165718079,
    25.3203887939,
    25.4239826202,
    25.5270805359,
    25.6294403076,
    25.7316246033,
    25.8329868317
};

static const Interpolator_data_1d p0_interpolator_bin3_data = {
    101,
    -5.00000018885,
    5.00000018885,
    p0_interpolator_bin3_data_data
};

static double p0_interpolator_bin3(double xvariable)
{
    return linear_interpolate_1d(&p0_interpolator_bin3_data, (float)xvariable);
}

static const float p0_interpolator_bin4_data_data[101] = {
    9.69939136505,
    10.1089057922,
    10.4825763702,
    10.8295068741,
    11.1551513672,
    11.4638519287,
    11.7578735352,
    12.0401039124,
    12.3113441467,
    12.5733528137,
    12.8269996643,
    13.0732126236,
    13.3126430511,
    13.5459547043,
    13.7735891342,
    13.9960432053,
    14.2136716843,
    14.4268131256,
    14.6357746124,
    14.8408517838,
    15.0422306061,
    15.2401418686,
    15.434803009,
    15.6263313293,
    15.8149337769,
    16.0007305145,
    16.1838645935,
    16.3643302917,
    16.5426139832,
    16.7184486389,
    16.8921413422,
    17.0635089874,
    17.3099460602,
    17.4730129242,
    17.634103775,
    17.7934150696,
    17.9509658813,
    18.1068096161,
    18.2610206604,
    18.4136428833,
    18.5647239685,
    18.7143115997,
    18.8624515533,
    19.0091819763,
    19.1545467377,
    19.2986850739,
    19.4413356781,
    19.5827884674,
    19.7230377197,
    19.8621025085,
    20.0,
    20.1367549896,
    20.272397995,
    20.4069366455,
    20.5403900146,
    20.6727542877,
    20.8043861389,
    20.9348545074,
    21.0643558502,
    21.1929149628,
    21.3205432892,
    21.4472694397,
    21.5731048584,
    21.6980705261,
    21.8221874237,
    21.9454669952,
    22.0679225922,
    22.1895656586,
    22.3104381561,
    22.4305419922,
    22.5498542786,
    22.668428421,
    22.7862815857,
    22.9034156799,
    23.0198459625,
    23.1355323792,
    23.2506599426,
    23.3650760651,
    23.4787807465,
    23.5917854309,
    23.7042369843,
    23.8161449432,
    23.9271965027,
    24.037858963,
    24.1478595734,
    24.2569828033,
    24.3660526276,
    24.4744338989,
    24.581905365,
    24.6890239716,
    24.7955989838,
    24.901758194,
    25.0071258545,
    25.1122531891,
    25.2165718079,
    25.3205299377,
    25.4239902496,
    25.5269584656,
    25.6294193268,
    25.7313499451,
    25.8327407837
};

static const Interpolator_data_1d p0_interpolator_bin4_data = {
    101,
    -5.00000018885,
    5.00000018885,
    p0_interpolator_bin4_data_data
};

static double p0_interpolator_bin4(double xvariable)
{
    return linear_interpolate_1d(&p0_interpolator_bin4_data, (float)xvariable);
}

static const float p0_interpolator_bin5_data_data[101] = {
    10.0244760513,
    10.4081087112,
    10.7620420456,
    11.0924301147,
    11.4048976898,
    11.7014722824,
    11.9852790833,
    12.2577514648,
    12.5204257965,
    12.7743473053,
    13.0204954147,
    13.2595796585,
    13.4923019409,
    13.7191343307,
    13.9405899048,
    14.1570987701,
    14.3689451218,
    14.5764703751,
    14.780049324,
    14.9798240662,
    15.1759872437,
    15.3688030243,
    15.5585699081,
    15.7450857162,
    15.9288244247,
    16.1098613739,
    16.2883758545,
    16.464132309,
    16.6378097534,
    16.8090457916,
    16.9780693054,
    17.1450576782,
    17.3099594116,
    17.472990036,
    17.6341228485,
    17.7934207916,
    17.9509620667,
    18.1068172455,
    18.2610149384,
    18.4136428833,
    18.5647220612,
    18.7143115997,
    18.8624515533,
    19.0091819763,
    19.1545448303,
    19.2986984253,
    19.4413471222,
    19.5827980042,
    19.7230415344,
    19.8621044159,
    20.0,
    20.1367530823,
    20.2723846436,
    20.4069061279,
    20.5403385162,
    20.6729278564,
    20.8043842316,
    20.9348545074,
    21.0643558502,
    21.1929149628,
    21.3205432892,
    21.4472694397,
    21.5731067657,
    21.6980667114,
    21.8221817017,
    21.9454650879,
    22.0679264069,
    22.1895828247,
    22.3104457855,
    22.4305343628,
    22.5498523712,
    22.6684703827,
    22.786277771,
    22.9034385681,
    23.0198574066,
    23.1355648041,
    23.2505569458,
    23.3650302887,
    23.4788665771,
    23.5919742584,
    23.7042732239,
    23.8158950806,
    23.9274215698,
    24.03789711,
    24.147731781,
    24.2572956085,
    24.3660411835,
    24.4742355347,
    24.5819072723,
    24.6890468597,
    24.7955932617,
    24.9016208649,
    25.0071163177,
    25.1121082306,
    25.2163600922,
    25.3207263947,
    25.4238128662,
    25.5269508362,
    25.6294155121,
    25.7314186096,
    25.8329429626
};

static const Interpolator_data_1d p0_interpolator_bin5_data = {
    101,
    -5.00000018885,
    5.00000018885,
    p0_interpolator_bin5_data_data
};

static double p0_interpolator_bin5(double xvariable)
{
    return linear_interpolate_1d(&p0_interpolator_bin5_data, (float)xvariable);
}

double p0_interpolator(const double jes, const double eta)
{
    const double aeta = fabs(eta);
    if (aeta < 0.2)
        return p0_interpolator_bin1(jes);
    else if (aeta < 0.6)
        return p0_interpolator_bin2(jes);
    else if (aeta <= 0.9)
        return p0_interpolator_bin3(jes);
    else if (aeta <= 1.4)
        return p0_interpolator_bin4(jes);
    else
        return p0_interpolator_bin5(jes);
}

double p0_interpolator_hs_fcn(double x, double y, double z, int mode,
                              const double *pars, int *ierr);

double p0_interpolator_hs_fcn(double x, double y, double z, int mode,
                              const double *pars, int *ierr)
{
    *ierr = 0;
    return p0_interpolator(x, y);
}

#ifdef __cplusplus
}
#endif
