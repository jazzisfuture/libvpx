common_forward_decls() {
cat <<EOF
struct blockd;
struct loop_filter_info;
EOF
}
forward_decls common_forward_decls

#
# Dequant
#
prototype void vp8_dequantize_b "struct blockd*, short *dqc"
specialize vp8_dequantize_b mmx v6 neon

prototype void vp8_dequant_idct_add "short *input, short *dq, unsigned char *output, int stride"
specialize vp8_dequant_idct_add mmx v6 neon

prototype void vp8_dequant_idct_add_y_block "short *q, short *dq, unsigned char *dst, int stride, char *eobs"
specialize vp8_dequant_idct_add_y_block mmx sse2 v6 neon

prototype void vp8_dequant_idct_add_uv_block "short *q, short *dq, unsigned char *dst_u, unsigned char *dst_v, int stride, char *eobs"
specialize vp8_dequant_idct_add_uv_block mmx sse2 v6 neon

#
# Loopfilter
#
prototype void vp8_loop_filter_mbv "unsigned char *y, unsigned char *u, unsigned char *v, int ystride, int uv_stride, struct loop_filter_info *lfi"
specialize vp8_loop_filter_mbv mmx sse2 v6 neon
vp8_loop_filter_mbv_v6=vp8_loop_filter_mbv_armv6

prototype void vp8_loop_filter_bv "unsigned char *y, unsigned char *u, unsigned char *v, int ystride, int uv_stride, struct loop_filter_info *lfi"
specialize vp8_loop_filter_bv mmx sse2 v6 neon
vp8_loop_filter_bv_v6=vp8_loop_filter_bv_armv6

prototype void vp8_loop_filter_mbh "unsigned char *y, unsigned char *u, unsigned char *v, int ystride, int uv_stride, struct loop_filter_info *lfi"
specialize vp8_loop_filter_mbh mmx sse2 v6 neon
vp8_loop_filter_mbh_v6=vp8_loop_filter_mbh_armv6

prototype void vp8_loop_filter_bh "unsigned char *y, unsigned char *u, unsigned char *v, int ystride, int uv_stride, struct loop_filter_info *lfi"
specialize vp8_loop_filter_bh mmx sse2 v6 neon
vp8_loop_filter_bh_v6=vp8_loop_filter_bh_armv6


prototype void vp8_loop_filter_simple_mbv "unsigned char *y, int ystride, const unsigned char *blimit"
specialize vp8_loop_filter_simple_mbv mmx sse2 v6
vp8_loop_filter_simple_mbv_c=vp8_loop_filter_simple_vertical_edge_c
vp8_loop_filter_simple_mbv_mmx=vp8_loop_filter_simple_vertical_edge_mmx
vp8_loop_filter_simple_mbv_sse2=vp8_loop_filter_simple_vertical_edge_sse2
vp8_loop_filter_simple_mbv_v6=vp8_loop_filter_simple_vertical_edge_armv6


prototype void vp8_loop_filter_simple_mbh "unsigned char *y, int ystride, const unsigned char *blimit"
specialize vp8_loop_filter_simple_mbh mmx sse2 v6
vp8_loop_filter_simple_mbh_c=vp8_loop_filter_simple_horizontal_edge_c
vp8_loop_filter_simple_mbh_mmx=vp8_loop_filter_simple_horizontal_edge_mmx
vp8_loop_filter_simple_mbh_sse2=vp8_loop_filter_simple_horizontal_edge_sse2
vp8_loop_filter_simple_mbh_v6=vp8_loop_filter_simple_horizontal_edge_armv6

prototype void vp8_loop_filter_simple_bv "unsigned char *y, int ystride, const unsigned char *blimit"
specialize vp8_loop_filter_simple_bv mmx sse2 v6
vp8_loop_filter_simple_bv_c=vp8_loop_filter_bvs_c
vp8_loop_filter_simple_bv_mmx=vp8_loop_filter_bvs_mmx
vp8_loop_filter_simple_bv_sse2=vp8_loop_filter_bvs_sse2
vp8_loop_filter_simple_bv_v6=vp8_loop_filter_bvs_armv6

prototype void vp8_loop_filter_simple_bh "unsigned char *y, int ystride, const unsigned char *blimit"
specialize vp8_loop_filter_simple_bh mmx sse2 v6
vp8_loop_filter_simple_bh_c=vp8_loop_filter_bhs_c
vp8_loop_filter_simple_bh_mmx=vp8_loop_filter_bhs_mmx
vp8_loop_filter_simple_bh_sse2=vp8_loop_filter_bhs_sse2
vp8_loop_filter_simple_bh_v6=vp8_loop_filter_bhs_armv6

#
# IDCT
#
#idct16
prototype void vp8_short_idct4x4llm "short *input, unsigned char *pred, int pitch, unsigned char *dst, int dst_stride"
specialize vp8_short_idct4x4llm mmx v6
vp8_short_idct4x4llm_v6=vp8_short_idct4x4llm_v6_dual

#iwalsh1
prototype void vp8_short_inv_walsh4x4_1 "short *input, short *output"
specialize vp8_short_inv_walsh4x4_1 c #no asm yet

#iwalsh16
prototype void vp8_short_inv_walsh4x4 "short *input, short *output"
specialize vp8_short_inv_walsh4x4 mmx sse2 v6 neon

#idct1_scalar_add
prototype void vp8_dc_only_idct_add "short input, unsigned char *pred, int pred_stride, unsigned char *dst, int dst_stride"
specialize vp8_dc_only_idct_add	mmx v6 neon
