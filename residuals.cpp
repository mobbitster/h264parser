#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "parser.h"
#include "prediction.h"
#include "residuals.h"
#include "codetables.h"
#include "scanorder.h"

namespace h264 {

bool Residuals::code_tables_initialized = false;

static struct code_table *coeff_token_code_table[4];
static struct code_table *coeff_token_code_table_ChromaDC;
static struct code_table *total_zeros_code_table_4x4[15];
static struct code_table *total_zeros_code_table_ChromaDC[3];
static struct code_table *run_before_code_table[6];

struct code_table *Residuals::initCodeTable(
		struct code_table_item *items) {
	struct code_table *res = (struct code_table *) malloc(
			sizeof(struct code_table));
	struct code_table_item *pos;
	int count = 0;
	for (pos = items; pos && pos->code != 0xFFFFFFFF; ++pos)
		++count;
	res->items = items;
	res->count = count;
	return res;
}

void Residuals::initCodeTables() {
	for (int i = 0; i < 4; ++i)
		coeff_token_code_table[i] = initCodeTable(CoeffTokenCodes[i]);

	coeff_token_code_table_ChromaDC = initCodeTable(CoeffTokenCodes_ChromaDC);

	for (int i = 0; i < 15; ++i)
		total_zeros_code_table_4x4[i] = initCodeTable(TotalZerosCodes_4x4[i]);

	for (int i = 0; i < 3; ++i)
		total_zeros_code_table_ChromaDC[i] = initCodeTable(
				TotalZerosCodes_ChromaDC[i]);

	for (int i = 0; i < 6; ++i)
		run_before_code_table[i] = initCodeTable(RunBeforeCodes[i]);
}

int Residuals::getCode(struct code_table *table) {
	unsigned int code = mb.bs.peekBits(24) << 8;
	int min=0, max=table->count;

	while (max - min > 1) {
		int mid = (min + max) >> 1;
		if (code >= table->items[mid].code)
			min = mid;
		else
			max=mid;
	}
	mb.bs.advanceBits(table->items[min].bits);
	return table->items[min].data;
}

int Residuals::residualBlock(int *coeff_level, int max_num_coeff, int nc)
{
	int coeff_token, total_coeff, trailing_ones;
	int i, suffixLength, zeros_left, coeff_num;
	int level[16], run[16];

	switch (nc) {
	case -1:
		coeff_token = getCode(coeff_token_code_table_ChromaDC);
		break;
	case 0:
	case 1:
		coeff_token = getCode(coeff_token_code_table[0]);
		break;
	case 2:
	case 3:
		coeff_token = getCode(coeff_token_code_table[1]);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		coeff_token = getCode(coeff_token_code_table[2]);
		break;
	default:
		coeff_token = getCode(coeff_token_code_table[3]);
	}

	total_coeff = coeff_token >> 2;
	trailing_ones = coeff_token & 3;

	if (total_coeff > 10 && trailing_ones < 3)
		suffixLength = 1;
	else
		suffixLength = 0;

	if (!total_coeff)
		return 0;

	for (i = 0; i < total_coeff; ++i)
		if (i < trailing_ones)
			level[i] = 1 - 2 * mb.bs.getBit();
		else {
			int level_prefix;
			int levelSuffixSize = suffixLength;
			int levelCode;

			for (level_prefix = 0; !mb.bs.getBit(); ++level_prefix)
				;
			levelCode = level_prefix << suffixLength;

			if (level_prefix == 14 && suffixLength == 0)
				levelSuffixSize = 4;
			else if (level_prefix == 15)
				levelSuffixSize = 12;

			if (levelSuffixSize)
				levelCode += mb.bs.getBits(levelSuffixSize);

			if (level_prefix == 15 && suffixLength == 0)
				levelCode += 15;

			if (i == trailing_ones && trailing_ones < 3)
				levelCode += 2;

			if (levelCode & 1)
				level[i] = (-levelCode - 1) >> 1;
			else
				level[i] = (levelCode + 2) >> 1;

			if (suffixLength == 0)
				suffixLength = 1;

			if (abs(level[i]) > (3 << (suffixLength - 1)) && suffixLength < 6)
				++suffixLength;
		}

	if (total_coeff < max_num_coeff) {
		if (nc < 0)
			zeros_left = getCode(
					total_zeros_code_table_ChromaDC[total_coeff - 1]);
		else
			zeros_left = getCode(total_zeros_code_table_4x4[total_coeff - 1]);
	} else
		zeros_left = 0;

	for (i = 0; i < total_coeff - 1; ++i) {
		if (zeros_left > 6) {
			int run_before = 7 - mb.bs.getBits(3);
			if (run_before == 7)
				while (!mb.bs.getBit())
					++run_before;
			run[i] = run_before;
		} else {
			if (zeros_left > 0)
				run[i] = getCode(run_before_code_table[zeros_left - 1]);
			else
				run[i] = 0;
		}
		zeros_left -= run[i];
	}

  	run[total_coeff-1] = zeros_left;

	coeff_num = -1;
	for (int i = total_coeff - 1; i >= 0; --i) {
		coeff_num += run[i] + 1;
		coeff_level[coeff_num] = level[i];
	}

	return total_coeff;
}

#define custom_clip(i,min,max) (((i)<min)?min:(((i)>max)?max:(i)))

void Residuals::parse()
{
	int QPi, QPy, QPc;
	int mb_qp_delta;

	int luma_dc_level[16];      // === Intra16x16DCLevel
	int luma_ac_level[16][16];  // === Intra16x16ACLevel
	int chroma_dc_level[2][4];
	int chroma_ac_level[2][4][16];

	static int qpc_table[22] = { 29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37,
			37, 37, 38, 38, 38, 39, 39, 39, 39 };

	if (!code_tables_initialized) {
		code_tables_initialized = true;
		initCodeTables();
	}
#ifdef DEBUG
	printf("Parse residuals\n");
#endif
	// bstream_show_stat(bs);

	memset(luma_dc_level, 0, sizeof(luma_dc_level));
	memset(luma_ac_level, 0, sizeof(luma_ac_level));
	memset(chroma_dc_level, 0, sizeof(chroma_dc_level));
	memset(chroma_ac_level, 0, sizeof(chroma_ac_level));

	mb_qp_delta = mb.bs.getSignedEG();

#ifdef DEBUG
	printf("\tmb_qp_delta %d\n", mb_qp_delta);
#endif

	QPy = mb.sliceHeader.slice_qpy;

	QPy = (QPy + mb_qp_delta + 52) % 52;
	QPi = QPy + mb.parser.pps.chroma_qp_index_offset;
	QPi = custom_clip(QPi, 0, 51);
	if (QPi < 30) 
		QPc = QPi;
	else 
		QPc = qpc_table[QPi - 30];

// printf("parse_residual mb.mb_part_pred_mode[0] %d\ncoded_block_pattern_luma %x coded_block_pattern_chroma %x\n",
// 	mb.mb_part_pred_mode[0], mb.coded_block_pattern_luma, mb.coded_block_pattern_chroma);

	if (mb.mb_part_pred_mode[0] == Intra_16x16) {
		int luma_dc_nc = mb.parser.pred.getLumaNC(mb.mb_pos_x, mb.mb_pos_y);

// printf("Intra_16x16 luma_dc_nc %d\n", luma_dc_nc);
		residualBlock(&luma_dc_level[0], 16, luma_dc_nc);
	}

	for (int i8x8 = 0; i8x8 < 4; ++i8x8)
		for (int i4x4=0; i4x4 < 4; ++i4x4)
			if (mb.coded_block_pattern_luma & (1 << i8x8)) {
				int idx = i8x8 * 4 + i4x4;
				int x = ScanOrderIntra4x4::getX(idx, mb.mb_pos_x);
				int y = ScanOrderIntra4x4::getY(idx, mb.mb_pos_y);
				int luma_ac_nc = mb.parser.pred.getLumaNC(x, y);
				int val;

// printf("luma_ac_nc %d\n", luma_ac_nc);
				if (mb.mb_part_pred_mode[0] == Intra_16x16)
					val = residualBlock(&luma_ac_level[idx][1], 15, luma_ac_nc);
				else
					val = residualBlock(&luma_ac_level[idx][0], 16, luma_ac_nc);
// printf("val %d\n", val);
				mb.parser.pred.setTotalCoeffL(x >> 2, y >> 2, val);
			};

	for (int iCbCr = 0; iCbCr < 2; iCbCr++)
		if (mb.coded_block_pattern_chroma & 3) {
			int chroma_dc_nc = -1;
// printf("chroma_dc_nc %d\n", chroma_dc_nc);
			residualBlock(&chroma_dc_level[iCbCr][0], 4, chroma_dc_nc);
		}

	for (int iCbCr = 0; iCbCr < 2; iCbCr++)
		for (int i4x4 = 0; i4x4 < 4; ++i4x4)
			if (mb.coded_block_pattern_chroma & 2) {
				int x = mb.mb_pos_x + (i4x4 & 1) * 8;
				int y = mb.mb_pos_y + (i4x4 >> 1) * 8;
				int chroma_ac_nc = mb.parser.pred.getChromaNC(x, y, iCbCr);
				int val;
// printf("chroma_ac_nc %d\n", chroma_ac_nc);
				val = residualBlock(&chroma_ac_level[iCbCr][i4x4][1], 15, chroma_ac_nc);
				mb.parser.pred.setTotalCoeffC(x >> 3, y >> 3, iCbCr, val);
			}

}

}
