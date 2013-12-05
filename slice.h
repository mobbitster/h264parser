#ifndef __SLICE_H
#define __SLICE_H

#include "bitstream.h"
#include "parser.h"
#include "sliceheader.h"

namespace h264 {

class Slice {
private:
	Bitstream &bs;
	Parser &parser;
	SPS const& sps;
	PPS const& pps;
	SliceHeader sliceHeader;
public:
	Slice(Bitstream &bs, Parser &parser) :
		bs(bs), parser(parser), sps(parser.sps), pps(parser.pps), sliceHeader(bs, parser)
	{
	};

	int parse();
private:
	int parseData();
};

}


#endif
