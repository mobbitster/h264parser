#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "prediction.h"

namespace h264 {

void Prediction::allocate(int width, int height)
{
	int x, y;

#ifdef DEBUG
	printf("allocate prediction %dx%d\n", width, height);
#endif

	x = this->mb_width = this->mb_pitch = width >> 4;
	y = this->mb_height = height >> 4;
	this->mb_mode = (int *)malloc(x * y * sizeof(int));
 	// per-chroma block information    (8x8)
	x = this->cb_width = this->cb_pitch = width >> 3;
	y = this->cb_height = height >> 3;
	this->total_coeff_c[0] = (int *)malloc(x * y * sizeof(int));
	this->total_coeff_c[1] = (int *)malloc(x * y * sizeof(int));
 	// per-transform block information (4x4)
	x = this->tb_width = this->tb_pitch = width >> 2;
	y = this->tb_height = height >> 2;
	this->total_coeff_l = (int *)malloc( x * y * sizeof(int));
	this->intra_4x4_pm = (int *)malloc(x * y * sizeof(int));
	this->mvx = (int *)malloc(x * y * sizeof(int));
	this->mvy = (int *)malloc(x * y * sizeof(int));

	this->allocated = true;
}

Prediction::~Prediction()
{
#warning Not implemented
}

void Prediction::clear() const
{
	if (this->mb_mode)
		memset(this->mb_mode, 0xFF, this->mb_pitch * this->mb_height * sizeof(int));
	if (this->total_coeff_c[0])
		memset(this->total_coeff_c[0], 0,this->cb_pitch * this->cb_height * sizeof(int));
	if (this->total_coeff_c[1])
		memset(this->total_coeff_c[1], 0,this->cb_pitch * this->cb_height * sizeof(int));
	if (this->total_coeff_l)
		memset(this->total_coeff_l, 0, this->tb_pitch * this->tb_height * sizeof(int));
	if (this->intra_4x4_pm)
		memset(this->intra_4x4_pm, 0xFF,this->tb_pitch*this->tb_height * sizeof(int));
	if (this->mvx)
		memset(this->mvx, MV_NA & 0xFF, this->tb_pitch*this->tb_height * sizeof(int));
	if (this->mvy)
		memset(this->mvy, MV_NA & 0xFF, this->tb_pitch*this->tb_height * sizeof(int));
}

int Prediction::getLumaNN(int x, int y) const
{
	if (x < 0 || y < 0)
		return -1;
// printf("mpi_total_coeff_l ofs %d\n", (y) * mpi->tb_pitch + (x));
	return getTotalCoeffL(x >> 2, y >> 2);
}

int Prediction::getChromaNN(int x, int y, int iCbCr) const
{
	if (x < 0 || y < 0)
		return -1;
	return getTotalCoeffC(x >> 3, y >> 3, iCbCr);
}

int Prediction::getLumaNC(int x, int y) const
{
	int nA = getLumaNN(x - 4, y);
	int nB = getLumaNN(x, y - 4);

// printf("nA %d nB %d\n", nA, nB);

	if (nA < 0 && nB < 0)
		return 0;
	if (nA >= 0 && nB >= 0)
		return (nA + nB + 1) >> 1;
	if (nA >= 0)
		return nA;
	else
		return nB;
}

int Prediction::getChromaNC(int x, int y, int iCbCr) const
{
	int nA = getChromaNN(x - 8, y, iCbCr);
	int nB = getChromaNN(x, y - 8, iCbCr);

	if (nA < 0 && nB < 0)
		return 0;
	if (nA >= 0 && nB >= 0)
		return (nA + nB + 1) >> 1;
	if (nA >= 0)
		return nA;
	else
		return nB;
}

}
