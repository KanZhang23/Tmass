
const int   BEG_RUN02V3=152275;
const int   END_RUN02V3=156487;

const int NP02V3[3]={47,47,47};

const double X02V3[3][47]={
 {
   -3.27,   -2.72,  -2.454,  -2.212,  -2.013,  -1.853,  -1.709,  -1.579,
  -1.465,  -1.368,  -1.262,  -1.151,  -1.047, -0.9565, -0.8691, -0.7731,
 -0.6691, -0.5655, -0.4429, -0.3219, -0.1842, -0.0974, -0.0492,  0.0003,
  0.0494,  0.0983,  0.1871,  0.3222,  0.4438,  0.5648,  0.6695,  0.7732,
  0.8677,  0.9575,   1.047,   1.151,   1.262,   1.367,   1.466,   1.579,
    1.71,   1.853,   2.013,   2.213,   2.452,    2.72,   3.271
 },
 {
   -3.12,  -2.756,  -2.447,  -2.216,  -2.015,  -1.855,  -1.708,  -1.581,
  -1.464,  -1.367,  -1.262,  -1.152,  -1.048,  -0.956, -0.8685, -0.7734,
 -0.6699, -0.5659, -0.4431, -0.3224, -0.1853, -0.0983, -0.0484,  0.0005,
  0.0488,  0.0988,  0.1855,  0.3193,   0.441,  0.5643,  0.6691,  0.7727,
  0.8692,  0.9576,   1.049,   1.151,   1.264,   1.366,   1.466,   1.581,
   1.709,   1.854,   2.015,   2.215,   2.448,   2.755,   3.125
 },
 {
  -3.091,  -2.752,  -2.458,  -2.212,  -2.019,  -1.856,  -1.709,   -1.58,
  -1.464,  -1.366,  -1.262,  -1.151,  -1.048, -0.9569, -0.8687, -0.7731,
 -0.6698, -0.5651, -0.4417, -0.3216, -0.1841, -0.0981, -0.0488,  0.0003,
   0.049,  0.0985,  0.1857,  0.3217,  0.4424,  0.5652,  0.6696,  0.7728,
  0.8692,  0.9567,   1.049,   1.151,   1.263,   1.367,   1.466,   1.581,
    1.71,   1.856,   2.019,   2.211,   2.457,   2.751,   3.089
  }
};
const double Y02V3[3][47]={
 {
  0.7765,  0.9755,   1.107,   1.127,   1.138,   1.147,   1.134,   1.119,
   1.085,    1.02,  0.9312,  0.8386,  0.8553,  0.9321,  0.9553,  0.9811,
  0.9768,   1.009,   1.011,  0.9912,   1.014,   1.001,  0.9193,  0.8729,
  0.9098,   0.977,  0.9906,  0.9926,  0.9966,  0.9757,  0.9746,  0.9647,
  0.9485,  0.9052,  0.8677,  0.8314,  0.9111,  0.9756,    1.04,   1.086,
   1.106,   1.113,    1.09,   1.093,   1.073,  0.9653,  0.7084
 },
 {
  0.9736,   1.137,   1.149,   1.172,   1.167,   1.147,   1.134,   1.109,
   1.075,   1.033,  0.9585,  0.8929,  0.8791,  0.9467,  0.9721,  0.9865,
  0.9919,  0.9957,   1.014,   1.002,   1.007,  0.9815,  0.9421,  0.9332,
  0.9245,   0.981,   1.002,   1.008,  0.9887,  0.9765,  0.9733,  0.9729,
  0.9661,  0.9291,  0.8915,  0.8758,  0.9399,   1.001,   1.043,   1.083,
   1.106,   1.117,   1.134,   1.131,   1.122,   1.095,  0.9179
 },
 {
   1.046,   1.153,   1.169,   1.168,   1.167,   1.147,   1.129,     1.1,
   1.086,   1.038,  0.9814,   0.925,  0.9157,  0.9622,  0.9668,  0.9917,
  0.9874,   1.011,   1.004,  0.9999,   1.002,  0.9815,   0.965,  0.9413,
  0.9635,  0.9668,  0.9924,   1.004,       1,   0.974,  0.9748,   0.978,
  0.9712,  0.9471,  0.9166,  0.9022,  0.9611,   1.012,   1.043,   1.083,
   1.096,   1.121,   1.136,   1.132,   1.136,   1.108,  0.9703
  }
};
const double E02V3[3][47]={
 {
  0.0187,    0.01,  0.0079,  0.0074,  0.0073,  0.0075,  0.0073,  0.0077,
  0.0078,  0.0086,  0.0073,  0.0084,  0.0081,  0.0094,  0.0083,  0.0075,
  0.0072,  0.0097,  0.0101,  0.0089,  0.0077,  0.0093,  0.0134,  0.0102,
  0.0142,  0.0096,  0.0074,  0.0095,  0.0098,  0.0092,  0.0072,  0.0079,
  0.0084,  0.0091,  0.0084,  0.0082,  0.0077,   0.008,  0.0075,  0.0076,
  0.0074,  0.0073,  0.0072,  0.0075,  0.0079,  0.0097,  0.0174
 },
 {
  0.0133,  0.0068,  0.0057,  0.0054,  0.0057,  0.0057,  0.0057,  0.0059,
  0.0061,  0.0068,   0.006,  0.0069,  0.0067,  0.0074,  0.0067,  0.0062,
  0.0059,  0.0073,  0.0078,  0.0074,  0.0059,  0.0074,  0.0103,  0.0086,
  0.0107,  0.0077,   0.006,  0.0075,   0.008,   0.007,  0.0058,  0.0062,
  0.0069,  0.0076,  0.0069,  0.0067,  0.0061,  0.0068,  0.0061,  0.0059,
  0.0057,  0.0055,  0.0055,  0.0056,  0.0058,  0.0066,  0.0139
 },
 {
  0.0116,  0.0054,  0.0045,  0.0042,  0.0043,  0.0045,  0.0046,  0.0047,
  0.0052,  0.0056,  0.0049,  0.0055,  0.0056,  0.0061,  0.0054,  0.0052,
  0.0049,  0.0063,  0.0066,  0.0063,  0.0049,  0.0061,  0.0091,  0.0063,
  0.0093,  0.0061,  0.0049,  0.0063,  0.0064,  0.0061,   0.005,  0.0053,
  0.0055,  0.0064,  0.0055,  0.0055,  0.0051,  0.0057,   0.005,  0.0049,
  0.0046,  0.0045,  0.0042,  0.0043,  0.0045,  0.0053,  0.0136
  }
};
const double Y202V3[3][47]={
 {
   1.822,  0.2956,    -2.6,  0.4339,  0.2818,   -1.64,  0.4626,  -1.284,
  -4.554,  -1.032,  -2.569,   11.91,   10.65,  -13.89,   5.498,  -8.159,
   8.468,  -5.351,  -1.788,   3.569,   2.408,  -39.22,   22.39,   41.69,
   16.35,   -30.9,   4.313, -0.1271,  -3.402,    3.61,   -2.36,  0.9407,
  -6.208,   3.782,  -4.621,   17.87,  -6.918,   3.387,  -3.824,  -2.192,
  0.0994,  -2.543,   2.096, -0.7502,  -1.425,   -1.02,   3.006
 },
 {
   5.535,   -3.69,     1.6,  -1.055, -0.8011,  0.9461,  -1.399, -0.5308,
 -0.7679,  -4.573,   2.157,   2.223,   15.35,   -11.5,   0.855,  -1.547,
  -0.546,   2.773,  -4.484,   3.317,  -2.605,  -14.97,   27.22,  -19.88,
   52.67,  -30.64,    4.62,  -4.023,   1.639,  0.4067,  0.1754,  0.4471,
  -6.123,   1.414,  0.9192,   10.35,  -1.865,  -2.049, -0.0168,   -1.92,
 -0.8077,  0.7023,  -1.091, -0.1183,   0.617,  -2.912,   3.753
 },
 {
   4.014,  -2.404,  0.3216,  0.1323,  -1.159,  0.7279,  -1.984,    3.21,
  -6.233,  0.9375, -0.6455,   3.333,   11.35,   -12.6,   8.651,  -8.589,
   7.474,  -5.867,   1.521,   1.214,  -3.977,    1.66,  -14.48,   38.35,
   -24.1,   10.97,  -5.106,  0.6665,  -3.306,   3.894, -0.4781, -0.6362,
  -3.125,  -0.247,  0.3785,   9.878,  -2.445,  -2.445,   2.094,  -4.017,
   1.958,  -1.034,   -0.84,  0.4759, -0.0768,  -2.442,   3.703
  }
};
