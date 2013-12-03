#ifndef __SLICEHEADER_H
#define __SLICEHEADER_H

#include "bitstream.h"
#include "params.h"
#include "parser.h"

namespace h264 {

class SliceHeader
{
public:
	int first_mb_in_slice;
	int slice_type;
	int pic_parameter_set_id;
	int frame_num;
	int field_pic_flag;
	int mb_aff_frame_flag;
	int pic_height_in_mbs;
	int pic_height_in_samples;
	int pic_size_in_mbs;
	int bottom_field_flag;
	int idr_pic_id;
	int pic_order_cnt_lsb;
	int delta_pic_order_cnt_bottom;
	int delta_pic_order_cnt[2];
	int redundant_pic_cnt;
	int direct_spatial_mv_pred_flag;
	int num_ref_idx_active_override_flag;
	int num_ref_idx_l0_active;
	int num_ref_idx_l1_active;
	int ref_pic_list_reordering_flag_l0;
	int ref_pic_list_reordering_flag_l1;
// <dec_ref_pic_marking>
	int no_output_of_prior_pics_flag;
	int long_term_reference_flag;
	int adaptive_ref_pic_marking_mode_flag;
// </dec_ref_pic_marking>
	int cabac_init_idc;
	int slice_qp_delta;
	int slice_qpy;
	int sp_for_switch_flag;
	int slice_qs_delta;
	int disable_deblocking_filter_idc;
	int slice_alpha_c0_offset_div2;
	int slice_beta_offset_div2;
	int slice_group_change_cycle;

private:
	Bitstream &bs;
	Parser const& parser;
	SPS const& sps;
	PPS const& pps;
public:
	SliceHeader(Bitstream &bs, Parser const& parser) : 
		bs(bs), parser(parser), sps(parser.sps), pps(parser.pps)
	{
	};

	int parse();

	bool isType(int type) const
	{
		return slice_type == type; 
	}
private:
	const char *sliceTypeToString(int type);
	void skip_ref_pic_list_reordering();
	void skip_adaptive_ref_pic_marking();
};

}

#endif