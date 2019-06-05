const int NP0402[47]={
  1,  2,  3,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  3,  3,  2,  2,  1,  1};
const double ETA0402[47]={
-3.271,-2.719,-2.453, -2.21,-2.013,-1.851,
-1.709,-1.579,-1.466,-1.368,-1.261, -1.15,
-1.047,-0.9549,-0.8681,-0.7711,-0.6688,-0.565,
-0.4411,-0.3215,-0.1836,-0.0969,-0.0503,-0.001,
0.0493,0.0982,0.1851,0.3212,0.4415,0.5664,
0.6693,0.7732,0.8671,0.9582, 1.049, 1.152,
 1.263, 1.369, 1.466, 1.577, 1.708, 1.853,
 2.013, 2.211, 2.452, 2.724, 3.271};
const double Y0402[47][4]={
{0.9829  ,0.0  ,0.0  ,0.0},
{0.9912, 1.071  ,0.0  ,0.0},
{ 1.006,0.9405,0.9458  ,0.0},
{0.9926, 1.013, 1.006,0.9638},
{ 1.002,0.9784,0.9435, 0.936},
{ 1.002,0.9774,0.9592,0.9433},
{0.9932,     1, 1.005,0.9992},
{ 1.003,0.9793,0.9933,0.9452},
{0.9972, 1.006,     1,0.9675},
{0.9975, 1.005,     1,0.9893},
{ 0.994, 1.017, 0.997,0.9813},
{0.9869,0.9897,0.9925, 1.005},
{0.9873,0.9947, 1.022, 1.011},
{ 0.997, 1.003,0.9671, 1.007},
{0.9978, 1.023, 1.057, 1.019},
{0.9957, 1.029, 1.056, 1.016},
{ 0.997,  1.01,0.9988, 1.011},
{0.9984,0.9793,  1.01, 1.012},
{0.9986,0.9662,0.9759,0.9872},
{0.9977,0.9742,0.9725, 1.001},
{0.9989,0.9713,0.9956,0.9897},
{ 1.002, 1.001,0.9877, 1.015},
{0.9946, 1.031, 1.017,0.9722},
{0.9932,0.9519,0.9261,0.9398},
{0.9927,  0.99, 1.011, 1.018},
{ 1.002, 1.017,0.9821, 1.024},
{ 0.997, 1.015,  1.02,     1},
{0.9986,0.9955,0.9994, 1.004},
{     1, 1.022, 1.004,0.9874},
{0.9941,     1, 1.015, 1.021},
{ 1.001, 1.016,0.9999, 1.004},
{0.9926, 1.011, 1.035, 1.025},
{ 1.003, 1.002, 1.004, 1.016},
{0.9901, 1.048, 1.064, 1.036},
{0.9934, 0.982,0.9772, 1.005},
{0.9871,0.9763,0.9911,0.9908},
{0.9917, 1.017, 1.003, 0.992},
{ 0.998, 1.032,0.9923,0.9699},
{0.9981,0.9896, 1.006,0.9648},
{0.9991, 1.012,0.9858,0.9542},
{0.9953, 1.006, 1.007,0.9644},
{ 1.001,0.9957, 0.978  ,0.0},
{0.9951,0.9742,0.9837  ,0.0},
{0.9957,0.9885  ,0.0  ,0.0},
{0.9972,0.9864  ,0.0  ,0.0},
{0.9875  ,0.0  ,0.0  ,0.0},
{0.9703  ,0.0  ,0.0  ,0.0}
};
const double E0402[47][4]={
{0.0435  ,0.0  ,0.0  ,0.0},
{0.0186,0.0372  ,0.0  ,0.0},
{0.0126, 0.013,0.0279  ,0.0},
{0.0111,0.0122,0.0224,0.0176},
{0.0113,0.0103,0.0154,0.0121},
{0.0117,0.0107,0.0144,0.0118},
{0.0108,0.0099,0.0153,0.0088},
{0.0114, 0.011,0.0185,0.0087},
{0.0117,0.0108,0.0156,0.0083},
{0.0142,0.0119,0.0187,0.0092},
{0.0118,0.0116,0.0147,0.0078},
{0.0152,0.0147,0.0183,0.0102},
{ 0.015, 0.014,0.0208,0.0105},
{ 0.016,0.0147,0.0183,0.0099},
{0.0132,0.0124,0.0187,0.0087},
{ 0.012,0.0115,0.0168,0.0072},
{0.0115,0.0108,0.0132,0.0064},
{0.0138,0.0126,0.0177,0.0078},
{0.0149,0.0133,0.0178,0.0092},
{0.0148,0.0119,0.0155,0.0083},
{0.0125,0.0106,0.0136,0.0067},
{0.0142, 0.015,0.0199,0.0091},
{ 0.022,0.0202,0.0321,0.0142},
{0.0167,0.0189,0.0253,0.0127},
{0.0274,0.0209, 0.031,0.0149},
{0.0151,0.0138, 0.016,0.0089},
{0.0115, 0.011,0.0136,0.0066},
{0.0142,0.0124,0.0174,0.0082},
{0.0154,0.0152,0.0181,0.0086},
{0.0147,0.0129,0.0185,0.0091},
{0.0121,0.0109,0.0152,0.0069},
{0.0128,0.0115,0.0159, 0.008},
{0.0134,0.0121,0.0149,0.0084},
{0.0161,0.0152,0.0235,0.0102},
{ 0.015,0.0132,0.0189,0.0106},
{ 0.015,0.0138,0.0189,0.0109},
{0.0126,0.0108,0.0171,0.0087},
{0.0136,0.0138,0.0183, 0.009},
{0.0111,0.0118,0.0155,0.0078},
{0.0116,0.0104,0.0151,0.0085},
{0.0108,0.0104,0.0166,0.0088},
{0.0107,0.0114,0.0156  ,0.0},
{0.0109,0.0108,0.0138  ,0.0},
{0.0115,0.0132  ,0.0  ,0.0},
{0.0124,0.0159  ,0.0  ,0.0},
{0.0182  ,0.0  ,0.0  ,0.0},
{0.0451  ,0.0  ,0.0  ,0.0}
};
const int NP0702[47]={
  1,  2,  2,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  3,  2,  2,  2,  1,  1};
const double ETA0702[47]={
 -3.12,-2.747,-2.444,-2.214,-2.015,-1.852,
-1.708,-1.581,-1.465,-1.367,-1.261,-1.151,
-1.047,-0.9559,-0.8688,-0.7717,-0.6681,-0.5672,
-0.443,-0.3229,-0.1851,-0.0975,-0.049,-0.0015,
0.0477,0.0989,0.1849,0.3227,0.4432,0.5623,
0.6688,0.7714,0.8678,0.9571,  1.05,  1.15,
 1.262, 1.366, 1.466, 1.579,  1.71, 1.855,
 2.015, 2.214, 2.443, 2.758, 3.134};
const double Y0702[47][4]={
{0.9853  ,0.0  ,0.0  ,0.0},
{  1.01, 1.029  ,0.0  ,0.0},
{ 1.004,0.9929  ,0.0  ,0.0},
{ 1.003, 1.004,     1,0.9407},
{ 1.005,0.9917,0.9696,0.9473},
{ 1.002,0.9857,0.9707,0.9469},
{ 1.006, 0.976,0.9788, 0.971},
{     1,0.9929, 1.008,0.9719},
{ 1.005, 1.004, 1.008,0.9772},
{     1,  1.01, 1.032,0.9822},
{ 1.005, 1.009, 1.013, 0.979},
{0.9948, 1.006, 1.022, 1.028},
{0.9957, 1.013, 1.029, 1.042},
{ 1.003,0.9935,0.9929, 1.033},
{ 1.002, 1.017, 1.045,  1.03},
{ 1.002, 1.012, 1.032, 1.023},
{ 1.002,  1.01,0.9896,  1.02},
{ 1.001, 0.992, 1.009, 1.014},
{ 1.006,0.9748,0.9652,0.9863},
{ 1.003, 1.013, 1.009,0.9921},
{0.9982,0.9919, 1.033, 1.007},
{  1.01,0.9905, 1.014, 1.014},
{0.9979, 1.044, 1.022, 1.008},
{ 1.005,0.9659,0.9719,0.9827},
{0.9979, 1.018, 1.021, 1.029},
{ 1.007, 1.018, 0.978,0.9939},
{ 1.001,0.9999,0.9998,0.9944},
{ 1.004,0.9867, 1.018, 1.004},
{ 1.002, 1.019, 1.008,0.9982},
{ 1.002, 1.013,0.9842, 1.013},
{ 1.002, 1.006, 1.004,  1.01},
{ 1.003, 1.021, 1.023, 1.028},
{ 1.001, 1.035, 1.043, 1.045},
{0.9993, 1.014, 1.043, 1.037},
{ 1.001,0.9935, 1.005, 1.014},
{ 0.992, 1.007, 1.023, 1.006},
{ 1.004, 1.002,0.9817,0.9865},
{ 1.002,0.9901, 1.017,0.9907},
{ 1.002,0.9912, 1.001, 0.981},
{ 1.004,  1.01,0.9883,0.9578},
{     1, 1.002, 0.986,0.9637},
{ 1.005, 0.999,0.9775  ,0.0},
{     1,0.9663  ,0.0  ,0.0},
{ 1.003, 1.003  ,0.0  ,0.0},
{0.9977,0.9916  ,0.0  ,0.0},
{ 1.008  ,0.0  ,0.0  ,0.0},
{0.9875  ,0.0  ,0.0  ,0.0}
};
const double E0702[47][4]={
{0.0287  ,0.0  ,0.0  ,0.0},
{0.0111, 0.039  ,0.0  ,0.0},
{0.0086,0.0157  ,0.0  ,0.0},
{0.0081,0.0108,0.0221,0.0242},
{0.0085,0.0097,0.0164,0.0127},
{0.0082,0.0102,0.0149,0.0125},
{0.0087, 0.009,0.0131,0.0094},
{0.0087,0.0102,0.0158,0.0086},
{0.0092,0.0101,0.0174,0.0087},
{0.0104, 0.013,0.0172,0.0086},
{0.0096,0.0103,0.0141, 0.007},
{0.0113,0.0106,0.0158,0.0085},
{0.0126,0.0126,0.0164,0.0092},
{0.0121,0.0126,0.0151,0.0082},
{0.0103,0.0119,0.0169, 0.008},
{0.0096,0.0101,0.0139,0.0066},
{0.0086,0.0091,0.0143,0.0062},
{0.0108,0.0119,0.0145,0.0075},
{0.0115,0.0126,0.0173,0.0078},
{0.0109,0.0132,0.0156,0.0078},
{0.0093,  0.01,0.0146,0.0064},
{0.0108, 0.013,0.0164,0.0086},
{0.0162,0.0187,0.0226,0.0112},
{0.0132,0.0157,0.0206,0.0112},
{0.0161,  0.02,0.0267,0.0126},
{0.0117,0.0127,0.0166,0.0081},
{ 0.009,  0.01,0.0115,0.0059},
{0.0111,0.0122,0.0192,0.0076},
{0.0111,0.0134,0.0157,0.0075},
{0.0114,0.0115,0.0161,0.0077},
{0.0089,0.0094,0.0132,0.0064},
{0.0098,0.0105,0.0144,0.0072},
{0.0105,0.0116,0.0134,0.0077},
{0.0127,0.0122,0.0205,0.0089},
{0.0118,0.0117,0.0172, 0.009},
{0.0116,0.0117,0.0186,0.0087},
{0.0094,0.0104,0.0145,0.0075},
{0.0107,0.0118,0.0154,0.0087},
{0.0091,0.0109,0.0146,0.0081},
{0.0085,0.0101,0.0129,0.0082},
{0.0085,0.0103,0.0132,0.0082},
{ 0.008,0.0101,0.0169  ,0.0},
{0.0081,0.0105  ,0.0  ,0.0},
{0.0081,0.0139  ,0.0  ,0.0},
{0.0088,0.0146  ,0.0  ,0.0},
{ 0.011  ,0.0  ,0.0  ,0.0},
{0.0266  ,0.0  ,0.0  ,0.0}
};
const int NP1002[47]={
  1,  2,  3,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  3,  3,  2,  2,  1,  1};
const double ETA1002[47]={
-3.086,-2.746,-2.455, -2.21, -2.02,-1.854,
-1.708,-1.581,-1.466,-1.367,-1.262,-1.151,
 -1.05,-0.9561,-0.8691,-0.7725,-0.6686,-0.565,
-0.441,-0.3233,-0.1842,-0.0975,-0.0483,0.0009,
0.0489,0.0982,0.1843,0.3218,0.4436,0.5621,
0.6683,0.7719,0.8694, 0.956, 1.048, 1.149,
 1.263, 1.366, 1.465, 1.583, 1.708, 1.855,
  2.02, 2.209, 2.454, 2.752, 3.097};
const double Y1002[47][4]={
{0.9835  ,0.0  ,0.0  ,0.0},
{ 1.002, 1.027  ,0.0  ,0.0},
{0.9962,0.9781,0.9967  ,0.0},
{0.9935,0.9876,0.9759,0.9347},
{0.9961,0.9729,0.9474,0.9288},
{0.9952,0.9805, 0.978,0.9347},
{0.9968,0.9851,0.9869, 0.953},
{0.9951, 1.001,0.9914,0.9687},
{0.9967,0.9816,0.9897,0.9704},
{0.9951,0.9923, 1.005, 0.975},
{0.9943,0.9823,0.9797,0.9645},
{0.9919,0.9627,0.9737,0.9779},
{0.9899, 1.005,0.9851,0.9954},
{ 0.995, 0.984,0.9969, 1.022},
{0.9946, 1.006, 1.026, 1.013},
{ 0.998,0.9959,0.9909, 1.013},
{0.9923, 1.016, 1.002, 1.013},
{0.9968,0.9921,  1.01, 1.008},
{0.9955,0.9823, 1.016, 1.018},
{ 0.994, 1.033, 1.022, 1.023},
{0.9953,0.9905, 1.011,0.9977},
{0.9985,0.9918, 1.009, 1.005},
{0.9932, 0.983,  1.01, 1.002},
{0.9939,0.9569, 0.957,0.9863},
{0.9933,0.9895,0.9982,0.9803},
{0.9973, 1.017,0.9801, 0.992},
{0.9935,0.9942,0.9959,0.9923},
{0.9986,  0.99,0.9909,0.9742},
{0.9938, 1.027,0.9974, 1.007},
{0.9961,0.9966,0.9965, 1.017},
{0.9938, 1.013,0.9992, 1.003},
{0.9964,     1, 1.011, 1.005},
{0.9958,0.9982, 1.017, 1.012},
{0.9928,0.9997,  1.02, 1.009},
{0.9933,0.9756,  1.01, 1.001},
{0.9908,0.9705,0.9639,0.9777},
{0.9946,0.9986, 0.997,0.9699},
{ 0.995,0.9779, 1.007,0.9681},
{0.9946, 1.007, 1.003, 0.972},
{0.9978,0.9925,0.9926,0.9503},
{ 0.994,0.9894,0.9638,0.9584},
{ 0.997,0.9953,0.9725  ,0.0},
{0.9955,0.9587,0.9777  ,0.0},
{0.9949,0.9869  ,0.0  ,0.0},
{0.9944,0.9677  ,0.0  ,0.0},
{ 1.004  ,0.0  ,0.0  ,0.0},
{0.9853  ,0.0  ,0.0  ,0.0}
};
const double E1002[47][4]={
{0.0222  ,0.0  ,0.0  ,0.0},
{0.0086,0.0299  ,0.0  ,0.0},
{0.0066,0.0132,0.0387  ,0.0},
{0.0061,0.0093,0.0177, 0.021},
{0.0062,0.0083,0.0148,0.0118},
{0.0066,0.0086,0.0134,0.0097},
{0.0065,0.0086, 0.011,0.0078},
{ 0.007,0.0082,0.0141,0.0075},
{0.0077,0.0084,0.0143,0.0071},
{0.0082,0.0097,0.0129,0.0076},
{0.0074, 0.009,0.0113,0.0065},
{0.0088,0.0093,0.0134,0.0075},
{0.0091,0.0107,0.0134,0.0077},
{0.0094,0.0096,0.0144,0.0073},
{0.0081,0.0099, 0.014,0.0065},
{0.0074,0.0088,0.0126,0.0057},
{0.0071,0.0083, 0.011,0.0053},
{ 0.009,0.0105,0.0147,0.0068},
{0.0094,0.0113, 0.014,0.0072},
{0.0091,0.0116,0.0138,0.0066},
{0.0071,0.0083,0.0115,0.0055},
{0.0088,0.0109,0.0132,0.0069},
{0.0122,0.0174,0.0195,0.0111},
{0.0092,0.0126,0.0186,0.0094},
{0.0134, 0.015,0.0197,0.0109},
{0.0089,0.0104,0.0141,0.0069},
{0.0072,0.0085,0.0111, 0.005},
{0.0085,0.0114,0.0152,0.0067},
{0.0096,0.0113,0.0154,0.0068},
{0.0088, 0.011,0.0135,0.0068},
{0.0076,0.0082,0.0113,0.0053},
{0.0074, 0.009,0.0117,0.0059},
{ 0.008,0.0097,0.0127,0.0065},
{0.0098,0.0114,0.0175,0.0074},
{0.0089,0.0101, 0.014,0.0076},
{0.0087,0.0096,0.0141,0.0077},
{0.0078,0.0086,0.0123,0.0063},
{0.0083,0.0099,0.0124,0.0073},
{0.0071,0.0095,0.0139,0.0068},
{0.0068,0.0085,0.0126,0.0071},
{0.0066,0.0084,0.0108,0.0076},
{0.0064,0.0085,0.0131  ,0.0},
{0.0059,0.0086,0.0136  ,0.0},
{0.0062,0.0101  ,0.0  ,0.0},
{0.0068,0.0136  ,0.0  ,0.0},
{0.0081  ,0.0  ,0.0  ,0.0},
{0.0208  ,0.0  ,0.0  ,0.0}
};



const int  NPETA02[3]={47,47,47};

const double SPLINTETA02[3][47]={
  {
 -3.2707, -2.7187, -2.4534, -2.2099, -2.0126, -1.8509, -1.7086, -1.5788,
 -1.4659, -1.3683, -1.2611, -1.1498, -1.0469, -0.9549, -0.8681, -0.7711,
 -0.6688,  -0.565, -0.4411, -0.3215, -0.1836, -0.0969, -0.0503,  -0.001,
  0.0493,  0.0982,  0.1851,  0.3212,  0.4415,  0.5664,  0.6693,  0.7732,
  0.8671,  0.9582,  1.0489,  1.1522,  1.2628,  1.3687,  1.4659,  1.5775,
  1.7076,  1.8532,  2.0126,  2.2113,   2.452,  2.7243,  3.2707  },
  {
 -3.1205,  -2.747, -2.4437, -2.2144, -2.0147, -1.8523, -1.7075, -1.5812,
 -1.4652,  -1.367, -1.2607, -1.1513, -1.0467, -0.9559, -0.8688, -0.7717,
 -0.6681, -0.5672,  -0.443, -0.3229, -0.1851, -0.0975,  -0.049, -0.0015,
  0.0477,  0.0989,  0.1849,  0.3227,  0.4432,  0.5623,  0.6688,  0.7714,
  0.8678,  0.9571,  1.0496,  1.1498,   1.262,  1.3656,  1.4659,  1.5788,
  1.7096,  1.8548,  2.0149,  2.2144,  2.4434,  2.7583,   3.134  },
  {
 -3.0864,  -2.746, -2.4546, -2.2102, -2.0199, -1.8542, -1.7078, -1.5806,
 -1.4658, -1.3667, -1.2623, -1.1513, -1.0502, -0.9561, -0.8691, -0.7725,
 -0.6686,  -0.565,  -0.441, -0.3233, -0.1842, -0.0975, -0.0483,  0.0009,
  0.0489,  0.0982,  0.1843,  0.3218,  0.4436,  0.5621,  0.6683,  0.7719,
  0.8694,   0.956,  1.0483,  1.1486,  1.2633,  1.3662,  1.4648,  1.5827,
   1.708,  1.8546,    2.02,  2.2088,  2.4541,  2.7521,  3.0968  }
};


const double SPLINTP002[3][47]={
  {
       0.9829,      0.90911,       1.0526,       1.0113,       1.0233,       1.0182,      0.99491,       1.0216,
       1.0204,        1.008,       1.0144,      0.97815,      0.98117,      0.98901,       1.0067,       1.0105,
       0.9962,      0.97959,      0.98217,      0.97401,      0.98605,      0.99152,       1.0326,      0.99704,
      0.97623,      0.99289,       1.0093,      0.99326,       1.0205,      0.98417,       1.0085,      0.98951,
      0.99438,       1.0045,      0.97729,      0.97905,       1.0093,       1.0317,        1.014,       1.0323,
       1.0197,       1.0137,       1.0006,       1.0031,       1.0083,       0.9875,      0.9703
  },
  {
       0.9853,      0.98956,       1.0144,       1.0212,       1.0263,       1.0211,       1.0094,        1.012,
       1.0189,       1.0157,       1.0223,      0.98403,      0.98159,      0.97866,      0.99698,       0.9987,
      0.99507,      0.99034,      0.99759,        1.014,      0.99375,        1.001,       1.0158,       0.9978,
      0.99195,       1.0162,        1.004,      0.99711,       1.0123,      0.99927,      0.99934,       1.0002,
      0.99698,      0.98946,      0.99003,      0.99435,       1.0103,       1.0041,       1.0084,       1.0293,
       1.0181,        1.018,       1.0348,       1.0038,        1.004,       1.0077,      0.9875
  },
  {
       0.9835,      0.97612,       1.0089,       1.0109,       1.0192,        1.017,        1.013,       1.0102,
       1.0031,        1.005,       1.0038,      0.98414,      0.99252,      0.97453,      0.99203,      0.98878,
      0.99222,      0.98979,      0.98052,      0.99642,      0.99344,      0.99346,      0.98751,      0.98425,
      0.99916,       1.0066,      0.99473,       1.0072,       1.0012,      0.98521,      0.99845,       0.9947,
      0.99046,      0.98954,      0.98322,      0.98674,         1.01,       1.0034,       1.0098,       1.0186,
        1.008,       1.0087,       1.0132,       1.0031,       1.0218,       1.0037,      0.9853
  }
};
const double SPLINTP102[3][47]={
  {
            0,     0.0026026,     -0.0016139,     -0.00024421,     -0.00074243,     -0.00062499,     4.9069e-05,     -0.0005981,
     -0.00039201,     -0.00013728,     -0.0002412,     0.00021007,     0.00026105,     0.00011358,     0.00014644,     9.1899e-05,
     0.00011482,     0.00025565,     1.047e-05,     0.0001807,     2.1053e-05,     0.00017156,     -0.00043263,     -0.00050781,
     0.00033444,     0.00021787,     -4.7809e-05,     8.2026e-05,     -0.00024779,     0.00029873,     -3.4099e-05,     0.0003129,
     0.00016094,     0.00031353,     0.00018657,     8.7793e-05,     -0.00010994,     -0.0004632,     -0.00036883,     -0.00059243,
     -0.00039488,     -0.00036254,     -0.00028986,     -0.00023453,     -0.00035179,            0,           0
  },
  {
            0,     0.00063844,     -0.00034528,     -0.00043229,     -0.00062425,     -0.00059036,     -0.00034555,     -0.00029434,
     -0.00030827,     -0.00021931,     -0.0003203,     0.00036444,     0.00049454,     0.00040009,     0.00029104,     0.00021218,
     0.00018423,     0.00018485,     -0.00012615,     -0.00015773,     0.00012219,     9.1488e-05,     -2.5785e-05,     -0.00016626,
     0.00030689,     -0.00018925,     -7.3636e-05,     5.1408e-05,     -9.8654e-05,     9.7247e-05,     8.3045e-05,     0.00023368,
     0.00041706,     0.00039754,     0.00017864,     0.00012074,     -0.00019996,     -8.8939e-05,     -0.00021054,     -0.00053888,
     -0.00041425,     -0.00038383,     -0.001101,     -1.3029e-05,     -0.0001987,            0,           0
  },
  {
            0,     0.00081433,     -0.0004177,     -0.00048227,     -0.00074771,     -0.00061989,     -0.00045195,     -0.00030208,
     -0.00026243,     -0.00021177,     -0.00031405,     -8.2972e-05,     2.3416e-05,     0.00035208,     0.00019105,     0.00017555,
     0.00017308,     0.0001496,     0.00029396,     0.00024014,     4.215e-05,     9.6923e-05,     0.0001191,     -4.7032e-05,
     -0.00013767,     -0.00011539,     -1.6582e-05,     -0.00026069,     5.7907e-05,     0.00024205,     4.7014e-05,     9.3348e-05,
     0.00017904,     0.00016664,     0.00013729,     -0.00010771,     -0.00029644,     -0.00025625,     -0.00026782,     -0.00051138,
     -0.000406,     -0.00032035,     -0.00065159,     -0.00026059,     -0.00086971,            0,           0
  }
};
