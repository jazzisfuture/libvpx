vpx_mem_forward_decls() {
cat <<EOF
struct yv12_buffer_config;
EOF
}
forward_decls vpx_mem_forward_decls

prototype void vp8_yv12_extend_frame_borders "struct yv12_buffer_config *ybf"
specialize vp8_yv12_extend_frame_borders neon

prototype void vp8_yv12_copy_frame "struct yv12_buffer_config *src_ybc, struct yv12_buffer_config *dst_ybc"
specialize vp8_yv12_copy_frame neon

prototype void vp8_yv12_copy_y "struct yv12_buffer_config *src_ybc, struct yv12_buffer_config *dst_ybc"
specialize vp8_yv12_copy_y neon
