#include "macroblock.h"
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

			parser.pred.deriveMVSkip(mb_pos_x, mb_pos_y);
		}

	}
}

void Macroblock::parseType()
{
	int rawType = bs.getEG();
	MacroblockType::decode(*this, rawType);

	parser.pred.setMacroblockMode(mb_pos_x, mb_pos_y, mb_type);
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

void Macroblock::parseIntraChroma()
{
	int intra_chroma_pred_mode;

#ifdef DEBUG
	printf("\tParse IntraChroma\n");
#endif


	if (chroma_array_type == 1)
		intra_chroma_pred_mode = bs.getEG();
}

void Macroblock::parseI4x4()
{
#ifdef DEBUG
	printf("\tParse Intra4x4\n");
#endif

	for (int luma_4x_blk_idx = 0; luma_4x_blk_idx < 16; ++luma_4x_blk_idx) {
		int x = ScanOrderIntra4x4::getX(luma_4x_blk_idx, mb_pos_x);
		int y = ScanOrderIntra4x4::getY(luma_4x_blk_idx, mb_pos_y);
		int pred_intra_4x4_pm = parser.pred.calculateIntra4x4PredMode(x, y);

		int prev_intra4x4_pred_mode_flag = bs.getBit();

		// printf("prev_intra4x4_pred_mode_flag %d\n", prev_intra4x4_pred_mode_flag);

		if (prev_intra4x4_pred_mode_flag)
			parser.pred.setIntra4x4PredMode(x, y, pred_intra_4x4_pm);
		else {
			int rem_intra4x4_pred_mode = bs.getBits(3);

			// printf("rem_intra4x4_pred_mode %d vs pred_intra_4x4_pm %d\n", 
			// 	rem_intra4x4_pred_mode, pred_intra_4x4_pm);

			if (rem_intra4x4_pred_mode >= pred_intra_4x4_pm)
				rem_intra4x4_pred_mode += 1;

			parser.pred.setIntra4x4PredMode(x, y, rem_intra4x4_pred_mode);
		}
	}

	parseIntraChroma();
}

void Macroblock::parseI8x8()
{
	parseIntraChroma();
}

void Macroblock::parseI16x16()
{
	parseIntraChroma();
}

void Macroblock::parseP16x16()
{
	/* FIXME: need to doublecheck
	 * */
	parseInter();
}

void Macroblock::parseP16x8()
{
	parseInter();
}

void Macroblock::parseRefList(int idx)
{
	int ref_idx = bs.getTruncEG(
			idx == 0 ?
					parser.pps.num_ref_idx_l0_active :
					parser.pps.num_ref_idx_l1_active);
#ifdef DEBUG
	printf("\t\tref list %d = %d\n", idx, ref_idx);
#endif
}

void Macroblock::parseSubBlocks()
{
	for (int mb_part_idx = 0; mb_part_idx < 4; ++mb_part_idx) {
		int rawSubType = bs.getEG();
		MacroblockType::decodeSub(*this, mb_part_idx, rawSubType);
	}

	for (int mb_part_idx = 0; mb_part_idx < 4; ++mb_part_idx) {
		if (parser.pps.num_ref_idx_l0_active > 0 && mb_type != P_8x8ref0
				&& sub[mb_part_idx].sub_mb_type != B_Direct_8x8
				&& sub[mb_part_idx].sub_mb_pm != Pred_L1)
			parseRefList(0);
	}

	for (int mb_part_idx = 0; mb_part_idx < 4; ++mb_part_idx) {
		if (parser.pps.num_ref_idx_l1_active > 0
				&& sub[mb_part_idx].sub_mb_type != B_Direct_8x8
				&& sub[mb_part_idx].sub_mb_pm != Pred_L0)
			parseRefList(1);
	}

	for (int mb_part_idx = 0; mb_part_idx < 4; ++mb_part_idx) {
		SubMacroblock &sub = this->sub[mb_part_idx];
		int sof = (sub.sub_mb_type == P_L0_8x4) ? 2 : 1;

		if (sub.sub_mb_type != B_Direct_8x8
				&& sub.sub_mb_pm != Pred_L1) {

			for (int sub_mb_part_idx = 0; sub_mb_part_idx < sub.num_sub_mb_part;
					++sub_mb_part_idx) {
				parseMVd(mb_part_idx * 4 + sub_mb_part_idx * sof,
						sub.sub_mb_part_width, sub.sub_mb_part_height);
			}
		}
	}

	for (int mb_part_idx = 0; mb_part_idx < 4; ++mb_part_idx) {
		SubMacroblock &sub = this->sub[mb_part_idx];
		int sof = (sub.sub_mb_type == P_L0_8x4) ? 2 : 1;

		if (sub.sub_mb_type != B_Direct_8x8
				&& sub.sub_mb_pm != Pred_L0) {

			for (int sub_mb_part_idx = 0; sub_mb_part_idx < sub.num_sub_mb_part;
					++sub_mb_part_idx) {
				parseMVd(mb_part_idx * 4 + sub_mb_part_idx * sof,
						sub.sub_mb_part_width, sub.sub_mb_part_height);
			}
		}
	}

}

void Macroblock::parseMVd(int offset, int width, int height)
{
	int mvdx = bs.getSignedEG();
	int mvdy = bs.getSignedEG();

	int x = ScanOrderIntra4x4::getX(offset, mb_pos_x);
	int y = ScanOrderIntra4x4::getY(offset, mb_pos_y);

	printf("mb_pos_x = %d, mb_pos_y = %d, offset = %d, x = %d, y = %d\n", mb_pos_x, mb_pos_y, offset, x, y);

	parser.pred.deriveMV(x, y, width, height, mvdx, mvdy);
}

void Macroblock::parseInter()
{
	int sof = (mb_type == P_L0_L0_16x8) ? 8 : 4;

#ifdef DEBUG
	printf("Parse InterXXX\n");
#endif

	printf("sof = %d\n", sof);

	for (int mb_part_idx = 0; mb_part_idx < num_mb_parts; ++mb_part_idx) {
		if (parser.pps.num_ref_idx_l0_active > 0
				&& mb_part_pred_mode[mb_part_idx] != Pred_L1)
					parseRefList(0);
	}

	for (int mb_part_idx = 0; mb_part_idx < num_mb_parts; ++mb_part_idx) {
		if (parser.pps.num_ref_idx_l1_active > 0
				&& mb_part_pred_mode[mb_part_idx] != Pred_L0)
					parseRefList(1);
	}

	for (int mb_part_idx = 0; mb_part_idx < num_mb_parts; ++mb_part_idx) {
		if (mb_part_pred_mode[mb_part_idx] != Pred_L1) {
			printf("mb_part_idx = %d of %d\n", mb_part_idx, num_mb_parts);
			parseMVd(mb_part_idx * sof, mb_part_width, mb_part_height);
		}
	}

	for (int mb_part_idx = 0; mb_part_idx < num_mb_parts; ++mb_part_idx) {
		if (mb_part_pred_mode[mb_part_idx] != Pred_L0) {
			parseMVd(mb_part_idx * sof, mb_part_width, mb_part_height);
		}
	}
}

void Macroblock::parseP8x8()
{
	parseSubBlocks();
}

void Macroblock::parseP8x8ref0()
{
	parseSubBlocks();
}

void Macroblock::parseBDirect16x16()
{

}

void Macroblock::parseB16x16()
{
	parseInter();
}

void Macroblock::parseB16x8()
{
	parseInter();
}

void Macroblock::parseB8x8()
{
	parseSubBlocks();
}

void Macroblock::parsePrediction()
{
	allDone = false;

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
			parseP8x8();
			break;
		case P_8x8ref0:
			parseP8x8ref0();
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
//		case P_SKIP:
//			parseSkip();
//			break;
		default:
			printf("Invalid or unsupported macroblock type %d\n", mb_type);
	}
}

void Macroblock::parseCodedBlock()
{

}

void Macroblock::parseResiduals()
{

}

void Macroblock::parse(int &cur_mb_addr, int mb_count)
{
	checkAndSkip(cur_mb_addr, mb_count);

	if (bs.remain() <= 0)
		return;

	mb_pos_x = cur_mb_addr % parser.sps.pic_width_in_mbs;
	mb_pos_y = cur_mb_addr / parser.sps.pic_width_in_mbs;

#ifdef DEBUG
	printf("Parse macroblock cur_mb_addr = %d x = %d (%d) y = %d (%d)\n",
			cur_mb_addr, mb_pos_x, mb_pos_x << 4, mb_pos_y, mb_pos_y << 4);
#endif

	parseType();

	mb_pos_x <<= 4;
	mb_pos_y <<= 4;

	bs.showStat();

	parsePrediction();

	if (mb_part_pred_mode[0] != Intra_16x16) {
		CodedBlock::parse(*this);
		bs.showStat();
	}

	if (coded_block_pattern_luma > 0 || coded_block_pattern_chroma > 0
			|| mb_part_pred_mode[0] == Intra_16x16) {
		Residuals residuals(*this);
		residuals.parse();

		bs.showStat();
	}
}


}
