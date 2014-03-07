// PPA_REGISTER_CPU_EVENT(x)
/*
*	@brief register a CPU event 
*	@param x CPU register events
*/


// PPA_REGISTER_CPU_EVENT2GROUP(x,y)
/*
*	@brief register CPU events with group
*	@param x CPU register events
*	@param y group name if y is "NoGrp" it means there is no group
*/

PPA_REGISTER_CPU_EVENT(vp9_decode_frame_head_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_entropy_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_dequant_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_inter_pred_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_inter_transform_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_intra_pred_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_loopfilter_inline_time)
PPA_REGISTER_CPU_EVENT(vp9_frame_entropy_dec_cpu_time)
PPA_REGISTER_CPU_EVENT(vp9_dequant_time)
PPA_REGISTER_CPU_EVENT(vp9_inter_pred_time)
PPA_REGISTER_CPU_EVENT(vp9_intra_pred_time)
PPA_REGISTER_CPU_EVENT(vp9_loopfilter_time)
PPA_REGISTER_CPU_EVENT(vp9_decode_frame_time)
PPA_REGISTER_CPU_EVENT(process_task_time)
PPA_REGISTER_CPU_EVENT(loopfilter_time)
PPA_REGISTER_CPU_EVENT(loopfilter_inline_time)
PPA_REGISTER_CPU_EVENT(vp9_inter_transform_time)
PPA_REGISTER_CPU_EVENT(vp9_receive_compressed_data_time)
PPA_REGISTER_CPU_EVENT(vp9_decode_time)
PPA_REGISTER_CPU_EVENT(vp9_read_time)
PPA_REGISTER_CPU_EVENT(vp9_tiles_inter_pred_rs_time)
