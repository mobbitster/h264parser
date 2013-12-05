#ifndef __DEFINES_H
#define __DEFINES_H

namespace h264 {

enum {
	NAL_NONIDR	= 0x01,
	NAL_IDR 	= 0x05,
	NAL_SPS 	= 0x07,
	NAL_PPS 	= 0x08,
};

enum {
	P_SLICE 	= 0,
	B_SLICE		= 1,
	I_SLICE 	= 2,
	SP_SLICE	= 3,
	SI_SLICE	= 4,
};

enum {
	I_4x4,
	I_16x16_0_0_0,
	I_16x16_1_0_0,
	I_16x16_2_0_0,
	I_16x16_3_0_0,
	I_16x16_0_1_0,
	I_16x16_1_1_0,
	I_16x16_2_1_0,
	I_16x16_3_1_0,
	I_16x16_0_2_0,
	I_16x16_1_2_0,
	I_16x16_2_2_0,
	I_16x16_3_2_0,
	I_16x16_0_0_1,
	I_16x16_1_0_1,
	I_16x16_2_0_1,
	I_16x16_3_0_1,
	I_16x16_0_1_1,
	I_16x16_1_1_1,
	I_16x16_2_1_1,
	I_16x16_3_1_1,
	I_16x16_0_2_1,
	I_16x16_1_2_1,
	I_16x16_2_2_1,
	I_16x16_3_2_1,
	I_PCM,

	P_L0_16x16,
	P_L0_L0_16x8,
	P_L0_L0_8x16,
	P_8x8,
	P_8x8ref0,

	B_Direct_16x16,
	B_L0_16x16,
	B_L1_16x16,
	B_Bi_16x16,
	B_L0_L0_16x8,
	B_L0_L0_8x16,
	B_L1_L1_16x8,
	B_L1_L1_8x16,
	B_L0_L1_16x8,
	B_L0_L1_8x16,
	B_L1_L0_16x8,
	B_L1_L0_8x16,
	B_L0_Bi_16x8,
	B_L0_Bi_8x16,
	B_L1_Bi_16x8,
	B_L1_Bi_8x16,
	B_Bi_L0_16x8,
	B_Bi_L0_8x16,
	B_Bi_L1_16x8,
	B_Bi_L1_8x16,
	B_Bi_Bi_16x8,
	B_Bi_Bi_8x16,
	B_8x8,

	LAST_NONINFERRED,

	B_SKIP			= 0xff,
	P_SKIP			= 0xff,

};

#define is_intra(m)	((m) >= I_4x4 && (m) <= I_PCM)
#define is_inter(m) (((m) >= P_L0_16x16 && (m) <= B_8x8) || (m) == P_SKIP || (m) == B_SKIP)

enum {
	P_L0_8x8		= 0,
	P_L0_8x4,
	P_L0_4x8,
	P_L0_4x4,
};

enum {
	B_Direct_8x8	= 0,
	B_L0_8x8,
	B_L1_8x8,
	B_Bi_8x8,
	B_L0_8x4,
	B_L0_4x8,
	B_L1_8x4,
	B_L1_4x8,
	B_Bi_8x4,
	B_Bi_4x8,
	B_L0_4x4,
	B_L1_4x4,
	B_Bi_4x4,
};

enum {
	Intra_4x4,
	Intra_16x16,
	Pred_L0,
	Pred_L1,
	BiPred,
	Direct,
};

#define NA	(-1)

#define chroma_array_type	(1)

}

#endif
