/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 *  Detokenization of each 16-coefficient block is done using a
 *  finite state machine.
 *
 *  Each node of the machine specifies:
 *   * the index in the coefficient probabilities array where
 *       the decoding probability can be found;
 *   * the next state depending on the value of the decoded bit;
 *   * what to add to the decoded token value if the bit is 1;
 *   * if the token value needs to be negated if the bit is 1;
 *   * whether or not to move to the next destination address,
 *       and by how much.
 *
 *  The approach reduces the number of branch mispredictions at
 *  the expense of use of the data cache.
 */
#include "vp8/common/type_aliases.h"
#include "vp8/common/blockd.h"
#include "onyxd_int.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "detokenize.h"

#ifdef _MSC_VER
#define __builtin_expect(x, y) x
#endif

#define BOOL_DATA UINT8

#define OCB_X PREV_COEF_CONTEXTS * ENTROPY_NODES

/* This switches between the faster 8-byte and the slower, but more cache
   friendly, 6-byte version of the FSM node. The latter could be faster
   on some platforms. */
#if 1
typedef struct
{
    /* Index in the probabilities array to use for decoding of the bit. */
    INT16 prob;
    /* Value to add if the decoded bit is 1. */
    INT16 add_one;

    /* Value by which the destination pointer needs to be adjusted (applied
       before processing the bit). */
    INT8 dest_inc;
    /* -1 if this is the sign bit. The value is xor'ed with this if the
       decoded bit is 1, and it is also used to clear the token value before
       decoding the next token. */
    INT8 sign;

    /* Offset to add to the state index in the FSM if the bit is 1. */
    INT8 on_one;
    /* Offset to add to the state index in the FSM if the bit is 0. */
    INT8 on_zero;
} DETOKENIZE_FSM;
#define NODE(prob, add_one, dest_inc, sign, on_one, on_zero) \
    { prob, add_one, dest_inc, sign, on_one, on_zero }

#define GET_PROB(ptr) (ptr->prob)
#define GET_DEST_INC(ptr) (ptr->dest_inc)
#define GET_ADD_ONE(ptr) (ptr->add_one)
#define GET_SIGN(ptr) (ptr->sign)
#define GET_ON_ONE(ptr) (ptr->on_one)
#define GET_ON_ZERO(ptr) (ptr->on_zero)
#define GET_JUMP(ptr, val) ((&ptr->on_zero)[val])
#else
typedef struct
{
    /* at least 5 bytes, using 6 to avoid difficult reconstruction */
    INT16 prob_dest;     /* u12 prob + s4 dest_inc */
    INT16 add_one_sign;  /* u11 add_one + u4 reserved + s1 sign */

    INT8 on_one;    /* s7 bits */
    INT8 on_zero;   /* s7 bits */
} DETOKENIZE_FSM;
#define NODE(prob, add_one, dest_inc, sign, on_one, on_zero) \
    { prob + ((dest_inc)<<12), add_one + ((sign)<<15), on_one, on_zero }

#define GET_PROB(ptr) (ptr->prob_dest & 0xfff)
#define GET_DEST_INC(ptr) (ptr->prob_dest >> 12)
#define GET_ADD_ONE(ptr) (ptr->add_one_sign & 0x7fff)
#define GET_SIGN(ptr) (ptr->add_one_sign >> 15)
#define GET_ON_ONE(ptr) (ptr->on_one)
#define GET_ON_ZERO(ptr) (ptr->on_zero)
#define GET_JUMP(ptr, val) ((&ptr->on_zero)[val])
#endif

#define TREE_NODE(pos_idx, ctx_val, offset, on_zero, on_one, add_one) \
    NODE(pos_idx * OCB_X + ctx_val * ENTROPY_NODES + offset, add_one, 0, 0, on_one, on_zero)
#define EOB_NODE(c, pos_idx, ctx_val, dest_inc) \
    NODE(pos_idx * OCB_X + ctx_val * ENTROPY_NODES, 0, dest_inc, 0, 1, -c)
#define ZERO_NODE(pos_idx, ctx_val, on_zero, on_one, dest_inc) \
    NODE(pos_idx * OCB_X + ctx_val * ENTROPY_NODES + 1, 1, dest_inc, 0, on_one, on_zero)
#define LIT_NODE(offset, next, add) \
    NODE(8 * OCB_X + offset, add, 0, 0, next, next)
#define SIGN_NODE(next, dest_inc) \
    NODE(8 * OCB_X + 26, 1, 0, -1, next, next)

DECLARE_ALIGNED(16, static DETOKENIZE_FSM, detokenize_fsm[]) =
{
#define TREE(c, pos_idx, ctx_val, next_0, dest_inc) \
    EOB_NODE (c, pos_idx, ctx_val, dest_inc), \
    ZERO_NODE(pos_idx, ctx_val, ctx_val*11 + next_0, 1, 0), \
    TREE_NODE(pos_idx, ctx_val,  2, ctx_val*11 + 36, 1, 1), \
    TREE_NODE(pos_idx, ctx_val,  3, 1, 3, 3), \
    TREE_NODE(pos_idx, ctx_val,  4, ctx_val*11 + 33, 1, 1), \
    TREE_NODE(pos_idx, ctx_val,  5, ctx_val*11 + 32, ctx_val*11 + 32, 1), \
    TREE_NODE(pos_idx, ctx_val,  6, 1, 2, 6), \
    TREE_NODE(pos_idx, ctx_val,  7, ctx_val*11 + 4, ctx_val*11 + 5, 2), \
    TREE_NODE(pos_idx, ctx_val,  8, 1, 2, 24), \
    TREE_NODE(pos_idx, ctx_val,  9, ctx_val*11 + 5, ctx_val*11 + 8, 8), \
    TREE_NODE(pos_idx, ctx_val, 10, ctx_val*11 + 11, ctx_val*11 + 16, 32)

#define PROCESS_TOKEN(c, pos_idx, next_pos, next_0, next_1, next_2, dest_inc) \
    ZERO_NODE(pos_idx, 0, 62, 2*11+3, dest_inc), \
    TREE(c, pos_idx, 2, next_0, dest_inc), \
    TREE(c, pos_idx, 1, next_0, dest_inc), \
    TREE(c, pos_idx, 0, next_0, dest_inc), \
    LIT_NODE(0, 26, 1), \
    LIT_NODE(1, 1, 2), \
    LIT_NODE(2, 24, 1), \
    LIT_NODE(3, 1, 4), \
    LIT_NODE(4, 1, 2), \
    LIT_NODE(5, 21, 1), \
    LIT_NODE(6, 1, 8), \
    LIT_NODE(7, 1, 4), \
    LIT_NODE(8, 1, 2), \
    LIT_NODE(9, 17, 1), \
    LIT_NODE(10, 1, 16), \
    LIT_NODE(11, 1, 8), \
    LIT_NODE(12, 1, 4), \
    LIT_NODE(13, 1, 2), \
    LIT_NODE(14, 12, 1), \
    LIT_NODE(15, 1, 1024), \
    LIT_NODE(16, 1, 512), \
    LIT_NODE(17, 1, 256), \
    LIT_NODE(18, 1, 128), \
    LIT_NODE(19, 1, 64), \
    LIT_NODE(20, 1, 32), \
    LIT_NODE(21, 1, 16), \
    LIT_NODE(22, 1, 8), \
    LIT_NODE(23, 1, 4), \
    LIT_NODE(24, 1, 2), \
    LIT_NODE(25, 1, 1), \
    SIGN_NODE(next_2, dest_inc), \
    SIGN_NODE(next_1, dest_inc)


    PROCESS_TOKEN(0, 0, 1, 38, 2 + 1*11, 3, 0),
    PROCESS_TOKEN(1, 1, 2, 38, 2 + 1*11, 3, 1),
    PROCESS_TOKEN(2, 2, 3, 38, 2 + 1*11, 3, 3),
    PROCESS_TOKEN(3, 3, 6, 38, 2 + 1*11, 3, 4),
    PROCESS_TOKEN(4, 6, 4, 38, 2 + 1*11, 3, -3),
    PROCESS_TOKEN(5, 4, 5, 38, 2 + 1*11, 3, -3),
    PROCESS_TOKEN(6, 5, 6, 38, 2 + 1*11, 3, 1),
    PROCESS_TOKEN(7, 6, 6, 38, 2 + 1*11, 3, 3),
    PROCESS_TOKEN(8, 6, 6, 38, 2 + 1*11, 3, 3),
    PROCESS_TOKEN(9, 6, 6, 38, 2 + 1*11, 3, 3),
    PROCESS_TOKEN(10, 6, 6, 38, 2 + 1*11, 3, 1),
    PROCESS_TOKEN(11, 6, 6, 38, 2 + 1*11, 3, -3),
    PROCESS_TOKEN(12, 6, 6, 38, 2 + 1*11, 3, -3),
    PROCESS_TOKEN(13, 6, 6, 38, 2 + 1*11, 3, 4),
    PROCESS_TOKEN(14, 6, 7, 38, 2 + 1*11, 3, 3),
    PROCESS_TOKEN(15, 7, 7, -16, -16, -16, 1),
};

#define TOKEN_SIZE 62

void vp8_reset_mb_tokens_context(MACROBLOCKD *x)
{
    /* Clear entropy contexts for Y2 blocks */
    if (x->mode_info_context->mbmi.mode != B_PRED &&
        x->mode_info_context->mbmi.mode != SPLITMV)
    {
        vpx_memset(x->above_context, 0, sizeof(ENTROPY_CONTEXT_PLANES));
        vpx_memset(x->left_context, 0, sizeof(ENTROPY_CONTEXT_PLANES));
    }
    else
    {
        vpx_memset(x->above_context, 0, sizeof(ENTROPY_CONTEXT_PLANES) - 1);
        vpx_memset(x->left_context, 0, sizeof(ENTROPY_CONTEXT_PLANES) - 1);
    }
}

DECLARE_ALIGNED(16, extern const unsigned char, vp8_norm[256]);

#define NORMALIZE \
    { \
        unsigned char shift = vp8_norm[range]; \
        range <<= shift; \
        value <<= shift; \
        count -= shift; \
    }

/* Main operation of the FSM loop. There are three versions of it because
   the best implementation changes with processors and compilers. */

/* Using an explicit if. */
#define PROCESS_BIT1(token) \
    if (value >= bigsplit) \
    { \
        fsm_add = GET_ON_ONE(state); \
        token = (token ^ GET_SIGN(state)) + GET_ADD_ONE(state); \
        value -= bigsplit; \
        range = range-split; \
    } else { \
        fsm_add = GET_ON_ZERO(state); \
        range = split; \
    }

/* Letting the compiler choose how to create a mask. */
#define PROCESS_BIT2(token) \
    { \
        VP8_BD_VALUE mask = (value >= bigsplit) ? -1 : 0; \
        fsm_add = GET_JUMP(state, mask); \
        range = split + ((range - 2*split) & mask); \
        value -= bigsplit & mask; \
        token = (token ^ (GET_SIGN(state) & mask)) + (GET_ADD_ONE(state) & mask); \
    }

/* Creating a mask using arithmetic shifts. */
#define PROCESS_BIT3(token) \
    { \
        VP8_BD_VALUE mask = ((VP8_BD_VALUE_SIGNED)(split - 1 - (value>>(VP8_BD_VALUE_SIZE - 8)))) >> 8;\
        fsm_add = GET_JUMP(state, mask); \
        range = split + ((range - 2*split) & mask); \
        value -= bigsplit & mask; \
        token = (token ^ (GET_SIGN(state) & mask)) + (GET_ADD_ONE(state) & mask); \
    }

/* Switch controls if the token value is prepared in a register (avoiding
   reads from cache, but still writing on every node). Because of the
   freed register, 32-bit x86 should work better with direct writes. */
#if 1
#define DECLARE_TOKEN INT16 token = 0; INT16 sign = 0;
#define STORE_TOKEN *dest = token;
#define CLEAR_TOKEN_ON_LAST_SIGN \
    token &= ~sign; \
    sign = GET_SIGN(state);
#define PROCESS_BIT PROCESS_BIT3(token)
#else
#define DECLARE_TOKEN
#define STORE_TOKEN
#define CLEAR_TOKEN_ON_LAST_SIGN
#define PROCESS_BIT PROCESS_BIT3(*dest)
#endif

/* Fill bits from the buffer. */

/* Standard branchy version that only accesses bufptr and bufend if
   filling needs to be done. */
#define FILL_BITS1(bufptr, bufend) \
    if(__builtin_expect(count < 0, 0)) \
        VP8DX_BOOL_DECODER_FILL(count, value, bufptr, bufend);

/* Branchless fill, compiler generates mask. */
#define FILL_BITS2(bufptr, bufend) \
    if (__builtin_expect(bufptr < bufend, 1)) { \
        int mask = (count >= 8) ? 0 : -1; \
        count -= 8*mask; \
        bufptr -= mask; \
        value |= (VP8_BD_VALUE)(bufptr[-1]) << (VP8_BD_VALUE_SIZE - 8 - count); \
    }

/* Branchless fill, mask generated with shifts. */
#define FILL_BITS3(bufptr, bufend) \
    if (__builtin_expect(bufptr < bufend, 1)) { \
        int mask = (count-8) >> 31; \
        count -= 8*mask; \
        bufptr -= mask; \
        value |= (VP8_BD_VALUE)(bufptr[-1]) << (VP8_BD_VALUE_SIZE - 8 - count); \
    }

#define CALC_SPLITS(prob) \
    split = 1 +  ((( coef_probs[prob]*(range-1) ) )>> 8); \
    bigsplit = (VP8_BD_VALUE)split << (VP8_BD_VALUE_SIZE - 8);

/* Switch controls whether or not to declare and use shadows of the
   buffer pointers. Branchless fills do not work well without them. */
#if 0
#define DECLARE_BUFPTR \
    const BOOL_DATA *bufend  = bc->user_buffer_end; \
    const BOOL_DATA *bufptr  = bc->user_buffer;

#define APPLY_BUFPTR \
    bc->user_buffer = bufptr;

#define FILL_BITS(bufptr, bufend) FILL_BITS1(bufptr, bufend)
#else
#define DECLARE_BUFPTR
#define APPLY_BUFPTR
#define FILL_BITS(bufptr, bufend) FILL_BITS1(bc->user_buffer, bc->user_buffer_end)
#endif

static int vp8_apply_detok_fsm(INT16* dest,
                               DETOKENIZE_FSM *state, const vp8_prob *coef_probs,
                               BOOL_DECODER *bc, VP8_BD_VALUE split)
{
    VP8_BD_VALUE bigsplit;
    ptrdiff_t fsm_add;

    DECLARE_BUFPTR
    VP8_BD_VALUE value   = bc->value - (split << (VP8_BD_VALUE_SIZE - 8));
    int count   = bc->count;
    VP8_BD_VALUE range   = bc->range - split;

    /* The first bit processed is a zero node. It won't have sign or dest_inc. */
    DECLARE_TOKEN

    NORMALIZE
    CALC_SPLITS(GET_PROB(state))
    FILL_BITS(bufptr, bufend)
    PROCESS_BIT

    do
    {
        NORMALIZE

        state += fsm_add;
        STORE_TOKEN
        CLEAR_TOKEN_ON_LAST_SIGN

        dest += GET_DEST_INC(state);

        CALC_SPLITS(GET_PROB(state))
        FILL_BITS(bufptr, bufend)
        PROCESS_BIT
    }
    while (fsm_add > 0);

    /* finish iteration */
    STORE_TOKEN

    NORMALIZE
    FILL_BITS(bufptr, bufend)

    APPLY_BUFPTR
    bc->value = value;
    bc->count = count;
    bc->range = (unsigned int) range;
    return (int) - fsm_add;
}

/**
 * Decodes a block, starting with a quick check if the initial bit is an EOB.
 * If it is, which happens quite often, the detokenization function call is
 * skipped, improving performance significantly.
 *
 * A macro to ensure the code is inlined.
 */
#define DETOKENIZE(nz, i, aofs, lofs) \
    { \
        size_t ctx_ofs = (a[aofs] + l[lofs])*ENTROPY_NODES; \
        unsigned split = 1 + (((bc->range - 1) * coef_probs[ctx_ofs + nz * OCB_X]) >> 8); \
        VP8_BD_VALUE bigsplit = (VP8_BD_VALUE)split << (VP8_BD_VALUE_SIZE - 8); \
        \
        if (bc->value < bigsplit) { \
            unsigned char shift = vp8_norm[split]; \
            a[aofs] = l[lofs] = 0; \
            eobs[i] = nz; \
            eobtotal += nz; \
            bc->range = split << shift; \
            bc->value <<= shift; \
            bc->count -= shift; \
            \
            if (__builtin_expect(bc->count < 0, 0)) \
                vp8dx_bool_decoder_fill(bc); \
        } else { \
            a[aofs] = l[lofs] = 1; \
            eobtotal += eobs[i] = vp8_apply_detok_fsm(qcoeff_ptr + i*16 + nz, \
                                  detokenize_fsm + nz*TOKEN_SIZE + (2*ENTROPY_NODES - ctx_ofs) + 2, \
                                  coef_probs, bc, split); \
        } \
    }

int vp8_decode_mb_tokens(VP8D_COMP *dx, MACROBLOCKD *x)
{
    const VP8_COMMON *const oc = & dx->common;
    char *eobs = x->eobs;

    int eobtotal = 0;

    INT16 *qcoeff_ptr;
    const vp8_prob *coef_probs;

    ENTROPY_CONTEXT *a = ((ENTROPY_CONTEXT *)x->above_context);
    ENTROPY_CONTEXT *l = ((ENTROPY_CONTEXT *)x->left_context);

    BOOL_DECODER *bc = x->current_bc;

    qcoeff_ptr = &x->qcoeff[0];

    if (x->mode_info_context->mbmi.mode != B_PRED && x->mode_info_context->mbmi.mode != SPLITMV)
    {
        coef_probs = oc->fc.coef_probs [1] [ 0 ] [0];
        DETOKENIZE(0, 24, 8, 8);
        eobtotal -= 16;

        coef_probs = oc->fc.coef_probs [0] [ 0 ] [0];

        DETOKENIZE(1, 0, 0, 0);
        DETOKENIZE(1, 1, 1, 0);
        DETOKENIZE(1, 2, 2, 0);
        DETOKENIZE(1, 3, 3, 0);

        DETOKENIZE(1, 4, 0, 1);
        DETOKENIZE(1, 5, 1, 1);
        DETOKENIZE(1, 6, 2, 1);
        DETOKENIZE(1, 7, 3, 1);

        DETOKENIZE(1, 8, 0, 2);
        DETOKENIZE(1, 9, 1, 2);
        DETOKENIZE(1, 10, 2, 2);
        DETOKENIZE(1, 11, 3, 2);

        DETOKENIZE(1, 12, 0, 3);
        DETOKENIZE(1, 13, 1, 3);
        DETOKENIZE(1, 14, 2, 3);
        DETOKENIZE(1, 15, 3, 3);
    }
    else
    {
        coef_probs = oc->fc.coef_probs [3] [ 0 ] [0];

        DETOKENIZE(0, 0, 0, 0);
        DETOKENIZE(0, 1, 1, 0);
        DETOKENIZE(0, 2, 2, 0);
        DETOKENIZE(0, 3, 3, 0);

        DETOKENIZE(0, 4, 0, 1);
        DETOKENIZE(0, 5, 1, 1);
        DETOKENIZE(0, 6, 2, 1);
        DETOKENIZE(0, 7, 3, 1);

        DETOKENIZE(0, 8, 0, 2);
        DETOKENIZE(0, 9, 1, 2);
        DETOKENIZE(0, 10, 2, 2);
        DETOKENIZE(0, 11, 3, 2);

        DETOKENIZE(0, 12, 0, 3);
        DETOKENIZE(0, 13, 1, 3);
        DETOKENIZE(0, 14, 2, 3);
        DETOKENIZE(0, 15, 3, 3);
    }

    coef_probs = oc->fc.coef_probs [2] [ 0 ] [0];


    DETOKENIZE(0, 16, 4, 4);
    DETOKENIZE(0, 17, 5, 4);

    DETOKENIZE(0, 18, 4, 5);
    DETOKENIZE(0, 19, 5, 5);

    DETOKENIZE(0, 20, 6, 6);
    DETOKENIZE(0, 21, 7, 6);

    DETOKENIZE(0, 22, 6, 7);
    DETOKENIZE(0, 23, 7, 7);

    return eobtotal;
}
