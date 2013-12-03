#ifndef __MACROBLOCK_H
#define __MACROBLOCK_H

#include "bitstream.h"
#include "parser.h"
#include "sliceheader.h"

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
public:
	int mb_type;
	int mb_pos_x, mb_pos_y;
	int num_mb_parts;
	int mb_part_pred_mode[2];
	int intra_16x16_pm;
	int mb_part_width;
	int mb_part_height;
	int coded_block_pattern_chroma;
	int coded_block_pattern_luma;

	int intra_chroma_pred_mode;

	SubMacroblock sub[4];

	Bitstream &bs;
	Parser const& parser;
	SliceHeader const& sliceHeader;
public:
	Macroblock(Bitstream &bs, Parser const& parser, SliceHeader const& sliceHeader) : 
		bs(bs), parser(parser), sliceHeader(sliceHeader)
	{
	};

	void parse(int &cur_mb_addr, int mb_count);
private:
	void parseType();
	void checkAndSkip(int &cur_mb_addr, int mb_count);

	void parseCodedBlock();
	void parseResiduals();

	int getPredIntra4x4PredMode(int x, int y);

	void parseIPCM();
	void parseI4x4();
	void parseI16x16();
	void parseP16x16();
	void parseP16x8();
	void parseP8x8();
	void parseBDirect16x16();
	void parseB16x16();
	void parseB16x8();
	void parseB8x8();
};

}

#endif
