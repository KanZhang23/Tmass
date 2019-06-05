
const int BEG_RUN04V5=203819;
const int END_RUN04V5=207513;

const int NP04V5[6]={49,49,49,49,49,49};

const double X04V5[6][49]={
 {
 -3.271000, -2.719000, -2.452000, -2.211000, -2.013000, -1.853000, -1.708000, -1.579000,
 -1.466000, -1.367000, -1.262000, -1.151000, -1.047000, -0.957200, -0.868500, -0.773200,
 -0.668700, -0.565400, -0.443500, -0.321300, -0.225400, -0.165300, -0.097200, -0.050100,
  0.000000,  0.049400,  0.097500,  0.165100,  0.224400,  0.321000,  0.442800,  0.564800,
  0.669200,  0.772700,  0.868900,  0.957200,  1.049000,  1.151000,  1.263000,  1.366000,
  1.466000,  1.579000,  1.708000,  1.853000,  2.013000,  2.210000,  2.451000,  2.719000,
  3.271000
 }, {
 -3.119000, -2.754000, -2.447000, -2.215000, -2.014000, -1.855000, -1.708000, -1.581000,
 -1.465000, -1.366000, -1.261000, -1.151000, -1.048000, -0.956500, -0.867900, -0.773100,
 -0.668900, -0.565100, -0.442900, -0.321300, -0.225100, -0.164400, -0.098000, -0.048900,
  0.000100,  0.049000,  0.098200,  0.164500,  0.225100,  0.321300,  0.442300,  0.564900,
  0.669200,  0.772600,  0.869000,  0.957000,  1.050000,  1.151000,  1.262000,  1.366000,
  1.466000,  1.581000,  1.708000,  1.854000,  2.015000,  2.215000,  2.447000,  2.755000,
  3.120000
 }, {
 -3.088000, -2.753000, -2.457000, -2.212000, -2.018000, -1.857000, -1.709000, -1.580000,
 -1.465000, -1.366000, -1.262000, -1.151000, -1.049000, -0.957400, -0.869100, -0.773200,
 -0.669000, -0.565600, -0.442300, -0.322300, -0.225500, -0.164000, -0.097900, -0.048800,
  0.000200,  0.048600,  0.097900,  0.164200,  0.225800,  0.321000,  0.443100,  0.564600,
  0.669200,  0.772800,  0.869200,  0.956800,  1.050000,  1.150000,  1.262000,  1.366000,
  1.466000,  1.581000,  1.709000,  1.856000,  2.018000,  2.211000,  2.457000,  2.753000,
  3.087000
 }, {
 -3.271000, -2.719000, -2.452000, -2.211000, -2.013000, -1.853000, -1.708000, -1.579000,
 -1.466000, -1.367000, -1.262000, -1.151000, -1.047000, -0.957200, -0.868500, -0.773200,
 -0.668700, -0.565400, -0.443500, -0.321300, -0.225400, -0.165300, -0.097200, -0.050100,
  0.000000,  0.049400,  0.097500,  0.165100,  0.224400,  0.321000,  0.442800,  0.564800,
  0.669200,  0.772700,  0.868900,  0.957200,  1.049000,  1.151000,  1.263000,  1.366000,
  1.466000,  1.579000,  1.708000,  1.853000,  2.013000,  2.210000,  2.451000,  2.719000,
  3.271000
 }, {
 -3.119000, -2.754000, -2.447000, -2.215000, -2.014000, -1.855000, -1.708000, -1.581000,
 -1.465000, -1.366000, -1.261000, -1.151000, -1.048000, -0.956500, -0.867900, -0.773100,
 -0.668900, -0.565100, -0.442900, -0.321300, -0.225100, -0.164400, -0.098000, -0.048900,
  0.000100,  0.049000,  0.098200,  0.164500,  0.225100,  0.321300,  0.442300,  0.564900,
  0.669200,  0.772600,  0.869000,  0.957000,  1.050000,  1.151000,  1.262000,  1.366000,
  1.466000,  1.581000,  1.708000,  1.854000,  2.015000,  2.215000,  2.447000,  2.755000,
  3.120000
 }, {
 -3.088000, -2.753000, -2.457000, -2.212000, -2.018000, -1.857000, -1.709000, -1.580000,
 -1.465000, -1.366000, -1.262000, -1.151000, -1.049000, -0.957400, -0.869100, -0.773200,
 -0.669000, -0.565600, -0.442300, -0.322300, -0.225500, -0.164000, -0.097900, -0.048800,
  0.000200,  0.048600,  0.097900,  0.164200,  0.225800,  0.321000,  0.443100,  0.564600,
  0.669200,  0.772800,  0.869200,  0.956800,  1.050000,  1.150000,  1.262000,  1.366000,
  1.466000,  1.581000,  1.709000,  1.856000,  2.018000,  2.211000,  2.457000,  2.753000,
  3.087000
 }
 };


const double Y04V5[6][49]={
 {
  0.702600,  0.968500,  1.081000,  1.097000,  1.119000,  1.118000,  1.118000,  1.093000,
  1.062000,  0.991065,  0.916622,  0.837914,  0.847565,  0.905398,  0.948100,  0.967200,
  0.965700,  0.980100,  1.013000,  1.007000,  1.014000,  1.012000,  0.977700,  0.909100,
  0.864900,  0.904300,  0.966800,  0.990000,  1.008000,  1.014000,  0.997200,  0.983200,
  0.954900,  0.955300,  0.922762,  0.887734,  0.836770,  0.827647,  0.910759,  0.988937,
  1.052732,  1.086000,  1.109000,  1.123000,  1.104000,  1.094000,  1.068000,  0.954000,
  0.673400
 }, {
  0.939200,  1.107000,  1.130000,  1.148000,  1.139000,  1.134000,  1.118000,  1.091000,
  1.051000,  1.002927,  0.941676,  0.883878,  0.879634,  0.924197,  0.958000,  0.975000,
  0.972600,  0.982100,  1.001000,  1.012000,  1.011000,  1.005000,  0.979600,  0.942400,
  0.918600,  0.936500,  0.977200,  1.003000,  1.004000,  1.005000,  0.995500,  0.977300,
  0.967100,  0.960000,  0.933834,  0.908301,  0.867031,  0.872429,  0.930266,  1.000687,
  1.045838,  1.092000,  1.113000,  1.134000,  1.140000,  1.151000,  1.125000,  1.097000,
  0.938000
 }, {
  1.022000,  1.126000,  1.154000,  1.151000,  1.145000,  1.135000,  1.118000,  1.093000,
  1.061000,  1.018400,  0.970740,  0.916150,  0.907427,  0.936797,  0.963600,  0.975700,
  0.980100,  0.993500,  1.006000,  1.011000,  1.007000,  1.000000,  0.985900,  0.961400,
  0.945900,  0.953200,  0.974800,  0.998200,  0.992400,  1.005000,  0.995500,  0.980700,
  0.974800,  0.968400,  0.943916,  0.920675,  0.888332,  0.904292,  0.956602,  1.009197,
  1.056582,  1.095000,  1.112000,  1.133000,  1.144000,  1.149000,  1.149000,  1.115000,
  0.998300
 }, {
  0.702600,  0.968500,  1.081000,  1.097000,  1.119000,  1.118000,  1.118000,  1.093000,
  1.062000,  0.991065,  0.916622,  0.837914,  0.847565,  0.905398,  0.948100,  0.967200,
  0.965700,  0.980100,  1.013000,  1.007000,  1.014000,  1.012000,  0.977700,  0.909100,
  0.864900,  0.904300,  0.966800,  0.990000,  1.008000,  1.014000,  0.997200,  0.983200,
  0.954900,  0.955300,  0.922762,  0.887734,  0.836770,  0.827647,  0.910759,  0.988937,
  1.052732,  1.086000,  1.109000,  1.123000,  1.104000,  1.094000,  1.068000,  0.954000,
  0.673400
 }, {
  0.939200,  1.107000,  1.130000,  1.148000,  1.139000,  1.134000,  1.118000,  1.091000,
  1.051000,  1.002927,  0.941676,  0.883878,  0.879634,  0.924197,  0.958000,  0.975000,
  0.972600,  0.982100,  1.001000,  1.012000,  1.011000,  1.005000,  0.979600,  0.942400,
  0.918600,  0.936500,  0.977200,  1.003000,  1.004000,  1.005000,  0.995500,  0.977300,
  0.967100,  0.960000,  0.933834,  0.908301,  0.867031,  0.872429,  0.930266,  1.000687,
  1.045838,  1.092000,  1.113000,  1.134000,  1.140000,  1.151000,  1.125000,  1.097000,
  0.938000
 }, {
  1.022000,  1.126000,  1.154000,  1.151000,  1.145000,  1.135000,  1.118000,  1.093000,
  1.061000,  1.018400,  0.970740,  0.916150,  0.907427,  0.936797,  0.963600,  0.975700,
  0.980100,  0.993500,  1.006000,  1.011000,  1.007000,  1.000000,  0.985900,  0.961400,
  0.945900,  0.953200,  0.974800,  0.998200,  0.992400,  1.005000,  0.995500,  0.980700,
  0.974800,  0.968400,  0.943916,  0.920675,  0.888332,  0.904292,  0.956602,  1.009197,
  1.056582,  1.095000,  1.112000,  1.133000,  1.144000,  1.149000,  1.149000,  1.115000,
  0.998300
 }
 };


const double E04V5[6][49]={
 {
  0.013900,  0.006400,  0.004800,  0.004400,  0.004400,  0.004500,  0.004400,  0.004500,
  0.004500,  0.005000,  0.004200,  0.004900,  0.004800,  0.005500,  0.004800,  0.004500,
  0.004400,  0.005700,  0.005800,  0.005400,  0.007600,  0.005500,  0.005500,  0.008100,
  0.006400,  0.008200,  0.005600,  0.005600,  0.007500,  0.005700,  0.005500,  0.005500,
  0.004300,  0.004500,  0.004900,  0.005200,  0.005000,  0.004800,  0.004400,  0.004900,
  0.004500,  0.004400,  0.004300,  0.004300,  0.004300,  0.004400,  0.004700,  0.006200,
  0.013600
 }, {
  0.006500,  0.003400,  0.002900,  0.002700,  0.002700,  0.002800,  0.002800,  0.003000,
  0.003000,  0.003300,  0.002900,  0.003300,  0.003300,  0.003600,  0.003300,  0.003100,
  0.003000,  0.003700,  0.003900,  0.003800,  0.005000,  0.003700,  0.003700,  0.005500,
  0.004200,  0.005600,  0.003800,  0.003700,  0.005100,  0.003800,  0.003800,  0.003700,
  0.003000,  0.003100,  0.003300,  0.003700,  0.003400,  0.003400,  0.003000,  0.003400,
  0.003100,  0.003000,  0.002800,  0.002700,  0.002700,  0.002700,  0.002800,  0.003200,
  0.006700
 }, {
  0.006200,  0.002600,  0.002200,  0.002100,  0.002100,  0.002200,  0.002300,  0.002400,
  0.002500,  0.002800,  0.002400,  0.002800,  0.002700,  0.003000,  0.002700,  0.002600,
  0.002500,  0.003200,  0.003300,  0.003200,  0.004300,  0.003100,  0.003200,  0.004500,
  0.003300,  0.004500,  0.003100,  0.003100,  0.004200,  0.003200,  0.003200,  0.003100,
  0.002500,  0.002600,  0.002800,  0.003000,  0.002800,  0.002800,  0.002500,  0.002800,
  0.002600,  0.002400,  0.002300,  0.002200,  0.002100,  0.002100,  0.002200,  0.002600,
  0.006000
 }, {
  0.013900,  0.006400,  0.004800,  0.004400,  0.004400,  0.004500,  0.004400,  0.004500,
  0.004500,  0.005000,  0.004200,  0.004900,  0.004800,  0.005500,  0.004800,  0.004500,
  0.004400,  0.005700,  0.005800,  0.005400,  0.007600,  0.005500,  0.005500,  0.008100,
  0.006400,  0.008200,  0.005600,  0.005600,  0.007500,  0.005700,  0.005500,  0.005500,
  0.004300,  0.004500,  0.004900,  0.005200,  0.005000,  0.004800,  0.004400,  0.004900,
  0.004500,  0.004400,  0.004300,  0.004300,  0.004300,  0.004400,  0.004700,  0.006200,
  0.013600
 }, {
  0.006500,  0.003400,  0.002900,  0.002700,  0.002700,  0.002800,  0.002800,  0.003000,
  0.003000,  0.003300,  0.002900,  0.003300,  0.003300,  0.003600,  0.003300,  0.003100,
  0.003000,  0.003700,  0.003900,  0.003800,  0.005000,  0.003700,  0.003700,  0.005500,
  0.004200,  0.005600,  0.003800,  0.003700,  0.005100,  0.003800,  0.003800,  0.003700,
  0.003000,  0.003100,  0.003300,  0.003700,  0.003400,  0.003400,  0.003000,  0.003400,
  0.003100,  0.003000,  0.002800,  0.002700,  0.002700,  0.002700,  0.002800,  0.003200,
  0.006700
 }, {
  0.006200,  0.002600,  0.002200,  0.002100,  0.002100,  0.002200,  0.002300,  0.002400,
  0.002500,  0.002800,  0.002400,  0.002800,  0.002700,  0.003000,  0.002700,  0.002600,
  0.002500,  0.003200,  0.003300,  0.003200,  0.004300,  0.003100,  0.003200,  0.004500,
  0.003300,  0.004500,  0.003100,  0.003100,  0.004200,  0.003200,  0.003200,  0.003100,
  0.002500,  0.002600,  0.002800,  0.003000,  0.002800,  0.002800,  0.002500,  0.002800,
  0.002600,  0.002400,  0.002300,  0.002200,  0.002100,  0.002100,  0.002200,  0.002600,
  0.006000
 }
 };


const double Y204V5[6][49]={
 {
  3.070057, -0.904212, -2.156177,  1.254568, -1.583563,  1.132877, -2.759895,  1.436937,
 -7.280334,  2.741317, -3.356631, 10.464986,  6.575579, -3.671038, -2.881027, -3.149676,
  2.339997,  2.702436, -5.545018,  3.794533, -2.554939, -3.400637,-26.385964, 12.613216,
 44.634932, 11.428436,-29.577505,  8.246165, -5.587875, -2.021482,  1.826890, -4.143554,
  6.852571, -7.411466,  2.061301, -4.511710,  5.363057, 11.075727, -2.663976,  0.064720,
 -4.782442, -0.268806, -0.202974, -2.376218,  1.169463, -0.237919, -1.513284, -1.145798,
  3.237153
 }, {
  5.539089, -3.521037,  1.308344, -1.350992,  0.657957, -0.768569, -0.670973, -1.116980,
 -1.425077, -1.032359, -0.230933,  5.046850,  7.580732, -3.271644, -1.672535, -3.268429,
  2.353606,  0.470124, -0.637858, -1.082676, -0.581664, -4.020282, -9.711404,  5.287921,
 21.854674, 11.704528,-12.155006, -5.982343,  1.458148, -1.365748, -0.664772,  0.566926,
  1.228788, -3.818108,  1.888377, -5.007020,  7.792049,  4.213400,  2.092086, -4.143195,
  1.191468, -3.466060,  0.978129, -1.527267,  0.965986, -1.725772,  1.273518, -2.753445,
  3.412737
 }, {
  3.794788, -2.029318, -0.018153, -0.084559, -0.172201, -0.300850, -0.695059, -0.341188,
 -2.179238,  0.603274, -1.894735,  4.962894,  5.233028, -1.042749, -2.341008, -1.144032,
  1.714632, -0.662505, -0.374725, -0.785369, -1.161474,  0.142335, -8.499929,  4.785421,
 11.721995,  5.888174,  0.120707,-12.509057,  8.266752, -4.861793,  0.532289,  0.578273,
  0.633499, -3.441124,  1.634819, -3.856734,  8.165435,  2.441343, -0.061920, -0.141779,
 -1.268560, -2.426692,  0.919703, -0.918044, -0.108521, -0.135720, -0.062334, -1.987271,
  3.338906
 }, {
  3.070057, -0.904212, -2.156177,  1.254568, -1.583563,  1.132877, -2.759895,  1.436937,
 -7.280334,  2.741317, -3.356631, 10.464986,  6.575579, -3.671038, -2.881027, -3.149676,
  2.339997,  2.702436, -5.545018,  3.794533, -2.554939, -3.400637,-26.385964, 12.613216,
 44.634932, 11.428436,-29.577505,  8.246165, -5.587875, -2.021482,  1.826890, -4.143554,
  6.852571, -7.411466,  2.061301, -4.511710,  5.363057, 11.075727, -2.663976,  0.064720,
 -4.782442, -0.268806, -0.202974, -2.376218,  1.169463, -0.237919, -1.513284, -1.145798,
  3.237153
 }, {
  5.539089, -3.521037,  1.308344, -1.350992,  0.657957, -0.768569, -0.670973, -1.116980,
 -1.425077, -1.032359, -0.230933,  5.046850,  7.580732, -3.271644, -1.672535, -3.268429,
  2.353606,  0.470124, -0.637858, -1.082676, -0.581664, -4.020282, -9.711404,  5.287921,
 21.854674, 11.704528,-12.155006, -5.982343,  1.458148, -1.365748, -0.664772,  0.566926,
  1.228788, -3.818108,  1.888377, -5.007020,  7.792049,  4.213400,  2.092086, -4.143195,
  1.191468, -3.466060,  0.978129, -1.527267,  0.965986, -1.725772,  1.273518, -2.753445,
  3.412737
 }, {
  3.794788, -2.029318, -0.018153, -0.084559, -0.172201, -0.300850, -0.695059, -0.341188,
 -2.179238,  0.603274, -1.894735,  4.962894,  5.233028, -1.042749, -2.341008, -1.144032,
  1.714632, -0.662505, -0.374725, -0.785369, -1.161474,  0.142335, -8.499929,  4.785421,
 11.721995,  5.888174,  0.120707,-12.509057,  8.266752, -4.861793,  0.532289,  0.578273,
  0.633499, -3.441124,  1.634819, -3.856734,  8.165435,  2.441343, -0.061920, -0.141779,
 -1.268560, -2.426692,  0.919703, -0.918044, -0.108521, -0.135720, -0.062334, -1.987271,
  3.338906
 }
 };


