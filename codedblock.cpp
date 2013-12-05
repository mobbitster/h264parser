#include "codedblock.h"
#include "defines.h"
#include "macroblock.h"

namespace h264 {

void CodedBlock::parse(Macroblock &mb)
{
	static int coded_block_pattern_mapping_intra4x4[] = 
	{
		47,31,15, 0,23,27,29,30, 7,11,13,14,39,43,45,46,
		16, 3, 5,10,12,19,21,26,28,35,37,42,44, 1, 2, 4,
		8,17,18,20,24, 6, 9,22,25,32,33,34,36,40,38,41
	};
	static int coded_block_pattern_mapping_inter[] = 
	{
		0,16, 1, 2, 4, 8,32, 3, 5,10,12,15,47, 7,11,13,
		14, 6, 9,31,35,37,42,44,33,34,36,40,39,43,45,46,
		17,18,20,24,19,21,26,28,23,27,29,30,22,25,38,41
	};
	int coded_block_pattern = mb.bs.getEG();

#ifdef DEBUG
	printf("Parse coded block idx %d\n", coded_block_pattern);
#endif
	/* FIXME: Check ChromaArrayType */
	if (mb.mb_part_pred_mode[0] == Intra_4x4)
		coded_block_pattern = coded_block_pattern_mapping_intra4x4[coded_block_pattern];
	else
		coded_block_pattern = coded_block_pattern_mapping_inter[coded_block_pattern];

	mb.coded_block_pattern_luma = coded_block_pattern & 15;
	mb.coded_block_pattern_chroma = coded_block_pattern >> 4;

#ifdef DEBUG
	printf("\tcoded_block_pattern %d coded_block_pattern_luma %d coded_block_pattern_chroma %d\n",
			coded_block_pattern, mb.coded_block_pattern_luma, mb.coded_block_pattern_chroma);
#endif
}

}
