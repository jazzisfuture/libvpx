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
#include "vpx_scale/vpxscale.h"
#include "vp8/common/systemdependent.h"
#include "vpx_ports/vpx_timer.h"
#include "detokenize.h"
#if CONFIG_ERROR_CONCEALMENT
#include "error_concealment.h"
#endif
#if ARCH_ARM
#include "vpx_ports/arm.h"
#endif

extern void vp8_init_loop_filter(VP8_COMMON *cm);
extern void vp8cx_init_de_quantizer(VP8D_COMP *pbi);
static int get_free_fb (VP8_COMMON *cm);
static void ref_cnt_fb (int *buf, int *idx, int new_idx);

int vp8_destroy_frame_pool(FRAME_BUFFERS *fb)
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

int vp8_create_frame_pool(FRAME_BUFFERS *fb, unsigned int max_frames,
                          int width, int height)
{
    unsigned int i;

    vp8_destroy_frame_pool(fb);

    fb->free_alloc =
    fb->free = (FBNODE *) vpx_malloc(sizeof(FBNODE) * (max_frames+1));
    vpx_memset(fb->free, 0, sizeof(FBNODE) * (max_frames+1));

    /* our internal buffers are always multiples of 16 */
    if ((width & 0xf) != 0)
        width += 16 - (width & 0xf);

    if ((height & 0xf) != 0)
        height += 16 - (height & 0xf);


    for(i = 0; i < max_frames; i++)
    {
        if (vp8_yv12_alloc_frame_buffer(&fb->free[i].yv12_fb, width, height,
                                        VP8BORDERINPIXELS) < 0)
        {
            vp8_destroy_frame_pool(fb);
            printf("error allocating frame............\n");
            return 1;
        }

        fb->free[i].next = &fb->free[i+1];
        fb->free[i].frame_id = i;

//        printf("vp8_create_frame_pool(): fb->free[%d].next %x\n", i, fb->free[i].next);
    }

    /* terminate */
    fb->free[max_frames-1].next = 0;

    /* for debug purposes... we should never use this extra node */
    fb->free[max_frames].frame_id = i;

    fb->max_allocated_frames = max_frames;
    fb->decoded = (FBNODE *)0;
    fb->decoded_head = (FBNODE *)0;
    fb->decoded_to_show = (FBNODE *)0;

    return 0;
}

/* for debug purposes */
void vp8_show_free_buffers(FRAME_BUFFERS *fb)
{
    FBNODE *this_node = fb->free;

    if(this_node)
    {
        while(this_node)
        {
            printf("this_node->frame_id %d this_node %x\n", this_node->frame_id,
                this_node);
            this_node = this_node->next;
        }
    }
    else
        printf("fb->free is NULL\n");
}

/* for debug purposes */
void vp8_show_decoded_buffers(FRAME_BUFFERS *fb)
{
    FBNODE *this_node = fb->decoded;

    if(this_node)
    {
        while(this_node)
        {
            printf("vp8_show_decoded_buffers(): frame_id %d this_node %x K:%d "
                "G:%d A:%d rL:%d\n", this_node->frame_id, this_node,
                this_node->is_key, this_node->is_gld, this_node->is_alt,
            this_node->is_refresh_last);
            this_node = this_node->next;
        }
    }
    else
        printf("fb->decoded is NULL\n");
}

static FBNODE * pop_free_fb(FRAME_BUFFERS *fb)
{
    FBNODE *free_node = fb->free;

    if(fb->free)
    {
        /* remove empty buffer from pool */
        fb->free = fb->free->next;
    }

    return free_node;
}

/* reset before adding back to free pool */
static void clear_fbnode(FBNODE *free_node)
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
    }
}

/* add buffer back to pool */
static void push_free_fb(FRAME_BUFFERS *fb, FBNODE *free_node)
{
    clear_fbnode(free_node);

    if(fb->free)
        free_node->next = fb->free;

    fb->free = free_node;
}

/* add frame buffer to decoded list */
static void add_fb_to_decoded(FRAME_BUFFERS *fb, FBNODE *fbn)
{
    if(fb->decoded)
    {
        fb->decoded_head->next = fbn;
        fbn->prev = fb->decoded_head;
        fbn->next = (FBNODE *)0;
    }
    else
    {
        fb->decoded_to_show = fbn;
        fb->decoded = fbn;
        fb->decoded->prev = (FBNODE *)0;
        fb->decoded->next = (FBNODE *)0;
    }

    fb->decoded_head = fbn;
    fb->decoded_size++;
}

static FBNODE * remove_fb_frome_decoded(FRAME_BUFFERS *fb, FBNODE * node_to_find)
{
    FBNODE * this_node = fb->decoded;
    FBNODE * decoded_fb = 0;

    while(this_node)
    {
        if(this_node == node_to_find)
        {
            decoded_fb = this_node;
            if(this_node->next)
            {
                if(this_node->prev)
                {
                    this_node->prev->next = this_node->next;
                    this_node->next->prev = this_node->prev;
                }
                else
                {
                    fb->decoded = this_node->next;
                    this_node->next->prev = (FBNODE *)0;
                }
            }
            else
            {
                if(this_node->prev)
                {
                    this_node->prev->next = (FBNODE *)0;
                    fb->decoded_head = this_node->prev;
                }
                else
                {
                    /* node found is the only one in the list */
                    fb->decoded = (FBNODE *)0;
                    fb->decoded_head = (FBNODE *)0;
                }
            }
        }
        //			printf("remove_fb_frome_decoded(): this_node->frame_id %d this_node %x\n", this_node->frame_id, this_node);
        this_node = this_node->next;
    }

    if(decoded_fb)
    {
        decoded_fb->next = (FBNODE *)0;
        decoded_fb->prev = (FBNODE *)0;
        fb->decoded_size--;
    //		printf("remove_fb_frome_decoded(): this_node->frame_id %d this_node %x\n", decoded_fb->frame_id, decoded_fb);
    }
    return decoded_fb;
}

/* single thread version */
FBNODE * vp8_get_to_show(FRAME_BUFFERS *fb)
{
    FBNODE * to_show_fb = 0;

    if(fb->decoded_size)
    {
        FBNODE * this_node;

        to_show_fb = fb->decoded_head;

        /* */
        if(!to_show_fb->show)
            return (FBNODE *)0;

        if(to_show_fb->is_key)
        {
            /* since this is a keyframe, previous frames are no longer used */
            this_node = to_show_fb->prev;
            /* search the list and remove frames */
            while(this_node)
            {
                FBNODE *n = this_node;

                /* we are about to remove this node from list, so point to the
                 * previous frame before the ptrs are adjusted */
                this_node = this_node->prev;

                n = remove_fb_frome_decoded(fb, n);
                push_free_fb(fb, n);
            }
        }
        else if(fb->decoded_size > 2)
        {

            /*
            vp8_show_decoded_buffers(fb);
            */

            this_node = to_show_fb->prev;

            while(this_node)
            {
                if(!this_node->is_alt && !this_node->is_gld)
                {
                    FBNODE *n = this_node;

                    /* we are about to remove this node from list, so point to
                     * the previous frame before the ptrs are adjusted */
                    this_node = this_node->prev;

                    n = remove_fb_frome_decoded(fb, n);
                    push_free_fb(fb, n);
                }
                else
                  this_node = this_node->prev;
            }
        }
    }

    return to_show_fb;
}

int vp8dx_get_raw_frame2(FRAME_BUFFERS *fb, VP8D_COMP *pbi,
                        YV12_BUFFER_CONFIG *sd,int64_t *time_stamp,
                        int64_t *time_end_stamp, vp8_ppflags_t *flags)
{
    int ret = -1;
    FBNODE *to_show_fb;

    if (pbi->ready_for_new_data == 1)
        return ret;


    //TODO: add post proc back

    to_show_fb = vp8_get_to_show(fb);
    if(to_show_fb)
    {
    	*sd = to_show_fb->yv12_fb;

        sd->y_width = pbi->common.Width;
        sd->y_height = pbi->common.Height;
        sd->uv_height = pbi->common.Height / 2;
        ret = 0;

        pbi->ready_for_new_data = 1;
        *time_stamp = pbi->last_time_stamp;
        *time_end_stamp = 0;

        sd->clrtype = pbi->common.clr_type;
    }

    return ret;
}

/* setup reference frames for vp8_decode_frame */
int vp8_assign_fb_ref(FRAME_BUFFERS *fb, VP8D_COMP *pbi)
{
    FBNODE * this_node = fb->decoded_head;

    if(this_node)
    {
        pbi->dec_fb_ref[ALTREF_FRAME] =
        pbi->dec_fb_ref[GOLDEN_FRAME] =
        pbi->dec_fb_ref[LAST_FRAME] =
        pbi->dec_fb_ref[INTRA_FRAME] = &this_node->yv12_fb;

        if(this_node->prev)
        {
            int found_gld, found_alt;

            /* the prev frame should be the last frame */
            this_node = this_node->prev;
            if(this_node)
            {
                if(!this_node->is_refresh_last)
                {
                    /* we were wrong... this frame is not used as the last ...
                     * skip it */
                    this_node = this_node->prev;
                    if(!this_node)
                    {
                      printf("vp8_assign_fb_ref(): error.........\n");
                      return -1;
                    }
                  }
                  pbi->dec_fb_ref[LAST_FRAME] = &this_node->yv12_fb;
              }

              /* reset node in case is_refresh_last = 0 */
              this_node = fb->decoded_head->prev;

              found_gld = found_alt = 0;
              while(this_node)
              {
                  if(this_node->is_gld && !found_gld)
                  {
                      pbi->dec_fb_ref[GOLDEN_FRAME] = &this_node->yv12_fb;
                      found_gld = 1;
                  }

                  if(this_node->is_alt && !found_alt)
                  {
                      pbi->dec_fb_ref[ALTREF_FRAME] = &this_node->yv12_fb;
                      found_alt = 1;
                  }
                  if(found_gld && found_alt)
                    break;

                  this_node = this_node->prev;
              }
        }

    //		printf("I: %x L: %x G: %x A: %x \n", pbi->dec_fb_ref[INTRA_FRAME], pbi->dec_fb_ref[LAST_FRAME], pbi->dec_fb_ref[GOLDEN_FRAME], pbi->dec_fb_ref[ALTREF_FRAME]);

      }
      else
      {
          printf("vp8_assign_fb_ref(): this should never happen!!!/n");
      }
      return 0;
}

int vp8_update_fb_ref_flags(FBNODE * this_node, VP8D_COMP *pbi)
{
    VP8_COMMON *cm = &pbi->common;
    FBNODE * temp;


     if (cm->frame_type == KEY_FRAME)
     {
         this_node->is_key = 1;
     }

    /* The alternate reference frame or golden frame can be updated
     *  using the new, last, or golden/alt ref frame.  If it
     *  is updated using the newly decoded frame it is a refresh.
     *  An update using the last or golden/alt ref frame is a copy.
     */

    if (cm->copy_buffer_to_arf)
    {
        temp = this_node->prev;
        if (cm->copy_buffer_to_arf == 1)
        {
            //new_fb = cm->lst_fb_idx;
            /* the prev frame should be the last frame */
            if(!temp->is_refresh_last)
            {
                /* we were wrong... this frame is not used as the last ... skip it */
                temp = temp->prev;
                if(!temp)
                {
                    printf("vp8_update_fb_ref_flags(): error.........\n");
                    return -1;
                }
            }

            temp->is_alt = 1;
        }
        else if (cm->copy_buffer_to_arf == 2)
        {
            //new_fb = cm->gld_fb_idx;
            while(temp)
            {
              /* clear out any frames previously marked as alt ref */
              temp->is_alt = 0;

              /* look for the latest golden frame */
              if(temp->is_gld)
              {
                  temp->is_alt = 1;
//                  printf("copy_buffer_to_arf 2 fbnode %x\n", temp);
                  break;
              }
              temp = temp->prev;
            }
        }

        /* search backwards and clear out any alt flags */
        /* NOTE: additional logic required for frame-based multithreading */
        temp = temp->prev;
        while(temp)
        {
            if(temp->is_alt)
            {
                temp->is_alt = 0;
        //    			printf("copy_buffer_to_arf: cleared is_alt flag fbnode %x\n", temp);
                break;
            }
            temp = temp->prev;
        }
    }

    if (cm->copy_buffer_to_gf)
    {
        temp = this_node->prev;
        if (cm->copy_buffer_to_gf == 1)
        {
            //new_fb = cm->lst_fb_idx;
            /* the prev frame should be the last frame */
            if(!temp->is_refresh_last)
            {
                /* we were wrong... this frame is not used as the last ... skip it */
                temp = temp->prev;
                if(!temp)
                {
                    printf("vp8_update_fb_ref_flags() 2: error.........\n");
                    return -1;
                }
            }
            temp->is_gld = 1;
        }
        else if (cm->copy_buffer_to_gf == 2)
        {
            //new_fb = cm->gld_fb_idx;
            while(temp)
            {
                /* clear out any frames previously marked as gld ref */
                temp->is_gld = 0;

                /* look for the latest golden frame */
                if(temp->is_alt)
                {
                    temp->is_gld = 1;
      //        			printf("copy_buffer_to_gld 2 fbnode %x\n", temp);
                    break;
                }
                temp = temp->prev;
            }
        }

        /* search backwards and clear out any alt flags */
        /* NOTE: additional logic required for frame-based multithreading */
        temp = temp->prev;
        while(temp)
        {
            if(temp->is_gld)
            {
                temp->is_gld = 0;
      //    			printf("copy_buffer_to_arf: cleared is_alt flag fbnode %x\n", temp);
                break;
            }
            temp = temp->prev;
        }
    }


    if (cm->refresh_golden_frame)
    {
        this_node->is_gld = 1;
        temp = this_node->prev;
        while(temp)
        {
            if(temp->is_gld)
            {
                temp->is_gld = 0;
      //    			printf("cleared is_gld flag fbnode %x\n", temp);
            }
            temp = temp->prev;
        }
    }

    if (cm->refresh_alt_ref_frame)
    {
        this_node->is_alt = 1;
        temp = this_node->prev;
        while(temp)
        {
            if(temp->is_alt)
            {
                temp->is_alt = 0;
      //    			printf("cleared is_alt flag fbnode %x\n", temp);
            }
            temp = temp->prev;
        }
    }


    this_node->show = cm->show_frame;

    this_node->is_refresh_last = cm->refresh_last_frame;

    cm->frame_to_show = &this_node->yv12_fb;

    return 0;
}

struct VP8D_COMP * vp8dx_create_decompressor(VP8D_CONFIG *oxcf)
{
    int max_ref_frames = 3;
    VP8D_COMP *pbi = vpx_memalign(32, sizeof(VP8D_COMP));

    if (!pbi)
        return NULL;

    vpx_memset(pbi, 0, sizeof(VP8D_COMP));

    if (setjmp(pbi->common.error.jmp))
    {
        pbi->common.error.setjmp = 0;
        vp8dx_remove_decompressor(pbi);
        return 0;
    }

    pbi->common.error.setjmp = 1;

    vp8_create_common(&pbi->common);

    pbi->common.current_video_frame = 0;
    pbi->ready_for_new_data = 1;

#if CONFIG_MULTITHREAD
    max_ref_frames += oxcf->max_threads;
    pbi->max_threads = oxcf->max_threads;
    vp8_decoder_create_threads(pbi);
#else
    max_ref_frames += 1; /* single thread */
#endif



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

    pbi->input_fragments = oxcf->input_fragments;
    pbi->num_fragments = 0;

    /* Independent partitions is activated when a frame updates the
     * token probability table to have equal probabilities over the
     * PREV_COEF context.
     */
    pbi->independent_partitions = 0;

    vp8_setup_block_dptrs(&pbi->mb);

    return pbi;
}

void vp8dx_remove_decoder_frame(VP8_COMMON *oci)
{
    vp8_yv12_de_alloc_frame_buffer(&oci->temp_scale_frame);
#if CONFIG_POSTPROC
    vp8_yv12_de_alloc_frame_buffer(&oci->post_proc_buffer);
    if (oci->post_proc_buffer_int_used)
        vp8_yv12_de_alloc_frame_buffer(&oci->post_proc_buffer_int);
#endif

    vpx_free(oci->above_context);
    vpx_free(oci->mip);
#if CONFIG_ERROR_CONCEALMENT
    vpx_free(oci->prev_mip);
    oci->prev_mip = NULL;
#endif

    oci->above_context = NULL;
    oci->mip = NULL;
}

int vp8dx_create_decoder_frame(VP8D_COMP *pbi, int width, int height, YV12_BUFFER_CONFIG *yv12_fb_new)
{
    VP8_COMMON *const oci = & pbi->common;
#if CONFIG_MULTITHREAD
    int prev_mb_rows = oci->mb_rows;
    int i;
#endif

    vp8dx_remove_decoder_frame(oci);

    /* our internal buffers are always multiples of 16 */
    if ((width & 0xf) != 0)
        width += 16 - (width & 0xf);

    if ((height & 0xf) != 0)
        height += 16 - (height & 0xf);

    if (vp8_yv12_alloc_frame_buffer(&oci->temp_scale_frame,   width, 16, VP8BORDERINPIXELS) < 0)
    {
        vp8dx_remove_decoder_frame(oci);
        return 1;
    }

#if CONFIG_POSTPROC
    if (vp8_yv12_alloc_frame_buffer(&oci->post_proc_buffer, width, height, VP8BORDERINPIXELS) < 0)
    {
        vp8dx_remove_decoder_frame(oci);
        return 1;
    }

    oci->post_proc_buffer_int_used = 0;
    vpx_memset(&oci->postproc_state, 0, sizeof(oci->postproc_state));
    vpx_memset((&oci->post_proc_buffer)->buffer_alloc,128,(&oci->post_proc_buffer)->frame_size);
#endif

    oci->mb_rows = height >> 4;
    oci->mb_cols = width >> 4;
    oci->MBs = oci->mb_rows * oci->mb_cols;
    oci->mode_info_stride = oci->mb_cols + 1;
    oci->mip = vpx_calloc((oci->mb_cols + 1) * (oci->mb_rows + 1), sizeof(MODE_INFO));

    if (!oci->mip)
    {
        vp8dx_remove_decoder_frame(oci);
        return 1;
    }

    oci->mi = oci->mip + oci->mode_info_stride + 1;

    oci->above_context = vpx_calloc(sizeof(ENTROPY_CONTEXT_PLANES) * oci->mb_cols, 1);

    if (!oci->above_context)
    {
        vp8dx_remove_decoder_frame(oci);
        return 1;
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
            vp8dx_remove_decoder_frame(oci);
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

    /* here we are only interested in the strides */
    vpx_memcpy(&pbi->mb.pre, yv12_fb_new, sizeof(YV12_BUFFER_CONFIG));
    vpx_memcpy(&pbi->mb.dst, yv12_fb_new, sizeof(YV12_BUFFER_CONFIG));

    //TODO: remove duplication in pre and dst

#if CONFIG_MULTITHREAD
    for (i = 0; i < pbi->allocated_decoding_thread_count; i++)
    {
        //pbi->mb_row_di[i].mbd.dst = oci->yv12_fb[0];
        vpx_memcpy(&pbi->mb_row_di[i].mbd.dst, yv12_fb_new, sizeof(YV12_BUFFER_CONFIG));
        vp8_build_block_doffsets(&pbi->mb_row_di[i].mbd);
    }
#endif

    vp8_build_block_doffsets(&pbi->mb);

    return 0;
}

void vp8dx_remove_decompressor(VP8D_COMP *pbi)
{
    if (!pbi)
        return;

#if CONFIG_MULTITHREAD
    if (pbi->b_multithreaded_rd)
        vp8mt_de_alloc_temp_buffers(pbi, pbi->common.mb_rows);
    vp8_decoder_remove_threads(pbi);
#endif
#if CONFIG_ERROR_CONCEALMENT
    vp8_de_alloc_overlap_lists(pbi);
#endif
    vp8_remove_common(&pbi->common);
    vpx_free(pbi->mbc);
    vpx_free(pbi);
}


vpx_codec_err_t vp8dx_get_reference(VP8D_COMP *pbi, VP8_REFFRAME ref_frame_flag, YV12_BUFFER_CONFIG *sd)
{
    VP8_COMMON *cm = &pbi->common;
    int ref_fb_idx;

    /*
     * TODO: FIX ME ... invalid reference frames
     * */

    if (ref_frame_flag == VP8_LAST_FLAG)
        ref_fb_idx = cm->lst_fb_idx;
    else if (ref_frame_flag == VP8_GOLD_FLAG)
        ref_fb_idx = cm->gld_fb_idx;
    else if (ref_frame_flag == VP8_ALT_FLAG)
        ref_fb_idx = cm->alt_fb_idx;
    else{
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Invalid reference frame");
        return pbi->common.error.error_code;
    }

    if(cm->yv12_fb[ref_fb_idx].y_height != sd->y_height ||
        cm->yv12_fb[ref_fb_idx].y_width != sd->y_width ||
        cm->yv12_fb[ref_fb_idx].uv_height != sd->uv_height ||
        cm->yv12_fb[ref_fb_idx].uv_width != sd->uv_width){
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Incorrect buffer dimensions");
    }
    else
        vp8_yv12_copy_frame(&cm->yv12_fb[ref_fb_idx], sd);

    return pbi->common.error.error_code;
}


vpx_codec_err_t vp8dx_set_reference(VP8D_COMP *pbi, VP8_REFFRAME ref_frame_flag, YV12_BUFFER_CONFIG *sd)
{
    VP8_COMMON *cm = &pbi->common;
    int *ref_fb_ptr = NULL;
    int free_fb;
    /*
     * TODO: FIX ME ... invalid reference frames
     * */
    if (ref_frame_flag == VP8_LAST_FLAG)
        ref_fb_ptr = &cm->lst_fb_idx;
    else if (ref_frame_flag == VP8_GOLD_FLAG)
        ref_fb_ptr = &cm->gld_fb_idx;
    else if (ref_frame_flag == VP8_ALT_FLAG)
        ref_fb_ptr = &cm->alt_fb_idx;
    else{
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Invalid reference frame");
        return pbi->common.error.error_code;
    }

    if(cm->yv12_fb[*ref_fb_ptr].y_height != sd->y_height ||
        cm->yv12_fb[*ref_fb_ptr].y_width != sd->y_width ||
        cm->yv12_fb[*ref_fb_ptr].uv_height != sd->uv_height ||
        cm->yv12_fb[*ref_fb_ptr].uv_width != sd->uv_width){
        vpx_internal_error(&pbi->common.error, VPX_CODEC_ERROR,
            "Incorrect buffer dimensions");
    }
    else{
        /* Find an empty frame buffer. */
        free_fb = get_free_fb(cm);
        /* Decrease fb_idx_ref_cnt since it will be increased again in
         * ref_cnt_fb() below. */
        cm->fb_idx_ref_cnt[free_fb]--;

        /* Manage the reference counters and copy image. */
        ref_cnt_fb (cm->fb_idx_ref_cnt, ref_fb_ptr, free_fb);
        vp8_yv12_copy_frame(sd, &cm->yv12_fb[*ref_fb_ptr]);
    }

   return pbi->common.error.error_code;
}

/*For ARM NEON, d8-d15 are callee-saved registers, and need to be saved by us.*/
#if HAVE_NEON
extern void vp8_push_neon(int64_t *store);
extern void vp8_pop_neon(int64_t *store);
#endif

static int get_free_fb (VP8_COMMON *cm)
{
    int i;
    for (i = 0; i < NUM_YV12_BUFFERS; i++)
        if (cm->fb_idx_ref_cnt[i] == 0)
            break;

    assert(i < NUM_YV12_BUFFERS);
    cm->fb_idx_ref_cnt[i] = 1;
    return i;
}

static void ref_cnt_fb (int *buf, int *idx, int new_idx)
{
    if (buf[*idx] > 0)
        buf[*idx]--;

    *idx = new_idx;

    buf[new_idx]++;
}

/* If any buffer copy / swapping is signalled it should be done here. */
static int swap_frame_buffers (VP8_COMMON *cm)
{
    int err = 0;

    /* The alternate reference frame or golden frame can be updated
     *  using the new, last, or golden/alt ref frame.  If it
     *  is updated using the newly decoded frame it is a refresh.
     *  An update using the last or golden/alt ref frame is a copy.
     */
    if (cm->copy_buffer_to_arf)
    {
        int new_fb = 0;

        if (cm->copy_buffer_to_arf == 1)
            new_fb = cm->lst_fb_idx;
        else if (cm->copy_buffer_to_arf == 2)
            new_fb = cm->gld_fb_idx;
        else
            err = -1;

        ref_cnt_fb (cm->fb_idx_ref_cnt, &cm->alt_fb_idx, new_fb);
    }

    if (cm->copy_buffer_to_gf)
    {
        int new_fb = 0;

        if (cm->copy_buffer_to_gf == 1)
            new_fb = cm->lst_fb_idx;
        else if (cm->copy_buffer_to_gf == 2)
            new_fb = cm->alt_fb_idx;
        else
            err = -1;

        ref_cnt_fb (cm->fb_idx_ref_cnt, &cm->gld_fb_idx, new_fb);
    }

    if (cm->refresh_golden_frame)
        ref_cnt_fb (cm->fb_idx_ref_cnt, &cm->gld_fb_idx, cm->new_fb_idx);

    if (cm->refresh_alt_ref_frame)
        ref_cnt_fb (cm->fb_idx_ref_cnt, &cm->alt_fb_idx, cm->new_fb_idx);

    if (cm->refresh_last_frame)
    {
        ref_cnt_fb (cm->fb_idx_ref_cnt, &cm->lst_fb_idx, cm->new_fb_idx);

        cm->frame_to_show = &cm->yv12_fb[cm->lst_fb_idx];
    }
    else
        cm->frame_to_show = &cm->yv12_fb[cm->new_fb_idx];

    cm->fb_idx_ref_cnt[cm->new_fb_idx]--;

    return err;
}

int vp8dx_receive_compressed_data2(FRAME_BUFFERS *fb, VP8D_COMP *pbi, unsigned long size,
		const unsigned char *source, int64_t time_stamp)
{
    VP8_COMMON *cm = &pbi->common;
    int retcode = 0;
    FBNODE *this_fb;

    pbi->common.error.error_code = VPX_CODEC_OK;

    //TODO: fix fragments
    if (pbi->num_fragments == 0)
    {
        /* New frame, reset fragment pointers and sizes */
        vpx_memset((void*)pbi->fragments, 0, sizeof(pbi->fragments));
        vpx_memset(pbi->fragment_sizes, 0, sizeof(pbi->fragment_sizes));
    }

    if (!pbi->input_fragments)
    {
        pbi->fragments[0] = source;
        pbi->fragment_sizes[0] = size;
        pbi->num_fragments = 1;
    }

    /* find a free buffer */
    this_fb = pop_free_fb(fb);
    if(!this_fb)
    {
        pbi->common.error.error_code = VPX_CODEC_ERROR;
        pbi->common.error.setjmp = 0;
        pbi->num_fragments = 0;
        return -1;
    }
    /* add into decoded list */
    add_fb_to_decoded(fb, this_fb);
    vp8_assign_fb_ref(fb, pbi);

    pbi->common.error.setjmp = 1;

    retcode = vp8_decode_frame(pbi);

    if (retcode < 0)
    {
        pbi->common.error.error_code = VPX_CODEC_ERROR;
        pbi->common.error.setjmp = 0;
        pbi->num_fragments = 0;
        remove_fb_frome_decoded(fb, this_fb);
        return retcode;
    }

    {
        vp8_update_fb_ref_flags(this_fb, pbi);

        if(cm->filter_level)
        {
            /* Apply the loop filter if appropriate. */
            vp8_loop_filter_frame(cm, &pbi->mb);
        }
        vp8_yv12_extend_frame_borders(cm->frame_to_show);
    }

    vp8_clear_system_state();

    /*vp8_print_modes_and_motion_vectors( cm->mi, cm->mb_rows,cm->mb_cols, cm->current_video_frame);*/

    if (cm->show_frame)
        cm->current_video_frame++;

    pbi->ready_for_new_data = 0;
    pbi->last_time_stamp = time_stamp;
    pbi->num_fragments = 0;
    pbi->common.error.setjmp = 0;

    return retcode;
}

int vp8dx_receive_compressed_data(VP8D_COMP *pbi, unsigned long size, const unsigned char *source, int64_t time_stamp)
{
#if HAVE_NEON
    int64_t dx_store_reg[8];
#endif
    VP8_COMMON *cm = &pbi->common;
    int retcode = 0;

    pbi->common.error.error_code = VPX_CODEC_OK;

    if (pbi->num_fragments == 0)
    {
        /* New frame, reset fragment pointers and sizes */
        vpx_memset((void*)pbi->fragments, 0, sizeof(pbi->fragments));
        vpx_memset(pbi->fragment_sizes, 0, sizeof(pbi->fragment_sizes));
    }
    if (pbi->input_fragments && !(source == NULL && size == 0))
    {
        /* Store a pointer to this fragment and return. We haven't
         * received the complete frame yet, so we will wait with decoding.
         */
        assert(pbi->num_fragments < MAX_PARTITIONS);
        pbi->fragments[pbi->num_fragments] = source;
        pbi->fragment_sizes[pbi->num_fragments] = size;
        pbi->num_fragments++;
        if (pbi->num_fragments > (1 << EIGHT_PARTITION) + 1)
        {
            pbi->common.error.error_code = VPX_CODEC_UNSUP_BITSTREAM;
            pbi->common.error.setjmp = 0;
            pbi->num_fragments = 0;
            return -1;
        }
        return 0;
    }

    if (!pbi->input_fragments)
    {
        pbi->fragments[0] = source;
        pbi->fragment_sizes[0] = size;
        pbi->num_fragments = 1;
    }
    assert(pbi->common.multi_token_partition <= EIGHT_PARTITION);
    if (pbi->num_fragments == 0)
    {
        pbi->num_fragments = 1;
        pbi->fragments[0] = NULL;
        pbi->fragment_sizes[0] = 0;
    }

    if (!pbi->ec_active &&
        pbi->num_fragments <= 1 && pbi->fragment_sizes[0] == 0)
    {
        /* If error concealment is disabled we won't signal missing frames
         * to the decoder.
         */
        if (cm->fb_idx_ref_cnt[cm->lst_fb_idx] > 1)
        {
            /* The last reference shares buffer with another reference
             * buffer. Move it to its own buffer before setting it as
             * corrupt, otherwise we will make multiple buffers corrupt.
             */
            const int prev_idx = cm->lst_fb_idx;
            cm->fb_idx_ref_cnt[prev_idx]--;
            cm->lst_fb_idx = get_free_fb(cm);
            vp8_yv12_copy_frame(&cm->yv12_fb[prev_idx],
                                    &cm->yv12_fb[cm->lst_fb_idx]);
        }
        /* This is used to signal that we are missing frames.
         * We do not know if the missing frame(s) was supposed to update
         * any of the reference buffers, but we act conservative and
         * mark only the last buffer as corrupted.
         */
        cm->yv12_fb[cm->lst_fb_idx].corrupted = 1;

        /* Signal that we have no frame to show. */
        cm->show_frame = 0;

        pbi->num_fragments = 0;

        /* Nothing more to do. */
        return 0;
    }

#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
    if (cm->cpu_caps & HAS_NEON)
#endif
    {
        vp8_push_neon(dx_store_reg);
    }
#endif

    cm->new_fb_idx = get_free_fb (cm);

    /* setup reference frames for vp8_decode_frame */
    pbi->dec_fb_ref[INTRA_FRAME]  = &cm->yv12_fb[cm->new_fb_idx];
    pbi->dec_fb_ref[LAST_FRAME]   = &cm->yv12_fb[cm->lst_fb_idx];
    pbi->dec_fb_ref[GOLDEN_FRAME] = &cm->yv12_fb[cm->gld_fb_idx];
    pbi->dec_fb_ref[ALTREF_FRAME] = &cm->yv12_fb[cm->alt_fb_idx];


    if (setjmp(pbi->common.error.jmp))
    {
#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
        if (cm->cpu_caps & HAS_NEON)
#endif
        {
            vp8_pop_neon(dx_store_reg);
        }
#endif
        pbi->common.error.setjmp = 0;

        pbi->num_fragments = 0;

       /* We do not know if the missing frame(s) was supposed to update
        * any of the reference buffers, but we act conservative and
        * mark only the last buffer as corrupted.
        */
        cm->yv12_fb[cm->lst_fb_idx].corrupted = 1;

        if (cm->fb_idx_ref_cnt[cm->new_fb_idx] > 0)
          cm->fb_idx_ref_cnt[cm->new_fb_idx]--;
        return -1;
    }

    pbi->common.error.setjmp = 1;

    retcode = vp8_decode_frame(pbi);

    if (retcode < 0)
    {
#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
        if (cm->cpu_caps & HAS_NEON)
#endif
        {
            vp8_pop_neon(dx_store_reg);
        }
#endif
        pbi->common.error.error_code = VPX_CODEC_ERROR;
        pbi->common.error.setjmp = 0;
        pbi->num_fragments = 0;
        if (cm->fb_idx_ref_cnt[cm->new_fb_idx] > 0)
          cm->fb_idx_ref_cnt[cm->new_fb_idx]--;
        return retcode;
    }

#if CONFIG_MULTITHREAD
    if (pbi->b_multithreaded_rd && cm->multi_token_partition != ONE_PARTITION)
    {
        if (swap_frame_buffers (cm))
        {
#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
            if (cm->cpu_caps & HAS_NEON)
#endif
            {
                vp8_pop_neon(dx_store_reg);
            }
#endif
            pbi->common.error.error_code = VPX_CODEC_ERROR;
            pbi->common.error.setjmp = 0;
            pbi->num_fragments = 0;
            return -1;
        }
    } else
#endif
    {
        if (swap_frame_buffers (cm))
        {
#if HAVE_NEON
#if CONFIG_RUNTIME_CPU_DETECT
            if (cm->cpu_caps & HAS_NEON)
#endif
            {
                vp8_pop_neon(dx_store_reg);
            }
#endif
            pbi->common.error.error_code = VPX_CODEC_ERROR;
            pbi->common.error.setjmp = 0;
            pbi->num_fragments = 0;
            return -1;
        }

        if(cm->filter_level)
        {
            /* Apply the loop filter if appropriate. */
            vp8_loop_filter_frame(cm, &pbi->mb);
        }
        vp8_yv12_extend_frame_borders(cm->frame_to_show);
    }


    vp8_clear_system_state();

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

    /*vp8_print_modes_and_motion_vectors( cm->mi, cm->mb_rows,cm->mb_cols, cm->current_video_frame);*/

    if (cm->show_frame)
        cm->current_video_frame++;

    pbi->ready_for_new_data = 0;
    pbi->last_time_stamp = time_stamp;
    pbi->num_fragments = 0;

#if 0
    {
        int i;
        int64_t earliest_time = pbi->dr[0].time_stamp;
        int64_t latest_time = pbi->dr[0].time_stamp;
        int64_t time_diff = 0;
        int bytes = 0;

        pbi->dr[pbi->common.current_video_frame&0xf].size = pbi->bc.pos + pbi->bc2.pos + 4;;
        pbi->dr[pbi->common.current_video_frame&0xf].time_stamp = time_stamp;

        for (i = 0; i < 16; i++)
        {

            bytes += pbi->dr[i].size;

            if (pbi->dr[i].time_stamp < earliest_time)
                earliest_time = pbi->dr[i].time_stamp;

            if (pbi->dr[i].time_stamp > latest_time)
                latest_time = pbi->dr[i].time_stamp;
        }

        time_diff = latest_time - earliest_time;

        if (time_diff > 0)
        {
            pbi->common.bitrate = 80000.00 * bytes / time_diff  ;
            pbi->common.framerate = 160000000.00 / time_diff ;
        }

    }
#endif

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
int vp8dx_get_raw_frame(VP8D_COMP *pbi, YV12_BUFFER_CONFIG *sd, int64_t *time_stamp, int64_t *time_end_stamp, vp8_ppflags_t *flags)
{
    int ret = -1;

    if (pbi->ready_for_new_data == 1)
        return ret;

    /* ie no raw frame to show!!! */
    if (pbi->common.show_frame == 0)
        return ret;

    pbi->ready_for_new_data = 1;
    *time_stamp = pbi->last_time_stamp;
    *time_end_stamp = 0;

    sd->clrtype = pbi->common.clr_type;
#if CONFIG_POSTPROC
    ret = vp8_post_proc_frame(&pbi->common, sd, flags);
#else

    if (pbi->common.frame_to_show)
    {
        *sd = *pbi->common.frame_to_show;
        sd->y_width = pbi->common.Width;
        sd->y_height = pbi->common.Height;
        sd->uv_height = pbi->common.Height / 2;
        ret = 0;
    }
    else
    {
        ret = -1;
    }

#endif /*!CONFIG_POSTPROC*/
    vp8_clear_system_state();
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
