#include <stdio.h>

#include "bitstream.h"
#include "parser.h"

using namespace h264;

int main(int argc, char **argv)
{
	Bitstream bs;
	Parser parser(bs);

	if (bs.load(argv[1]) != 0) {
		printf("Error loading file %s\n", argv[1]);
		return -1;
	}

	while (parser.parseNAL() == Parser::OK)
		;;

	return 0;
}