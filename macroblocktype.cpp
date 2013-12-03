#include "macroblocktype.h"
#include "sliceheader.h"
#include "defines.h"

namespace h264 {

static int i_slice_mb_modes[][4] = {
	{Intra_4x4,   NA, NA, NA},
	{Intra_16x16,  0,  0,  0},
	{Intra_16x16,  1,  0,  0},
	{Intra_16x16,  2,  0,  0},
	{Intra_16x16,  3,  0,  0},
	{Intra_16x16,  0,  1,  0},
	{Intra_16x16,  1,  1,  0},
	{Intra_16x16,  2,  1,  0},
	{Intra_16x16,  3,  1,  0},
	{Intra_16x16,  0,  2,  0},
	{Intra_16x16,  1,  2,  0},
	{Intra_16x16,  2,  2,  0},
	{Intra_16x16,  3,  2,  0},
	{Intra_16x16,  0,  0, 15},
	{Intra_16x16,  1,  0, 15},
	{Intra_16x16,  2,  0, 15},
	{Intra_16x16,  3,  0, 15},
	{Intra_16x16,  0,  1, 15},
	{Intra_16x16,  1,  1, 15},
	{Intra_16x16,  2,  1, 15},
	{Intra_16x16,  3,  1, 15},
	{Intra_16x16,  0,  2, 15},
	{Intra_16x16,  1,  2, 15},
	{Intra_16x16,  2,  2, 15},
	{Intra_16x16,  3,  2, 15},
 	{NA, NA, NA, NA} // I_PCM
};

static int p_slice_mb_modes[][5] = {
	{1, Pred_L0, NA,      16, 16},
	{2, Pred_L0, Pred_L0, 16,  8},
	{2, Pred_L0, Pred_L0,  8, 16},
	{4, NA,      NA,       8,  8},
	{4, NA,      NA,       8,  8},
	{1, Pred_L0, NA,      16, 16}  // P_Skip
};

static int p_slice_sub_mb_modes[][4]={
	{1, Pred_L0, 8, 8},
	{2, Pred_L0, 8, 4},
	{2, Pred_L0, 4, 8},
	{4, Pred_L0, 4, 4},
};

static int b_slice_mb_modes[][22] = {
	{0,	Direct,		NA,		8, 	8},
	{1,	Pred_L0, 	NA,		16,	16},
	{1,	Pred_L1, 	NA,		16,	16},
	{1,	BiPred, 	NA,		16,	16},
	{2,	Pred_L0, 	Pred_L0,	16,	8},
	{2,	Pred_L0, 	Pred_L0,	8,	16},
	{2,	Pred_L1, 	Pred_L1,	16,	8},
	{2,	Pred_L1, 	Pred_L1,	8,	16},
	{2,	Pred_L0, 	Pred_L1,	16,	8},
	{2,	Pred_L0, 	Pred_L1,	8,	16},
	{2,	Pred_L1, 	Pred_L0,	16,	8},
	{2,	Pred_L1, 	Pred_L0,	8,	16},
	{2,	Pred_L0, 	BiPred,		16,	8},
	{2,	Pred_L0, 	BiPred,		8,	16},
	{2,	Pred_L1, 	BiPred,		16,	8},
	{2,	Pred_L1, 	BiPred,		8,	16},
	{2,	BiPred, 	Pred_L0,	16,	8},
	{2,	BiPred, 	Pred_L0,	8,	16},
	{2,	BiPred, 	Pred_L1,	16,	8},
	{2,	BiPred, 	Pred_L1,	8,	16},
	{2,	BiPred, 	BiPred,		16,	8},
	{2,	BiPred, 	BiPred,		8,	16},
	{4,	NA,		NA,		8,	8},
	{0,	Direct,		NA,		8,	8}, // P_Skip
};

static int b_slice_sub_mb_modes[][13]={
	{4, Direct, 4, 4},
	{1, Pred_L0, 8, 8},
	{1, Pred_L1, 8, 8},
	{1, BiPred, 8, 8},
	{2, Pred_L0, 8, 4},
	{2, Pred_L0, 4, 8},
	{2, Pred_L1, 8, 4},
	{2, Pred_L1, 4, 8},
	{2, BiPred, 8, 4},
	{2, BiPred, 4, 8},
	{4, Pred_L0, 4, 4},
	{4, Pred_L1, 4, 4},
	{4, BiPred, 4, 4},
};

const char *MacroblockType::typeToString(int type)
{
	static const char *types[LAST_NONINFERRED] = {
		"I_4x4",
		"I_16x16_0_0_0",
		"I_16x16_1_0_0",
		"I_16x16_2_0_0",
		"I_16x16_3_0_0",
		"I_16x16_0_1_0",
		"I_16x16_1_1_0",
		"I_16x16_2_1_0",
		"I_16x16_3_1_0",
		"I_16x16_0_2_0",
		"I_16x16_1_2_0",
		"I_16x16_2_2_0",
		"I_16x16_3_2_0",
		"I_16x16_0_0_1",
		"I_16x16_1_0_1",
		"I_16x16_2_0_1",
		"I_16x16_3_0_1",
		"I_16x16_0_1_1",
		"I_16x16_1_1_1",
		"I_16x16_2_1_1",
		"I_16x16_3_1_1",
		"I_16x16_0_2_1",
		"I_16x16_1_2_1",
		"I_16x16_2_2_1",
		"I_16x16_3_2_1",
		"I_PCM",
		"P_L0_16x16",
		"P_L0_L0_16x8",
		"P_L0_L0_8x16",
		"P_8x8",
		"P_8x8ref0",
		"B_Direct_16x16",
		"B_L0_16x16",
		"B_L1_16x16",
		"B_Bi_16x16",
		"B_L0_L0_16x8",
		"B_L0_L0_8x16",
		"B_L1_L1_16x8",
		"B_L1_L1_8x16",
		"B_L0_L1_16x8",
		"B_L0_L1_8x16",
		"B_L1_L0_16x8",
		"B_L1_L0_8x16",
		"B_L0_Bi_16x8",
		"B_L0_Bi_8x16",
		"B_L1_Bi_16x8",
		"B_L1_Bi_8x16",
		"B_Bi_L0_16x8",
		"B_Bi_L0_8x16",
		"B_Bi_L1_16x8",
		"B_Bi_L1_8x16",
		"B_Bi_Bi_16x8",
		"B_Bi_Bi_8x16",
		"B_8x8",
	};

	if (type == NA)
		return "undef";
	else if (type < LAST_NONINFERRED)
		return types[type];
	else
		return "SKIP";
}

const char *MacroblockType::predictionModeToString(int mode)
{
	static const char *modes[] = {
		"Intra_4x4",
		"Intra_16x16",
		"Pred_L0",
		"Pred_L1",
		"BiPred",
		"Direct",
	};

	if (mode == NA)
		return "undef";
	return modes[mode];
}

void MacroblockType::setUndef(Macroblock &mb)
{
	mb.mb_type = 0xff;
	mb.mb_part_pred_mode[0] = 0xff;
	mb.mb_part_pred_mode[1] = 0xff;
}

void MacroblockType::decodeISlice(Macroblock &mb, int rawType)
{
	if (rawType > 25) {
		setUndef(mb);
		return;
	}

	mb.mb_type = I_4x4 + rawType;
	mb.num_mb_parts = 1;
	mb.mb_part_pred_mode[0] = i_slice_mb_modes[rawType][0];
	mb.mb_part_pred_mode[1] = -1;
	mb.intra_16x16_pm = i_slice_mb_modes[rawType][1];
	mb.mb_part_width = 16;
	mb.mb_part_height = 16;
	mb.coded_block_pattern_chroma = i_slice_mb_modes[rawType][2];
	mb.coded_block_pattern_luma = i_slice_mb_modes[rawType][3];
}

void MacroblockType::decodePSlice(Macroblock &mb, int rawType)
{
	mb.mb_type = P_L0_16x16 + rawType;
	mb.num_mb_parts = p_slice_mb_modes[rawType][0];
	mb.mb_part_pred_mode[0] = p_slice_mb_modes[rawType][1];
	mb.mb_part_pred_mode[1] = p_slice_mb_modes[rawType][2];
	mb.intra_16x16_pm = NA;
	mb.mb_part_width = p_slice_mb_modes[rawType][3];
	mb.mb_part_height = p_slice_mb_modes[rawType][4];
	mb.coded_block_pattern_chroma = NA;
	mb.coded_block_pattern_luma = NA;
}

void MacroblockType::decodeBSlice(Macroblock &mb, int rawType)
{
	mb.mb_type = B_Direct_16x16 + rawType;
	mb.num_mb_parts = b_slice_mb_modes[rawType][0];
	mb.mb_part_pred_mode[0] = b_slice_mb_modes[rawType][1];
	mb.mb_part_pred_mode[1] = b_slice_mb_modes[rawType][2];
	mb.intra_16x16_pm = NA;
	mb.mb_part_width = b_slice_mb_modes[rawType][3];
	mb.mb_part_height = b_slice_mb_modes[rawType][4];
	mb.coded_block_pattern_chroma = NA;
	mb.coded_block_pattern_luma = NA;
}

void MacroblockType::decode(Macroblock &mb, int rawType)
{
	switch (mb.sliceHeader.slice_type) {
		case I_SLICE:
			MacroblockType::decodeISlice(mb, rawType);
			break;
		case P_SLICE:
			MacroblockType::decodePSlice(mb, rawType);
			break;
		case B_SLICE:
			MacroblockType::decodeBSlice(mb, rawType);
			break;
		default:
			MacroblockType::setUndef(mb);
			break;
	}	

#ifdef DEBUG
	printf("\tDecode mb mode type %s\n",
		MacroblockType::typeToString(mb.mb_type));
	printf("\t\tpred_mode[0] %s pred_mode[1] %s\n",
		MacroblockType::predictionModeToString(mb.mb_part_pred_mode[0]), 
		MacroblockType::predictionModeToString(mb.mb_part_pred_mode[1]));
#endif
}

void MacroblockType::decodePSliceSub(Macroblock &mb, int idx, int rawSubType)
{
	mb.sub[idx].sub_mb_type = rawSubType;
	mb.sub[idx].num_sub_mb_part = p_slice_sub_mb_modes[rawSubType][0];
	mb.sub[idx].sub_mb_pm = p_slice_sub_mb_modes[rawSubType][1];
	mb.sub[idx].sub_mb_part_width = p_slice_sub_mb_modes[rawSubType][2];
	mb.sub[idx].sub_mb_part_height = p_slice_sub_mb_modes[rawSubType][3];
}

void MacroblockType::decodeBSliceSub(Macroblock &mb, int idx, int rawSubType)
{
	mb.sub[idx].sub_mb_type = rawSubType;
	mb.sub[idx].num_sub_mb_part = b_slice_sub_mb_modes[rawSubType][0];
	mb.sub[idx].sub_mb_pm = b_slice_sub_mb_modes[rawSubType][1];
	mb.sub[idx].sub_mb_part_width = b_slice_sub_mb_modes[rawSubType][2];
	mb.sub[idx].sub_mb_part_height = b_slice_sub_mb_modes[rawSubType][3];
}

void MacroblockType::decodeSub(Macroblock &mb, int idx, int rawSubType)
{
	switch (mb.sliceHeader.slice_type) {
		case P_SLICE:
			MacroblockType::decodePSliceSub(mb, idx, rawSubType);
			break;
		case B_SLICE:
			MacroblockType::decodeBSliceSub(mb, idx, rawSubType);
			break;
		default:
			printf("Invalid sub macroblock type %d\n", rawSubType);
			break;
	}
}

}
