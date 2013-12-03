#include "parser.h"
#include "params.h"
#include "defines.h"
#include "slice.h"

namespace h264 {

Parser::Parser(Bitstream &bs) : bs(bs), pps(bs), sps(bs)
{
};

const char *Parser::nalToString(int type)
{
	if (type == NAL_SPS)
		return "SPS";
	else if (type == NAL_PPS)
		return "PPS";
	else if (type == NAL_NONIDR)
		return "NON IDR";
	else if (type == NAL_IDR)
		return "IDR";
	else
		return "undef";
}

void Parser::parseSlice()
{
	Slice slice(bs, *this);
	slice.parse();
}

int Parser::parseNAL()
{
	int res = Parser::OK, rc;

	rc = bs.findBits(0x00000001, 32);
	if (rc == 0) {
		bs.showStat();
		printf("Unable to find NAL tag\n");
		return Parser::ERROR;
	}

	bs.advanceBits(32);

	// if (bstream_remain(bs) <= 4)
	// 	return res;

	nal_forbidden_zero_bit = bs.getBit();
	nal_ref_idc = bs.getBits(2);
	nal_type = bs.getBits(5);

#ifdef DEBUG
	printf("nal_forbidden_zero_bit %d nal_ref_idc %d nal_type %d (%s)\n", 
		nal_forbidden_zero_bit, nal_ref_idc, nal_type, nalToString(nal_type));
#endif

	switch (nal_type) {
		case NAL_SPS:
			sps.parse();
			break;
		case NAL_PPS:
			pps.parse();
			break;
		case NAL_NONIDR:
		case NAL_IDR:
			parseSlice();
			break;
		default:
			printf("Unsupported NAL type %x\n", nal_type);
			break;
	}

	if (sps.isParsed() && pps.isParsed() && !pred.isAllocated())
	 	pred.allocate(sps.pic_width_in_samples, sps.frame_height_in_samples);

	bs.alignToByte();

	return res;
}

}
