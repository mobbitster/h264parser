#ifndef __MACROBLOCKTYPE_H
#define __MACROBLOCKTYPE_H

#include "macroblock.h"

namespace h264 {

class Macroblock;

class MacroblockType
{
public:
	static void decode(Macroblock &mb, int rawType);
	static void decodeSub(Macroblock &mb, int idx, int rawSubType);
private:
	static void setUndef(Macroblock &mb);
	static void decodeISlice(Macroblock &mb, int rawType);
	static void decodePSlice(Macroblock &mb, int rawType);
	static void decodeBSlice(Macroblock &mb, int rawType);

	static void decodePSliceSub(Macroblock &mb, int idx, int rawSubType);
	static void decodeBSliceSub(Macroblock &mb, int idx, int rawSubType);

	static const char *typeToString(int type);
	static const char *predictionModeToString(int mode);
};

}

#endif
