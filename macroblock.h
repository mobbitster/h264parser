#ifndef __MACROBLOCK_H
#define __MACROBLOCK_H

#include "bitstream.h"
#include "defines.h"
#include "parser.h"
#include "sliceheader.h"
#include "macroblocktype.h"
#include "codedblock.h"
#include "residuals.h"

namespace h264 {

class SubMacroblock {
public:
	int sub_mb_type;
	int num_sub_mb_part;
	int sub_mb_pm;
	int sub_mb_part_width;
	int sub_mb_part_height;
};

class Macroblock
{
friend class MacroblockType;
friend class CodedBlock;
friend class Residuals;

public:
	int mb_type;
	int num_mb_parts;
	int mb_part_pred_mode[2];
	int intra_16x16_pm;
	int mb_part_width;
	int mb_part_height;
	int coded_block_pattern_chroma;
	int coded_block_pattern_luma;
private:
	int mb_pos_x, mb_pos_y;
	bool allDone;

	SubMacroblock sub[4];

	Bitstream &bs;
	Parser &parser;
	SliceHeader &sliceHeader;
public:
	Macroblock(Bitstream &bs, Parser &parser, SliceHeader &sliceHeader) :
		bs(bs), parser(parser), sliceHeader(sliceHeader) {
	};

	bool isType(int type) const {
		return mb_type == type;
	}

	void parse(int &cur_mb_addr, int mb_count);
private:
	void parseType();
	void checkAndSkip(int &cur_mb_addr, int mb_count);

	void parsePrediction();
	void parseCodedBlock();
	void parseResiduals();

	int getPredIntra4x4PredMode(int x, int y);

	void parseRefList(int idx);
	void parseSubBlocks();
	void parseMVd(int offset, int width, int height);
	void parseInter();

	void parseIPCM();
	void parseIntraChroma();
	void parseI4x4();
	void parseI8x8();
	void parseI16x16();
	void parseP16x16();
	void parseP16x8();
	void parseP8x8();
	void parseP8x8ref0();
	void parseBDirect16x16();
	void parseB16x16();
	void parseB16x8();
	void parseB8x8();
};

}

#endif
