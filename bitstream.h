#ifndef __BITSTREAM_H
#define __BITSTREAM_H

#include <stdio.h>
#include <stdlib.h>

namespace h264 {

class Bitstream {
public:
	enum {
		FILE_ERROR	= -1,
		ALLOC_ERROR	= -2,
	};

private:
	FILE *file;
	unsigned char *buf;
	int ofs;
	int size;
public:

	Bitstream();
	~Bitstream();

	int load(const char *filename);

	int remain();
	unsigned int getBit();
	unsigned int getBits(int bits);
	unsigned int peekBits(int bits);
	void advanceBits(int bits);
	void alignToByte();
	int findBits(unsigned int val_to_find, int bits);

	unsigned int getEG();
	unsigned int getSignedEG();

	void showStat();
};

}

#endif
