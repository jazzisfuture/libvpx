/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef ONYXD_INT_H_
#define ONYXD_INT_H_

#include "vpx_config.h"
#include "vp8/common/onyxd.h"
#include "treereader.h"
#include "vp8/common/onyxc_int.h"
#include "vp8/common/threading.h"

#if CONFIG_ERROR_CONCEALMENT
#include "ec_types.h"
#endif

typedef struct
{
    int ithread;
    void *ptr1;
    void *ptr2;
} DECODETHREAD_DATA;

typedef struct
{
    MACROBLOCKD  mbd;
} MB_ROW_DEC;


typedef struct
{
    int enabled;
    unsigned int count;
    const unsigned char *ptrs[MAX_PARTITIONS];
    unsigned int sizes[MAX_PARTITIONS];
} FRAGMENT_DATA;

#define MAX_FB_MT_DEC 32

struct fbnode
{
    YV12_BUFFER_CONFIG yv12_fb;

    /* debug / testing purposes */
    unsigned int is_key;
    unsigned int is_gld;
    unsigned int is_alt;
    unsigned int is_refresh_last;
    unsigned int frame_id;
    /* debug / testing purposes */

    unsigned int is_decoded;
    unsigned int show;

    int64_t last_time_stamp;

    YUV_TYPE clr_type;              /* color type */

    /* for vp8dx_get_raw_frame () */
    int y_width;
    int y_height;

    void *user_priv;

    int current_cx_frame;

    int ref_cnt;
    int ithread;
    unsigned int size_id;

    struct VP8D_COMP *this_pbi;

    struct fbnode *this_ref_fb[NUM_YV12_BUFFERS];
    struct fbnode *next_ref_fb[NUM_YV12_BUFFERS];

    struct fbnode *next;
    struct fbnode *prev;
};


struct frame_buffers
{
    struct fbnode *free_alloc;

    struct fbnode *free;

    struct fbnode *decoded;

    struct fbnode *decoded_head;

    /* decoded frame buffer ready to show */
    struct fbnode *decoded_to_show;

    unsigned int decoded_size;
    unsigned int max_allocated_frames;
    unsigned int size_id;
    unsigned int new_y_width;
    unsigned int new_y_height;

    unsigned int cx_data_count;

    /* enable/disable frame-based threading */
    int     use_frame_threads;

    /* decoder instances */
    struct VP8D_COMP *pbi[MAX_FB_MT_DEC];

    /* post proc data */
#if CONFIG_POSTPROC
    YV12_BUFFER_CONFIG post_proc_buffer;
    YV12_BUFFER_CONFIG post_proc_buffer_int;
    int post_proc_buffer_int_used;
    unsigned char *pp_limits_buffer;   /* post-processing filter coefficients */
    struct postproc_state  postproc_state;
#endif

};

typedef struct VP8D_COMP
{
    DECLARE_ALIGNED(16, MACROBLOCKD, mb);

    YV12_BUFFER_CONFIG *dec_fb_ref[NUM_YV12_BUFFERS];

    struct fbnode *this_fb;

    DECLARE_ALIGNED(16, VP8_COMMON, common);

    /* the last partition will be used for the modes/mvs */
    vp8_reader mbc[MAX_PARTITIONS];

    VP8D_CONFIG oxcf;

    FRAGMENT_DATA fragments;

#if CONFIG_MULTITHREAD
    /* variable for threading */

    volatile int b_multithreaded_rd;
    int max_threads;
    int current_mb_col_main;
    unsigned int decoding_thread_count;
    int allocated_decoding_thread_count;

    int mt_baseline_filter_level[MAX_MB_SEGMENTS];
    int sync_range;
    int *mt_current_mb_col;                  /* Each row remembers its already decoded column. */

    unsigned char **mt_yabove_row;           /* mb_rows x width */
    unsigned char **mt_uabove_row;
    unsigned char **mt_vabove_row;
    unsigned char **mt_yleft_col;            /* mb_rows x 16 */
    unsigned char **mt_uleft_col;            /* mb_rows x 8 */
    unsigned char **mt_vleft_col;            /* mb_rows x 8 */

    MB_ROW_DEC           *mb_row_di;
    DECODETHREAD_DATA    *de_thread_data;

    pthread_t           *h_decoding_thread;
    sem_t               *h_event_start_decoding;
    sem_t                h_event_end_decoding;
    /* end of threading data */
#endif

    int64_t last_time_stamp;
    int   ready_for_new_data;

    vp8_prob prob_intra;
    vp8_prob prob_last;
    vp8_prob prob_gf;
    vp8_prob prob_skip_false;

#if CONFIG_ERROR_CONCEALMENT
    MB_OVERLAP *overlaps;
    /* the mb num from which modes and mvs (first partition) are corrupt */
    unsigned int mvs_corrupt_from_mb;
#endif
    int ec_enabled;
    int ec_active;
    int decoded_key_frame;
    int independent_partitions;
    int frame_corrupt_residual;

    const unsigned char *decrypt_key;
} VP8D_COMP;

int vp8_decode_frame(VP8D_COMP *cpi);

int vp8_create_decoder_instances(struct frame_buffers *fb, VP8D_CONFIG *oxcf);
int vp8_remove_decoder_instances(struct frame_buffers *fb);
int vp8_create_frame_pool(struct frame_buffers *fb, unsigned int max_frames);
int vp8_destroy_frame_pool(struct frame_buffers *fb);
int vp8_adjust_decoder_frames(struct frame_buffers *fb, int width, int height);
int vp8dx_create_postproc_frame(struct frame_buffers *fb,
                                int width, int height);
void vp8dx_remove_postproc_frame(struct frame_buffers *fb);

void vp8_mark_last_as_corrupted(VP8D_COMP *pbi);

#if CONFIG_DEBUG
#define CHECK_MEM_ERROR(lval,expr) do {\
        lval = (expr); \
        if(!lval) \
            vpx_internal_error(&pbi->common.error, VPX_CODEC_MEM_ERROR,\
                               "Failed to allocate "#lval" at %s:%d", \
                               __FILE__,__LINE__);\
    } while(0)
#else
#define CHECK_MEM_ERROR(lval,expr) do {\
        lval = (expr); \
        if(!lval) \
            vpx_internal_error(&pbi->common.error, VPX_CODEC_MEM_ERROR,\
                               "Failed to allocate "#lval);\
    } while(0)
#endif

#endif  // ONYXD_INT_H_
