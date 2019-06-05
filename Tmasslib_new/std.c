#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void std_(double plab[10][4], double spl[10][10][2], double smn[10][10][2]);

#define RE 0
#define IM 1

void std_(double plab[10][4], double spl[10][10][2], double smn[10][10][2])
{
    int i;
    double root[10];

    static int init = 1;
    if (init)
    {
	for (i=0; i<10; ++i)
	{
	    spl[i][i][RE] = 0.0;
	    smn[i][i][IM] = 0.0;
	}
	init = 0;
    }

    for (i=0; i<10; ++i)
    {
	register const double diff = plab[i][3] - plab[i][0];
	root[i] = diff > 0.0 ? sqrt(diff) : 0.0;
    }

    /* Unroll the big 2d loop */
    {
        register const double rat = root[0]/root[1];
        register const double re  = plab[1][1]*rat - plab[0][1]/rat;
        register const double im  = plab[1][2]*rat - plab[0][2]/rat;
        spl[0][1][RE] = re;
        spl[0][1][IM] = im;
        spl[1][0][RE] = -re;
        spl[1][0][IM] = -im;
        smn[0][1][RE] = -re;
        smn[0][1][IM] = im;
        smn[1][0][RE] = re;
        smn[1][0][IM] = -im;
    }
    {
        register const double rat = root[0]/root[2];
        register const double re  = plab[2][1]*rat - plab[0][1]/rat;
        register const double im  = plab[2][2]*rat - plab[0][2]/rat;
        spl[0][2][RE] = re;
        spl[0][2][IM] = im;
        spl[2][0][RE] = -re;
        spl[2][0][IM] = -im;
        smn[0][2][RE] = -re;
        smn[0][2][IM] = im;
        smn[2][0][RE] = re;
        smn[2][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[2];
        register const double re  = plab[2][1]*rat - plab[1][1]/rat;
        register const double im  = plab[2][2]*rat - plab[1][2]/rat;
        spl[1][2][RE] = re;
        spl[1][2][IM] = im;
        spl[2][1][RE] = -re;
        spl[2][1][IM] = -im;
        smn[1][2][RE] = -re;
        smn[1][2][IM] = im;
        smn[2][1][RE] = re;
        smn[2][1][IM] = -im;
    }
    {
        register const double rat = root[0]/root[3];
        register const double re  = plab[3][1]*rat - plab[0][1]/rat;
        register const double im  = plab[3][2]*rat - plab[0][2]/rat;
        spl[0][3][RE] = re;
        spl[0][3][IM] = im;
        spl[3][0][RE] = -re;
        spl[3][0][IM] = -im;
        smn[0][3][RE] = -re;
        smn[0][3][IM] = im;
        smn[3][0][RE] = re;
        smn[3][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[3];
        register const double re  = plab[3][1]*rat - plab[1][1]/rat;
        register const double im  = plab[3][2]*rat - plab[1][2]/rat;
        spl[1][3][RE] = re;
        spl[1][3][IM] = im;
        spl[3][1][RE] = -re;
        spl[3][1][IM] = -im;
        smn[1][3][RE] = -re;
        smn[1][3][IM] = im;
        smn[3][1][RE] = re;
        smn[3][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[3];
        register const double re  = plab[3][1]*rat - plab[2][1]/rat;
        register const double im  = plab[3][2]*rat - plab[2][2]/rat;
        spl[2][3][RE] = re;
        spl[2][3][IM] = im;
        spl[3][2][RE] = -re;
        spl[3][2][IM] = -im;
        smn[2][3][RE] = -re;
        smn[2][3][IM] = im;
        smn[3][2][RE] = re;
        smn[3][2][IM] = -im;
    }
    {
        register const double rat = root[0]/root[4];
        register const double re  = plab[4][1]*rat - plab[0][1]/rat;
        register const double im  = plab[4][2]*rat - plab[0][2]/rat;
        spl[0][4][RE] = re;
        spl[0][4][IM] = im;
        spl[4][0][RE] = -re;
        spl[4][0][IM] = -im;
        smn[0][4][RE] = -re;
        smn[0][4][IM] = im;
        smn[4][0][RE] = re;
        smn[4][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[4];
        register const double re  = plab[4][1]*rat - plab[1][1]/rat;
        register const double im  = plab[4][2]*rat - plab[1][2]/rat;
        spl[1][4][RE] = re;
        spl[1][4][IM] = im;
        spl[4][1][RE] = -re;
        spl[4][1][IM] = -im;
        smn[1][4][RE] = -re;
        smn[1][4][IM] = im;
        smn[4][1][RE] = re;
        smn[4][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[4];
        register const double re  = plab[4][1]*rat - plab[2][1]/rat;
        register const double im  = plab[4][2]*rat - plab[2][2]/rat;
        spl[2][4][RE] = re;
        spl[2][4][IM] = im;
        spl[4][2][RE] = -re;
        spl[4][2][IM] = -im;
        smn[2][4][RE] = -re;
        smn[2][4][IM] = im;
        smn[4][2][RE] = re;
        smn[4][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[4];
        register const double re  = plab[4][1]*rat - plab[3][1]/rat;
        register const double im  = plab[4][2]*rat - plab[3][2]/rat;
        spl[3][4][RE] = re;
        spl[3][4][IM] = im;
        spl[4][3][RE] = -re;
        spl[4][3][IM] = -im;
        smn[3][4][RE] = -re;
        smn[3][4][IM] = im;
        smn[4][3][RE] = re;
        smn[4][3][IM] = -im;
    }
    {
        register const double rat = root[0]/root[5];
        register const double re  = plab[5][1]*rat - plab[0][1]/rat;
        register const double im  = plab[5][2]*rat - plab[0][2]/rat;
        spl[0][5][RE] = re;
        spl[0][5][IM] = im;
        spl[5][0][RE] = -re;
        spl[5][0][IM] = -im;
        smn[0][5][RE] = -re;
        smn[0][5][IM] = im;
        smn[5][0][RE] = re;
        smn[5][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[5];
        register const double re  = plab[5][1]*rat - plab[1][1]/rat;
        register const double im  = plab[5][2]*rat - plab[1][2]/rat;
        spl[1][5][RE] = re;
        spl[1][5][IM] = im;
        spl[5][1][RE] = -re;
        spl[5][1][IM] = -im;
        smn[1][5][RE] = -re;
        smn[1][5][IM] = im;
        smn[5][1][RE] = re;
        smn[5][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[5];
        register const double re  = plab[5][1]*rat - plab[2][1]/rat;
        register const double im  = plab[5][2]*rat - plab[2][2]/rat;
        spl[2][5][RE] = re;
        spl[2][5][IM] = im;
        spl[5][2][RE] = -re;
        spl[5][2][IM] = -im;
        smn[2][5][RE] = -re;
        smn[2][5][IM] = im;
        smn[5][2][RE] = re;
        smn[5][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[5];
        register const double re  = plab[5][1]*rat - plab[3][1]/rat;
        register const double im  = plab[5][2]*rat - plab[3][2]/rat;
        spl[3][5][RE] = re;
        spl[3][5][IM] = im;
        spl[5][3][RE] = -re;
        spl[5][3][IM] = -im;
        smn[3][5][RE] = -re;
        smn[3][5][IM] = im;
        smn[5][3][RE] = re;
        smn[5][3][IM] = -im;
    }
    {
        register const double rat = root[4]/root[5];
        register const double re  = plab[5][1]*rat - plab[4][1]/rat;
        register const double im  = plab[5][2]*rat - plab[4][2]/rat;
        spl[4][5][RE] = re;
        spl[4][5][IM] = im;
        spl[5][4][RE] = -re;
        spl[5][4][IM] = -im;
        smn[4][5][RE] = -re;
        smn[4][5][IM] = im;
        smn[5][4][RE] = re;
        smn[5][4][IM] = -im;
    }
    {
        register const double rat = root[0]/root[6];
        register const double re  = plab[6][1]*rat - plab[0][1]/rat;
        register const double im  = plab[6][2]*rat - plab[0][2]/rat;
        spl[0][6][RE] = re;
        spl[0][6][IM] = im;
        spl[6][0][RE] = -re;
        spl[6][0][IM] = -im;
        smn[0][6][RE] = -re;
        smn[0][6][IM] = im;
        smn[6][0][RE] = re;
        smn[6][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[6];
        register const double re  = plab[6][1]*rat - plab[1][1]/rat;
        register const double im  = plab[6][2]*rat - plab[1][2]/rat;
        spl[1][6][RE] = re;
        spl[1][6][IM] = im;
        spl[6][1][RE] = -re;
        spl[6][1][IM] = -im;
        smn[1][6][RE] = -re;
        smn[1][6][IM] = im;
        smn[6][1][RE] = re;
        smn[6][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[6];
        register const double re  = plab[6][1]*rat - plab[2][1]/rat;
        register const double im  = plab[6][2]*rat - plab[2][2]/rat;
        spl[2][6][RE] = re;
        spl[2][6][IM] = im;
        spl[6][2][RE] = -re;
        spl[6][2][IM] = -im;
        smn[2][6][RE] = -re;
        smn[2][6][IM] = im;
        smn[6][2][RE] = re;
        smn[6][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[6];
        register const double re  = plab[6][1]*rat - plab[3][1]/rat;
        register const double im  = plab[6][2]*rat - plab[3][2]/rat;
        spl[3][6][RE] = re;
        spl[3][6][IM] = im;
        spl[6][3][RE] = -re;
        spl[6][3][IM] = -im;
        smn[3][6][RE] = -re;
        smn[3][6][IM] = im;
        smn[6][3][RE] = re;
        smn[6][3][IM] = -im;
    }
    {
        register const double rat = root[4]/root[6];
        register const double re  = plab[6][1]*rat - plab[4][1]/rat;
        register const double im  = plab[6][2]*rat - plab[4][2]/rat;
        spl[4][6][RE] = re;
        spl[4][6][IM] = im;
        spl[6][4][RE] = -re;
        spl[6][4][IM] = -im;
        smn[4][6][RE] = -re;
        smn[4][6][IM] = im;
        smn[6][4][RE] = re;
        smn[6][4][IM] = -im;
    }
    {
        register const double rat = root[5]/root[6];
        register const double re  = plab[6][1]*rat - plab[5][1]/rat;
        register const double im  = plab[6][2]*rat - plab[5][2]/rat;
        spl[5][6][RE] = re;
        spl[5][6][IM] = im;
        spl[6][5][RE] = -re;
        spl[6][5][IM] = -im;
        smn[5][6][RE] = -re;
        smn[5][6][IM] = im;
        smn[6][5][RE] = re;
        smn[6][5][IM] = -im;
    }
    {
        register const double rat = root[0]/root[7];
        register const double re  = plab[7][1]*rat - plab[0][1]/rat;
        register const double im  = plab[7][2]*rat - plab[0][2]/rat;
        spl[0][7][RE] = re;
        spl[0][7][IM] = im;
        spl[7][0][RE] = -re;
        spl[7][0][IM] = -im;
        smn[0][7][RE] = -re;
        smn[0][7][IM] = im;
        smn[7][0][RE] = re;
        smn[7][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[7];
        register const double re  = plab[7][1]*rat - plab[1][1]/rat;
        register const double im  = plab[7][2]*rat - plab[1][2]/rat;
        spl[1][7][RE] = re;
        spl[1][7][IM] = im;
        spl[7][1][RE] = -re;
        spl[7][1][IM] = -im;
        smn[1][7][RE] = -re;
        smn[1][7][IM] = im;
        smn[7][1][RE] = re;
        smn[7][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[7];
        register const double re  = plab[7][1]*rat - plab[2][1]/rat;
        register const double im  = plab[7][2]*rat - plab[2][2]/rat;
        spl[2][7][RE] = re;
        spl[2][7][IM] = im;
        spl[7][2][RE] = -re;
        spl[7][2][IM] = -im;
        smn[2][7][RE] = -re;
        smn[2][7][IM] = im;
        smn[7][2][RE] = re;
        smn[7][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[7];
        register const double re  = plab[7][1]*rat - plab[3][1]/rat;
        register const double im  = plab[7][2]*rat - plab[3][2]/rat;
        spl[3][7][RE] = re;
        spl[3][7][IM] = im;
        spl[7][3][RE] = -re;
        spl[7][3][IM] = -im;
        smn[3][7][RE] = -re;
        smn[3][7][IM] = im;
        smn[7][3][RE] = re;
        smn[7][3][IM] = -im;
    }
    {
        register const double rat = root[4]/root[7];
        register const double re  = plab[7][1]*rat - plab[4][1]/rat;
        register const double im  = plab[7][2]*rat - plab[4][2]/rat;
        spl[4][7][RE] = re;
        spl[4][7][IM] = im;
        spl[7][4][RE] = -re;
        spl[7][4][IM] = -im;
        smn[4][7][RE] = -re;
        smn[4][7][IM] = im;
        smn[7][4][RE] = re;
        smn[7][4][IM] = -im;
    }
    {
        register const double rat = root[5]/root[7];
        register const double re  = plab[7][1]*rat - plab[5][1]/rat;
        register const double im  = plab[7][2]*rat - plab[5][2]/rat;
        spl[5][7][RE] = re;
        spl[5][7][IM] = im;
        spl[7][5][RE] = -re;
        spl[7][5][IM] = -im;
        smn[5][7][RE] = -re;
        smn[5][7][IM] = im;
        smn[7][5][RE] = re;
        smn[7][5][IM] = -im;
    }
    {
        register const double rat = root[6]/root[7];
        register const double re  = plab[7][1]*rat - plab[6][1]/rat;
        register const double im  = plab[7][2]*rat - plab[6][2]/rat;
        spl[6][7][RE] = re;
        spl[6][7][IM] = im;
        spl[7][6][RE] = -re;
        spl[7][6][IM] = -im;
        smn[6][7][RE] = -re;
        smn[6][7][IM] = im;
        smn[7][6][RE] = re;
        smn[7][6][IM] = -im;
    }
    {
        register const double rat = root[0]/root[8];
        register const double re  = plab[8][1]*rat - plab[0][1]/rat;
        register const double im  = plab[8][2]*rat - plab[0][2]/rat;
        spl[0][8][RE] = re;
        spl[0][8][IM] = im;
        spl[8][0][RE] = -re;
        spl[8][0][IM] = -im;
        smn[0][8][RE] = -re;
        smn[0][8][IM] = im;
        smn[8][0][RE] = re;
        smn[8][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[8];
        register const double re  = plab[8][1]*rat - plab[1][1]/rat;
        register const double im  = plab[8][2]*rat - plab[1][2]/rat;
        spl[1][8][RE] = re;
        spl[1][8][IM] = im;
        spl[8][1][RE] = -re;
        spl[8][1][IM] = -im;
        smn[1][8][RE] = -re;
        smn[1][8][IM] = im;
        smn[8][1][RE] = re;
        smn[8][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[8];
        register const double re  = plab[8][1]*rat - plab[2][1]/rat;
        register const double im  = plab[8][2]*rat - plab[2][2]/rat;
        spl[2][8][RE] = re;
        spl[2][8][IM] = im;
        spl[8][2][RE] = -re;
        spl[8][2][IM] = -im;
        smn[2][8][RE] = -re;
        smn[2][8][IM] = im;
        smn[8][2][RE] = re;
        smn[8][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[8];
        register const double re  = plab[8][1]*rat - plab[3][1]/rat;
        register const double im  = plab[8][2]*rat - plab[3][2]/rat;
        spl[3][8][RE] = re;
        spl[3][8][IM] = im;
        spl[8][3][RE] = -re;
        spl[8][3][IM] = -im;
        smn[3][8][RE] = -re;
        smn[3][8][IM] = im;
        smn[8][3][RE] = re;
        smn[8][3][IM] = -im;
    }
    {
        register const double rat = root[4]/root[8];
        register const double re  = plab[8][1]*rat - plab[4][1]/rat;
        register const double im  = plab[8][2]*rat - plab[4][2]/rat;
        spl[4][8][RE] = re;
        spl[4][8][IM] = im;
        spl[8][4][RE] = -re;
        spl[8][4][IM] = -im;
        smn[4][8][RE] = -re;
        smn[4][8][IM] = im;
        smn[8][4][RE] = re;
        smn[8][4][IM] = -im;
    }
    {
        register const double rat = root[5]/root[8];
        register const double re  = plab[8][1]*rat - plab[5][1]/rat;
        register const double im  = plab[8][2]*rat - plab[5][2]/rat;
        spl[5][8][RE] = re;
        spl[5][8][IM] = im;
        spl[8][5][RE] = -re;
        spl[8][5][IM] = -im;
        smn[5][8][RE] = -re;
        smn[5][8][IM] = im;
        smn[8][5][RE] = re;
        smn[8][5][IM] = -im;
    }
    {
        register const double rat = root[6]/root[8];
        register const double re  = plab[8][1]*rat - plab[6][1]/rat;
        register const double im  = plab[8][2]*rat - plab[6][2]/rat;
        spl[6][8][RE] = re;
        spl[6][8][IM] = im;
        spl[8][6][RE] = -re;
        spl[8][6][IM] = -im;
        smn[6][8][RE] = -re;
        smn[6][8][IM] = im;
        smn[8][6][RE] = re;
        smn[8][6][IM] = -im;
    }
    {
        register const double rat = root[7]/root[8];
        register const double re  = plab[8][1]*rat - plab[7][1]/rat;
        register const double im  = plab[8][2]*rat - plab[7][2]/rat;
        spl[7][8][RE] = re;
        spl[7][8][IM] = im;
        spl[8][7][RE] = -re;
        spl[8][7][IM] = -im;
        smn[7][8][RE] = -re;
        smn[7][8][IM] = im;
        smn[8][7][RE] = re;
        smn[8][7][IM] = -im;
    }
    {
        register const double rat = root[0]/root[9];
        register const double re  = plab[9][1]*rat - plab[0][1]/rat;
        register const double im  = plab[9][2]*rat - plab[0][2]/rat;
        spl[0][9][RE] = re;
        spl[0][9][IM] = im;
        spl[9][0][RE] = -re;
        spl[9][0][IM] = -im;
        smn[0][9][RE] = -re;
        smn[0][9][IM] = im;
        smn[9][0][RE] = re;
        smn[9][0][IM] = -im;
    }
    {
        register const double rat = root[1]/root[9];
        register const double re  = plab[9][1]*rat - plab[1][1]/rat;
        register const double im  = plab[9][2]*rat - plab[1][2]/rat;
        spl[1][9][RE] = re;
        spl[1][9][IM] = im;
        spl[9][1][RE] = -re;
        spl[9][1][IM] = -im;
        smn[1][9][RE] = -re;
        smn[1][9][IM] = im;
        smn[9][1][RE] = re;
        smn[9][1][IM] = -im;
    }
    {
        register const double rat = root[2]/root[9];
        register const double re  = plab[9][1]*rat - plab[2][1]/rat;
        register const double im  = plab[9][2]*rat - plab[2][2]/rat;
        spl[2][9][RE] = re;
        spl[2][9][IM] = im;
        spl[9][2][RE] = -re;
        spl[9][2][IM] = -im;
        smn[2][9][RE] = -re;
        smn[2][9][IM] = im;
        smn[9][2][RE] = re;
        smn[9][2][IM] = -im;
    }
    {
        register const double rat = root[3]/root[9];
        register const double re  = plab[9][1]*rat - plab[3][1]/rat;
        register const double im  = plab[9][2]*rat - plab[3][2]/rat;
        spl[3][9][RE] = re;
        spl[3][9][IM] = im;
        spl[9][3][RE] = -re;
        spl[9][3][IM] = -im;
        smn[3][9][RE] = -re;
        smn[3][9][IM] = im;
        smn[9][3][RE] = re;
        smn[9][3][IM] = -im;
    }
    {
        register const double rat = root[4]/root[9];
        register const double re  = plab[9][1]*rat - plab[4][1]/rat;
        register const double im  = plab[9][2]*rat - plab[4][2]/rat;
        spl[4][9][RE] = re;
        spl[4][9][IM] = im;
        spl[9][4][RE] = -re;
        spl[9][4][IM] = -im;
        smn[4][9][RE] = -re;
        smn[4][9][IM] = im;
        smn[9][4][RE] = re;
        smn[9][4][IM] = -im;
    }
    {
        register const double rat = root[5]/root[9];
        register const double re  = plab[9][1]*rat - plab[5][1]/rat;
        register const double im  = plab[9][2]*rat - plab[5][2]/rat;
        spl[5][9][RE] = re;
        spl[5][9][IM] = im;
        spl[9][5][RE] = -re;
        spl[9][5][IM] = -im;
        smn[5][9][RE] = -re;
        smn[5][9][IM] = im;
        smn[9][5][RE] = re;
        smn[9][5][IM] = -im;
    }
    {
        register const double rat = root[6]/root[9];
        register const double re  = plab[9][1]*rat - plab[6][1]/rat;
        register const double im  = plab[9][2]*rat - plab[6][2]/rat;
        spl[6][9][RE] = re;
        spl[6][9][IM] = im;
        spl[9][6][RE] = -re;
        spl[9][6][IM] = -im;
        smn[6][9][RE] = -re;
        smn[6][9][IM] = im;
        smn[9][6][RE] = re;
        smn[9][6][IM] = -im;
    }
    {
        register const double rat = root[7]/root[9];
        register const double re  = plab[9][1]*rat - plab[7][1]/rat;
        register const double im  = plab[9][2]*rat - plab[7][2]/rat;
        spl[7][9][RE] = re;
        spl[7][9][IM] = im;
        spl[9][7][RE] = -re;
        spl[9][7][IM] = -im;
        smn[7][9][RE] = -re;
        smn[7][9][IM] = im;
        smn[9][7][RE] = re;
        smn[9][7][IM] = -im;
    }
    {
        register const double rat = root[8]/root[9];
        register const double re  = plab[9][1]*rat - plab[8][1]/rat;
        register const double im  = plab[9][2]*rat - plab[8][2]/rat;
        spl[8][9][RE] = re;
        spl[8][9][IM] = im;
        spl[9][8][RE] = -re;
        spl[9][8][IM] = -im;
        smn[8][9][RE] = -re;
        smn[8][9][IM] = im;
        smn[9][8][RE] = re;
        smn[9][8][IM] = -im;
    }
}

#ifdef __cplusplus
}
#endif
