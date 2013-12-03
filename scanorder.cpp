#include "scanorder.h"

int ScanOrderIntra4x4::intra_4x4_scan_order[16][2] =
{
	{ 0, 0},  { 4, 0},  { 0, 4},  { 4, 4},
	{ 8, 0},  {12, 0},  { 8, 4},  {12, 4},
	{ 0, 8},  { 4, 8},  { 0,12},  { 4,12},
	{ 8, 8},  {12, 8},  { 8,12},  {12,12}
};

int ScanOrderIntra4x4::getX(int idx, int x)
{
	return x + intra_4x4_scan_order[idx][0];
}

int ScanOrderIntra4x4::getY(int idx, int y)
{
	return y + intra_4x4_scan_order[idx][1];
}
