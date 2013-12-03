#include "slice.h"
#include "macroblock.h"

namespace h264 {

int Slice::parseData()
{
	int res = 0;
	int cur_mb_addr, mb_count;
	int data_remain;
	// int mb_skip_run;
	int mb_pos_x, mb_pos_y;
	
#ifdef DEBUG
	printf("Parse slice data\n");
#endif

	parser.pred.clear();

	cur_mb_addr = sliceHeader.first_mb_in_slice * (1 + sliceHeader.mb_aff_frame_flag);
	mb_count = parser.pred.mb_width * parser.pred.mb_height;

	data_remain = bs.remain();
	while (data_remain > 0 && cur_mb_addr < mb_count) {
#ifdef DEBUG
		printf("\tcur_mb_addr %d (of %d) (%dx%d)\n",
			cur_mb_addr, mb_count, parser.pred.mb_width, parser.pred.mb_height);
#endif

		bs.showStat();

		Macroblock mb(bs, parser, sliceHeader);
		mb.parse(cur_mb_addr, mb_count);

		data_remain = bs.remain();
		++cur_mb_addr;
	}

	return res;
}

int Slice::parse()
{
	int res = 0;

	res = sliceHeader.parse();
	if (res == 0)
		res = parseData();

	return res;
}

}
