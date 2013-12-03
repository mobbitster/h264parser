#ifndef __PARSER_H
#define __PARSER_H

#include "bitstream.h"
#include "params.h"
#include "prediction.h"

namespace h264 {

class Parser {
public:
	enum {
		OK	= 0,
		ERROR,
	};

	int nal_forbidden_zero_bit;
	int nal_ref_idc;
	int nal_type;

	SPS sps;
	PPS pps;
	Prediction pred;

private:
	Bitstream &bs;

public:
	Parser(Bitstream &bs);
	int parseNAL();

private:
	const char *nalToString(int type);
	void parseSlice();
};

}


#endif
 