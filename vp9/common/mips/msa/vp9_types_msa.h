/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_MIPS_MSA_VP9_TYPES_MSA_H_
#define VP9_COMMON_MIPS_MSA_VP9_TYPES_MSA_H_

#if HAVE_MSA

/* MIPS MSA wide types */
#define WRLEN               128
#define NUMWRELEM           (WRLEN >> 3)

/* Vector of signed bytes */
typedef signed char VINT8 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of unsigned bytes */
typedef unsigned char VUINT8 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of signed halfwords */
typedef int16_t VINT16 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of unsigned halfwords */
typedef uint16_t VUINT16 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of signed words */
typedef int32_t VINT32 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of unsigned words */
typedef uint32_t VUINT32 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of signed doublewords */
typedef int64_t VINT64 __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of unsigned doublewords */
typedef uint64_t VUINT64 __attribute__ ((vector_size(NUMWRELEM)));

/* Vector of single precision floating-point values */
typedef float VFLOAT __attribute__ ((vector_size(NUMWRELEM)));
/* Vector of double precision floating-point values */
typedef double VDOUBLE __attribute__ ((vector_size(NUMWRELEM)));

#define  __add_a_b     __builtin_msa_add_a_b
#define  __add_a_h     __builtin_msa_add_a_h
#define  __add_a_w     __builtin_msa_add_a_w
#define  __add_a_d     __builtin_msa_add_a_d

#define  __adds_a_b    __builtin_msa_adds_a_b
#define  __adds_a_h    __builtin_msa_adds_a_h
#define  __adds_a_w    __builtin_msa_adds_a_w
#define  __adds_a_d    __builtin_msa_adds_a_d

#define  __adds_s_b    __builtin_msa_adds_s_b
#define  __adds_s_h    __builtin_msa_adds_s_h
#define  __adds_s_w    __builtin_msa_adds_s_w
#define  __adds_s_d    __builtin_msa_adds_s_d

#define  __adds_u_b    __builtin_msa_adds_u_b
#define  __adds_u_h    __builtin_msa_adds_u_h
#define  __adds_u_w    __builtin_msa_adds_u_w
#define  __adds_u_d    __builtin_msa_adds_u_d

#define  __andi_b      __builtin_msa_andi_b

#define  __asub_s_b    __builtin_msa_asub_s_b
#define  __asub_s_h    __builtin_msa_asub_s_h
#define  __asub_s_w    __builtin_msa_asub_s_w
#define  __asub_s_d    __builtin_msa_asub_s_d

#define  __asub_u_b    __builtin_msa_asub_u_b
#define  __asub_u_h    __builtin_msa_asub_u_h
#define  __asub_u_w    __builtin_msa_asub_u_w
#define  __asub_u_d    __builtin_msa_asub_u_d

#define  __ave_s_b     __builtin_msa_ave_s_b
#define  __ave_s_h     __builtin_msa_ave_s_h
#define  __ave_s_w     __builtin_msa_ave_s_w
#define  __ave_s_d     __builtin_msa_ave_s_d

#define  __ave_u_b     __builtin_msa_ave_u_b
#define  __ave_u_h     __builtin_msa_ave_u_h
#define  __ave_u_w     __builtin_msa_ave_u_w
#define  __ave_u_d     __builtin_msa_ave_u_d

#define  __aver_s_b    __builtin_msa_aver_s_b
#define  __aver_s_h    __builtin_msa_aver_s_h
#define  __aver_s_w    __builtin_msa_aver_s_w
#define  __aver_s_d    __builtin_msa_aver_s_d

#define  __aver_u_b    __builtin_msa_aver_u_b
#define  __aver_u_h    __builtin_msa_aver_u_h
#define  __aver_u_w    __builtin_msa_aver_u_w
#define  __aver_u_d    __builtin_msa_aver_u_d

#define  __bclr_b      __builtin_msa_bclr_b
#define  __bclr_h      __builtin_msa_bclr_h
#define  __bclr_w      __builtin_msa_bclr_w
#define  __bclr_d      __builtin_msa_bclr_d

#define  __bclri_b     __builtin_msa_bclri_b
#define  __bclri_h     __builtin_msa_bclri_h
#define  __bclri_w     __builtin_msa_bclri_w
#define  __bclri_d     __builtin_msa_bclri_d

#define  __binsl_b     __builtin_msa_binsl_b
#define  __binsl_h     __builtin_msa_binsl_h
#define  __binsl_w     __builtin_msa_binsl_w
#define  __binsl_d     __builtin_msa_binsl_d

#define  __binsli_b    __builtin_msa_binsli_b
#define  __binsli_h    __builtin_msa_binsli_h
#define  __binsli_w    __builtin_msa_binsli_w
#define  __binsli_d    __builtin_msa_binsli_d

#define  __binsr_b     __builtin_msa_binsr_b
#define  __binsr_h     __builtin_msa_binsr_h
#define  __binsr_w     __builtin_msa_binsr_w
#define  __binsr_d     __builtin_msa_binsr_d

#define  __binsri_b    __builtin_msa_binsri_b
#define  __binsri_h    __builtin_msa_binsri_h
#define  __binsri_w    __builtin_msa_binsri_w
#define  __binsri_d    __builtin_msa_binsri_d

#define  __bmnz_v      __builtin_msa_bmnz_v
#define  __bmnzi_b     __builtin_msa_bmnzi_b
#define  __bmz_v       __builtin_msa_bmz_v
#define  __bmzi_b      __builtin_msa_bmzi_b
#define  __bneg_b      __builtin_msa_bneg_b

#define  __bnegi_b     __builtin_msa_bnegi_b
#define  __bnegi_h     __builtin_msa_bnegi_h
#define  __bnegi_w     __builtin_msa_bnegi_w
#define  __bnegi_d     __builtin_msa_bnegi_d

#define  __bnz_v       __builtin_msa_bnz_v

#define  __bsel_v      __builtin_msa_bsel_v
#define  __bseli_b     __builtin_msa_bseli_b

#define  __bset_b      __builtin_msa_bset_b
#define  __bset_h      __builtin_msa_bset_h
#define  __bset_w      __builtin_msa_bset_w
#define  __bset_d      __builtin_msa_bset_d

#define  __bseti_b     __builtin_msa_bseti_b
#define  __bseti_h     __builtin_msa_bseti_h
#define  __bseti_w     __builtin_msa_bseti_w
#define  __bseti_d     __builtin_msa_bseti_d

#define  __bz_v        __builtin_msa_bz_v

#define  __ceqi_b      __builtin_msa_ceqi_b
#define  __ceqi_h      __builtin_msa_ceqi_h
#define  __ceqi_w      __builtin_msa_ceqi_w
#define  __ceqi_d      __builtin_msa_ceqi_d

#define  __clei_s_b    __builtin_msa_clei_s_b
#define  __clei_s_h    __builtin_msa_clei_s_h
#define  __clei_s_w    __builtin_msa_clei_s_w
#define  __clei_s_d    __builtin_msa_clei_s_d

#define  __clei_u_b    __builtin_msa_clei_u_b
#define  __clei_u_h    __builtin_msa_clei_u_h
#define  __clei_u_w    __builtin_msa_clei_u_w
#define  __clei_u_d    __builtin_msa_clei_u_d

#define  __clti_s_b    __builtin_msa_clti_s_b
#define  __clti_s_h    __builtin_msa_clti_s_h
#define  __clti_s_w    __builtin_msa_clti_s_w
#define  __clti_s_d    __builtin_msa_clti_s_d

#define  __clt_s_b     __builtin_msa_clt_s_b
#define  __clt_s_h     __builtin_msa_clt_s_h
#define  __clt_s_w     __builtin_msa_clt_s_w
#define  __clt_s_d     __builtin_msa_clt_s_d

#define  __clti_u_b    __builtin_msa_clti_u_b
#define  __clti_u_h    __builtin_msa_clti_u_h
#define  __clti_u_w    __builtin_msa_clti_u_w
#define  __clti_u_d    __builtin_msa_clti_u_d

#define  __copy_s_b    __builtin_msa_copy_s_b
#define  __copy_s_h    __builtin_msa_copy_s_h
#define  __copy_s_w    __builtin_msa_copy_s_w
#define  __copy_s_d    __builtin_msa_copy_s_d

#define  __copy_u_b    __builtin_msa_copy_u_b
#define  __copy_u_h    __builtin_msa_copy_u_h
#define  __copy_u_w    __builtin_msa_copy_u_w
#define  __copy_u_d    __builtin_msa_copy_u_d

#define  __dotp_s_h    __builtin_msa_dotp_s_h
#define  __dotp_s_w    __builtin_msa_dotp_s_w
#define  __dotp_s_d    __builtin_msa_dotp_s_d

#define  __dotp_u_h    __builtin_msa_dotp_u_h
#define  __dotp_u_w    __builtin_msa_dotp_u_w
#define  __dotp_u_d    __builtin_msa_dotp_u_d

#define  __dpadd_s_h   __builtin_msa_dpadd_s_h
#define  __dpadd_s_w   __builtin_msa_dpadd_s_w
#define  __dpadd_s_d   __builtin_msa_dpadd_s_d

#define  __dpadd_u_h   __builtin_msa_dpadd_u_h
#define  __dpadd_u_w   __builtin_msa_dpadd_u_w
#define  __dpadd_u_d   __builtin_msa_dpadd_u_d

#define  __dpsub_s_h   __builtin_msa_dpsub_s_h
#define  __dpsub_s_w   __builtin_msa_dpsub_s_w
#define  __dpsub_s_d   __builtin_msa_dpsub_s_d

#define  __dpsub_u_h   __builtin_msa_dpsub_u_h
#define  __dpsub_u_w   __builtin_msa_dpsub_u_w
#define  __dpsub_u_d   __builtin_msa_dpsub_u_d

#define  __fill_b      __builtin_msa_fill_b
#define  __fill_h      __builtin_msa_fill_h
#define  __fill_w      __builtin_msa_fill_w
#define  __fill_d      __builtin_msa_fill_d

#define  __hadd_s_h    __builtin_msa_hadd_s_h
#define  __hadd_s_w    __builtin_msa_hadd_s_w
#define  __hadd_s_d    __builtin_msa_hadd_s_d

#define  __hadd_u_h    __builtin_msa_hadd_u_h
#define  __hadd_u_w    __builtin_msa_hadd_u_w
#define  __hadd_u_d    __builtin_msa_hadd_u_d

#define  __hsub_s_h    __builtin_msa_hsub_s_h
#define  __hsub_s_w    __builtin_msa_hsub_s_w
#define  __hsub_s_d    __builtin_msa_hsub_s_d

#define  __hsub_u_h    __builtin_msa_hsub_u_h
#define  __hsub_u_w    __builtin_msa_hsub_u_w
#define  __hsub_u_d    __builtin_msa_hsub_u_d

#define  __ilvev_b     __builtin_msa_ilvev_b
#define  __ilvev_h     __builtin_msa_ilvev_h
#define  __ilvev_w     __builtin_msa_ilvev_w
#define  __ilvev_d     __builtin_msa_ilvev_d

#define  __ilvl_b      __builtin_msa_ilvl_b
#define  __ilvl_h      __builtin_msa_ilvl_h
#define  __ilvl_w      __builtin_msa_ilvl_w
#define  __ilvl_d      __builtin_msa_ilvl_d

#define  __ilvod_b     __builtin_msa_ilvod_b
#define  __ilvod_h     __builtin_msa_ilvod_h
#define  __ilvod_w     __builtin_msa_ilvod_w
#define  __ilvod_d     __builtin_msa_ilvod_d

#define  __ilvr_b      __builtin_msa_ilvr_b
#define  __ilvr_h      __builtin_msa_ilvr_h
#define  __ilvr_w      __builtin_msa_ilvr_w
#define  __ilvr_d      __builtin_msa_ilvr_d

#define  __insert_b    __builtin_msa_insert_b
#define  __insert_h    __builtin_msa_insert_h
#define  __insert_w    __builtin_msa_insert_w
#define  __insert_d    __builtin_msa_insert_d

#define  __insve_b     __builtin_msa_insve_b
#define  __insve_h     __builtin_msa_insve_h
#define  __insve_w     __builtin_msa_insve_w
#define  __insve_d     __builtin_msa_insve_d

#define  __ld_b        __builtin_msa_ld_b
#define  __ld_h        __builtin_msa_ld_h
#define  __ld_w        __builtin_msa_ld_w
#define  __ld_d        __builtin_msa_ld_d

#define  __ldi_b       __builtin_msa_ldi_b
#define  __ldi_h       __builtin_msa_ldi_h
#define  __ldi_w       __builtin_msa_ldi_w
#define  __ldi_d       __builtin_msa_ldi_d

#define  __madd_q_h    __builtin_msa_madd_q_h
#define  __madd_q_w    __builtin_msa_madd_q_w

#define  __maddr_q_h   __builtin_msa_maddr_q_h
#define  __maddr_q_w   __builtin_msa_maddr_q_w

#define  __max_a_b     __builtin_msa_max_a_b
#define  __max_a_h     __builtin_msa_max_a_h
#define  __max_a_w     __builtin_msa_max_a_w
#define  __max_a_d     __builtin_msa_max_a_d

#define  __max_s_b     __builtin_msa_max_s_b
#define  __max_s_h     __builtin_msa_max_s_h
#define  __max_s_w     __builtin_msa_max_s_w
#define  __max_s_d     __builtin_msa_max_s_d

#define  __max_u_b     __builtin_msa_max_u_b
#define  __max_u_h     __builtin_msa_max_u_h
#define  __max_u_w     __builtin_msa_max_u_w
#define  __max_u_d     __builtin_msa_max_u_d

#define  __maxi_s_b    __builtin_msa_maxi_s_b
#define  __maxi_s_h    __builtin_msa_maxi_s_h
#define  __maxi_s_w    __builtin_msa_maxi_s_w
#define  __maxi_s_d    __builtin_msa_maxi_s_d

#define  __maxi_u_b    __builtin_msa_maxi_u_b
#define  __maxi_u_h    __builtin_msa_maxi_u_h
#define  __maxi_u_w    __builtin_msa_maxi_u_w
#define  __maxi_u_d    __builtin_msa_maxi_u_d

#define  __min_a_b     __builtin_msa_min_a_b
#define  __min_a_h     __builtin_msa_min_a_h
#define  __min_a_w     __builtin_msa_min_a_w
#define  __min_a_d     __builtin_msa_min_a_d

#define  __min_s_b     __builtin_msa_min_s_b
#define  __min_s_h     __builtin_msa_min_s_h
#define  __min_s_w     __builtin_msa_min_s_w
#define  __min_s_d     __builtin_msa_min_s_d

#define  __min_u_b     __builtin_msa_min_u_b
#define  __min_u_h     __builtin_msa_min_u_h
#define  __min_u_w     __builtin_msa_min_u_w
#define  __min_u_d     __builtin_msa_min_u_d

#define  __mini_s_b    __builtin_msa_mini_s_b
#define  __mini_s_h    __builtin_msa_mini_s_h
#define  __mini_s_w    __builtin_msa_mini_s_w
#define  __mini_s_d    __builtin_msa_mini_s_d

#define  __mini_u_b    __builtin_msa_mini_u_b
#define  __mini_u_h    __builtin_msa_mini_u_h
#define  __mini_u_w    __builtin_msa_mini_u_w
#define  __mini_u_d    __builtin_msa_mini_u_d

#define  __msub_q_h    __builtin_msa_msub_q_h
#define  __msub_q_w    __builtin_msa_msub_q_w

#define  __msubr_q_h   __builtin_msa_msubr_q_h
#define  __msubr_q_w   __builtin_msa_msubr_q_w

#define  __mul_q_h     __builtin_msa_mul_q_h
#define  __mul_q_w     __builtin_msa_mul_q_w

#define  __mulr_q_h    __builtin_msa_mulr_q_h
#define  __mulr_q_w    __builtin_msa_mulr_q_w

#define  __nloc_b      __builtin_msa_nloc_b
#define  __nloc_h      __builtin_msa_nloc_h
#define  __nloc_w      __builtin_msa_nloc_w
#define  __nloc_d      __builtin_msa_nloc_d

#define  __nlzc_b      __builtin_msa_nlzc_b
#define  __nlzc_h      __builtin_msa_nlzc_h
#define  __nlzc_w      __builtin_msa_nlzc_w
#define  __nlzc_d      __builtin_msa_nlzc_d

#define  __nor_v       __builtin_msa_nor_v
#define  __nori_b      __builtin_msa_nori_b

#define  __ori_b       __builtin_msa_ori_b

#define  __pckev_b     __builtin_msa_pckev_b
#define  __pckev_h     __builtin_msa_pckev_h
#define  __pckev_w     __builtin_msa_pckev_w
#define  __pckev_d     __builtin_msa_pckev_d

#define  __pckod_b     __builtin_msa_pckod_b
#define  __pckod_h     __builtin_msa_pckod_h
#define  __pckod_w     __builtin_msa_pckod_w
#define  __pckod_d     __builtin_msa_pckod_d

#define  __sat_s_b     __builtin_msa_sat_s_b
#define  __sat_s_h     __builtin_msa_sat_s_h
#define  __sat_s_w     __builtin_msa_sat_s_w
#define  __sat_s_d     __builtin_msa_sat_s_d

#define  __sat_u_b     __builtin_msa_sat_u_b
#define  __sat_u_h     __builtin_msa_sat_u_h
#define  __sat_u_w     __builtin_msa_sat_u_w
#define  __sat_u_d     __builtin_msa_sat_u_d

#define  __shf_b       __builtin_msa_shf_b
#define  __shf_h       __builtin_msa_shf_h
#define  __shf_w       __builtin_msa_shf_w

#define  __sld_b       __builtin_msa_sld_b
#define  __sld_h       __builtin_msa_sld_h
#define  __sld_w       __builtin_msa_sld_w
#define  __sld_d       __builtin_msa_sld_d

#define  __sldi_b      __builtin_msa_sldi_b
#define  __sldi_h      __builtin_msa_sldi_h
#define  __sldi_w      __builtin_msa_sldi_w
#define  __sldi_d      __builtin_msa_sldi_d

#define  __splat_b     __builtin_msa_splat_b
#define  __splat_h     __builtin_msa_splat_h
#define  __splat_w     __builtin_msa_splat_w
#define  __splat_d     __builtin_msa_splat_d

#define  __splati_b    __builtin_msa_splati_b
#define  __splati_h    __builtin_msa_splati_h
#define  __splati_w    __builtin_msa_splati_w
#define  __splati_d    __builtin_msa_splati_d

#define  __srar_b      __builtin_msa_srar_b
#define  __srar_h      __builtin_msa_srar_h
#define  __srar_w      __builtin_msa_srar_w
#define  __srar_d      __builtin_msa_srar_d

#define  __srari_b     __builtin_msa_srari_b
#define  __srari_h     __builtin_msa_srari_h
#define  __srari_w     __builtin_msa_srari_w
#define  __srari_d     __builtin_msa_srari_d

#define  __srl_b       __builtin_msa_srl_b
#define  __srl_h       __builtin_msa_srl_h
#define  __srl_w       __builtin_msa_srl_w
#define  __srl_d       __builtin_msa_srl_d

#define  __srli_b      __builtin_msa_srli_b
#define  __srli_h      __builtin_msa_srli_h
#define  __srli_w      __builtin_msa_srli_w
#define  __srli_d      __builtin_msa_srli_d

#define  __srlr_b      __builtin_msa_srlr_b
#define  __srlr_h      __builtin_msa_srlr_h
#define  __srlr_w      __builtin_msa_srlr_w
#define  __srlr_d      __builtin_msa_srlr_d

#define  __srlri_b     __builtin_msa_srlri_b
#define  __srlri_h     __builtin_msa_srlri_h
#define  __srlri_w     __builtin_msa_srlri_w
#define  __srlri_d     __builtin_msa_srlri_d

#define  __st_b        __builtin_msa_st_b
#define  __st_h        __builtin_msa_st_h
#define  __st_w        __builtin_msa_st_w
#define  __st_d        __builtin_msa_st_d

#define  __subs_s_b    __builtin_msa_subs_s_b
#define  __subs_s_h    __builtin_msa_subs_s_h
#define  __subs_s_w    __builtin_msa_subs_s_w
#define  __subs_s_d    __builtin_msa_subs_s_d

#define  __subs_u_b    __builtin_msa_subs_u_b
#define  __subs_u_h    __builtin_msa_subs_u_h
#define  __subs_u_w    __builtin_msa_subs_u_w
#define  __subs_u_d    __builtin_msa_subs_u_d

#define  __subsus_u_b  __builtin_msa_subsus_u_b
#define  __subsus_u_h  __builtin_msa_subsus_u_h
#define  __subsus_u_w  __builtin_msa_subsus_u_w
#define  __subsus_u_d  __builtin_msa_subsus_u_d

#define  __subsuu_s_b  __builtin_msa_subsuu_s_b
#define  __subsuu_s_h  __builtin_msa_subsuu_s_h
#define  __subsuu_s_w  __builtin_msa_subsuu_s_w
#define  __subsuu_s_d  __builtin_msa_subsuu_s_d

#define  __vshf_b      __builtin_msa_vshf_b
#define  __vshf_h      __builtin_msa_vshf_h
#define  __vshf_w      __builtin_msa_vshf_w
#define  __vshf_d      __builtin_msa_vshf_d

#define  __xori_b      __builtin_msa_xori_b

#endif  // #if HAVE_MSA
#endif  // VP9_COMMON_MIPS_MSA_VP9_TYPES_MSA_H_
