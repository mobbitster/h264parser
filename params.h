#ifndef __SPS_H
#define __SPS_H

#include "bitstream.h"

namespace h264 {

class SPS {
private:
	Bitstream &bs;
	bool parsed;

public:
	int profile_idc;
	int constraint_set0_flag;
	int constraint_set1_flag;
	int constraint_set2_flag;
	int reserved_zero_5bits;
	int level_idc;
	int seq_parameter_set_id;
	int log2_max_frame_num;
	int max_frame_num;
	int pic_order_cnt_type;
	int log2_max_pic_order_cnt_lsb;
	int max_pic_order_cnt_lsb;
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int num_ref_frames_in_pic_order_cnt_cycle;
	int offset_for_ref_frame[256];
	int num_ref_frames;
	int gaps_in_frame_num_value_allowed_flag;
	int pic_width_in_mbs;
	int pic_width_in_samples;
	int pic_height_in_map_units;
	int pic_size_in_map_units;
	int frame_height_in_mbs;
	int frame_height_in_samples;
	int frame_mbs_only_flag;
	int mb_adaptive_frame_field_flag;
	int direct_8x8_inference_flag;
	int frame_cropping_flag;
	int frame_crop_left_offset;
	int frame_crop_right_offset;
	int frame_crop_top_offset;
	int frame_crop_bottom_offset;
	int vui_parameters_present_flag;

public:
	SPS(Bitstream &bs);

	int parse();
	bool isParsed() 
	{
		return parsed;
	}
};

class PPS {
private:
	Bitstream &bs;
	bool parsed;

public:
	int pic_parameter_set_id;
	int seq_parameter_set_id;
	int entropy_coding_mode_flag;
	int pic_order_present_flag;
	int num_slice_groups;
	int slice_group_map_type;
	int run_length[8];
	int top_left[8];
	int bottom_right[8];
	int slice_group_change_direction_flag;
	int slice_group_change_rate;
	int pic_size_in_map_units;
	int slice_group_id[8192];
	int num_ref_idx_l0_active;
	int num_ref_idx_l1_active;
	int weighted_pred_flag;
	int weighted_bipred_idc;
	int pic_init_qp;
	int pic_init_qs;
	int chroma_qp_index_offset;
	int deblocking_filter_control_present_flag;
	int constrained_intra_pred_flag;
	int redundant_pic_cnt_present_flag;

public:
	PPS(Bitstream &bs);

	int parse();
	bool isParsed() 
	{
		return parsed;
	}
};

}

#endif