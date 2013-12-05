#ifndef __CODEDBLOCK_H
#define __CODEDBLOCK_H

#include "macroblock.h"

class Macroblock;

namespace h264 {

class CodedBlock {
public:
	static void parse(Macroblock &mb);
};

}

#endif
