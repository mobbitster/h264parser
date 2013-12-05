#include "bitstream.h"

namespace h264 {

Bitstream::Bitstream() 
{
	this->buf = NULL;
}

Bitstream::~Bitstream()
{
	if (this->buf != NULL) {
		free(this->buf);
		this->buf = NULL;
	}
}

int Bitstream::load(const char *filename)
{
	int res = 0;
	size_t size;
	
	this->buf = NULL;

	this->file = fopen(filename, "rb");
	if (this->file == NULL) {
		res = Bitstream::FILE_ERROR;
		goto out_free;
	}

	fseek(this->file, 0, SEEK_END); 
	size = ftell(this->file);
	fseek(this->file, 0, SEEK_SET);

	this->buf = (unsigned char *)malloc(size);
	if (fread(this->buf, 1, size, this->file) != size) {
		perror(filename);
		res = Bitstream::ALLOC_ERROR;
		goto out_close;
	}

	fclose(this->file);

	this->ofs = 0;
	this->size = size;

out:
	return res;

out_close:
	fclose(this->file);

out_free:
	if (this->buf)
		free(this->buf);
	this->buf = NULL;
	goto out;
}

int Bitstream::remain()
{
	return this->size - (this->ofs >> 3);
}

unsigned int Bitstream::getBit()
{
	unsigned int value = ((*(this->buf + (this->ofs >> 0x3))) >> (0x7 - (this->ofs & 0x7))) & 0x1;
	this->ofs++;
	return value;
}

unsigned int Bitstream::peekBits(int bits)
{
	unsigned int value;
	value = *(this->buf + (this->ofs >> 0x3)) << 24 |
		*(this->buf + (this->ofs >> 0x3) + 1) << 16 |
		*(this->buf + (this->ofs >> 0x3) + 2) << 8 |
		*(this->buf + (this->ofs >> 0x3) + 3);

	value = (value >> (32 - bits - (this->ofs & 7))) & ((1UL << bits) - 1);

	return value;
}

unsigned int Bitstream::getBits(int bits)
{
	unsigned int value = 0;
	value = peekBits(bits);
	advanceBits(bits);

	return value;
}

void Bitstream::advanceBits(int bits)
{
	this->ofs += bits;
}

void Bitstream::alignToByte()
{
	for (;(this->ofs % 8) != 0;this->ofs++)
		;;
}

int Bitstream::findBits(unsigned int val_to_find, int bits)
{
	int res = 0;
	unsigned int val;

	while (remain() > (bits >> 3)) {
		val = peekBits(bits);

		if (val_to_find == val) {
			res = 1;
			break;
		}
		advanceBits(8);
	}
	return res;
}

unsigned int Bitstream::getEG()
{
	int i, zeros = 0;
	unsigned int value = 0;

	// printf("Initial offset %d\n", *offset);

	// printf(">ofs %d\n", *offset);

	while (getBit() == 0) {
		zeros++;
	}

	// printf("First non-zero offset %d\n", *offset);

	value = 1 << zeros;
	// (*offset)++;

	for (i = zeros - 1; i >= 0; i--) {
		value |= getBit() << i;
	}

	// printf(">val %d ofs %d\n", value - 1, *offset);

	return (value - 1);
}

int Bitstream::getSignedEG()
{
	int value = getEG();
	return (value & 0x01) ? (value + 1) >> 1 : -(value >> 1);
}

void Bitstream::showStat()
{
#ifdef DEBUG
	printf("************************** size %d ofs %d (%d) ************************************\n",
			this->size, this->ofs >> 3, this->ofs);
#endif
}

}
