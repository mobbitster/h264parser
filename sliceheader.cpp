#include "sliceheader.h"
#include "defines.h"

namespace h264 {

const char *SliceHeader::sliceTypeToString(int type)
{
	if (type == P_SLICE)
		return "P_SLICE";
	else if (type == B_SLICE)
		return "B_SLICE";
	else if (type == I_SLICE)
		return "I_SLICE";
	else if (type == SP_SLICE)
		return "SP_SLICE";
	else if (type == SI_SLICE)
		return "SI_SLICE";
	else
		return "undef";
}

void SliceHeader::skip_ref_pic_list_reordering() 
{
	int reordering_of_pic_nums_idc;
	int abs_diff_pic_num;
	int long_term_pic_num;

#ifdef DEBUG
	printf("Warning: I do not support reference picture list reordering.\n"
		"         Watch out for decoding errors!\n");
#endif

	do {
		reordering_of_pic_nums_idc = bs.getEG();
		if (reordering_of_pic_nums_idc == 0 || reordering_of_pic_nums_idc == 1)
			abs_diff_pic_num=bs.getEG() + 1;
		else if (reordering_of_pic_nums_idc == 2)
			long_term_pic_num = bs.getEG();
	} while (reordering_of_pic_nums_idc != 3);
}

void SliceHeader::skip_adaptive_ref_pic_marking() 
{
	int memory_management_control_operation;
	int difference_of_pic_nums;
	int long_term_pic_num;
	int long_term_frame_idx;
	int max_long_term_frame_idx;

#ifdef DEBUG
	printf("Warning: I do not support adaptive reference picture marking.\n"
		"         Watch out for decoding errors!\n");
#endif

	do {
		memory_management_control_operation=bs.getEG();
		if (memory_management_control_operation == 1 || memory_management_control_operation == 3)
			difference_of_pic_nums = bs.getEG() + 1;
		if (memory_management_control_operation == 2)
			long_term_pic_num = bs.getEG();
		if (memory_management_control_operation == 3 || memory_management_control_operation == 6)
			long_term_frame_idx = bs.getEG();
		if (memory_management_control_operation == 4)
			max_long_term_frame_idx = bs.getEG() - 1;
	} while (memory_management_control_operation != 0);
}

int SliceHeader::parse()
{
	int res = 0;

#ifdef DEBUG
	printf("Parse slice header\n");
#endif

	this->first_mb_in_slice = bs.getEG();
	this->slice_type = bs.getEG() % 5;
	this->pic_parameter_set_id = bs.getEG();
#ifdef DEBUG
	printf("\tslice type %s\n", sliceTypeToString(this->slice_type));
#endif
	this->frame_num = bs.getBits(sps.log2_max_frame_num);
#ifdef DEBUG
	printf("\tframe num %d\n", this->frame_num);
#endif
	if (!sps.frame_mbs_only_flag) {
		this->field_pic_flag = bs.getBit();
		if (this->field_pic_flag)
			this->bottom_field_flag = bs.getBit();
	}

	this->mb_aff_frame_flag = (sps.mb_adaptive_frame_field_flag && !this->field_pic_flag);
	this->pic_height_in_mbs = sps.frame_height_in_mbs/(1 + this->field_pic_flag);
	this->pic_height_in_samples = (this->pic_height_in_mbs) << 4;
	this->pic_size_in_mbs = sps.pic_width_in_mbs * this->pic_height_in_mbs;

	if (parser.nal_type == NAL_IDR)
		this->idr_pic_id = bs.getEG();

	if (sps.pic_order_cnt_type == 0) {
		this->pic_order_cnt_lsb = bs.getBits(sps.log2_max_pic_order_cnt_lsb);
		if (pps.pic_order_present_flag && !this->field_pic_flag)
			this->delta_pic_order_cnt_bottom = bs.getSignedEG();
	}
	if (sps.pic_order_cnt_type == 1 && !sps.delta_pic_order_always_zero_flag) {
		this->delta_pic_order_cnt[0] = bs.getSignedEG();
		if (pps.pic_order_present_flag && !this->field_pic_flag)
			this->delta_pic_order_cnt[1] = bs.getSignedEG();
	}

	if (pps.redundant_pic_cnt_present_flag)
		this->redundant_pic_cnt = bs.getEG();

	// if (this->slice_type == B_SLICE)
	// 	this->direct_spatial_mv_pred_flag = bs.getBit();

	// if (this->slice_type == P_SLICE || this->slice_type == B_SLICE || this->slice_type == SP_SLICE) {
	if (this->slice_type != I_SLICE) {
		if (this->slice_type == B_SLICE)
			this->direct_spatial_mv_pred_flag = bs.getBit();

		this->num_ref_idx_active_override_flag = bs.getBit();
		if (this->num_ref_idx_active_override_flag) {
			this->num_ref_idx_l0_active = bs.getEG() + 1;
			if (this->slice_type == B_SLICE)
				this->num_ref_idx_l1_active = bs.getEG() + 1;
			else
				this->num_ref_idx_l1_active = 1;
		}
	}

	if (this->slice_type != I_SLICE && this->slice_type != SI_SLICE) {
		this->ref_pic_list_reordering_flag_l0 = bs.getBit();
		if (this->ref_pic_list_reordering_flag_l0)
			skip_ref_pic_list_reordering();
	}

	if (this->slice_type == B_SLICE) {
		this->ref_pic_list_reordering_flag_l1 = bs.getBit();
		if (this->ref_pic_list_reordering_flag_l1)
			skip_ref_pic_list_reordering();
	}

	if ((pps.weighted_pred_flag && (this->slice_type == P_SLICE || 
		this->slice_type == SP_SLICE)) ||
			(pps.weighted_bipred_idc == 1 && this->slice_type == B_SLICE)) {
		fprintf(stderr,"sorry, I _really_ do not support weighted prediction!\n");
		exit(1);
	}

	if (parser.nal_ref_idc != 0) {
		if (parser.nal_type == NAL_IDR) {
			this->no_output_of_prior_pics_flag = bs.getBit();
			this->long_term_reference_flag = bs.getBit();
		} else {
			this->adaptive_ref_pic_marking_mode_flag = bs.getBit();
			if (this->adaptive_ref_pic_marking_mode_flag)
				skip_adaptive_ref_pic_marking();
		}
	}

	if (pps.entropy_coding_mode_flag && this->slice_type != I_SLICE && this->slice_type != SI_SLICE)
		this->cabac_init_idc = bs.getEG();

	this->slice_qp_delta = bs.getSignedEG();

	this->slice_qpy=pps.pic_init_qp+this->slice_qp_delta;

	if (this->slice_type==SP_SLICE || this->slice_type==SI_SLICE) {
		if(this->slice_type==SP_SLICE)
			this->sp_for_switch_flag = bs.getBit();
		this->slice_qs_delta = bs.getSignedEG();
	}

	if (pps.deblocking_filter_control_present_flag) {
		this->disable_deblocking_filter_idc = bs.getEG();
		if(this->disable_deblocking_filter_idc!=1) {
			this->slice_alpha_c0_offset_div2 = bs.getSignedEG();
			this->slice_beta_offset_div2 = bs.getSignedEG();
		}
	}

	if (pps.num_slice_groups > 1 && pps.slice_group_map_type >= 3 && pps.slice_group_map_type <= 5)
		this->slice_group_change_cycle = bs.getEG();

	return res;
}

}
