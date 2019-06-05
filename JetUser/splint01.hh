
const int BEGIN_RUN01=145671;
const int END_RUN01  =152274;

 const int NP01[3]={47,47,47};

const double X01[3][47]={
 {
  -3.271,  -2.714,  -2.452,  -2.207,  -2.013,  -1.853,  -1.709,  -1.579,
  -1.465,  -1.367,  -1.262,  -1.152,  -1.049, -0.9576, -0.8692, -0.7736,
 -0.6683,  -0.564,  -0.442, -0.3219, -0.1844, -0.0976, -0.0505,  0.0002,
  0.0498,  0.0976,  0.1847,  0.3222,  0.4444,  0.5637,  0.6681,  0.7717,
  0.8699,  0.9562,    1.05,   1.152,   1.262,   1.366,   1.466,    1.58,
   1.708,   1.852,   2.012,   2.209,   2.451,   2.718,   3.271
 },
 {
  -3.132,  -2.751,  -2.444,  -2.213,  -2.015,  -1.855,  -1.707,   -1.58,
  -1.465,  -1.367,  -1.261,  -1.151,  -1.047, -0.9565,  -0.868, -0.7742,
  -0.669, -0.5643, -0.4431, -0.3201, -0.1838, -0.0978, -0.0495,  0.0005,
  0.0491,   0.098,  0.1868,  0.3209,  0.4437,  0.5662,  0.6687,  0.7727,
  0.8695,  0.9568,    1.05,   1.151,   1.259,   1.367,   1.464,   1.579,
   1.708,   1.854,   2.014,   2.212,   2.445,   2.752,   3.122
 },
 {
  -3.092,  -2.751,  -2.456,   -2.21,  -2.018,  -1.856,  -1.709,  -1.581,
  -1.466,  -1.366,  -1.262,   -1.15,   -1.05, -0.9569, -0.8687, -0.7728,
 -0.6694, -0.5649, -0.4437, -0.3215, -0.1852, -0.0983, -0.0481,  0.0004,
  0.0487,  0.0984,  0.1852,  0.3227,  0.4456,  0.5655,  0.6689,  0.7721,
  0.8689,   0.956,    1.05,    1.15,   1.261,   1.366,   1.465,   1.579,
   1.707,   1.855,   2.017,   2.211,   2.456,   2.751,   3.093
  }
};
const double Y01[3][47]={
 {
  0.5747,  0.7736,  0.9138,  0.9529,  0.9893,   1.016,   1.018,   1.023,
  0.9892,  0.9457,  0.8658,  0.8023,  0.8413,  0.8869,  0.9394,  0.9702,
  0.9979,   1.025,   1.005,   1.028,  0.9916,  0.9598,  0.9369,  0.8673,
  0.9151,   0.953,  0.9929,  0.9915,   0.966,  0.9857,  0.9749,  0.9754,
  0.9284,  0.8759,  0.8131,  0.7973,  0.8636,  0.9419,  0.9839,   1.017,
   1.026,   1.016,   1.004,  0.9508,  0.9262,  0.7828,  0.6057
 },
 {
  0.8129,  0.9299,   0.969,    1.01,   1.028,   1.034,   1.028,    1.01,
  0.9815,  0.9607,  0.9059,  0.8514,  0.8663,  0.9105,  0.9464,  0.9835,
  0.9849,  0.9982,   1.005,   1.017,  0.9994,  0.9709,  0.9663,  0.9107,
  0.9523,  0.9817,  0.9858,   1.007,  0.9873,  0.9784,  0.9759,  0.9645,
  0.9453,  0.9038,  0.8583,  0.8548,  0.8891,  0.9609,  0.9996,   1.017,
   1.027,    1.03,   1.036,   1.007,  0.9683,  0.9335,  0.8104
 },
 {
  0.8597,   0.967,   1.009,   1.023,   1.024,   1.031,   1.037,   1.021,
   1.006,  0.9651,   0.929,  0.8844,  0.8962,  0.9214,  0.9555,  0.9739,
   1.004,   1.008,   0.997,   1.011,  0.9931,  0.9799,  0.9653,  0.9523,
  0.9695,  0.9936,  0.9948,  0.9972,  0.9892,  0.9944,  0.9843,  0.9644,
  0.9488,  0.9156,  0.8854,  0.8799,   0.928,  0.9649,   1.012,   1.022,
   1.038,   1.035,   1.043,   1.029,   1.012,  0.9474,   0.875
  }
};
const double E01[3][47]={
 {
  0.0213,  0.0123,  0.0096,  0.0089,  0.0089,  0.0089,  0.0086,  0.0089,
  0.0093,   0.011,  0.0085,  0.0105,  0.0101,  0.0112,  0.0098,  0.0098,
  0.0094,  0.0127,  0.0123,  0.0124,   0.009,  0.0112,  0.0181,  0.0139,
  0.0171,  0.0117,   0.009,  0.0117,  0.0118,  0.0111,  0.0095,    0.01,
  0.0105,  0.0111,  0.0097,  0.0101,  0.0087,  0.0102,  0.0092,  0.0094,
  0.0087,  0.0088,  0.0091,  0.0089,  0.0095,  0.0115,  0.0258
 },
 {
  0.0173,  0.0089,  0.0073,  0.0068,  0.0068,  0.0068,   0.007,  0.0072,
  0.0073,  0.0084,  0.0069,   0.008,  0.0079,  0.0089,  0.0079,   0.008,
  0.0075,  0.0093,  0.0092,  0.0093,  0.0072,  0.0085,  0.0138,    0.01,
  0.0133,  0.0092,  0.0073,  0.0093,  0.0088,   0.009,  0.0075,  0.0076,
  0.0079,  0.0091,   0.008,  0.0079,  0.0067,   0.008,  0.0076,  0.0074,
  0.0067,  0.0067,   0.007,  0.0069,  0.0069,  0.0089,  0.0171
 },
 {
  0.0147,  0.0067,  0.0053,  0.0054,  0.0049,  0.0051,  0.0054,  0.0055,
   0.006,  0.0061,  0.0056,  0.0063,  0.0063,  0.0068,  0.0062,  0.0062,
   0.006,  0.0073,  0.0075,  0.0075,  0.0058,  0.0073,  0.0102,  0.0077,
  0.0105,  0.0073,  0.0058,  0.0072,  0.0072,  0.0074,  0.0058,  0.0059,
  0.0065,  0.0069,  0.0063,  0.0061,  0.0057,  0.0064,  0.0061,  0.0056,
  0.0053,  0.0051,  0.0049,  0.0053,  0.0053,  0.0068,  0.0158
  }
};
const double Y201[3][47]={
 {
   1.679,  0.4936,  -2.571,  0.8998,  0.0556,  -2.058,   2.038,  -4.839,
  0.3688,   -4.78,   0.113,   14.06,  -2.675,   3.521,   -5.22, -0.1838,
   2.057,   -8.28,   8.236,  -7.126, -0.4426,   6.644,  -52.18,      90,
  -27.99, -0.7602,  -5.335,  -2.515,   6.937,  -6.707,    5.43,  -8.807,
  0.9125,  -2.903,   6.285,   8.941,   0.983,  -5.057, -0.0486,  -2.285,
  -1.416,   1.091,  -2.669,   2.672,  -3.303,   0.374,   2.842
 },
 {
   3.395,  -1.952,   1.023, -0.8532, -0.0948,  -0.575,  -0.629,  -1.606,
   2.067,   -4.88, -0.5101,   8.091,    3.87,  -2.982,   2.467,  -7.423,
   4.046,  -2.246,    1.47,  -1.583,    -5.5,   17.02,  -55.61,    80.3,
   -25.6,  -9.154,   4.988,  -5.573,   2.388,   0.299,  -1.331,  0.0526,
  -4.281, -0.9895,   6.801,   1.953,   5.558,  -4.847,   -2.26,   -0.23,
 -0.7644,   0.833,  -1.792,  0.0982,  0.7936,  -1.888,   2.701
 },
 {
   3.679,  -1.835,  0.1496, -0.5068,  0.3893,  0.4207,  -2.251,   1.588,
  -4.961,   3.019,  -3.552,   8.319, -0.5589,   3.248,  -4.891,   3.607,
  -3.829,  -2.132,   4.232,  -4.355,   1.683,  -3.004,  -3.309,   19.57,
   2.192,  -11.76,   3.036,  -2.336,   2.488,   -2.26,  -1.309,   1.921,
  -4.489,   1.599,    1.79,   7.586,  -4.212,    4.74,  -7.352,   2.852,
  -2.648,   1.688,  -1.544,  0.5521, -0.8023, -0.4619,   2.516
  }
};
