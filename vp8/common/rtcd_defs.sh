common_forward_decls() {
cat <<EOF
struct blockd;
EOF
}
forward_decls common_forward_decls

prototype void vp8_filter_block2d_4x4_8 "const unsigned char *src_ptr, const unsigned int src_stride, const short *HFilter_aligned16, const short *VFilter_aligned16, unsigned char *dst_ptr, unsigned int dst_stride"
specialize vp8_filter_block2d_4x4_8 sse4_1
prototype void vp8_filter_block2d_8x4_8 "const unsigned char *src_ptr, const unsigned int src_stride, const short *HFilter_aligned16, const short *VFilter_aligned16, unsigned char *dst_ptr, unsigned int dst_stride"
specialize vp8_filter_block2d_8x4_8 sse4_1
prototype void vp8_filter_block2d_8x8_8 "const unsigned char *src_ptr, const unsigned int src_stride, const short *HFilter_aligned16, const short *VFilter_aligned16, unsigned char *dst_ptr, unsigned int dst_stride"
specialize vp8_filter_block2d_8x8_8 sse4_1
prototype void vp8_filter_block2d_16x16_8 "const unsigned char *src_ptr, const unsigned int src_stride, const short *HFilter_aligned16, const short *VFilter_aligned16, unsigned char *dst_ptr, unsigned int dst_stride"
specialize vp8_filter_block2d_16x16_8 sse4_1
