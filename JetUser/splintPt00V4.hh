


const int NP0400V4[47]={
  2,  3,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  3,  3,  2,  1};
const double ETA0400V4[47]={
 -3.27,-2.719,-2.454,-2.211,-2.013,-1.853,
-1.708,-1.579,-1.465,-1.367,-1.262,-1.152,
-1.047,-0.9569,-0.8684,-0.7735,-0.669,-0.5649,
-0.4428,-0.3216,-0.1853,-0.0974,-0.0496,    -0,
0.0496,0.0979,0.1862,0.3213,0.4436,0.5649,
0.6692,0.7732,0.8687,0.9567, 1.048, 1.151,
 1.262, 1.367, 1.466, 1.579, 1.709, 1.854,
 2.014, 2.212, 2.453,  2.72, 3.271};
const double Y0400V4[47][4]={
{0.9884,0.9246  ,0.0  ,0.0},
{0.9964, 1.003,0.9961  ,0.0},
{ 1.005,0.9552,0.9474,0.9242},
{ 1.001,0.9701, 0.957,0.9188},
{ 1.002, 0.975,0.9566,0.9303},
{ 1.003,0.9554,0.9487,0.9343},
{ 1.002,0.9685,0.9556,0.9521},
{ 1.002,0.9701,0.9741,0.9535},
{ 1.002,0.9874,0.9793,0.9674},
{0.9989,0.9883,0.9806,0.9755},
{0.9954, 1.004,0.9873,0.9861},
{ 0.985,0.9795,0.9615, 0.974},
{0.9918,0.9895,0.9911, 1.002},
{ 0.996, 1.007, 1.009,  1.02},
{0.9969, 1.017, 1.019,  1.03},
{0.9953, 1.013, 1.016, 1.021},
{0.9963, 1.016,  1.02, 1.017},
{ 1.001, 1.001,     1,0.9964},
{0.9983,0.9996, 1.006,0.9972},
{ 1.004, 1.007, 1.006, 1.009},
{0.9977, 0.992,0.9982, 0.997},
{     1, 1.001, 1.014, 1.021},
{0.9934,0.9896, 0.967,0.9803},
{ 0.985,0.9626,0.9534,0.9588},
{0.9922,0.9932,0.9855,0.9889},
{0.9996, 1.012, 1.011, 1.022},
{0.9962, 1.015, 1.004, 1.008},
{0.9914, 1.004, 1.005,     1},
{0.9923, 1.002,0.9886,0.9927},
{0.9946, 1.001, 1.006,0.9964},
{0.9957,0.9975, 1.005, 1.014},
{0.9957, 1.013, 1.008,  1.02},
{0.9968, 1.022, 1.028, 1.033},
{0.9947,  1.01,  1.02, 1.034},
{0.9913,0.9968,0.9859, 1.008},
{0.9852,0.9783,0.9591,0.9912},
{0.9958, 1.004,0.9911,0.9994},
{0.9966,0.9989,0.9951,0.9954},
{ 1.001,0.9949,0.9753,0.9742},
{0.9991,0.9883,0.9675,0.9667},
{ 1.002,0.9723,0.9741,0.9544},
{ 1.002,0.9707,0.9555,0.9442},
{0.9993,0.9724,0.9641,0.9488},
{     1,0.9604,0.9484  ,0.0},
{ 1.001,0.9781,0.9245  ,0.0},
{0.9967,0.9797  ,0.0  ,0.0},
{0.9819  ,0.0  ,0.0  ,0.0}
};
const double E0400V4[47][4]={
{0.0154,0.0461  ,0.0  ,0.0},
{0.0065, 0.012,0.0249  ,0.0},
{0.0044,0.0062,0.0101,0.0207},
{0.0041, 0.005,0.0067, 0.008},
{ 0.004,0.0044,0.0057,0.0051},
{ 0.004,0.0041,0.0047,0.0042},
{ 0.004,0.0042,0.0045,0.0035},
{0.0041,0.0042,0.0047,0.0034},
{0.0044,0.0044,0.0049,0.0032},
{0.0051, 0.005,0.0053,0.0037},
{0.0047,0.0045, 0.005,0.0035},
{ 0.006, 0.006,0.0058, 0.004},
{0.0058,0.0057,0.0064,0.0043},
{0.0061, 0.006,0.0064,0.0041},
{0.0052,0.0051,0.0054,0.0034},
{0.0047,0.0047, 0.005, 0.003},
{0.0046,0.0044,0.0046,0.0027},
{ 0.006,0.0053,0.0054,0.0034},
{0.0059,0.0057,0.0056,0.0035},
{0.0056,0.0054,0.0053,0.0033},
{0.0046,0.0041,0.0042,0.0027},
{0.0057,0.0057,0.0057,0.0036},
{0.0091,0.0089,0.0094,0.0058},
{0.0072,0.0075,0.0086,0.0053},
{ 0.009,0.0084,0.0099,0.0058},
{0.0058,0.0058,0.0058,0.0035},
{0.0046,0.0042,0.0044,0.0027},
{0.0058,0.0053,0.0054,0.0033},
{0.0059,0.0056,0.0057,0.0034},
{0.0057,0.0055,0.0058,0.0036},
{0.0046,0.0045,0.0046,0.0029},
{0.0049,0.0048,0.0051,0.0032},
{0.0053, 0.005,0.0054,0.0035},
{ 0.006,0.0062,0.0066,0.0042},
{0.0059,0.0061,0.0066,0.0045},
{ 0.006,0.0058, 0.006,0.0044},
{0.0049,0.0048,0.0052,0.0035},
{ 0.005,0.0051,0.0057,0.0038},
{0.0044,0.0045,0.0047,0.0032},
{0.0042,0.0043,0.0046,0.0034},
{ 0.004,0.0041,0.0045,0.0035},
{0.0041,0.0044,0.0048, 0.004},
{0.0041,0.0045,0.0054,0.0051},
{0.0042,0.0049,0.0063  ,0.0},
{0.0045,0.0063,0.0096  ,0.0},
{0.0062,0.0114  ,0.0  ,0.0},
{0.0161  ,0.0  ,0.0  ,0.0}
};
const int NP0700V4[47]={
  1,  3,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  3,  3,  2,  1};
const double ETA0700V4[47]={
-3.122,-2.756,-2.446,-2.215,-2.015,-1.855,
-1.708,-1.581,-1.465,-1.367,-1.261,-1.151,
-1.048,-0.9566,-0.8683,-0.7734,-0.669,-0.5649,
-0.4428,-0.3223,-0.185,-0.098,-0.0489,0.0002,
 0.049,0.0981,0.1859,0.3207,0.4419,0.5645,
0.6692, 0.773, 0.869,0.9571, 1.049, 1.151,
 1.262, 1.366, 1.466, 1.581, 1.708, 1.854,
 2.016, 2.214, 2.449, 2.756, 3.122};
const double Y0700V4[47][4]={
{0.9945  ,0.0  ,0.0  ,0.0},
{ 1.012,0.9605,0.9624  ,0.0},
{ 1.001,0.9624,0.9541, 0.936},
{ 1.003,0.9568,0.9592,0.9411},
{ 1.003,0.9733,  0.96,0.9329},
{ 1.002, 0.971,0.9599,0.9493},
{ 1.002,0.9787,0.9675,0.9646},
{ 1.002,0.9853, 0.995,0.9766},
{ 1.001, 1.001,0.9862,0.9856},
{ 1.001, 1.004,0.9921,0.9865},
{0.9995, 1.002, 1.002,0.9934},
{0.9933,0.9877,0.9753,0.9932},
{0.9949,0.9978, 1.016,  1.02},
{0.9991, 1.006, 1.011, 1.029},
{0.9987,  1.01, 1.018,  1.02},
{0.9983, 1.005, 1.014, 1.017},
{0.9996, 1.011, 1.009, 1.012},
{0.9982, 1.004, 1.006, 1.008},
{0.9968,0.9905,0.9946,0.9949},
{0.9959,0.9909,0.9966,0.9984},
{ 0.999, 1.001,0.9964,0.9997},
{     1, 1.008,  1.02, 1.027},
{0.9984, 1.007,0.9953,0.9992},
{0.9943,0.9645, 0.955,0.9743},
{0.9977,0.9795,     1, 1.002},
{ 1.001, 1.017,0.9977, 1.014},
{0.9972, 0.998,0.9951,     1},
{0.9954,0.9863,0.9943, 0.994},
{0.9986, 1.007, 0.998, 1.005},
{0.9953,  1.01, 1.002, 1.009},
{0.9994,  1.01, 1.007, 1.017},
{0.9977, 1.014, 1.009, 1.025},
{     1, 1.019, 1.019, 1.028},
{0.9977, 1.013, 1.021, 1.047},
{0.9963, 1.012, 1.019, 1.026},
{0.9933, 0.987,0.9835, 1.002},
{0.9982,  1.01, 1.001, 1.009},
{0.9987, 1.002,0.9994,0.9955},
{ 1.002, 1.002,0.9844, 0.986},
{ 1.001,0.9842,0.9833,0.9685},
{ 1.001,0.9849,0.9801,0.9678},
{ 1.001,0.9823,0.9736,0.9595},
{ 1.002,0.9702,0.9531,0.9455},
{ 1.002,0.9609,0.9492  ,0.0},
{0.9981,0.9771,0.9585  ,0.0},
{ 1.008,0.9723  ,0.0  ,0.0},
{0.9876  ,0.0  ,0.0  ,0.0}
};
const double E0700V4[47][4]={
{0.0081  ,0.0  ,0.0  ,0.0},
{0.0038,0.0129,0.0311  ,0.0},
{0.0031,0.0063,0.0118,0.0267},
{0.0029,0.0047, 0.007,0.0113},
{0.0029,0.0041,0.0054, 0.006},
{ 0.003, 0.004,0.0046,0.0046},
{0.0031, 0.004,0.0042,0.0037},
{0.0032,0.0041,0.0045,0.0035},
{0.0034,0.0043,0.0046,0.0032},
{ 0.004,0.0048,0.0049,0.0037},
{0.0037,0.0041,0.0044, 0.003},
{0.0046,0.0048, 0.005,0.0038},
{0.0045,0.0051,0.0055,0.0039},
{0.0047,0.0053,0.0054,0.0036},
{0.0042,0.0046,0.0047,0.0031},
{0.0039,0.0044,0.0045,0.0028},
{0.0037, 0.004,0.0042,0.0025},
{0.0045, 0.005,0.0051,0.0032},
{0.0047,0.0052, 0.005,0.0032},
{0.0045,0.0048,0.0048,0.0031},
{0.0036, 0.004,0.0039,0.0026},
{0.0045,0.0053,0.0053,0.0034},
{0.0069,0.0084,0.0079,0.0053},
{0.0055,0.0062,0.0065,0.0045},
{ 0.007,0.0074,0.0086,0.0051},
{0.0047,0.0053,0.0052,0.0033},
{0.0036, 0.004,0.0041,0.0025},
{0.0044,0.0049,0.0048,0.0031},
{0.0048,0.0051,0.0053,0.0033},
{0.0044, 0.005, 0.005,0.0033},
{0.0037,0.0043, 0.004,0.0026},
{0.0039,0.0044,0.0046,0.0029},
{0.0043,0.0047,0.0048,0.0031},
{0.0048,0.0053,0.0056,0.0037},
{0.0047,0.0052,0.0056, 0.004},
{0.0046,0.0052,0.0053,0.0038},
{0.0038,0.0043,0.0045,0.0032},
{0.0041,0.0048,0.0052,0.0035},
{0.0036,0.0043,0.0044,0.0033},
{0.0034, 0.004,0.0043,0.0034},
{0.0031,0.0039,0.0044,0.0036},
{ 0.003, 0.004,0.0047,0.0044},
{0.0029,0.0044,0.0053,0.0057},
{ 0.003,0.0046,0.0069  ,0.0},
{0.0031,0.0061,0.0128  ,0.0},
{0.0038,0.0114  ,0.0  ,0.0},
{0.0087  ,0.0  ,0.0  ,0.0}
};
const int NP1000V4[47]={
  1,  3,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  3,  3,  1,  1};
const double ETA1000V4[47]={
 -3.09,-2.753,-2.457,-2.212,-2.019,-1.856,
-1.709, -1.58,-1.465,-1.366,-1.262, -1.15,
 -1.05,-0.9577,-0.8691,-0.7731,-0.6695,-0.566,
-0.4427,-0.3215,-0.1848,-0.0978,-0.0484,0.0002,
0.0489,0.0982,0.1862,0.3216,0.4437,0.5653,
0.6691,0.7733, 0.869,0.9568, 1.049,  1.15,
 1.262, 1.367, 1.466, 1.581, 1.709, 1.856,
 2.018, 2.211, 2.457, 2.753, 3.091};
const double Y1000V4[47][4]={
{0.9985  ,0.0  ,0.0  ,0.0},
{ 1.008,0.9878,0.9467  ,0.0},
{ 1.002,0.9709,0.9509,0.9593},
{ 1.001,0.9712,0.9665,0.9614},
{ 1.002,0.9692,0.9667,0.9459},
{ 1.001,0.9808,0.9685,0.9546},
{ 1.001,0.9856,0.9735,0.9774},
{     1,0.9936, 0.991,0.9821},
{ 1.002,0.9892,0.9866, 0.978},
{0.9995, 1.001, 0.989,0.9883},
{ 0.999,0.9879,0.9887,0.9842},
{0.9952,0.9762, 0.969,0.9789},
{0.9957,0.9871,0.9938,0.9963},
{0.9991,0.9918, 1.001, 1.018},
{0.9976, 1.022, 1.012,  1.02},
{0.9992, 1.005, 1.018, 1.017},
{0.9982, 1.007, 1.007, 1.008},
{ 1.002,     1, 1.003, 1.002},
{ 1.005, 1.003, 1.002,0.9994},
{ 1.003, 1.008, 1.003, 1.001},
{     1, 1.005, 1.006, 1.011},
{     1, 1.006, 1.012, 1.014},
{0.9988,0.9777,0.9852,0.9819},
{0.9964,0.9645,0.9583,0.9731},
{0.9986,0.9863,0.9873, 0.994},
{0.9989, 1.013, 1.008, 1.014},
{0.9994, 1.009, 1.002, 1.008},
{ 1.001, 0.994,0.9976,0.9971},
{ 1.002, 1.006,0.9985, 1.003},
{0.9985, 1.007, 0.999, 1.006},
{0.9992, 1.003, 1.002, 1.008},
{0.9979, 1.003, 1.007, 1.018},
{ 0.999, 1.005, 1.012, 1.026},
{0.9985, 1.004, 1.015, 1.029},
{0.9974,0.9898, 1.003,  1.01},
{0.9936,0.9792,0.9761,0.9915},
{0.9995, 0.987,0.9829,0.9931},
{0.9987,0.9976,0.9946,0.9889},
{0.9999, 1.004,0.9919,0.9924},
{ 1.001,0.9853,0.9887,0.9727},
{     1,0.9914,0.9781,0.9796},
{ 1.001,0.9811,0.9758,0.9647},
{     1,0.9754,0.9654,0.9536},
{     1,0.9695, 0.954  ,0.0},
{     1,0.9749,0.9517  ,0.0},
{ 1.005  ,0.0  ,0.0  ,0.0},
{0.9937  ,0.0  ,0.0  ,0.0}
};
const double E1000V4[47][4]={
{ 0.007  ,0.0  ,0.0  ,0.0},
{0.0029,0.0105,0.0299  ,0.0},
{0.0023,0.0052,0.0092,0.0252},
{0.0022, 0.004,0.0057, 0.009},
{0.0022,0.0036,0.0048,0.0051},
{0.0023,0.0035, 0.004, 0.004},
{0.0024,0.0034,0.0037,0.0033},
{0.0026,0.0035,0.0038,0.0031},
{0.0029,0.0038,0.0038,0.0028},
{0.0032, 0.004,0.0041,0.0033},
{ 0.003,0.0036,0.0037,0.0027},
{0.0036,0.0044,0.0045,0.0033},
{0.0036,0.0044,0.0049,0.0034},
{0.0037,0.0045,0.0048,0.0031},
{0.0034,0.0041,0.0041,0.0028},
{0.0031,0.0039,0.0041,0.0025},
{ 0.003,0.0036,0.0036,0.0023},
{0.0038,0.0046,0.0044,0.0028},
{ 0.004,0.0047,0.0046, 0.003},
{0.0039,0.0046,0.0042,0.0028},
{ 0.003,0.0036,0.0035,0.0023},
{0.0038,0.0047,0.0047,0.0031},
{0.0057,0.0072,0.0069,0.0046},
{0.0042,0.0053,0.0058,0.0038},
{0.0057,0.0069,0.0072,0.0046},
{0.0038,0.0047,0.0047, 0.003},
{ 0.003,0.0036,0.0035,0.0022},
{0.0038,0.0043,0.0043,0.0027},
{0.0038,0.0045,0.0047, 0.003},
{0.0038,0.0045,0.0045,0.0029},
{ 0.003,0.0037,0.0036,0.0023},
{0.0032,0.0039, 0.004,0.0026},
{0.0035,0.0042,0.0044,0.0028},
{0.0039,0.0047,0.0049,0.0033},
{0.0037,0.0046, 0.005,0.0036},
{0.0036,0.0044,0.0046,0.0034},
{0.0031,0.0037,0.0039,0.0028},
{0.0033,0.0041,0.0045,0.0032},
{0.0029,0.0037,0.0039,0.0029},
{0.0027,0.0035,0.0039, 0.003},
{0.0024,0.0034,0.0038,0.0033},
{0.0024,0.0035, 0.004,0.0039},
{0.0022,0.0036,0.0046,0.0051},
{0.0023,0.0039,0.0058  ,0.0},
{0.0023,0.0052,0.0096  ,0.0},
{0.0028  ,0.0  ,0.0  ,0.0},
{0.0075  ,0.0  ,0.0  ,0.0}
};



const int  NPETA00V4[3]={47,47,47};

const double SPLINTETA00V4[3][47]={
  {
-3.27040,-2.71940,-2.45370,-2.21140,-2.01280,-1.85260,-1.70830,-1.57930,
-1.46530,-1.36660,-1.26160,-1.15170,-1.04710,-0.95690,-0.86840,-0.77350,
-0.66900,-0.56490,-0.44280,-0.32160,-0.18530,-0.09740,-0.04960,-0.00000,
 0.04960, 0.09790, 0.18620, 0.32130, 0.44360, 0.56490, 0.66920, 0.77320,
 0.86870, 0.95670, 1.04760, 1.15140, 1.26200, 1.36670, 1.46610, 1.57930,
 1.70890, 1.85360, 2.01430, 2.21180, 2.45310, 2.71980, 3.27070  },
  {
-3.12190,-2.75570,-2.44640,-2.21510,-2.01510,-1.85470,-1.70760,-1.58090,
-1.46490,-1.36670,-1.26130,-1.15150,-1.04800,-0.95660,-0.86830,-0.77340,
-0.66900,-0.56490,-0.44280,-0.32230,-0.18500,-0.09800,-0.04890, 0.00020,
 0.04900, 0.09810, 0.18590, 0.32070, 0.44190, 0.56450, 0.66920, 0.77300,
 0.86900, 0.95710, 1.04920, 1.15050, 1.26190, 1.36600, 1.46560, 1.58090,
 1.70850, 1.85400, 2.01560, 2.21420, 2.44900, 2.75550, 3.12160  },
  {
-3.09030,-2.75300,-2.45700,-2.21190,-2.01860,-1.85640,-1.70910,-1.57970,
-1.46490,-1.36610,-1.26220,-1.15020,-1.04950,-0.95770,-0.86910,-0.77310,
-0.66950,-0.56600,-0.44270,-0.32150,-0.18480,-0.09780,-0.04840, 0.00020,
 0.04890, 0.09820, 0.18620, 0.32160, 0.44370, 0.56530, 0.66910, 0.77330,
 0.86900, 0.95680, 1.04900, 1.14980, 1.26240, 1.36660, 1.46580, 1.58050,
 1.70890, 1.85580, 2.01800, 2.21110, 2.45690, 2.75320, 3.09050  }
};


const double SPLINTP000V4[3][47]={
  {
      1.05395,      0.99294,      1.03774,      1.02807,      1.02476,      1.01351,      1.00857,      1.01034,
      1.01145,      1.00444,      1.00440,      0.98380,      0.98425,      0.98966,      0.99135,      0.99284,
      0.99884,      1.00437,      1.00167,      1.00305,      0.99489,      0.99033,      0.99410,      0.98465,
      0.99365,      0.99435,      1.00123,      0.99625,      0.99623,      0.99930,      0.98700,      0.99214,
      0.99396,      0.98268,      0.98292,      0.97287,      0.99710,      0.99822,      1.00976,      1.00809,
      1.01099,      1.01448,      1.01230,      1.03114,      1.04158,      1.01417,     0.98190
  },
  {
      0.99450,      1.05598,      1.03030,      1.02700,      1.02570,      1.01658,      1.01109,      1.00815,
      1.00786,      1.00856,      1.00456,      0.98802,      0.98481,      0.98718,      0.99423,      0.99271,
      0.99903,      0.99621,      0.99469,      0.99217,      0.99882,      0.99172,      1.00060,      0.98718,
      0.98667,      0.99956,      0.99550,      0.99202,      0.99858,      0.99497,      0.99474,      0.99072,
      0.99575,      0.97977,      0.98929,      0.98444,      0.99736,      1.00169,      1.00834,      1.01020,
      1.01047,      1.01364,      1.01975,      1.03549,      1.02050,      1.04407,     0.98760
  },
  {
      0.99850,      1.03375,      1.02918,      1.01774,      1.01968,      1.01567,      1.00656,      1.00654,
      1.00804,      1.00470,      1.00143,      0.99371,      0.99203,      0.98630,      0.99673,      0.99411,
      0.99775,      1.00139,      1.00644,      1.00592,      0.99694,      0.99665,      0.99834,      0.99306,
      0.99477,      0.99710,      0.99882,      0.99974,      1.00269,      0.99791,      0.99586,      0.99044,
      0.98884,      0.98627,      0.98851,      0.98754,      0.99616,      1.00293,      1.00444,      1.00878,
      1.00645,      1.01146,      1.01596,      1.02873,      1.02737,      1.00470,     0.99370
  }
};
const double SPLINTP100V4[3][47]={
  {
     -0.00208,      0.00012,     -0.00114,     -0.00088,     -0.00077,     -0.00070,     -0.00050,     -0.00047,
     -0.00036,     -0.00024,     -0.00014,     -0.00011,      0.00013,      0.00024,      0.00032,      0.00024,
      0.00017,     -0.00006,     -0.00003,      0.00005,      0.00002,      0.00025,     -0.00013,     -0.00024,
     -0.00004,      0.00022,      0.00006,      0.00004,     -0.00003,     -0.00001,      0.00021,      0.00023,
      0.00033,      0.00042,      0.00018,      0.00009,      0.00001,     -0.00002,     -0.00030,     -0.00035,
     -0.00047,     -0.00060,     -0.00054,     -0.00104,     -0.00120,     -0.00055,     0.00000
  },
  {
      0.00000,     -0.00141,     -0.00097,     -0.00087,     -0.00077,     -0.00060,     -0.00042,     -0.00025,
     -0.00018,     -0.00017,     -0.00008,      0.00001,      0.00029,      0.00032,      0.00022,      0.00020,
      0.00011,      0.00010,     -0.00000,      0.00005,      0.00000,      0.00029,     -0.00001,     -0.00016,
      0.00011,      0.00010,      0.00003,      0.00001,      0.00005,      0.00011,      0.00018,      0.00027,
      0.00027,      0.00053,      0.00031,      0.00011,      0.00009,     -0.00004,     -0.00019,     -0.00034,
     -0.00035,     -0.00045,     -0.00068,     -0.00110,     -0.00071,     -0.00115,     0.00000
  },
  {
      0.00000,     -0.00080,     -0.00088,     -0.00059,     -0.00064,     -0.00052,     -0.00028,     -0.00019,
     -0.00025,     -0.00014,     -0.00015,     -0.00016,      0.00002,      0.00023,      0.00020,      0.00019,
      0.00009,      0.00001,     -0.00006,     -0.00003,      0.00011,      0.00014,     -0.00015,     -0.00021,
     -0.00002,      0.00014,      0.00007,     -0.00003,      0.00000,      0.00006,      0.00010,      0.00021,
      0.00029,      0.00033,      0.00016,     -0.00001,     -0.00005,     -0.00011,     -0.00010,     -0.00029,
     -0.00024,     -0.00040,     -0.00055,     -0.00091,     -0.00086,      0.00000,     0.00000
  }
};