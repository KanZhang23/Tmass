const int NP0401V2[47]={
  1,  3,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  3,  3,  2,  1,  1};
const double ETA0401V2[47]={
-3.271,-2.714,-2.452,-2.207,-2.013,-1.853,
-1.709,-1.579,-1.465,-1.367,-1.262,-1.152,
-1.049,-0.9577,-0.8692,-0.7736,-0.6684,-0.5649,
-0.4436,-0.323,-0.1846,-0.0977,-0.0505,0.0001,
0.0498,0.0977,0.1852,0.3212,0.4438, 0.564,
0.6681,0.7716,0.8699,0.9562,  1.05, 1.152,
 1.262, 1.366, 1.466,  1.58, 1.708, 1.852,
 2.012, 2.209, 2.451, 2.718, 3.271};
const double Y0401V2[47][4]={
{0.9909  ,0.0  ,0.0  ,0.0},
{0.9895,  1.08,0.9701  ,0.0},
{0.9929,0.9995,0.9783,0.9533},
{0.9928,0.9855,0.9797,0.9359},
{0.9949, 0.967,0.9724,0.9508},
{0.9892,0.9734,0.9517, 0.941},
{0.9917,0.9906,0.9671,0.9662},
{ 0.994,0.9701,0.9782,0.9484},
{0.9886,0.9973,0.9964,0.9778},
{0.9926, 0.994,0.9735,0.9781},
{ 0.991,0.9982,0.9776,0.9836},
{0.9942,0.9763,0.9915,0.9968},
{0.9972, 0.984,0.9835,0.9909},
{0.9957, 1.017, 1.016, 1.044},
{0.9952, 1.018, 1.015, 1.026},
{0.9882, 1.011, 1.016, 1.008},
{ 0.997,0.9877,0.9822,0.9976},
{0.9906,0.9995,0.9914,0.9742},
{0.9978,0.9724, 1.003,0.9808},
{0.9988,0.9879,0.9842,0.9851},
{ 0.992, 1.007, 1.016, 1.014},
{0.9881, 1.025, 1.038, 1.045},
{ 1.008,0.9436,0.9506,0.9599},
{0.9856,0.9797,0.9565,0.9302},
{ 1.002, 0.989,0.9511,0.9924},
{0.9891, 1.008, 1.048, 1.029},
{0.9872, 1.022, 1.013, 1.008},
{0.9934,0.9979, 1.005,  1.01},
{ 0.989, 1.018, 1.005,0.9987},
{0.9868,0.9966,  1.02, 1.015},
{0.9951,0.9968,  1.01, 1.006},
{0.9971, 1.011,0.9708, 1.017},
{  0.99, 1.027, 1.024, 1.046},
{0.9957, 1.009, 1.022, 1.065},
{0.9916, 1.012, 1.024, 1.044},
{ 0.993,0.9978,0.9665,0.9976},
{0.9856, 1.008, 1.012,     1},
{0.9903,0.9984, 1.016,  0.99},
{0.9932,0.9944,0.9877,0.9896},
{0.9923,0.9897,0.9655,0.9706},
{0.9902,0.9884,0.9771,  0.96},
{0.9912, 0.973, 0.959,0.9744},
{0.9969,0.9647,0.9807  ,0.0},
{ 0.991,0.9701,0.9504  ,0.0},
{0.9947,0.9805  ,0.0  ,0.0},
{0.9941  ,0.0  ,0.0  ,0.0},
{0.9925  ,0.0  ,0.0  ,0.0}
};
const double E0401V2[47][4]={
{0.0378  ,0.0  ,0.0  ,0.0},
{0.0157,0.0327,0.0485  ,0.0},
{0.0105,0.0163,0.0261,0.0377},
{0.0094,0.0114,0.0153,0.0221},
{ 0.009,0.0107, 0.012,0.0108},
{0.0088,0.0102,0.0102,0.0088},
{0.0084,0.0103,0.0098,0.0077},
{0.0088,  0.01,0.0099,0.0079},
{0.0094,0.0111,0.0109,0.0075},
{0.0115,0.0112, 0.012,0.0083},
{0.0098,0.0101,0.0111,0.0081},
{ 0.013,0.0139,0.0134,0.0103},
{ 0.012,0.0147, 0.013,0.0098},
{0.0125,0.0138,0.0144,0.0104},
{0.0103,0.0128,0.0122,0.0083},
{  0.01, 0.012,0.0112,0.0068},
{0.0094,0.0103,0.0106,0.0062},
{0.0123,0.0123,0.0128,0.0076},
{0.0119,0.0132,0.0125,0.0077},
{0.0117,0.0143,0.0118,0.0076},
{0.0091,0.0104,0.0098,0.0063},
{0.0115,0.0141, 0.013,0.0085},
{0.0195,0.0216,0.0202,0.0134},
{0.0159,0.0161,0.0206,0.0129},
{0.0187,  0.02,0.0229,0.0143},
{0.0122,0.0145,0.0144,0.0083},
{ 0.009,0.0104,0.0101,0.0064},
{0.0119,0.0127,0.0123,0.0079},
{0.0121,0.0131,0.0154,0.0084},
{0.0114,0.0126,0.0134,0.0078},
{0.0098,0.0109,0.0107,0.0067},
{0.0103,0.0114,0.0106,0.0078},
{0.0112,0.0123,0.0124,0.0084},
{0.0125,0.0137,0.0147,0.0097},
{0.0117,0.0143,0.0158,0.0102},
{0.0126,0.0144,0.0132,0.0093},
{0.0099,0.0115,0.0119,0.0078},
{0.0107,0.0118,0.0132,0.0087},
{0.0093,0.0106,0.0101,0.0073},
{0.0093,0.0102,0.0097, 0.008},
{0.0085,0.0109,0.0097,0.0084},
{0.0086,0.0099,0.0104, 0.009},
{0.0091,0.0106,0.0136  ,0.0},
{0.0093,0.0107,0.0145  ,0.0},
{0.0103,0.0144  ,0.0  ,0.0},
{0.0148  ,0.0  ,0.0  ,0.0},
{0.0428  ,0.0  ,0.0  ,0.0}
};
const int NP0701V2[47]={
  1,  2,  3,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  3,  3,  2,  2,  1,  1};
const double ETA0701V2[47]={
-3.133,-2.751,-2.444,-2.213,-2.015,-1.855,
-1.707, -1.58,-1.465,-1.367,-1.261,-1.151,
-1.047,-0.9564,-0.868,-0.7743,-0.669,-0.566,
-0.4442,-0.3207,-0.1841,-0.0978,-0.0494,0.0005,
0.0491, 0.098,0.1863,0.3223,0.4438,0.5649,
0.6687,0.7727,0.8695,0.9569,  1.05, 1.151,
 1.259, 1.367, 1.464, 1.579, 1.708, 1.854,
 2.014, 2.212, 2.446, 2.752, 3.122};
const double Y0701V2[47][4]={
{0.9944  ,0.0  ,0.0  ,0.0},
{ 1.007, 1.066  ,0.0  ,0.0},
{0.9988, 1.007, 1.008  ,0.0},
{ 1.002,0.9792, 0.998,0.9581},
{ 1.002,0.9767, 0.984,0.9455},
{ 1.002,0.9736,0.9491,0.9492},
{ 1.002,0.9973,0.9717,0.9761},
{ 1.002, 1.004,0.9961,0.9804},
{0.9987, 1.021, 1.023,0.9969},
{ 1.001, 1.013,0.9879,0.9855},
{0.9984,0.9999,0.9979,0.9833},
{0.9937,0.9925, 1.001, 1.003},
{0.9953, 1.017, 1.022, 1.024},
{0.9988, 1.012, 1.031,  1.04},
{0.9975, 1.019, 1.038, 1.033},
{ 1.002, 1.004, 1.016, 1.011},
{0.9967, 1.011, 1.015,  1.01},
{ 1.002, 1.019, 1.003,0.9997},
{0.9967,     1,  0.98,0.9916},
{ 1.001,0.9747,0.9976,0.9761},
{ 1.001, 1.011, 1.001, 1.004},
{0.9971, 1.024, 1.027, 1.032},
{ 1.001,0.9936,0.9642,0.9761},
{0.9864,0.9696,0.9581,0.9897},
{0.9985,0.9602,0.9812, 0.979},
{ 1.001,0.9801, 1.012, 1.011},
{0.9969, 1.032, 1.016, 1.018},
{ 1.003,0.9898,0.9875,0.9997},
{0.9975, 1.017, 1.014, 1.014},
{ 1.001,     1,  1.02, 1.012},
{ 0.999, 1.004, 1.013, 1.017},
{0.9993, 1.016,0.9961,  1.04},
{     1, 1.031, 1.018, 1.035},
{0.9978,  1.03, 1.028, 1.052},
{0.9936, 1.021, 1.047, 1.041},
{0.9959,0.9822,0.9992, 1.015},
{0.9943, 1.016, 1.013, 1.021},
{     1,0.9915, 1.012,0.9949},
{     1, 1.008,0.9854,0.9857},
{0.9991,  0.99,0.9821,0.9806},
{0.9994,0.9913,  0.99,0.9656},
{0.9984,0.9797,0.9976  ,0.0},
{     1, 0.979,0.9821  ,0.0},
{0.9979,0.9652  ,0.0  ,0.0},
{0.9957,0.9809  ,0.0  ,0.0},
{ 1.003  ,0.0  ,0.0  ,0.0},
{0.9849  ,0.0  ,0.0  ,0.0}
};
const double E0701V2[47][4]={
{0.0215  ,0.0  ,0.0  ,0.0},
{0.0096,0.0305  ,0.0  ,0.0},
{0.0075,0.0136,0.0266  ,0.0},
{0.0068,0.0103,0.0169,0.0367},
{0.0067,0.0095,0.0126,0.0142},
{0.0066,0.0091,0.0103,0.0093},
{0.0068,0.0095,0.0096,0.0079},
{0.0071,0.0093,0.0099,0.0072},
{0.0075,0.0104,0.0111,0.0072},
{0.0087,0.0113,0.0105,0.0085},
{0.0076,0.0084,0.0093,0.0069},
{0.0093,0.0109,0.0118,0.0086},
{0.0091,0.0118,0.0114,0.0088},
{0.0098,0.0118,0.0118,0.0082},
{0.0084,0.0105,0.0101,0.0071},
{0.0081,0.0103,0.0096,0.0063},
{0.0076,0.0087,0.0092,0.0056},
{0.0095, 0.011,0.0102,0.0072},
{0.0095,0.0128,0.0108,0.0068},
{0.0091,0.0126,0.0101,0.0066},
{0.0072,0.0092, 0.009,0.0058},
{0.0088,0.0122,0.0115,0.0073},
{0.0144,0.0201,0.0179,0.0122},
{0.0109,0.0141,0.0141,0.0097},
{0.0139,0.0158,0.0193,0.0114},
{0.0094,0.0125,0.0123, 0.007},
{0.0074,0.0093,0.0093,0.0055},
{ 0.009,0.0119,0.0114,0.0066},
{0.0091,0.0115,0.0113,0.0073},
{ 0.009,0.0115,0.0114,0.0074},
{0.0077,0.0096,0.0087,0.0057},
{0.0079,  0.01,0.0098,0.0066},
{0.0085,0.0107,0.0105,0.0069},
{0.0101,0.0116,0.0119,0.0078},
{0.0093,0.0119,0.0134,0.0087},
{0.0092,0.0125,0.0109,0.0083},
{0.0075,0.0092,0.0097,0.0067},
{0.0084,0.0102,0.0116,0.0078},
{0.0076,  0.01,0.0092,0.0074},
{0.0073, 0.009,0.0095,0.0079},
{0.0065,0.0092,0.0096,0.0086},
{0.0065,0.0087,0.0112  ,0.0},
{0.0068,  0.01,0.0119  ,0.0},
{0.0069,0.0112  ,0.0  ,0.0},
{0.0071,0.0131  ,0.0  ,0.0},
{0.0095  ,0.0  ,0.0  ,0.0},
{ 0.021  ,0.0  ,0.0  ,0.0}
};
const int NP1001V2[47]={
  1,  2,  3,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  3,  3,  2,  2,  1,  1};
const double ETA1001V2[47]={
-3.092,-2.751,-2.456, -2.21,-2.018,-1.856,
-1.709,-1.581,-1.466,-1.366,-1.262, -1.15,
 -1.05,-0.9569,-0.8687,-0.7728,-0.6695,-0.5648,
-0.4435,-0.3221,-0.1841,-0.0983,-0.048,0.0003,
0.0487,0.0983,0.1863,0.3216,0.4459,0.5655,
0.6689,0.7721,0.8689, 0.956,  1.05,  1.15,
 1.261, 1.366, 1.465, 1.579, 1.707, 1.855,
 2.016, 2.211, 2.455, 2.751, 3.093};
const double Y1001V2[47][4]={
{0.9973  ,0.0  ,0.0  ,0.0},
{ 1.007, 1.017  ,0.0  ,0.0},
{ 1.002,0.9903,0.9772  ,0.0},
{ 1.002,0.9698,0.9753,0.9387},
{ 1.001,0.9867, 1.004, 0.964},
{ 1.002, 1.001,0.9807,  0.96},
{ 1.003,0.9936,0.9804,0.9823},
{     1,0.9963,0.9849,0.9766},
{ 1.003,0.9914, 1.003,0.9887},
{0.9995, 1.017,0.9797,0.9836},
{ 1.001,0.9921,0.9824,0.9702},
{0.9958,0.9843,0.9854,0.9818},
{0.9982,0.9878,0.9909,0.9906},
{0.9982, 1.007, 1.024, 1.028},
{     1, 1.016, 1.016, 1.013},
{0.9987, 1.008,  1.03, 1.026},
{ 1.001,0.9916,0.9939,0.9955},
{ 1.001, 0.997,0.9837,0.9908},
{0.9988, 0.981, 1.001,0.9835},
{ 1.001,0.9815,0.9951,0.9978},
{0.9993, 1.004, 1.016, 1.016},
{     1, 1.016, 1.011, 1.031},
{0.9999,0.9894, 0.989,0.9802},
{0.9966,0.9789,0.9559,0.9767},
{0.9992, 0.967,0.9685,0.9767},
{ 1.002,0.9633, 1.006, 0.999},
{0.9994, 1.004,0.9938, 1.007},
{ 1.002,0.9965,0.9947,0.9936},
{0.9982, 1.007, 1.012, 1.012},
{ 1.001, 1.003,     1, 1.005},
{ 1.001,0.9917, 1.013, 1.008},
{ 0.999, 1.021, 1.008, 1.036},
{ 1.001, 1.035, 1.018, 1.036},
{0.9991, 1.016, 1.008, 1.036},
{0.9976, 1.004, 1.026, 1.028},
{0.9953,0.9835,0.9931,0.9889},
{ 1.001,0.9898,0.9914,0.9929},
{0.9983, 1.008, 1.005,0.9958},
{ 1.003, 1.002,0.9829,0.9907},
{0.9987,0.9904,0.9829,0.9871},
{ 1.001,0.9824,0.9867,0.9773},
{0.9989,0.9946,0.9921  ,0.0},
{ 1.001,0.9788,0.9869  ,0.0},
{0.9985,0.9645  ,0.0  ,0.0},
{ 1.001,0.9599  ,0.0  ,0.0},
{0.9991  ,0.0  ,0.0  ,0.0},
{0.9956  ,0.0  ,0.0  ,0.0}
};
const double E1001V2[47][4]={
{0.0171  ,0.0  ,0.0  ,0.0},
{0.0069,0.0262  ,0.0  ,0.0},
{0.0053, 0.012,0.0217  ,0.0},
{0.0053,0.0089,0.0134,0.0221},
{0.0048,0.0076,0.0104,0.0122},
{ 0.005,0.0084,0.0091,0.0079},
{0.0053,0.0081,0.0085,0.0071},
{0.0054,0.0078,0.0084,0.0064},
{ 0.006,0.0084,0.0087,0.0068},
{0.0063,0.0093,0.0083, 0.007},
{ 0.006,0.0076, 0.008,0.0059},
{0.0071,0.0098,0.0091, 0.007},
{0.0071,0.0095,0.0096,0.0074},
{0.0074,0.0101,0.0101,0.0071},
{0.0065,0.0091,0.0091,0.0058},
{0.0064,0.0084,0.0086,0.0055},
{ 0.006,0.0077,0.0081,0.0049},
{0.0073,0.0098,0.0093,0.0065},
{0.0076,0.0115,0.0097, 0.006},
{0.0073,0.0102,0.0093,0.0058},
{0.0059, 0.008,0.0079,0.0051},
{0.0075,0.0104,0.0098,0.0067},
{0.0106,0.0146,0.0158,0.0101},
{ 0.008,0.0123, 0.012,0.0077},
{0.0108,0.0137, 0.017,0.0098},
{0.0074,0.0099,0.0103, 0.006},
{0.0058,0.0077,0.0077,0.0048},
{0.0069,0.0097,0.0089, 0.006},
{0.0075,0.0103,0.0099,0.0062},
{0.0075,0.0096,0.0102,0.0063},
{0.0059,0.0077,0.0078, 0.005},
{0.0062,0.0089,0.0084,0.0058},
{0.0069,0.0093,0.0095,0.0062},
{0.0075,0.0097,0.0099,0.0069},
{0.0071,0.0107,0.0112,0.0074},
{0.0069,0.0102,0.0099,0.0072},
{0.0062, 0.008,0.0082,0.0059},
{0.0066,0.0087,0.0096,0.0071},
{0.0061,0.0084,0.0084,0.0064},
{0.0055,0.0076,0.0087,0.0068},
{0.0051, 0.008,0.0084,0.0071},
{0.0049, 0.008,0.0092  ,0.0},
{0.0048,0.0081,0.0111  ,0.0},
{0.0052,0.0095  ,0.0  ,0.0},
{0.0053, 0.011  ,0.0  ,0.0},
{0.0071  ,0.0  ,0.0  ,0.0},
{0.0181  ,0.0  ,0.0  ,0.0}
};



const int  NPETA01V2[3]={47,47,47};

const double SPLINTETA01V2[3][47]={
  {
-3.27070,-2.71390,-2.45240,-2.20690,-2.01260,-1.85280,-1.70880,-1.57930,
-1.46530,-1.36720,-1.26210,-1.15190,-1.04880,-0.95770,-0.86920,-0.77360,
-0.66840,-0.56490,-0.44360,-0.32300,-0.18460,-0.09770,-0.05050, 0.00010,
 0.04980, 0.09770, 0.18520, 0.32120, 0.44380, 0.56400, 0.66810, 0.77160,
 0.86990, 0.95620, 1.04960, 1.15160, 1.26200, 1.36630, 1.46610, 1.58010,
 1.70800, 1.85200, 2.01210, 2.20880, 2.45080, 2.71770, 3.27070  },
  {
-3.13280,-2.75120,-2.44360,-2.21290,-2.01460,-1.85470,-1.70710,-1.58030,
-1.46520,-1.36690,-1.26140,-1.15050,-1.04700,-0.95640,-0.86800,-0.77430,
-0.66900,-0.56600,-0.44420,-0.32070,-0.18410,-0.09780,-0.04940, 0.00050,
 0.04910, 0.09800, 0.18630, 0.32230, 0.44380, 0.56490, 0.66870, 0.77270,
 0.86950, 0.95690, 1.04960, 1.15050, 1.25890, 1.36670, 1.46420, 1.57940,
 1.70820, 1.85370, 2.01400, 2.21220, 2.44560, 2.75150, 3.12210  },
  {
-3.09230,-2.75050,-2.45630,-2.21010,-2.01810,-1.85620,-1.70940,-1.58140,
-1.46590,-1.36620,-1.26240,-1.14990,-1.04970,-0.95690,-0.86870,-0.77280,
-0.66950,-0.56480,-0.44350,-0.32210,-0.18410,-0.09830,-0.04800, 0.00030,
 0.04870, 0.09830, 0.18630, 0.32160, 0.44590, 0.56550, 0.66890, 0.77210,
 0.86890, 0.95600, 1.04960, 1.14980, 1.26130, 1.36610, 1.46520, 1.57940,
 1.70700, 1.85480, 2.01640, 2.21130, 2.45550, 2.75050, 3.09320  }
};


const double SPLINTP001V2[3][47]={
  {
      0.99090,      0.96731,      1.00460,      1.01107,      1.00522,      1.00460,      1.00170,      1.00724,
      0.99941,      0.99883,      0.99689,      0.98404,      0.99340,      0.98059,      0.98997,      0.99143,
      0.98947,      1.00587,      0.99814,      0.99929,      0.99000,      0.97931,      0.99647,      1.01092,
      0.99466,      0.98439,      0.99456,      0.98743,      0.99924,      0.98040,      0.99238,      0.98591,
      0.97940,      0.96608,      0.97540,      0.98684,      0.99054,      0.99755,      0.99480,      0.99937,
      1.00367,      0.99017,      1.00713,      1.01472,      1.00929,      0.99410,     0.99250
  },
  {
      0.99440,      0.94720,      0.99186,      1.01155,      1.01821,      1.01686,      1.01097,      1.01245,
      1.00863,      1.01168,      1.00721,      0.98919,      0.99055,      0.98560,      0.99215,      0.99950,
      0.99792,      1.01027,      0.99733,      1.00660,      1.00278,      0.99127,      1.00617,      0.97541,
      0.99080,      0.99031,      1.00137,      0.99779,      0.99785,      0.99752,      0.99367,      0.98270,
      0.99518,      0.98481,      0.98437,      0.98191,      0.99069,      1.00074,      1.00857,      1.00388,
      1.01187,      1.00084,      1.01213,      1.03149,      1.01090,      1.00310,     0.98490
  },
  {
      0.99730,      0.99754,      1.01507,      1.02256,      1.01089,      1.01800,      1.00884,      1.00938,
      1.00609,      1.00984,      1.01201,      0.99823,      0.99807,      0.98884,      1.00084,      0.99149,
      0.99966,      1.00307,      1.00230,      0.99575,      0.99438,      0.99076,      1.00539,      0.99763,
      0.99647,      0.99100,      0.99598,      1.00369,      0.99533,      0.99904,      0.99578,      0.98757,
      0.99614,      0.98689,      0.98738,      0.99502,      1.00002,      1.00286,      1.00655,      1.00107,
      1.00706,      1.00295,      1.01326,      1.03343,      1.04322,      0.99910,     0.99560
  }
};
const double SPLINTP101V2[3][47]={
  {
      0.00000,      0.00088,     -0.00027,     -0.00048,     -0.00045,     -0.00053,     -0.00030,     -0.00046,
     -0.00014,     -0.00018,     -0.00012,      0.00008,     -0.00004,      0.00050,      0.00030,      0.00016,
      0.00004,     -0.00024,     -0.00013,     -0.00013,      0.00021,      0.00056,     -0.00036,     -0.00063,
     -0.00008,      0.00040,      0.00014,      0.00019,      0.00002,      0.00030,      0.00012,      0.00019,
      0.00055,      0.00077,      0.00056,      0.00004,      0.00011,     -0.00002,     -0.00005,     -0.00026,
     -0.00033,     -0.00019,     -0.00046,     -0.00074,     -0.00046,      0.00000,     0.00000
  },
  {
      0.00000,      0.00190,      0.00022,     -0.00037,     -0.00055,     -0.00061,     -0.00031,     -0.00024,
     -0.00004,     -0.00021,     -0.00017,      0.00011,      0.00030,      0.00045,      0.00036,      0.00011,
      0.00012,     -0.00007,     -0.00006,     -0.00024,      0.00001,      0.00035,     -0.00028,      0.00005,
     -0.00012,      0.00016,      0.00015,     -0.00001,      0.00015,      0.00013,      0.00019,      0.00042,
      0.00032,      0.00055,      0.00050,      0.00024,      0.00026,     -0.00003,     -0.00019,     -0.00020,
     -0.00034,     -0.00016,     -0.00042,     -0.00107,     -0.00048,      0.00000,     0.00000
  },
  {
      0.00000,      0.00031,     -0.00042,     -0.00069,     -0.00030,     -0.00044,     -0.00024,     -0.00026,
     -0.00013,     -0.00022,     -0.00034,     -0.00014,     -0.00007,      0.00032,      0.00012,      0.00030,
     -0.00004,     -0.00012,     -0.00014,      0.00000,      0.00018,      0.00031,     -0.00020,     -0.00022,
     -0.00020,      0.00005,      0.00008,     -0.00009,      0.00015,      0.00005,      0.00010,      0.00038,
      0.00033,      0.00037,      0.00034,     -0.00005,     -0.00007,     -0.00003,     -0.00015,     -0.00014,
     -0.00025,     -0.00013,     -0.00042,     -0.00111,     -0.00134,      0.00000,     0.00000
  }
};