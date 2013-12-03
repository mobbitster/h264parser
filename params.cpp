#include "params.h"

namespace h264 {

SPS::SPS(Bitstream &bs) : bs(bs), parsed(false)
{
}

int SPS::parse()
{
	int res = 0;

#ifdef DEBUG
	printf("parse SPS\n");
#endif

	this->profile_idc = bs.getBits(8);
	this->constraint_set0_flag = bs.getBit();
	this->constraint_set1_flag = bs.getBit();
	this->constraint_set2_flag = bs.getBit();
	this->reserved_zero_5bits = bs.getBits(5);
	this->level_idc = bs.getBits(8);
	this->seq_parameter_set_id = bs.getEG();
	this->log2_max_frame_num = bs.getEG() + 4;
	this->max_frame_num = 1 << this->log2_max_frame_num;
	this->pic_order_cnt_type = bs.getEG();
	if (this->pic_order_cnt_type == 0) {
		this->log2_max_pic_order_cnt_lsb = bs.getEG() + 4;
		this->max_pic_order_cnt_lsb = 1 << this->log2_max_pic_order_cnt_lsb;
	}
	else if (this->pic_order_cnt_type == 1) {
		this->delta_pic_order_always_zero_flag = bs.getBit();
		this->offset_for_non_ref_pic = bs.getSignedEG();
		this->offset_for_top_to_bottom_field = bs.getSignedEG();
		this->num_ref_frames_in_pic_order_cnt_cycle = bs.getEG();
		for (int i = 0; i < this->num_ref_frames_in_pic_order_cnt_cycle; ++i)
			this->offset_for_ref_frame[i] = bs.getSignedEG();
	}
	this->num_ref_frames = bs.getEG();
	this->gaps_in_frame_num_value_allowed_flag = bs.getBit();
	this->pic_width_in_mbs = bs.getEG() + 1;
	this->pic_width_in_samples = this->pic_width_in_mbs * 16;
	this->pic_height_in_map_units = bs.getEG() + 1;
	this->pic_size_in_map_units = this->pic_height_in_map_units;
	this->frame_mbs_only_flag = bs.getBit();
	this->frame_height_in_mbs = (2 - this->frame_mbs_only_flag) * this->pic_height_in_map_units;
	this->frame_height_in_samples = 16 * this->frame_height_in_mbs;
	if (!this->frame_mbs_only_flag)
		this->mb_adaptive_frame_field_flag = bs.getBit();
	this->direct_8x8_inference_flag = bs.getBit();
	this->frame_cropping_flag = bs.getBit();
	if (this->frame_cropping_flag) {
		this->frame_crop_left_offset = bs.getEG();
		this->frame_crop_right_offset = bs.getEG();
		this->frame_crop_top_offset = bs.getEG();
		this->frame_crop_bottom_offset = bs.getEG();
	}
	this->vui_parameters_present_flag = bs.getBit();

	bs.alignToByte();
	bs.findBits(0x000000, 24);

	parsed = true;

	return res;
}

PPS::PPS(Bitstream &bs) : bs(bs), parsed(false)
{
}

int PPS::parse()
{
	int res = 0;

#ifdef DEBUG
	printf("parse PPS\n");
#endif

	this->pic_parameter_set_id = bs.getEG();
	this->seq_parameter_set_id = bs.getEG();
	this->entropy_coding_mode_flag =bs.getBit();
	this->pic_order_present_flag =bs.getBit();
	this->num_slice_groups = bs.getEG() + 1;
	if (this->num_slice_groups>1) {
		this->slice_group_map_type = bs.getEG();
	
		if (this->slice_group_map_type == 0) {
			for (int i = 0; i<this->num_slice_groups; ++i)
				this->run_length[i] = bs.getEG();
		} else if (this->slice_group_map_type == 2) {
			for (int i = 0; i<this->num_slice_groups; ++i) {
				this->top_left[i] = bs.getEG();
				this->bottom_right[i] = bs.getEG();
			}
		} else if ((this->slice_group_map_type >= 3) && (this->slice_group_map_type <= 5)) {
			this->slice_group_change_direction_flag = bs.getBit();
			this->slice_group_change_rate = bs.getEG() + 1;
		} else if (this->slice_group_map_type == 6) {
			this->pic_size_in_map_units = bs.getEG() + 1;
			for (int i=0; i<this->pic_size_in_map_units; ++i)
				this->slice_group_id[i] = bs.getEG();
		}

	}

	this->num_ref_idx_l0_active = bs.getEG() + 1;
	this->num_ref_idx_l1_active = bs.getEG() + 1;

#ifdef DEBUG
	printf("num_ref_idx_l0_active %d num_ref_idx_l1_active %d\n", 
		this->num_ref_idx_l0_active, this->num_ref_idx_l1_active);
#endif
	this->weighted_pred_flag = bs.getBit();
	this->weighted_bipred_idc = bs.getBits(2);
	this->pic_init_qp = bs.getSignedEG() + 26;
	this->pic_init_qs = bs.getSignedEG() + 26;
	this->chroma_qp_index_offset = bs.getSignedEG();
	this->deblocking_filter_control_present_flag = bs.getBit();
	this->constrained_intra_pred_flag = bs.getBit();
	this->redundant_pic_cnt_present_flag = bs.getBit();

#ifdef DEBUG
	printf("this->entropy_coding_mode_flag %d\n", this->entropy_coding_mode_flag);
#endif

	parsed = true;

	return res;
}

}