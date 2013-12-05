#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "prediction.h"

namespace h264 {

#ifndef min
#define min(a, b)	((a) < (b) ? (a):(b))
#endif

#ifndef max
#define max(a, b)	((a) > (b) ? (a):(b))
#endif

#define median(a, b, c)	max(min(a,b),min(c,max(a,b)))

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

int Prediction::calculateIntra4x4PredMode(int x, int y) const
{
	int A = getIntra4x4PredMode(x - 4, y);
	int B = getIntra4x4PredMode(x, y - 4);
	int res = (A < B) ? A : B;

	// printf("intra4x4 pred mode x %d y %d A %d B %d res %d\n",
	// 	x, y, A, B, res);

	if (res < 0)
		res = 2;
	return res;
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

struct mv Prediction::getMV(int x, int y)
{
	struct mv res = { 0, 0, 0, 0 };

	x >>= 2;
	y >>= 2;

	if (x < 0 || y < 0 || x >= this->tb_width || y >= this->tb_height)
		return res;

	res.x = getMVx(x, y);
	res.y = getMVy(x, y);

	if (res.x == MV_NA) {
		res.x = 0;
		res.y = 0;
		if (is_intra(getMacroblockMode(x >> 2, y >> 2)))
			res.available = 1;
	} else {
		res.available = 1;
		res.valid = 1;
	}
	return res;
}

struct mv Prediction::predictMV(int org_x, int org_y, int width, int height)
{
	struct mv a, b, c, d, res;

	a = getMV(org_x - 1, org_y);
	b = getMV(org_x, org_y - 1);
	c = getMV(org_x + width, org_y - 1);

	if (!c.available)
		c = getMV(org_x - 1, org_y - 1);

	if (width == 16 && height == 8) {
		if (org_y & 8) {
			if (a.valid)
				return a;
		} else {
			if (b.valid)
				return b;
		}
	}

	if (width == 8 && height == 16) {
		if (org_x & 8) {
			if (c.valid)
				return c;
		} else {
			if (a.valid)
				return a;
		}
	}

	// If one and only one of the candidate predictors is available and valid,
	// it is returned
	if (!b.valid && !c.valid)
		return a;
	if (!a.valid && b.valid && !c.valid)
		return b;
	if (!a.valid && !b.valid && c.valid)
		return c;

	// median prediction
	res.x = median(a.x, b.x, c.x);
	res.y = median(a.y, b.y, c.y);

	return res;
}

struct mv Prediction::predictMVSkip(int org_x, int org_y)
{
	struct mv zero = { 0, 0, 0, 0 };

	if (org_x <= 0 || org_y <= 0)
		return zero;

	if (getMVx((org_x >> 2) - 1, org_y >> 2) == 0
			&& getMVy((org_x >> 2) - 1, org_y >> 2) == 0)
		return zero;
	if (getMVx(org_x >> 2, (org_y >> 2) - 1) == 0
			&& getMVy(org_x >> 2, (org_y >> 2) - 1) == 0)
		return zero;
	return predictMV(org_x, org_y, 16, 16);
}

void Prediction::fillMV(int org_x, int org_y, int width, int height, int mvx, int mvy)
{
	org_x >>= 2;
	org_y >>= 2;
	width >>= 2;
	height >>= 2;

	for (int y = org_y + height - 1; y >= org_y; --y) {
		for (int x = org_x + width - 1; x >= org_x; --x) {
			setMVx(x, y, mvx);
			setMVy(x, y, mvy);
		}
	}
}

void Prediction::deriveMVSkip(int org_x, int org_y)
{
	struct mv mv = predictMVSkip(org_x, org_y);
	fillMV(org_x, org_y, 16, 16, mv.x, mv.y);
}

void Prediction::deriveMV(int org_x, int org_y, int width, int height, int mvdx, int mvdy)
{
	struct mv v = predictMV(org_x, org_y, width, height);
	fillMV(org_x, org_y, width, height, v.x + mvdx, v.y + mvdy);
}

}
