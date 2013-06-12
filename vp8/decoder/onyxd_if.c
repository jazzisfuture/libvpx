/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp8/common/onyxc_int.h"
#if CONFIG_POSTPROC
#include "vp8/common/postproc.h"
#endif
#include "vp8/common/onyxd.h"
#include "onyxd_int.h"
#include "vpx_mem/vpx_mem.h"
#include "vp8/common/alloccommon.h"
#include "vp8/common/loopfilter.h"
#include "vp8/common/swapyv12buffer.h"
#include "vp8/common/threading.h"
#include "decoderthreading.h"
#include <stdio.h>
#include <assert.h>

#include "vp8/common/quant_common.h"
#include "./vpx_scale_rtcd.h"
#include "vpx_scale/vpx_scale.h"
#include "vp8/common/systemdependent.h"
#include "vpx_ports/vpx_timer.h"
#include "detokenize.h"
#if CONFIG_ERROR_CONCEALMENT
#include "error_concealment.h"
#endif
#if ARCH_ARM
#include "vpx_ports/arm.h"
#endif

extern void vp8cx_init_de_quantizer(VP8D_COMP *pbi);

int vp8_destroy_frame_pool(struct frame_buffers *fb)
{
    unsigned int max_frames = fb->max_allocated_frames;
    unsigned int i;

    if(fb->free_alloc)
    {
        for(i = 0; i < max_frames; i++)
        {
            vp8_yv12_de_alloc_frame_buffer(&fb->free_alloc[i].yv12_fb);
        }

        vpx_free(fb->free_alloc);

        fb->free_alloc = 0;
    }
    return 0;
}

int vp8_create_frame_pool(struct frame_buffers *fb, unsigned int max_frames)
{
    unsigned int i;

    vp8_destroy_frame_pool(fb);

    fb->free_alloc =
    fb->free = (struct fbnode *) vpx_malloc(sizeof(struct fbnode) * (max_frames+1));
    if(!fb->free)
        return -1;

    vpx_memset(fb->free, 0, sizeof(struct fbnode) * (max_frames+1));

    for(i = 0; i < max_frames; i++)
    {
        fb->free[i].next = &fb->free[i+1];

        /* for testing/debug purposes */
        fb->free[i].frame_id = i + 0xa0;
    }

    /* terminate */
    fb->free[max_frames-1].next = 0;

    /* for debug purposes... we should never use this extra node */
    fb->free[max_frames].frame_id = i;

    fb->max_allocated_frames = max_frames;
    fb->decoded = (struct fbnode *)0;
    fb->decoded_head = (struct fbnode *)0;
    fb->decoded_to_show = (struct fbnode *)0;
    fb->cx_data_count = 0;
    fb->size_id = 0;

    return 0;
}

#if 0
/* for debug purposes */
void vp8_show_free_buffers(struct frame_buffers *fb)
{
    struct fbnode *this_node = fb->free;

    if(this_node)
    {
        while(this_node)
        {
            printf("this_node->frame_id %d this_node %x\n", this_node->frame_id,
                (unsigned int)this_node);
            this_node = this_node->next;
        }
    }
    else
        printf("fb->free is NULL\n");
}

/* for debug purposes */
void vp8_show_decoded_buffers(struct frame_buffers *fb)
{
    struct fbnode *this_node = fb->decoded;

    if(this_node)
    {
        while(this_node)
        {
            printf("vp8_show_(): cx_frame_num %d frame_id %d this_node %x K:%d "
                "G:%d A:%d rL:%d refcnt:%d s:%x d:%x ithread:%x\n", this_node->current_cx_frame, this_node->frame_id, (unsigned int)this_node,
                this_node->is_key, this_node->is_gld, this_node->is_alt,
            this_node->is_refresh_last, this_node->ref_cnt, this_node->show, this_node->is_decoded,
            this_node->ithread);
            this_node = this_node->next;
        }
    }
    else
        printf("fb->decoded is NULL\n");
}
#endif

static struct fbnode * get_free_fb(struct frame_buffers *fb)
{
    struct fbnode *free_node = fb->free;

    if(fb->free)
    {
        /* remove empty buffer from pool */
        fb->free = fb->free->next;
    }

    return free_node;
}

/* reset before adding back to free pool */
static void clear_fbnode(struct fbnode *free_node)
{
    if(free_node)
    {
        free_node->next = 0;
        free_node->prev = 0;
        free_node->is_alt = 0;
        free_node->is_gld = 0;
        free_node->is_key = 0;
        free_node->is_decoded = 0;
        free_node->show = 0;
        free_node->last_time_stamp = 0;
        free_node->is_refresh_last = 0;

        free_node->ref_cnt = 0;
        free_node->dec_state = 0;
        free_node->mb_row = 0;
    }
}

/* add buffer back to pool */
static void put_free_fb(struct frame_buffers *fb, struct fbnode *free_node)
{
    clear_fbnode(free_node);

    if(fb->free)
        free_node->next = fb->free;

    fb->free = free_node;
}

static void update_frame_if_resize(struct frame_buffers *fb, struct fbnode *fbn,
                                 VP8D_COMP *pbi)
{
    pbi->this_fb = fbn;
    /* check if resize occurred */
    if(fbn->size_id != fb->size_id)
    {
        int width, height;
        /* dealloc only if a buffer has been created */
        if(fbn->y_width != 0 && fbn->y_height != 0)
            vp8_yv12_de_alloc_frame_buffer(&fbn->yv12_fb);

        width = fb->new_y_width;
        height = fb->new_y_height;
        /* our internal buffers are always multiples of 16 */
        if ((width & 0xf) != 0)
            width += 16 - (width & 0xf);

        if ((height & 0xf) != 0)
            height += 16 - (height & 0xf);

        if (vp8_yv12_alloc_frame_buffer(&fbn->yv12_fb, width, height,
                                        VP8BORDERINPIXELS) < 0)
        {
            vpx_internal_error(&pbi->common.error, VPX_CODEC_MEM_ERROR,
                               "Failed to allocate frame buffer");
        }

        pbi->mb.pre = pbi->mb.dst = fbn->yv12_fb;

#if CONFIG_MULTITHREAD
        if (pbi->b_multithreaded_rd)
        {
            int i;
            for (i = 0; i < pbi->allocated_decoding_thread_count; i++)
            {
                pbi->mb_row_di[i].mbd.dst = pbi->mb.dst;
                vp8_build_block_doffsets(&pbi->mb_row_di[i].mbd);
            }
        }
#endif

        vp8_build_block_doffsets(&pbi->mb);

        /* mark this frame as having the most current frame dimensions */
        fbn->size_id = fb->size_id;
    }
}

/* add frame buffer to decoded list */
static void add_fb_to_decoded(struct frame_buffers *fb, struct fbnode *fbn,
                             void *user_priv)
{
    fbn->user_priv = user_priv;
    if(fb->decoded)
    {
        fb->decoded_head->next = fbn;
        fbn->prev = fb->decoded_head;
        fbn->next = (struct fbnode *)0;
    }
    else
    {
        fb->decoded = fbn;
        fb->decoded->prev = (struct fbnode *)0;
        fb->decoded->next = (struct fbnode *)0;
    }

    fb->decoded_head = fbn;
    fb->decoded_size++;
}

static void remove_fb_frome_decoded(struct frame_buffers *fb,
                                    struct fbnode * this_node)
{
    if(this_node->next)
    {
        this_node->next->prev = this_node->prev;
        if(this_node->prev)
            this_node->prev->next = this_node->next;
        else
            fb->decoded = this_node->next;
    }
    else
    {
        fb->decoded_head = this_node->prev;
        if(this_node->prev)
            this_node->prev->next = (struct fbnode *)0;
        else
            /* list is now empty */
            fb->decoded = (struct fbnode *)0;
    }

    fb->decoded_size--;

    put_free_fb(fb, this_node);
}

int vp8_get_to_show(struct frame_buffers *fb)
{
    struct fbnode * to_show_fb = 0;

    fb->decoded_to_show = 0;

    vp8_fbmt_mutex_lock();

    if(fb->decoded_size)
    {
        struct fbnode * this_node;

        /* vp8_show_decoded_buffers(fb); */

        to_show_fb = fb->decoded;

        while(!to_show_fb->show)
        {
            if(to_show_fb->is_decoded == 0)
            {
                vp8_fbmt_mutex_unlock();
                return 1; /* buffer not available yet */
            }

            to_show_fb = to_show_fb->next;
            if(!to_show_fb)
            {
//                printf("~~~~~~~~~~~~ all buffers shown decoded size %d\n", fb->decoded_size);
//                vp8_show_decoded_buffers(fb);
                vp8_fbmt_mutex_unlock();
                return -1; /* empty buffer... all buffers have been shown */
            }
        }

        if(to_show_fb->is_decoded == 0)
        {
            printf("~~~~~~ buffer not ready current_cx_frame %d ref_cnt %d\n", to_show_fb->current_cx_frame, to_show_fb->ref_cnt);
            vp8_fbmt_mutex_unlock();
            return 1; /* buffer not available yet */
        }

        /* this frame buffer will be shown by the calling app, so lets set
         * to zero so we do not show again */
        to_show_fb->show = 0;

        /* remove unreferenced buffers */
        this_node = to_show_fb->prev;
        while(this_node)
        {
            if(this_node->ref_cnt <= 0)
            {
                struct fbnode *n = this_node;

                /* we are about to remove this node from list, so point to
                 * the previous frame before the ptrs are adjusted */
                this_node = this_node->prev;

                remove_fb_frome_decoded(fb, n);
            }
            else
            {
                this_node = this_node->prev;
            }
        }
    }

    fb->decoded_to_show = to_show_fb;
    vp8_fbmt_mutex_unlock();

    return 0;
}

static void remove_decompressor(VP8D_COMP *pbi)
{
  VP8_COMMON *oci = &pbi->common;
  int i;
#if CONFIG_ERROR_CONCEALMENT
    vp8_de_alloc_overlap_lists(pbi);
#endif
    for (i = 0; i < NUM_YV12_BUFFERS; i++)
        vp8_yv12_de_alloc_frame_buffer(&oci->yv12_fb[i]);

    vpx_free(oci->above_context);
    vpx_free(oci->mip);
#if CONFIG_ERROR_CONCEALMENT
    vpx_free(oci->prev_mip);
    oci->prev_mip = NULL;
#endif

    oci->above_context = NULL;
    oci->mip = NULL;

    vpx_free(pbi);
}

static struct VP8D_COMP * create_decompressor(VP8D_CONFIG *oxcf)
{
    VP8D_COMP *pbi = vpx_memalign(32, sizeof(VP8D_COMP));

    if (!pbi)
        return NULL;

    vpx_memset(pbi, 0, sizeof(VP8D_COMP));

    if (setjmp(pbi->common.error.jmp))
    {
        pbi->common.error.setjmp = 0;
        remove_decompressor(pbi);
        return 0;
    }

    pbi->common.error.setjmp = 1;

    vp8_create_common(&pbi->common);

    pbi->common.current_video_frame = 0;
    pbi->ready_for_new_data = 1;

    /* vp8cx_init_de_quantizer() is first called here. Add check in frame_init_dequantizer() to avoid
     *  unnecessary calling of vp8cx_init_de_quantizer() for every frame.
     */
    vp8cx_init_de_quantizer(pbi);

    vp8_loop_filter_init(&pbi->common);

    pbi->common.error.setjmp = 0;

#if CONFIG_ERROR_CONCEALMENT
    pbi->ec_enabled = oxcf->error_concealment;
    pbi->overlaps = NULL;
#else
    pbi->ec_enabled = 0;
#endif
    /* Error concealment is activated after a key frame has been
     * decoded without errors when error concealment is enabled.
     */
    pbi->ec_active = 0;

    pbi->decoded_key_frame = 0;

    /* Independent partitions is activated when a frame updates the
     * token probability table to have equal probabilities over the
     * PREV_COEF context.
     */
    pbi->independent_partitions = 0;

    vp8_setup_block_dptrs(&pbi->mb);

    return pbi;
}

void vp8dx_remove_postproc_frame(struct frame_buffers *fb)
{
#if CONFIG_POSTPROC
    vp8_yv12_de_alloc_frame_buffer(&fb->post_proc_buffer);
    if (fb->post_proc_buffer_int_used)
        vp8_yv12_de_alloc_frame_buffer(&fb->post_proc_buffer_int);

    vpx_free(fb->pp_limits_buffer);
    fb->pp_limits_buffer = NULL;
#endif
}

int vp8dx_create_postproc_frame(struct frame_buffers *fb, int width, int height)
{
    vp8dx_remove_postproc_frame(fb);

    /* our internal buffers are always multiples of 16 */
    if ((width & 0xf) != 0)
        width += 16 - (width & 0xf);

    if ((height & 0xf) != 0)
        height += 16 - (height & 0xf);

#if CONFIG_POSTPROC
    if (vp8_yv12_alloc_frame_buffer(&fb->post_proc_buffer, width, height,
                                    VP8BORDERINPIXELS) < 0)
    {
        vp8dx_remove_postproc_frame(fb);
        return 1;
    }

    fb->post_proc_buffer_int_used = 0;
    vpx_memset(&fb->postproc_state, 0, sizeof(fb->postproc_state));
    vpx_memset((&fb->post_proc_buffer)->buffer_alloc,128,(&fb->post_proc_buffer)->frame_size);

    /* Allocate buffer to store post-processing filter coefficients.
     *
     * Note: Round up mb_cols to support SIMD reads
     */
    fb->pp_limits_buffer = vpx_memalign(16, 24 * (((width >> 4) + 1) & ~1));
    if (!fb->pp_limits_buffer)
    {
        vp8dx_remove_postproc_frame(fb);
        return 1;
    }
#endif
    return 0;
}

void remove_decoder_frame(VP8_COMMON *oci)
{
#if CONFIG_ERROR_CONCEALMENT
    vpx_free(oci->prev_mip);
    oci->prev_mip = NULL;
#endif

    vpx_free(oci->above_context);
    vpx_free(oci->mip);

    oci->above_context = NULL;
    oci->mip = NULL;
}

static int create_decoder_frame(VP8D_COMP *pbi, int width, int height)
{
    VP8_COMMON *const oci = & pbi->common;
#if CONFIG_MULTITHREAD
    int prev_mb_rows = oci->mb_rows;
#endif

    remove_decoder_frame(oci);

    /* our internal buffers are always multiples of 16 */
    if ((width & 0xf) != 0)
        width += 16 - (width & 0xf);

    if ((height & 0xf) != 0)
        height += 16 - (height & 0xf);

    oci->mb_rows = height >> 4;
    oci->mb_cols = width >> 4;
    oci->MBs = oci->mb_rows * oci->mb_cols;
    oci->mode_info_stride = oci->mb_cols + 1;
    oci->mip = vpx_calloc((oci->mb_cols + 1) * (oci->mb_rows + 1),
                          sizeof(MODE_INFO));
    if (!oci->mip)
    {
        remove_decoder_frame(oci);
        vpx_internal_error(&oci->error, VPX_CODEC_MEM_ERROR,
                           "Failed to allocate"
                           "MODE_INFO array");
    }

    oci->mi = oci->mip + oci->mode_info_stride + 1;

    oci->above_context = vpx_calloc(sizeof(ENTROPY_CONTEXT_PLANES) * oci->mb_cols, 1);
    if (!oci->above_context)
    {
        remove_decoder_frame(oci);
        vpx_internal_error(&oci->error, VPX_CODEC_MEM_ERROR,
                           "Failed to allocate"
                           "above_context array");
    }

    /* allocate memory for last frame MODE_INFO array */
#if CONFIG_ERROR_CONCEALMENT
    if (pbi->ec_enabled)
    {
        /* old prev_mip was released by vp8dx_remove_decoder_frame() */
        oci->prev_mip = vpx_calloc(
                           (oci->mb_cols + 1) * (oci->mb_rows + 1),
                           sizeof(MODE_INFO));

        if (!oci->prev_mip)
        {
            remove_decoder_frame(oci);
            vpx_internal_error(&oci->error, VPX_CODEC_MEM_ERROR,
                               "Failed to allocate"
                               "last frame MODE_INFO array");
        }

        oci->prev_mi = oci->prev_mip + oci->mode_info_stride + 1;

        if (vp8_alloc_overlap_lists(pbi))
            vpx_internal_error(&oci->error, VPX_CODEC_MEM_ERROR,
                               "Failed to allocate overlap lists "
                               "for error concealment");
    }
#endif

#if CONFIG_MULTITHREAD
      if (pbi->b_multithreaded_rd)
          vp8mt_alloc_temp_buffers(pbi, width, prev_mb_rows);
#endif

    return 0;
}

vpx_codec_err_t vp8dx_get_reference(struct frame_buffers *fb, VP8D_COMP *pbi,
                                    enum vpx_ref_frame_type ref_frame_flag,
                                    YV12_BUFFER_CONFIG *sd)
{
  struct fbnode *ref_fb;
  YV12_BUFFER_CONFIG *yv12_ref;
  int frame_index;

  if (ref_frame_flag == VP8_LAST_FRAME)
      frame_index = LAST_FRAME;
  else if (ref_frame_flag == VP8_GOLD_FRAME)
      frame_index = GOLDEN_FRAME;
  else if (ref_frame_flag == VP8_ALTR_FRAME)
      frame_index = ALTREF_FRAME;
  else{
      vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
          "Invalid reference frame");
      return pbi->common.error.error_code;
  }

  ref_fb = pbi->this_fb->this_ref_fb[frame_index];
  yv12_ref = &ref_fb->yv12_fb;

  if(yv12_ref->y_height != sd->y_height ||
          yv12_ref->y_width != sd->y_width ||
          yv12_ref->uv_height != sd->uv_height ||
          yv12_ref->uv_width != sd->uv_width)
  {
      vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
          "Incorrect buffer dimensions");
  }
  else
      vp8_yv12_copy_frame(yv12_ref, sd);

    return pbi->common.error.error_code;
}

vpx_codec_err_t vp8dx_set_reference(struct frame_buffers *fb, VP8D_COMP *pbi,
                                    enum vpx_ref_frame_type ref_frame_flag,
                                    YV12_BUFFER_CONFIG *sd)
{
    struct fbnode *ref_fb;
    YV12_BUFFER_CONFIG *yv12_ref;
    int frame_index;

    if (ref_frame_flag == VP8_LAST_FRAME)
        frame_index = LAST_FRAME;
    else if (ref_frame_flag == VP8_GOLD_FRAME)
        frame_index = GOLDEN_FRAME;
    else if (ref_frame_flag == VP8_ALTR_FRAME)
        frame_index = ALTREF_FRAME;
    else{
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Invalid reference frame");
        return pbi->common.error.error_code;
    }

    ref_fb = pbi->this_fb->next_ref_fb[frame_index];
    yv12_ref = &ref_fb->yv12_fb;

    if(yv12_ref->y_height != sd->y_height ||
            yv12_ref->y_width != sd->y_width ||
            yv12_ref->uv_height != sd->uv_height ||
            yv12_ref->uv_width != sd->uv_width)
    {
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Incorrect buffer dimensions");
    }
    else
    {
        /* if only one reference, then ok to copy.
         * otherwise we must find a free buffer */
        if(ref_fb->ref_cnt > 1)
        {
            struct fbnode *new_ref_fb;

            /* remove reference from current buffer */
            ref_fb->ref_cnt--;

            /* find a free frame buffer */
            new_ref_fb = get_free_fb(fb);
            if(!new_ref_fb)
            {
                vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
                                   "Could not find a free frame buffer.");
                return pbi->common.error.error_code;
            }

            /* add into decoded list */
            add_fb_to_decoded(fb, new_ref_fb, NULL);

            /* add reference to current buffer */
            new_ref_fb->ref_cnt++;

            yv12_ref = &new_ref_fb->yv12_fb;

            pbi->this_fb->next_ref_fb[frame_index] = new_ref_fb;
        }

        vp8_yv12_copy_frame(sd, yv12_ref);
    }

    return pbi->common.error.error_code;
}

int check_fragments_for_errors(struct frame_buffers *fb, VP8D_COMP *pbi)
{
    if (!pbi->ec_active &&
        pbi->fragments.count <= 1 && pbi->fragments.sizes[0] == 0)
    {
        struct fbnode *ref_fb;

        ref_fb = pbi->this_fb->this_ref_fb[LAST_FRAME];

        /* If error concealment is disabled we won't signal missing frames
         * to the decoder.
         */

        if(ref_fb->ref_cnt > 1)
        {
            YV12_BUFFER_CONFIG *old_yv12_ref;
            YV12_BUFFER_CONFIG *new_yv12_ref;
            /* The last reference shares buffer with another reference
             * buffer. Move it to its own buffer before setting it as
             * corrupt, otherwise we will make multiple buffers corrupt.
             */
            struct fbnode *new_ref_fb;

            old_yv12_ref = &ref_fb->yv12_fb;
            /* remove reference from current buffer */
            ref_fb->ref_cnt--;

            /* find a free frame buffer */
            new_ref_fb = get_free_fb(fb);
            if(!new_ref_fb)
            {
                vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
                                   "Could not find a free frame buffer.");
                return pbi->common.error.error_code;
            }

            /* add into decoded list */
            add_fb_to_decoded(fb, new_ref_fb, NULL);

            /* add reference to current buffer */
            new_ref_fb->ref_cnt++;

            new_yv12_ref = &new_ref_fb->yv12_fb;

            pbi->this_fb->this_ref_fb[LAST_FRAME] = new_ref_fb;

            vp8_yv12_copy_frame(old_yv12_ref, new_yv12_ref);
        }

        /* This is used to signal that we are missing frames.
         * We do not know if the missing frame(s) was supposed to update
         * any of the reference buffers, but we act conservative and
         * mark only the last buffer as corrupted.
         */
        vp8_mark_last_as_corrupted(pbi);

        /* Signal that we have no frame to show. */
        pbi->common.show_frame = 0;

        /* Nothing more to do. */
        return 0;
    }

    return 1;
}

int vp8dx_receive_compressed_data2(struct frame_buffers *fb,
                                       unsigned long size,
                                       const unsigned char *source,
                                       int64_t time_stamp,
                                       void *user_priv)
{
    struct fbnode *this_fb;
    int retcode = 0;
    int i;
    VP8D_COMP *thread_pbi = NULL;
    int num_decoder_instances;

    num_decoder_instances = fb->fbmt.allocated_decoding_thread_count;
    if(!num_decoder_instances)
        num_decoder_instances = 1; /* single thread mode */

#if 0
    for(i = 0; i < num_decoder_instances; i++)
    {
        printf("fb->fbmt.thread_state[%d] : %d\n", i, fb->fbmt.thread_state[i]);
    }
#endif

    //printf("vp8dx_receive_compressed_data_fbmt4() fb->cx_data_count %d\n", fb->cx_data_count);

#if 1
    while(thread_pbi == NULL)
    {
        for(i = 0; i < num_decoder_instances; i++)
        {
            int state;

            vp8_fbmt_mutex_lock();
            state = fb->fbmt.thread_state[i];
            vp8_fbmt_mutex_unlock();
//printf("_____ waiting for thread\n");
            if(state == THREAD_READY)
            {
                thread_pbi = fb->pbi[i];
//printf("thread i:%d\n", i);

                vp8_fbmt_mutex_lock();
                fb->fbmt.thread_state[i] = THREAD_IN_USE;
                vp8_fbmt_mutex_unlock();
                break;
            }
        }
        /* if threads are busy, do other work */
//        if(!thread_pbi)
  //          return 0;
        if(num_decoder_instances > 1)
        {
//            x86_pause_hint();
  //          thread_sleep(0);
        }
    }
#else
    i = fb->cx_data_count;
    thread_pbi = fb->pbi[i&3];
#endif

#if 0
if(0)
    while(1)
    {
        if(fb->decoded_size >= (fb->max_allocated_frames - 1))
        {
            /* we are approaching a full condition, probably due to a long
             * decode */
            FBNODE *this_node = fb->decoded;
            int dec_size = fb->decoded_size;
            int all_decoded = 1;

            //printf("?? approaching a full condition dec_size:%d\n", dec_size);
            vp8_write_lock();
            while(this_node && dec_size)
            {
                all_decoded &= this_node->is_decoded;
                this_node = this_node->next;
                dec_size--;
            }
            vp8_write_unlock();

            if(all_decoded)
                break;
        }
        else
            break;
    }
#else
if(1)
    if(fb->decoded_size >= (fb->max_allocated_frames - 1))
    {
        /* we are approaching a full condition, probably due to a long
         * decode */
        struct fbnode *this_node = fb->decoded;
        int dec_size = fb->decoded_size;

        //printf("?? approaching a full condition dec_size:%d\n", dec_size);
//        vp8_write_lock();
//        vp8_write_unlock();

        /* search for the first frame not decoded */
        while(this_node && dec_size)
        {
            int is_decoded;
            is_decoded = this_node->is_decoded;

            if(!is_decoded)
                break;
            this_node = this_node->next;
            dec_size--;
        }

        if(dec_size)
        {
            /* wait for this frame to decode */
            while((!this_node->is_decoded))
            {
                x86_pause_hint();
                thread_sleep(0);
            }
        }
    }
#endif

    /* find a free buffer */
    this_fb = get_free_fb(fb);
    if(!this_fb)
    {
        printf("could not find free buffer cx_data_count:%d\n", fb->cx_data_count);
        //vp8_show_decoded_buffers(fb);
        printf("~~~~~~\n");
        return -1;
    }

    /* update internal buffers, if a resolution change occurred */
    update_frame_if_resize(fb, this_fb, thread_pbi);

    /* add into decoded list */
    add_fb_to_decoded(fb, this_fb, user_priv);

    //    printf("~~ current_cx_frame %d ref_cnt %d i:%d this_fb %x this_fb->prev %x thread_pbi %x\n",
  //         this_fb->current_cx_frame, this_fb->ref_cnt, i, this_fb, this_fb->prev, thread_pbi);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    thread_pbi->common.error.error_code = VPX_CODEC_OK;

    //TODO: fix fragments
//    if (thread_pbi->fragments.count == 0)
//    {
//        /* New frame, reset fragment pointers and sizes */
//        vpx_memset((void*)thread_pbi->fragments, 0, sizeof(thread_pbi->fragments));
//        vpx_memset(thread_pbi->fragment_sizes, 0, sizeof(thread_pbi->fragment_sizes));
///    }

    if (!thread_pbi->fragments.enabled)
    {
        thread_pbi->fragments.ptrs[0] = source;
        thread_pbi->fragments.sizes[0] = size;
        thread_pbi->fragments.count = 1;
    }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



    /* save the compressed data */
    this_fb->cx_data_ptr = vpx_malloc(thread_pbi->fragments.sizes[0]);
    vpx_memcpy(this_fb->cx_data_ptr, thread_pbi->fragments.ptrs[0],
               thread_pbi->fragments.sizes[0]);

    this_fb->current_cx_frame = fb->cx_data_count;
    this_fb->this_pbi = thread_pbi;

    thread_pbi->this_fb = this_fb;
    thread_pbi->fragments.ptrs[0] = this_fb->cx_data_ptr;

    /* number of compressed frames */
    fb->cx_data_count++;

    if(num_decoder_instances > 1)
    {
        sem_post(&fb->fbmt.h_event_start_decoding[i ]);

//printf("post %d sizes %d\n", this_fb->current_cx_frame, thread_pbi->fragments.sizes[0]);
        //for testing/debugging purposes....
        //sem_wait(&fb->fbmt.h_event_frame_done[i ]);
    }
    else
    {
        retcode = vp8_decode_frame(thread_pbi);

        fb->fbmt.thread_state[i] = THREAD_READY;
        thread_pbi->this_fb->is_decoded = 1;
        thread_pbi->this_fb->show = thread_pbi->common.show_frame;
        vpx_free(this_fb->cx_data_ptr);
    }



    vp8_clear_system_state();

//    printf("++ current_cx_frame %d ref_cnt %d i:%d this_fb %x\n", this_fb->current_cx_frame, this_fb->ref_cnt, i, this_fb);
    return retcode;
}
/*For ARM NEON, d8-d15 are callee-saved registers, and need to be saved by us.*/
#if HAVE_NEON
extern void vp8_push_neon(int64_t *store);
extern void vp8_pop_neon(int64_t *store);
#endif

int vp8dx_receive_compressed_data(struct frame_buffers *fb, size_t size,
                                  const uint8_t *source,
                                  int64_t time_stamp, void *user_priv)
{
#if HAVE_NEON
    int64_t dx_store_reg[8];
#endif
    VP8D_COMP *pbi = fb->pbi[0];
    VP8_COMMON *cm = &pbi->common;
    struct fbnode *this_fb;
    int retcode = -1;

    pbi->common.error.error_code = VPX_CODEC_OK;

    retcode = check_fragments_for_errors(fb, pbi);
    if(retcode <= 0)
        return retcode;

#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
    if (cm->cpu_caps & HAS_NEON)
#endif
    {
        vp8_push_neon(dx_store_reg);
    }
#endif

    /* find a free frame buffer */
    this_fb = get_free_fb(fb);
    if(!this_fb)
    {
        pbi->common.error.error_code = VPX_CODEC_ERROR;
        goto decode_exit;
    }

    if (setjmp(pbi->common.error.jmp))
    {
       /* We do not know if the missing frame(s) was supposed to update
        * any of the reference buffers, but we act conservative and
        * mark only the last buffer as corrupted.
        */
        vp8_mark_last_as_corrupted(pbi);
        remove_fb_frome_decoded(fb, this_fb);
        goto decode_exit;
    }

    /* update internal buffers, if a resolution change occurred */
    update_frame_if_resize(fb, this_fb, pbi);

    /* add into decoded list */
    add_fb_to_decoded(fb, this_fb, user_priv);

    this_fb->this_pbi = pbi;
    this_fb->current_cx_frame = fb->cx_data_count;
    this_fb->clr_type = pbi->common.clr_type;

    pbi->common.error.setjmp = 1;

    retcode = vp8_decode_frame(pbi);

    this_fb->clr_type = pbi->common.clr_type;

    if (retcode < 0)
    {
        remove_fb_frome_decoded(fb, this_fb);
        pbi->common.error.error_code = VPX_CODEC_ERROR;
        goto decode_exit;
    }

    /* number of compressed frames */
    fb->cx_data_count++;

    pbi->this_fb->is_decoded = 1;
    pbi->this_fb->show = pbi->common.show_frame;

    vp8_clear_system_state();

    if (cm->show_frame)
    {
        cm->current_video_frame++;
        cm->show_frame_mi = cm->mi;
    }

    #if CONFIG_ERROR_CONCEALMENT
    /* swap the mode infos to storage for future error concealment */
    if (pbi->ec_enabled && pbi->common.prev_mi)
    {
        MODE_INFO* tmp = pbi->common.prev_mi;
        int row, col;
        pbi->common.prev_mi = pbi->common.mi;
        pbi->common.mi = tmp;

        /* Propagate the segment_ids to the next frame */
        for (row = 0; row < pbi->common.mb_rows; ++row)
        {
            for (col = 0; col < pbi->common.mb_cols; ++col)
            {
                const int i = row*pbi->common.mode_info_stride + col;
                pbi->common.mi[i].mbmi.segment_id =
                        pbi->common.prev_mi[i].mbmi.segment_id;
            }
        }
    }
#endif

    pbi->ready_for_new_data = 0;
    pbi->last_time_stamp = time_stamp;

decode_exit:
#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
    if (cm->cpu_caps & HAS_NEON)
#endif
    {
        vp8_pop_neon(dx_store_reg);
    }
#endif

    pbi->common.error.setjmp = 0;
    return retcode;
}

int vp8dx_get_raw_frame(struct frame_buffers *fb,
                        YV12_BUFFER_CONFIG *sd,
                        int64_t *time_stamp,
                        int64_t *time_end_stamp,
                        void **user_priv, vp8_ppflags_t *flags)
{
    int ret = -1;

    ret = vp8_get_to_show(fb);
    if(!ret)
    {
        struct fbnode *to_show_fb = fb->decoded_to_show;
#if xCONFIG_POSTPROC
        VP8D_COMP *pbi = to_show_fb->this_pbi;

        /* For now, copy post proc data into common. */
        pbi->common.show_frame = 1;
        vpx_memcpy(&pbi->common.post_proc_buffer, &fb->post_proc_buffer,
                   sizeof(fb->post_proc_buffer));
        vpx_memcpy(&pbi->common.post_proc_buffer_int, &fb->post_proc_buffer_int,
                   sizeof(fb->post_proc_buffer_int));
        pbi->common.post_proc_buffer_int_used = fb->post_proc_buffer_int_used;
        pbi->common.pp_limits_buffer = fb->pp_limits_buffer;
        vpx_memcpy(&pbi->common.postproc_state, &fb->postproc_state,
                   sizeof(fb->postproc_state));

        ret = vp8_post_proc_frame(&pbi->common, sd, flags);

        /* save post proc data */
        vpx_memcpy(&fb->post_proc_buffer, &pbi->common.post_proc_buffer,
                   sizeof(fb->post_proc_buffer));
        vpx_memcpy(&fb->post_proc_buffer_int, &pbi->common.post_proc_buffer_int,
                   sizeof(fb->post_proc_buffer_int));
        pbi->common.post_proc_buffer_int_used = fb->post_proc_buffer_int_used;
        vpx_memcpy(&fb->postproc_state, &pbi->common.postproc_state,
                   sizeof(fb->postproc_state));

        vp8_clear_system_state();
#else
        *sd = to_show_fb->yv12_fb;
        sd->y_width = to_show_fb->y_width;
        sd->y_height = to_show_fb->y_height;
        sd->clrtype = to_show_fb->clr_type;
#endif
        *user_priv = (void *)to_show_fb->user_priv;
        *time_stamp = to_show_fb->last_time_stamp;
        *time_end_stamp = 0;
    }
    return ret;
}

/* This function as written isn't decoder specific, but the encoder has
 * much faster ways of computing this, so it's ok for it to live in a
 * decode specific file.
 */
int vp8dx_references_buffer( VP8_COMMON *oci, int ref_frame )
{
    const MODE_INFO *mi = oci->mi;
    int mb_row, mb_col;

    for (mb_row = 0; mb_row < oci->mb_rows; mb_row++)
    {
        for (mb_col = 0; mb_col < oci->mb_cols; mb_col++,mi++)
        {
            if( mi->mbmi.ref_frame == ref_frame)
              return 1;
        }
        mi++;
    }
    return 0;

}

int vp8_create_decoder_instances(struct frame_buffers *fb, VP8D_CONFIG *oxcf)
{
    if(!fb->use_frame_threads)
    {
        /* decoder instance for single thread mode */
        fb->pbi[0] = create_decompressor(oxcf);
        if(!fb->pbi[0])
            return VPX_CODEC_ERROR;

#if CONFIG_MULTITHREAD
        /* enable row-based threading only when use_frame_threads
         * is disabled */
        fb->pbi[0]->max_threads = oxcf->max_threads;
        vp8_decoder_create_threads(fb->pbi[0]);
#endif
    }
    else
    {
        int i;

         /* */
         if(vp8_create_decoder_frame_threads(fb, oxcf))
         {
             return -1;
         }

         /* decoder instance for each thread */
         for(i = 0; i < fb->fbmt.allocated_decoding_thread_count; i++)
         {
             fb->pbi[i] = create_decompressor(oxcf);
         }
    }

    return VPX_CODEC_OK;
}

int vp8_remove_decoder_instances(struct frame_buffers *fb)
{
    if(!fb->use_frame_threads)
    {
        VP8D_COMP *pbi = fb->pbi[0];

        if (!pbi)
            return VPX_CODEC_ERROR;
#if CONFIG_MULTITHREAD
        if (pbi->b_multithreaded_rd)
            vp8mt_de_alloc_temp_buffers(pbi, pbi->common.mb_rows);
        vp8_decoder_remove_threads(pbi);
#endif

        /* decoder instance for single thread mode */
        remove_decompressor(pbi);
    }
    else
    {
        /* TODO : remove frame threads and decoder instances for each
         * thread here */
    }

    return VPX_CODEC_OK;
}

int vp8_adjust_decoder_frames(struct frame_buffers *fb, int width, int height)
{
    int ithread;
    int num_decoder_instances;

    num_decoder_instances = fb->fbmt.allocated_decoding_thread_count;
    if(num_decoder_instances)
    {
        int i;

        /* wait for remaining cx frames to be decoded */
if(1)
        while(1)
        {
            struct fbnode *this_node = fb->decoded;
            int dec_size = fb->decoded_size;
            int all_decoded = 1;

            while(this_node && dec_size)
            {
                vp8_fbmt_mutex_lock();
                all_decoded &= this_node->is_decoded;
                vp8_fbmt_mutex_unlock();
                this_node = this_node->next;
                dec_size--;
            }

            if(all_decoded)
                break;
        }

        /* wait for the thread to complete */
if(0)
        for(i = 0; i < num_decoder_instances; i++)
        {
            int state;

            vp8_fbmt_mutex_lock();
            state = fb->fbmt.thread_state[i];
            vp8_fbmt_mutex_unlock();

            while(state != THREAD_READY)
            {
                vp8_fbmt_mutex_lock();
                state = fb->fbmt.thread_state[i];
                vp8_fbmt_mutex_unlock();

//                x86_pause_hint();
  //              thread_sleep(0);
                break;
            }
//            printf("thread %d READY\n", i);
        }
    }
    else
        num_decoder_instances = 1; /* single thread mode */

    for (ithread = 0; ithread < num_decoder_instances; ithread++)
    {
        VP8_COMMON *cm = &fb->pbi[ithread]->common;
        remove_decoder_frame(cm);
    }

    for (ithread = 0; ithread < num_decoder_instances; ithread++)
    {
        create_decoder_frame(fb->pbi[ithread], width, height);
    }

    fb->size_id += 1;
    fb->new_y_width = width;
    fb->new_y_height = height;

    return 0;
}
