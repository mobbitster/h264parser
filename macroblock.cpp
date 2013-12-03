#include "macroblock.h"
#include "macroblocktype.h"
#include "defines.h"
#include "codedblock.h"
#include "residuals.h"
#include "scanorder.h"

namespace h264 {

void Macroblock::checkAndSkip(int &cur_mb_addr, int mb_count)
{
	if (sliceHeader.isType(P_SLICE) || sliceHeader.isType(B_SLICE)) {
		int mb_skip_run = bs.getEG();

		if (mb_skip_run == 0)
			return;

		int mb_pos_x, mb_pos_y;

#ifdef DEBUG
		printf("mb_skip_run %d\n", mb_skip_run);
#endif
		for (;mb_skip_run;--mb_skip_run, ++cur_mb_addr) {
			if (cur_mb_addr >= mb_count)
				return;

			mb_pos_x = cur_mb_addr % parser.sps.pic_width_in_mbs;
			mb_pos_y = cur_mb_addr / parser.sps.pic_width_in_mbs;

			// mpi_mb_mode(mpi, mb_pos_x, mb_pos_y) = P_SKIP;
#ifdef DEBUG
			printf("Set mb mode type to P_SKIP\n");
#endif

			mb_pos_x <<= 4;
			mb_pos_y <<= 4;

#ifdef DEBUG
			printf("mb_pos_x %d mb_pos_y %d\n", mb_pos_x, mb_pos_y);
#endif

			// mpi_derive_p_skip_mv(mpi, mb_pos_x, mb_pos_y);
			// motion_prediction(mb_pos_x, mb_pos_y);
		}

	}
}

void Macroblock::parseType()
{
	int rawType = bs.getEG();
	MacroblockType::decode(*this, rawType);
}

void Macroblock::parseIPCM()
{
#ifdef DEBUG
	printf("Parse I_PCM\n");
#endif

	bs.alignToByte();

	/* For now just read all needed buts from stream to skip PCM block */
	for (int y = 16; y; --y) {
		for (int x = 16; x; --x)
			bs.getBits(8);
	}

	for (int iCbCr = 0; iCbCr < 2; ++iCbCr) {
		for (int y = 8; y; --y) {
			for (int x = 8; x; --x)
				bs.getBits(8);
		}
	}
}

int Macroblock::getPredIntra4x4PredMode(int x, int y)
{
	int A = parser.pred.getIntra4x4PredMode(x - 4, y);
	int B = parser.pred.getIntra4x4PredMode(x, y - 4);
	int res = (A < B) ? A : B;

	// printf("intra4x4 pred mode x %d y %d A %d B %d res %d\n",
	// 	x, y, A, B, res);

	if (res < 0)
		res = 2;
	return res;
}

void Macroblock::parseI4x4()
{
#ifdef DEBUG
	printf("\tslice_parse_intra4x4\n");
#endif

	for (int luma_4x_blk_idx = 0; luma_4x_blk_idx < 16; ++luma_4x_blk_idx) {
		int pred_intra_4x4_pm = getPredIntra4x4PredMode(
				ScanOrderIntra4x4::getX(luma_4x_blk_idx, mb_pos_x),
				ScanOrderIntra4x4::getY(luma_4x_blk_idx, mb_pos_y));

		int prev_intra4x4_pred_mode_flag = bs.getBit();

		// printf("prev_intra4x4_pred_mode_flag %d\n", prev_intra4x4_pred_mode_flag);

		if (prev_intra4x4_pred_mode_flag)
			parser.pred.setIntra4x4PredMode(
					ScanOrderIntra4x4::getX(luma_4x_blk_idx, mb_pos_x),
					ScanOrderIntra4x4::getY(luma_4x_blk_idx, mb_pos_y),
					pred_intra_4x4_pm);
		else {
			int rem_intra4x4_pred_mode = bs.getBits(3);

			// printf("rem_intra4x4_pred_mode %d vs pred_intra_4x4_pm %d\n", 
			// 	rem_intra4x4_pred_mode, pred_intra_4x4_pm);

			if (rem_intra4x4_pred_mode >= pred_intra_4x4_pm)
				rem_intra4x4_pred_mode += 1;

			parser.pred.setIntra4x4PredMode(
					ScanOrderIntra4x4::getX(luma_4x_blk_idx, mb_pos_x),
					ScanOrderIntra4x4::getY(luma_4x_blk_idx, mb_pos_y),
					rem_intra4x4_pred_mode);
		}
	}
	intra_chroma_pred_mode = bs.getEG();
}

void Macroblock::parseI16x16()
{

}

void Macroblock::parseP16x16()
{

}

void Macroblock::parseP16x8()
{

}

void Macroblock::parseP8x8()
{

}

void Macroblock::parseBDirect16x16()
{

}

void Macroblock::parseB16x16()
{

}

void Macroblock::parseB16x8()
{

}

void Macroblock::parseB8x8()
{

}

void Macroblock::parseCodedBlock()
{

}

void Macroblock::parseResiduals()
{

}

void Macroblock::parse(int &cur_mb_addr, int mb_count)
{
	bool allDone = false;

	checkAndSkip(cur_mb_addr, mb_count);

	if (bs.remain() <= 0)
		return;

#ifdef DEBUG
	printf("Parse macroblock addr %d x %d y %d\n",
			cur_mb_addr, mb_pos_x, mb_pos_y);
#endif

	parseType();

	mb_pos_x = cur_mb_addr % parser.sps.pic_width_in_mbs;
	mb_pos_y = cur_mb_addr / parser.sps.pic_width_in_mbs;

	parser.pred.setMacroblockMode(mb_pos_x, mb_pos_y, mb_type);

	mb_pos_x <<= 4;
	mb_pos_y <<= 4;

	bs.showStat();

	switch (mb_type) {
		case I_PCM:
			parseIPCM();
			allDone = true;
			break;
		case I_4x4:
			parseI4x4();
			break;
		case I_16x16_0_0_0:
		case I_16x16_1_0_0:
		case I_16x16_2_0_0:
		case I_16x16_3_0_0:
		case I_16x16_0_1_0:
		case I_16x16_1_1_0:
		case I_16x16_2_1_0:
		case I_16x16_3_1_0:
		case I_16x16_0_2_0:
		case I_16x16_1_2_0:
		case I_16x16_2_2_0:
		case I_16x16_3_2_0:
		case I_16x16_0_0_1:
		case I_16x16_1_0_1:
		case I_16x16_2_0_1:
		case I_16x16_3_0_1:
		case I_16x16_0_1_1:
		case I_16x16_1_1_1:
		case I_16x16_2_1_1:
		case I_16x16_3_1_1:
		case I_16x16_0_2_1:
		case I_16x16_1_2_1:
		case I_16x16_2_2_1:
		case I_16x16_3_2_1:
			parseI16x16();
			allDone = true;
			break;
		case P_L0_16x16:
			parseP16x16();
			break;
		case P_L0_L0_16x8:
		case P_L0_L0_8x16:
			parseP16x8();
			break;
		case P_8x8:
		case P_8x8ref0:
			parseP8x8();
			break;
		case B_Direct_16x16:
			parseBDirect16x16();
			break;
		case B_L0_16x16:
		case B_L1_16x16:
		case B_Bi_16x16:
			parseB16x16();
			break;
		case B_L0_L0_16x8:
		case B_L0_L0_8x16:
		case B_L1_L1_16x8:
		case B_L1_L1_8x16:
		case B_L0_L1_16x8:
		case B_L0_L1_8x16:
		case B_L1_L0_16x8:
		case B_L1_L0_8x16:
		case B_L0_Bi_16x8:
		case B_L0_Bi_8x16:
		case B_L1_Bi_16x8:
		case B_L1_Bi_8x16:
		case B_Bi_L0_16x8:
		case B_Bi_L0_8x16:
		case B_Bi_L1_16x8:
		case B_Bi_L1_8x16:
		case B_Bi_Bi_16x8:
		case B_Bi_Bi_8x16:
			parseB16x8();
		case B_8x8:
			parseB8x8();
			break;
		default:
			printf("Invalid or unsupported macroblock type %d\n", mb_type);
	}

	if (!allDone) {
		CodedBlock::parse(*this);

		bs.showStat();

		Residuals residuals(*this);
		residuals.parse();

		bs.showStat();
	}
}


}
