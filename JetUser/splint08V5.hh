
const int BEG_RUN08V5=237845;
const int END_RUN08V5=241657;

const int NP08V5[6]={49,49,49,49,49,49};
const double X08V5[6][49]={
 { -3.2710,
   -2.7190, -2.4520, -2.2110, -2.0130, -1.8530, -1.7080, -1.5790, -1.4660,
   -1.3670, -1.2620, -1.1510, -1.0470, -0.9572, -0.8685, -0.7732, -0.6687,
   -0.5654, -0.4435, -0.3213, -0.2254, -0.1653, -0.0972, -0.0501,  0.0000,
    0.0494,  0.0975,  0.1651,  0.2244,  0.3210,  0.4428,  0.5648,  0.6692,
    0.7727,  0.8689,  0.9572,  1.0490,  1.1510,  1.2630,  1.3660,  1.4660,
    1.5790,  1.7080,  1.8530,  2.0130,  2.2100,  2.4510,  2.7190,  3.2710},
 { -3.1190,
   -2.7540, -2.4470, -2.2150, -2.0140, -1.8550, -1.7080, -1.5810, -1.4650,
   -1.3660, -1.2610, -1.1510, -1.0480, -0.9565, -0.8679, -0.7731, -0.6689,
   -0.5651, -0.4429, -0.3213, -0.2251, -0.1644, -0.0980, -0.0489,  0.0001,
    0.0490,  0.0982,  0.1645,  0.2251,  0.3213,  0.4423,  0.5649,  0.6692,
    0.7726,  0.8690,  0.9570,  1.0500,  1.1510,  1.2620,  1.3660,  1.4660,
    1.5810,  1.7080,  1.8540,  2.0150,  2.2150,  2.4470,  2.7550,  3.1200},
 { -3.0880,
   -2.7530, -2.4570, -2.2120, -2.0180, -1.8570, -1.7090, -1.5800, -1.4650,
   -1.3660, -1.2620, -1.1510, -1.0490, -0.9574, -0.8691, -0.7732, -0.6690,
   -0.5656, -0.4423, -0.3223, -0.2255, -0.1640, -0.0979, -0.0488,  0.0002,
    0.0486,  0.0979,  0.1642,  0.2258,  0.3210,  0.4431,  0.5646,  0.6692,
    0.7728,  0.8692,  0.9568,  1.0500,  1.1500,  1.2620,  1.3660,  1.4660,
    1.5810,  1.7090,  1.8560,  2.0180,  2.2110,  2.4570,  2.7530,  3.0870},
 { -3.2710,
   -2.7190, -2.4520, -2.2110, -2.0130, -1.8530, -1.7080, -1.5790, -1.4660,
   -1.3670, -1.2620, -1.1510, -1.0470, -0.9572, -0.8685, -0.7732, -0.6687,
   -0.5654, -0.4435, -0.3213, -0.2254, -0.1653, -0.0972, -0.0501,  0.0000,
    0.0494,  0.0975,  0.1651,  0.2244,  0.3210,  0.4428,  0.5648,  0.6692,
    0.7727,  0.8689,  0.9572,  1.0490,  1.1510,  1.2630,  1.3660,  1.4660,
    1.5790,  1.7080,  1.8530,  2.0130,  2.2100,  2.4510,  2.7190,  3.2710},
 { -3.1190,
   -2.7540, -2.4470, -2.2150, -2.0140, -1.8550, -1.7080, -1.5810, -1.4650,
   -1.3660, -1.2610, -1.1510, -1.0480, -0.9565, -0.8679, -0.7731, -0.6689,
   -0.5651, -0.4429, -0.3213, -0.2251, -0.1644, -0.0980, -0.0489,  0.0001,
    0.0490,  0.0982,  0.1645,  0.2251,  0.3213,  0.4423,  0.5649,  0.6692,
    0.7726,  0.8690,  0.9570,  1.0500,  1.1510,  1.2620,  1.3660,  1.4660,
    1.5810,  1.7080,  1.8540,  2.0150,  2.2150,  2.4470,  2.7550,  3.1200},
 { -3.0880,
   -2.7530, -2.4570, -2.2120, -2.0180, -1.8570, -1.7090, -1.5800, -1.4650,
   -1.3660, -1.2620, -1.1510, -1.0490, -0.9574, -0.8691, -0.7732, -0.6690,
   -0.5656, -0.4423, -0.3223, -0.2255, -0.1640, -0.0979, -0.0488,  0.0002,
    0.0486,  0.0979,  0.1642,  0.2258,  0.3210,  0.4431,  0.5646,  0.6692,
    0.7728,  0.8692,  0.9568,  1.0500,  1.1500,  1.2620,  1.3660,  1.4660,
    1.5810,  1.7090,  1.8560,  2.0180,  2.2110,  2.4570,  2.7530,  3.0870}
};
const double Y08V5[6][49]={
 {  0.7026,
    0.9685,  1.0810,  1.0970,  1.1190,  1.1180,  1.1180,  1.0930,  1.0620,
    0.9985,  0.9044,  0.8265,  0.8257,  0.8762,  0.9184,  0.9632,  0.9657,
    0.9801,  1.0130,  1.0070,  1.0140,  1.0120,  0.9777,  0.9091,  0.8649,
    0.9043,  0.9668,  0.9900,  1.0080,  1.0140,  0.9972,  0.9832,  0.9549,
    0.9459,  0.9148,  0.8887,  0.8342,  0.8279,  0.9185,  0.9857,  1.0481,
    1.0868,  1.1139,  1.1270,  1.1048,  1.0940,  1.0680,  0.9540,  0.6734},
 {  0.9392,
    1.1070,  1.1300,  1.1480,  1.1390,  1.1340,  1.1180,  1.0910,  1.0510,
    1.0080,  0.9393,  0.8660,  0.8657,  0.9049,  0.9302,  0.9680,  0.9726,
    0.9821,  1.0010,  1.0120,  1.0110,  1.0050,  0.9796,  0.9424,  0.9186,
    0.9365,  0.9772,  1.0030,  1.0040,  1.0050,  0.9955,  0.9773,  0.9671,
    0.9625,  0.9270,  0.9071,  0.8660,  0.8778,  0.9390,  1.0024,  1.0481,
    1.0864,  1.1192,  1.1324,  1.1364,  1.1510,  1.1250,  1.0970,  0.9380},
 {  1.0220,
    1.1260,  1.1540,  1.1510,  1.1450,  1.1350,  1.1180,  1.0930,  1.0610,
    1.0200,  0.9674,  0.9038,  0.8933,  0.9271,  0.9434,  0.9671,  0.9801,
    0.9935,  1.0060,  1.0110,  1.0070,  1.0000,  0.9859,  0.9614,  0.9459,
    0.9532,  0.9748,  0.9982,  0.9924,  1.0050,  0.9955,  0.9807,  0.9748,
    0.9676,  0.9406,  0.9194,  0.8970,  0.9111,  0.9670,  1.0072,  1.0586,
    1.0867,  1.1146,  1.1359,  1.1488,  1.1490,  1.1490,  1.1150,  0.9983},
 {  0.7026,
    0.9685,  1.0810,  1.0970,  1.1190,  1.1180,  1.1180,  1.0930,  1.0620,
    0.9985,  0.9044,  0.8265,  0.8257,  0.8762,  0.9184,  0.9632,  0.9657,
    0.9801,  1.0130,  1.0070,  1.0140,  1.0120,  0.9777,  0.9091,  0.8649,
    0.9043,  0.9668,  0.9900,  1.0080,  1.0140,  0.9972,  0.9832,  0.9549,
    0.9459,  0.9148,  0.8887,  0.8342,  0.8279,  0.9185,  0.9857,  1.0481,
    1.0868,  1.1139,  1.1270,  1.1048,  1.0940,  1.0680,  0.9540,  0.6734},
 {  0.9392,
    1.1070,  1.1300,  1.1480,  1.1390,  1.1340,  1.1180,  1.0910,  1.0510,
    1.0080,  0.9393,  0.8660,  0.8657,  0.9049,  0.9302,  0.9680,  0.9726,
    0.9821,  1.0010,  1.0120,  1.0110,  1.0050,  0.9796,  0.9424,  0.9186,
    0.9365,  0.9772,  1.0030,  1.0040,  1.0050,  0.9955,  0.9773,  0.9671,
    0.9625,  0.9270,  0.9071,  0.8660,  0.8778,  0.9390,  1.0024,  1.0481,
    1.0864,  1.1192,  1.1324,  1.1364,  1.1510,  1.1250,  1.0970,  0.9380},
 {  1.0220,
    1.1260,  1.1540,  1.1510,  1.1450,  1.1350,  1.1180,  1.0930,  1.0610,
    1.0200,  0.9674,  0.9038,  0.8933,  0.9271,  0.9434,  0.9671,  0.9801,
    0.9935,  1.0060,  1.0110,  1.0070,  1.0000,  0.9859,  0.9614,  0.9459,
    0.9532,  0.9748,  0.9982,  0.9924,  1.0050,  0.9955,  0.9807,  0.9748,
    0.9676,  0.9406,  0.9194,  0.8970,  0.9111,  0.9670,  1.0072,  1.0586,
    1.0867,  1.1146,  1.1359,  1.1488,  1.1490,  1.1490,  1.1150,  0.9983}
};
const double E08V5[6][49]={
 {  0.0139,
    0.0064,  0.0048,  0.0044,  0.0044,  0.0045,  0.0044,  0.0045,  0.0045,
    0.0050,  0.0042,  0.0049,  0.0048,  0.0055,  0.0048,  0.0045,  0.0044,
    0.0057,  0.0058,  0.0054,  0.0076,  0.0055,  0.0055,  0.0081,  0.0064,
    0.0082,  0.0056,  0.0056,  0.0075,  0.0057,  0.0055,  0.0055,  0.0043,
    0.0045,  0.0049,  0.0052,  0.0050,  0.0048,  0.0044,  0.0049,  0.0045,
    0.0044,  0.0043,  0.0043,  0.0043,  0.0044,  0.0047,  0.0062,  0.0136},
 {  0.0065,
    0.0034,  0.0029,  0.0027,  0.0027,  0.0028,  0.0028,  0.0030,  0.0030,
    0.0033,  0.0029,  0.0033,  0.0033,  0.0036,  0.0033,  0.0031,  0.0030,
    0.0037,  0.0039,  0.0038,  0.0050,  0.0037,  0.0037,  0.0055,  0.0042,
    0.0056,  0.0038,  0.0037,  0.0051,  0.0038,  0.0038,  0.0037,  0.0030,
    0.0031,  0.0033,  0.0037,  0.0034,  0.0034,  0.0030,  0.0034,  0.0031,
    0.0030,  0.0028,  0.0027,  0.0027,  0.0027,  0.0028,  0.0032,  0.0067},
 {  0.0062,
    0.0026,  0.0022,  0.0021,  0.0021,  0.0022,  0.0023,  0.0024,  0.0025,
    0.0028,  0.0024,  0.0028,  0.0027,  0.0030,  0.0027,  0.0026,  0.0025,
    0.0032,  0.0033,  0.0032,  0.0043,  0.0031,  0.0032,  0.0045,  0.0033,
    0.0045,  0.0031,  0.0031,  0.0042,  0.0032,  0.0032,  0.0031,  0.0025,
    0.0026,  0.0028,  0.0030,  0.0028,  0.0028,  0.0025,  0.0028,  0.0026,
    0.0024,  0.0023,  0.0022,  0.0021,  0.0021,  0.0022,  0.0026,  0.0060},
 {  0.0139,
    0.0064,  0.0048,  0.0044,  0.0044,  0.0045,  0.0044,  0.0045,  0.0045,
    0.0050,  0.0042,  0.0049,  0.0048,  0.0055,  0.0048,  0.0045,  0.0044,
    0.0057,  0.0058,  0.0054,  0.0076,  0.0055,  0.0055,  0.0081,  0.0064,
    0.0082,  0.0056,  0.0056,  0.0075,  0.0057,  0.0055,  0.0055,  0.0043,
    0.0045,  0.0049,  0.0052,  0.0050,  0.0048,  0.0044,  0.0049,  0.0045,
    0.0044,  0.0043,  0.0043,  0.0043,  0.0044,  0.0047,  0.0062,  0.0136},
 {  0.0065,
    0.0034,  0.0029,  0.0027,  0.0027,  0.0028,  0.0028,  0.0030,  0.0030,
    0.0033,  0.0029,  0.0033,  0.0033,  0.0036,  0.0033,  0.0031,  0.0030,
    0.0037,  0.0039,  0.0038,  0.0050,  0.0037,  0.0037,  0.0055,  0.0042,
    0.0056,  0.0038,  0.0037,  0.0051,  0.0038,  0.0038,  0.0037,  0.0030,
    0.0031,  0.0033,  0.0037,  0.0034,  0.0034,  0.0030,  0.0034,  0.0031,
    0.0030,  0.0028,  0.0027,  0.0027,  0.0027,  0.0028,  0.0032,  0.0067},
 {  0.0062,
    0.0026,  0.0022,  0.0021,  0.0021,  0.0022,  0.0023,  0.0024,  0.0025,
    0.0028,  0.0024,  0.0028,  0.0027,  0.0030,  0.0027,  0.0026,  0.0025,
    0.0032,  0.0033,  0.0032,  0.0043,  0.0031,  0.0032,  0.0045,  0.0033,
    0.0045,  0.0031,  0.0031,  0.0042,  0.0032,  0.0032,  0.0031,  0.0025,
    0.0026,  0.0028,  0.0030,  0.0028,  0.0028,  0.0025,  0.0028,  0.0026,
    0.0024,  0.0023,  0.0022,  0.0021,  0.0021,  0.0022,  0.0026,  0.0060}
};
const double Y208V5[6][49]={
 {  3.0701,
   -0.9043, -2.1556,  1.2522, -1.5737,  1.0916, -2.5973,  0.7927, -4.7067,
   -2.9942,  1.5232,  7.3951,  7.8281, -4.2002,  3.0574, -8.2348,  3.1084,
    2.5123, -5.4937,  3.7790, -2.5499, -3.4019,-26.3856, 12.6131, 44.6349,
   11.4287,-29.5785,  8.2493, -5.6000, -1.9842,  1.7028, -3.6848,  5.0080,
   -5.7458,  3.7637, -7.6307,  6.8564, 12.1106, -5.8623,  2.1440, -4.3355,
   -0.4956, -0.5318, -2.4806,  1.3316, -0.2476, -1.5109, -1.1461,  3.2369},
 {  5.5391,
   -3.5211,  1.3086, -1.3521,  0.6626, -0.7879, -0.5953, -1.4211, -0.2337,
   -2.7449, -1.6651,  8.4329,  5.5888, -5.0038,  4.9120, -7.1440,  2.3608,
    0.4684, -0.6374, -1.0828, -0.5816, -4.0203, -9.7114,  5.2879, 21.8547,
   11.7044,-12.1544, -5.9844,  1.4660, -1.3901, -0.5837,  0.2688,  2.4305,
   -6.9264,  5.9144, -7.4641,  9.4684,  3.7870,  0.3395, -2.0396, -1.2147,
   -0.1886, -1.6623, -0.5220,  1.0378, -1.8764,  1.3077, -2.7599,  3.4077},
 {  3.7948,
   -2.0293, -0.0180, -0.0850, -0.1703, -0.3086, -0.6646, -0.4631, -1.6962,
   -0.3643, -2.2601,  5.4740,  7.2281, -5.6928,  3.1279, -2.8221,  0.8924,
   -0.4599, -0.4301, -0.7688, -1.1668,  0.1437, -8.5004,  4.7855, 11.7220,
    5.8882,  0.1207,-12.5090,  8.2666, -4.8612,  0.5305,  0.5850,  0.6066,
   -3.7908,  2.0072, -1.7466,  5.0719,  4.8781, -3.7944,  4.0391, -4.9740,
    1.0989, -0.9874, -0.2286, -0.6558,  0.1737, -0.1365, -1.9728,  3.3501},
 {  3.0701,
   -0.9043, -2.1556,  1.2522, -1.5737,  1.0916, -2.5973,  0.7927, -4.7067,
   -2.9942,  1.5232,  7.3951,  7.8281, -4.2002,  3.0574, -8.2348,  3.1084,
    2.5123, -5.4937,  3.7790, -2.5499, -3.4019,-26.3856, 12.6131, 44.6349,
   11.4287,-29.5785,  8.2493, -5.6000, -1.9842,  1.7028, -3.6848,  5.0080,
   -5.7458,  3.7637, -7.6307,  6.8564, 12.1106, -5.8623,  2.1440, -4.3355,
   -0.4956, -0.5318, -2.4806,  1.3316, -0.2476, -1.5109, -1.1461,  3.2369},
 {  5.5391,
   -3.5211,  1.3086, -1.3521,  0.6626, -0.7879, -0.5953, -1.4211, -0.2337,
   -2.7449, -1.6651,  8.4329,  5.5888, -5.0038,  4.9120, -7.1440,  2.3608,
    0.4684, -0.6374, -1.0828, -0.5816, -4.0203, -9.7114,  5.2879, 21.8547,
   11.7044,-12.1544, -5.9844,  1.4660, -1.3901, -0.5837,  0.2688,  2.4305,
   -6.9264,  5.9144, -7.4641,  9.4684,  3.7870,  0.3395, -2.0396, -1.2147,
   -0.1886, -1.6623, -0.5220,  1.0378, -1.8764,  1.3077, -2.7599,  3.4077},
 {  3.7948,
   -2.0293, -0.0180, -0.0850, -0.1703, -0.3086, -0.6646, -0.4631, -1.6962,
   -0.3643, -2.2601,  5.4740,  7.2281, -5.6928,  3.1279, -2.8221,  0.8924,
   -0.4599, -0.4301, -0.7688, -1.1668,  0.1437, -8.5004,  4.7855, 11.7220,
    5.8882,  0.1207,-12.5090,  8.2666, -4.8612,  0.5305,  0.5850,  0.6066,
   -3.7908,  2.0072, -1.7466,  5.0719,  4.8781, -3.7944,  4.0391, -4.9740,
    1.0989, -0.9874, -0.2286, -0.6558,  0.1737, -0.1365, -1.9728,  3.3501}
};
