#ifndef __RESIDUALS_H
#define __RESIDUALS_H

#include "macroblock.h"
#include "codetables.h"

namespace h264 {

class Residuals
{
private:
	Macroblock &mb;
	static bool code_tables_initialized;
public:
	Residuals(Macroblock &mb) : mb(mb) {};
	void parse();
private:

	static struct code_table *initCodeTable(struct code_table_item *items);
	static void initCodeTables();
	int getCode(struct code_table *table);
	int residualBlock(int *coeff_level, int max_num_coeff, int nc);
};

}

#endif
