/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "vp9_rtcd.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/mips/dspr2/vp9_common_dspr2.h"

#if HAVE_DSPR2
#define STORE_F0                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q1_f0],    1(%[s4])           \n\t"                   \
        "sb     %[q0_f0],    0(%[s4])           \n\t"                   \
        "sb     %[p0_f0],   -1(%[s4])           \n\t"                   \
        "sb     %[p1_f0],   -2(%[s4])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q1_f0] "r" (q1_f0), [q0_f0] "r" (q0_f0),                     \
          [p0_f0] "r" (p0_f0), [p1_f0] "r" (p1_f0),                     \
          [s4] "r" (s4)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q1_f0],   %[q1_f0],   8       \n\t"                   \
        "srl    %[q0_f0],   %[q0_f0],   8       \n\t"                   \
        "srl    %[p0_f0],   %[p0_f0],   8       \n\t"                   \
        "srl    %[p1_f0],   %[p1_f0],   8       \n\t"                   \
                                                                        \
        : [q1_f0] "+r" (q1_f0), [q0_f0] "+r" (q0_f0),                   \
          [p0_f0] "+r" (p0_f0), [p1_f0] "+r" (p1_f0)                    \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q1_f0],    1(%[s3])           \n\t"                   \
        "sb     %[q0_f0],    0(%[s3])           \n\t"                   \
        "sb     %[p0_f0],   -1(%[s3])           \n\t"                   \
        "sb     %[p1_f0],   -2(%[s3])           \n\t"                   \
                                                                        \
        : [p1_f0] "+r" (p1_f0)                                          \
        : [q1_f0] "r" (q1_f0), [q0_f0] "r" (q0_f0),                     \
          [s3] "r" (s3), [p0_f0] "r" (p0_f0)                            \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q1_f0],   %[q1_f0],   8       \n\t"                   \
        "srl    %[q0_f0],   %[q0_f0],   8       \n\t"                   \
        "srl    %[p0_f0],   %[p0_f0],   8       \n\t"                   \
        "srl    %[p1_f0],   %[p1_f0],   8       \n\t"                   \
                                                                        \
        : [q1_f0] "+r" (q1_f0), [q0_f0] "+r" (q0_f0),                   \
          [p0_f0] "+r" (p0_f0), [p1_f0] "+r" (p1_f0)                    \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q1_f0],    1(%[s2])           \n\t"                   \
        "sb     %[q0_f0],    0(%[s2])           \n\t"                   \
        "sb     %[p0_f0],   -1(%[s2])           \n\t"                   \
        "sb     %[p1_f0],   -2(%[s2])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q1_f0] "r" (q1_f0), [q0_f0] "r" (q0_f0),                     \
          [p0_f0] "r" (p0_f0), [p1_f0] "r" (p1_f0),                     \
          [s2] "r" (s2)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q1_f0],   %[q1_f0],   8       \n\t"                   \
        "srl    %[q0_f0],   %[q0_f0],   8       \n\t"                   \
        "srl    %[p0_f0],   %[p0_f0],   8       \n\t"                   \
        "srl    %[p1_f0],   %[p1_f0],   8       \n\t"                   \
                                                                        \
        : [q1_f0] "+r" (q1_f0), [q0_f0] "+r" (q0_f0),                   \
          [p0_f0] "+r" (p0_f0), [p1_f0] "+r" (p1_f0)                    \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q1_f0],    1(%[s1])           \n\t"                   \
        "sb     %[q0_f0],    0(%[s1])           \n\t"                   \
        "sb     %[p0_f0],   -1(%[s1])           \n\t"                   \
        "sb     %[p1_f0],   -2(%[s1])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q1_f0] "r" (q1_f0), [q0_f0] "r" (q0_f0),                     \
          [p0_f0] "r" (p0_f0), [p1_f0] "r" (p1_f0),                     \
          [s1] "r" (s1)                                                 \
    );

#define STORE_F1                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q2_r],     2(%[s4])           \n\t"                   \
        "sb     %[q1_r],     1(%[s4])           \n\t"                   \
        "sb     %[q0_r],     0(%[s4])           \n\t"                   \
        "sb     %[p0_r],    -1(%[s4])           \n\t"                   \
        "sb     %[p1_r],    -2(%[s4])           \n\t"                   \
        "sb     %[p2_r],    -3(%[s4])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q2_r] "r" (q2_r), [q1_r] "r" (q1_r), [q0_r] "r" (q0_r),      \
          [p0_r] "r" (p0_r), [p1_r] "r" (p1_r), [p2_r] "r" (p2_r),      \
          [s4] "r" (s4)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q2_r],    %[q2_r],    16      \n\t"                   \
        "srl    %[q1_r],    %[q1_r],    16      \n\t"                   \
        "srl    %[q0_r],    %[q0_r],    16      \n\t"                   \
        "srl    %[p0_r],    %[p0_r],    16      \n\t"                   \
        "srl    %[p1_r],    %[p1_r],    16      \n\t"                   \
        "srl    %[p2_r],    %[p2_r],    16      \n\t"                   \
                                                                        \
        : [q2_r] "+r" (q2_r), [q1_r] "+r" (q1_r), [q0_r] "+r" (q0_r),   \
          [p0_r] "+r" (p0_r), [p1_r] "+r" (p1_r), [p2_r] "+r" (p2_r)    \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q2_r],     2(%[s3])           \n\t"                   \
        "sb     %[q1_r],     1(%[s3])           \n\t"                   \
        "sb     %[q0_r],     0(%[s3])           \n\t"                   \
        "sb     %[p0_r],    -1(%[s3])           \n\t"                   \
        "sb     %[p1_r],    -2(%[s3])           \n\t"                   \
        "sb     %[p2_r],    -3(%[s3])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q2_r] "r" (q2_r), [q1_r] "r" (q1_r), [q0_r] "r" (q0_r),      \
          [p0_r] "r" (p0_r), [p1_r] "r" (p1_r), [p2_r] "r" (p2_r),      \
          [s3] "r" (s3)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q2_l],     2(%[s2])           \n\t"                   \
        "sb     %[q1_l],     1(%[s2])           \n\t"                   \
        "sb     %[q0_l],     0(%[s2])           \n\t"                   \
        "sb     %[p0_l],    -1(%[s2])           \n\t"                   \
        "sb     %[p1_l],    -2(%[s2])           \n\t"                   \
        "sb     %[p2_l],    -3(%[s2])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q2_l] "r" (q2_l), [q1_l] "r" (q1_l), [q0_l] "r" (q0_l),      \
          [p0_l] "r" (p0_l), [p1_l] "r" (p1_l), [p2_l] "r" (p2_l),      \
          [s2] "r" (s2)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q2_l],    %[q2_l],    16      \n\t"                   \
        "srl    %[q1_l],    %[q1_l],    16      \n\t"                   \
        "srl    %[q0_l],    %[q0_l],    16      \n\t"                   \
        "srl    %[p0_l],    %[p0_l],    16      \n\t"                   \
        "srl    %[p1_l],    %[p1_l],    16      \n\t"                   \
        "srl    %[p2_l],    %[p2_l],    16      \n\t"                   \
                                                                        \
        : [q2_l] "+r" (q2_l), [q1_l] "+r" (q1_l), [q0_l] "+r" (q0_l),   \
          [p0_l] "+r" (p0_l), [p1_l] "+r" (p1_l), [p2_l] "+r" (p2_l)    \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q2_l],     2(%[s1])           \n\t"                   \
        "sb     %[q1_l],     1(%[s1])           \n\t"                   \
        "sb     %[q0_l],     0(%[s1])           \n\t"                   \
        "sb     %[p0_l],    -1(%[s1])           \n\t"                   \
        "sb     %[p1_l],    -2(%[s1])           \n\t"                   \
        "sb     %[p2_l],    -3(%[s1])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q2_l] "r" (q2_l), [q1_l] "r" (q1_l), [q0_l] "r" (q0_l),      \
          [p0_l] "r" (p0_l), [p1_l] "r" (p1_l), [p2_l] "r" (p2_l),      \
          [s1] "r" (s1)                                                 \
    );

#define STORE_F2                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q6_r],     6(%[s4])           \n\t"                   \
        "sb     %[q5_r],     5(%[s4])           \n\t"                   \
        "sb     %[q4_r],     4(%[s4])           \n\t"                   \
        "sb     %[q3_r],     3(%[s4])           \n\t"                   \
        "sb     %[q2_r],     2(%[s4])           \n\t"                   \
        "sb     %[q1_r],     1(%[s4])           \n\t"                   \
        "sb     %[q0_r],     0(%[s4])           \n\t"                   \
        "sb     %[p0_r],    -1(%[s4])           \n\t"                   \
        "sb     %[p1_r],    -2(%[s4])           \n\t"                   \
        "sb     %[p2_r],    -3(%[s4])           \n\t"                   \
        "sb     %[p3_r],    -4(%[s4])           \n\t"                   \
        "sb     %[p4_r],    -5(%[s4])           \n\t"                   \
        "sb     %[p5_r],    -6(%[s4])           \n\t"                   \
        "sb     %[p6_r],    -7(%[s4])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q6_r] "r" (q6_r), [q5_r] "r" (q5_r), [q4_r] "r" (q4_r),      \
          [q3_r] "r" (q3_r), [q2_r] "r" (q2_r), [q1_r] "r" (q1_r),      \
          [q0_r] "r" (q0_r),                                            \
          [p0_r] "r" (p0_r), [p1_r] "r" (p1_r), [p2_r] "r" (p2_r),      \
          [p3_r] "r" (p3_r), [p4_r] "r" (p4_r), [p5_r] "r" (p5_r),      \
          [p6_r] "r" (p6_r),                                            \
          [s4] "r" (s4)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q6_r],    %[q6_r],    16      \n\t"                   \
        "srl    %[q5_r],    %[q5_r],    16      \n\t"                   \
        "srl    %[q4_r],    %[q4_r],    16      \n\t"                   \
        "srl    %[q3_r],    %[q3_r],    16      \n\t"                   \
        "srl    %[q2_r],    %[q2_r],    16      \n\t"                   \
        "srl    %[q1_r],    %[q1_r],    16      \n\t"                   \
        "srl    %[q0_r],    %[q0_r],    16      \n\t"                   \
        "srl    %[p0_r],    %[p0_r],    16      \n\t"                   \
        "srl    %[p1_r],    %[p1_r],    16      \n\t"                   \
        "srl    %[p2_r],    %[p2_r],    16      \n\t"                   \
        "srl    %[p3_r],    %[p3_r],    16      \n\t"                   \
        "srl    %[p4_r],    %[p4_r],    16      \n\t"                   \
        "srl    %[p5_r],    %[p5_r],    16      \n\t"                   \
        "srl    %[p6_r],    %[p6_r],    16      \n\t"                   \
                                                                        \
        : [q6_r] "+r" (q6_r), [q5_r] "+r" (q5_r), [q4_r] "+r" (q4_r),   \
          [q3_r] "+r" (q3_r), [q2_r] "+r" (q2_r), [q1_r] "+r" (q1_r),   \
          [q0_r] "+r" (q0_r),                                           \
          [p0_r] "+r" (p0_r), [p1_r] "+r" (p1_r), [p2_r] "+r" (p2_r),   \
          [p3_r] "+r" (p3_r), [p4_r] "+r" (p4_r), [p5_r] "+r" (p5_r),   \
          [p6_r] "+r" (p6_r)                                            \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q6_r],     6(%[s3])           \n\t"                   \
        "sb     %[q5_r],     5(%[s3])           \n\t"                   \
        "sb     %[q4_r],     4(%[s3])           \n\t"                   \
        "sb     %[q3_r],     3(%[s3])           \n\t"                   \
        "sb     %[q2_r],     2(%[s3])           \n\t"                   \
        "sb     %[q1_r],     1(%[s3])           \n\t"                   \
        "sb     %[q0_r],     0(%[s3])           \n\t"                   \
        "sb     %[p0_r],    -1(%[s3])           \n\t"                   \
        "sb     %[p1_r],    -2(%[s3])           \n\t"                   \
        "sb     %[p2_r],    -3(%[s3])           \n\t"                   \
        "sb     %[p3_r],    -4(%[s3])           \n\t"                   \
        "sb     %[p4_r],    -5(%[s3])           \n\t"                   \
        "sb     %[p5_r],    -6(%[s3])           \n\t"                   \
        "sb     %[p6_r],    -7(%[s3])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q6_r] "r" (q6_r), [q5_r] "r" (q5_r), [q4_r] "r" (q4_r),      \
          [q3_r] "r" (q3_r), [q2_r] "r" (q2_r), [q1_r] "r" (q1_r),      \
          [q0_r] "r" (q0_r),                                            \
          [p0_r] "r" (p0_r), [p1_r] "r" (p1_r), [p2_r] "r" (p2_r),      \
          [p3_r] "r" (p3_r), [p4_r] "r" (p4_r), [p5_r] "r" (p5_r),      \
          [p6_r] "r" (p6_r),                                            \
          [s3] "r" (s3)                                                 \
    );                                                                  \
                                                                        \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q6_l],     6(%[s2])           \n\t"                   \
        "sb     %[q5_l],     5(%[s2])           \n\t"                   \
        "sb     %[q4_l],     4(%[s2])           \n\t"                   \
        "sb     %[q3_l],     3(%[s2])           \n\t"                   \
        "sb     %[q2_l],     2(%[s2])           \n\t"                   \
        "sb     %[q1_l],     1(%[s2])           \n\t"                   \
        "sb     %[q0_l],     0(%[s2])           \n\t"                   \
        "sb     %[p0_l],    -1(%[s2])           \n\t"                   \
        "sb     %[p1_l],    -2(%[s2])           \n\t"                   \
        "sb     %[p2_l],    -3(%[s2])           \n\t"                   \
        "sb     %[p3_l],    -4(%[s2])           \n\t"                   \
        "sb     %[p4_l],    -5(%[s2])           \n\t"                   \
        "sb     %[p5_l],    -6(%[s2])           \n\t"                   \
        "sb     %[p6_l],    -7(%[s2])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q6_l] "r" (q6_l), [q5_l] "r" (q5_l), [q4_l] "r" (q4_l),      \
          [q3_l] "r" (q3_l), [q2_l] "r" (q2_l), [q1_l] "r" (q1_l),      \
          [q0_l] "r" (q0_l),                                            \
          [p0_l] "r" (p0_l), [p1_l] "r" (p1_l), [p2_l] "r" (p2_l),      \
          [p3_l] "r" (p3_l), [p4_l] "r" (p4_l), [p5_l] "r" (p5_l),      \
          [p6_l] "r" (p6_l),                                            \
          [s2] "r" (s2)                                                 \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "srl    %[q6_l],    %[q6_l],    16     \n\t"                    \
        "srl    %[q5_l],    %[q5_l],    16     \n\t"                    \
        "srl    %[q4_l],    %[q4_l],    16     \n\t"                    \
        "srl    %[q3_l],    %[q3_l],    16     \n\t"                    \
        "srl    %[q2_l],    %[q2_l],    16     \n\t"                    \
        "srl    %[q1_l],    %[q1_l],    16     \n\t"                    \
        "srl    %[q0_l],    %[q0_l],    16     \n\t"                    \
        "srl    %[p0_l],    %[p0_l],    16     \n\t"                    \
        "srl    %[p1_l],    %[p1_l],    16     \n\t"                    \
        "srl    %[p2_l],    %[p2_l],    16     \n\t"                    \
        "srl    %[p3_l],    %[p3_l],    16     \n\t"                    \
        "srl    %[p4_l],    %[p4_l],    16     \n\t"                    \
        "srl    %[p5_l],    %[p5_l],    16     \n\t"                    \
        "srl    %[p6_l],    %[p6_l],    16     \n\t"                    \
                                                                        \
        : [q6_l] "+r" (q6_l), [q5_l] "+r" (q5_l), [q4_l] "+r" (q4_l),   \
          [q3_l] "+r" (q3_l), [q2_l] "+r" (q2_l), [q1_l] "+r" (q1_l),   \
          [q0_l] "+r" (q0_l),                                           \
          [p0_l] "+r" (p0_l), [p1_l] "+r" (p1_l), [p2_l] "+r" (p2_l),   \
          [p3_l] "+r" (p3_l), [p4_l] "+r" (p4_l), [p5_l] "+r" (p5_l),   \
          [p6_l] "+r" (p6_l)                                            \
        :                                                               \
    );                                                                  \
                                                                        \
    __asm__ __volatile__ (                                              \
        "sb     %[q6_l],     6(%[s1])           \n\t"                   \
        "sb     %[q5_l],     5(%[s1])           \n\t"                   \
        "sb     %[q4_l],     4(%[s1])           \n\t"                   \
        "sb     %[q3_l],     3(%[s1])           \n\t"                   \
        "sb     %[q2_l],     2(%[s1])           \n\t"                   \
        "sb     %[q1_l],     1(%[s1])           \n\t"                   \
        "sb     %[q0_l],     0(%[s1])           \n\t"                   \
        "sb     %[p0_l],    -1(%[s1])           \n\t"                   \
        "sb     %[p1_l],    -2(%[s1])           \n\t"                   \
        "sb     %[p2_l],    -3(%[s1])           \n\t"                   \
        "sb     %[p3_l],    -4(%[s1])           \n\t"                   \
        "sb     %[p4_l],    -5(%[s1])           \n\t"                   \
        "sb     %[p5_l],    -6(%[s1])           \n\t"                   \
        "sb     %[p6_l],    -7(%[s1])           \n\t"                   \
                                                                        \
        :                                                               \
        : [q6_l] "r" (q6_l), [q5_l] "r" (q5_l), [q4_l] "r" (q4_l),      \
          [q3_l] "r" (q3_l), [q2_l] "r" (q2_l), [q1_l] "r" (q1_l),      \
          [q0_l] "r" (q0_l),                                            \
          [p0_l] "r" (p0_l), [p1_l] "r" (p1_l), [p2_l] "r" (p2_l),      \
          [p3_l] "r" (p3_l), [p4_l] "r" (p4_l), [p5_l] "r" (p5_l),      \
          [p6_l] "r" (p6_l),                                            \
          [s1] "r" (s1)                                                 \
    );

#define  PACK_LEFT_0TO3                                                 \
    __asm__ __volatile__ (                                              \
        "preceu.ph.qbl   %[p3_l],   %[p3]   \n\t"                       \
        "preceu.ph.qbl   %[p2_l],   %[p2]   \n\t"                       \
        "preceu.ph.qbl   %[p1_l],   %[p1]   \n\t"                       \
        "preceu.ph.qbl   %[p0_l],   %[p0]   \n\t"                       \
        "preceu.ph.qbl   %[q0_l],   %[q0]   \n\t"                       \
        "preceu.ph.qbl   %[q1_l],   %[q1]   \n\t"                       \
        "preceu.ph.qbl   %[q2_l],   %[q2]   \n\t"                       \
        "preceu.ph.qbl   %[q3_l],   %[q3]   \n\t"                       \
                                                                        \
        : [p3_l] "=&r" (p3_l), [p2_l] "=&r" (p2_l),                     \
          [p1_l] "=&r" (p1_l), [p0_l] "=&r" (p0_l),                     \
          [q0_l] "=&r" (q0_l), [q1_l] "=&r" (q1_l),                     \
          [q2_l] "=&r" (q2_l), [q3_l] "=&r" (q3_l)                      \
        : [p3] "r" (p3), [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),   \
          [q0] "r" (q0), [q1] "r" (q1), [q2] "r" (q2), [q3] "r" (q3)    \
    );

#define PACK_LEFT_4TO7                                                  \
    __asm__ __volatile__ (                                              \
        "preceu.ph.qbl   %[p7_l],   %[p7]   \n\t"                       \
        "preceu.ph.qbl   %[p6_l],   %[p6]   \n\t"                       \
        "preceu.ph.qbl   %[p5_l],   %[p5]   \n\t"                       \
        "preceu.ph.qbl   %[p4_l],   %[p4]   \n\t"                       \
        "preceu.ph.qbl   %[q4_l],   %[q4]   \n\t"                       \
        "preceu.ph.qbl   %[q5_l],   %[q5]   \n\t"                       \
        "preceu.ph.qbl   %[q6_l],   %[q6]   \n\t"                       \
        "preceu.ph.qbl   %[q7_l],   %[q7]   \n\t"                       \
                                                                        \
        : [p7_l] "=&r" (p7_l), [p6_l] "=&r" (p6_l),                     \
          [p5_l] "=&r" (p5_l), [p4_l] "=&r" (p4_l),                     \
          [q4_l] "=&r" (q4_l), [q5_l] "=&r" (q5_l),                     \
          [q6_l] "=&r" (q6_l), [q7_l] "=&r" (q7_l)                      \
        : [p7] "r" (p7), [p6] "r" (p6), [p5] "r" (p5), [p4] "r" (p4),   \
          [q4] "r" (q4), [q5] "r" (q5), [q6] "r" (q6), [q7] "r" (q7)    \
    );

#define  PACK_RIGHT_0TO3                                                \
    __asm__ __volatile__ (                                              \
        "preceu.ph.qbr   %[p3_r],   %[p3]  \n\t"                        \
        "preceu.ph.qbr   %[p2_r],   %[p2]   \n\t"                       \
        "preceu.ph.qbr   %[p1_r],   %[p1]   \n\t"                       \
        "preceu.ph.qbr   %[p0_r],   %[p0]   \n\t"                       \
        "preceu.ph.qbr   %[q0_r],   %[q0]   \n\t"                       \
        "preceu.ph.qbr   %[q1_r],   %[q1]   \n\t"                       \
        "preceu.ph.qbr   %[q2_r],   %[q2]   \n\t"                       \
        "preceu.ph.qbr   %[q3_r],   %[q3]   \n\t"                       \
                                                                        \
        : [p3_r] "=&r" (p3_r), [p2_r] "=&r" (p2_r),                     \
          [p1_r] "=&r" (p1_r), [p0_r] "=&r" (p0_r),                     \
          [q0_r] "=&r" (q0_r), [q1_r] "=&r" (q1_r),                     \
          [q2_r] "=&r" (q2_r), [q3_r] "=&r" (q3_r)                      \
        : [p3] "r" (p3), [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),   \
          [q0] "r" (q0), [q1] "r" (q1), [q2] "r" (q2), [q3] "r" (q3)    \
    );

#define PACK_RIGHT_4TO7                                                 \
    __asm__ __volatile__ (                                              \
        "preceu.ph.qbr   %[p7_r],   %[p7]   \n\t"                       \
        "preceu.ph.qbr   %[p6_r],   %[p6]   \n\t"                       \
        "preceu.ph.qbr   %[p5_r],   %[p5]   \n\t"                       \
        "preceu.ph.qbr   %[p4_r],   %[p4]   \n\t"                       \
        "preceu.ph.qbr   %[q4_r],   %[q4]   \n\t"                       \
        "preceu.ph.qbr   %[q5_r],   %[q5]   \n\t"                       \
        "preceu.ph.qbr   %[q6_r],   %[q6]   \n\t"                       \
        "preceu.ph.qbr   %[q7_r],   %[q7]   \n\t"                       \
                                                                        \
        : [p7_r] "=&r" (p7_r), [p6_r] "=&r" (p6_r),                     \
          [p5_r] "=&r" (p5_r), [p4_r] "=&r" (p4_r),                     \
          [q4_r] "=&r" (q4_r), [q5_r] "=&r" (q5_r),                     \
          [q6_r] "=&r" (q6_r), [q7_r] "=&r" (q7_r)                      \
        : [p7] "r" (p7), [p6] "r" (p6), [p5] "r" (p5), [p4] "r" (p4),   \
          [q4] "r" (q4), [q5] "r" (q5), [q6] "r" (q6), [q7] "r" (q7)    \
    );

#define COMBINE_LEFT_RIGHT_0TO2                                         \
    __asm__ __volatile__ (                                              \
        "precr.qb.ph    %[p2],  %[p2_l],    %[p2_r]    \n\t"            \
        "precr.qb.ph    %[p1],  %[p1_l],    %[p1_r]    \n\t"            \
        "precr.qb.ph    %[p0],  %[p0_l],    %[p0_r]    \n\t"            \
        "precr.qb.ph    %[q0],  %[q0_l],    %[q0_r]    \n\t"            \
        "precr.qb.ph    %[q1],  %[q1_l],    %[q1_r]    \n\t"            \
        "precr.qb.ph    %[q2],  %[q2_l],    %[q2_r]    \n\t"            \
                                                                        \
        : [p2] "=&r" (p2), [p1] "=&r" (p1), [p0] "=&r" (p0),            \
          [q0] "=&r" (q0), [q1] "=&r" (q1), [q2] "=&r" (q2)             \
        : [p2_l] "r" (p2_l), [p2_r] "r" (p2_r),                         \
          [p1_l] "r" (p1_l), [p1_r] "r" (p1_r),                         \
          [p0_l] "r" (p0_l), [p0_r] "r" (p0_r),                         \
          [q0_l] "r" (q0_l), [q0_r] "r" (q0_r),                         \
          [q1_l] "r" (q1_l), [q1_r] "r" (q1_r),                         \
          [q2_l] "r" (q2_l), [q2_r] "r" (q2_r)                          \
    );

#define COMBINE_LEFT_RIGHT_3TO6                                         \
    __asm__ __volatile__ (                                              \
        "precr.qb.ph    %[p6],  %[p6_l],    %[p6_r]    \n\t"            \
        "precr.qb.ph    %[p5],  %[p5_l],    %[p5_r]    \n\t"            \
        "precr.qb.ph    %[p4],  %[p4_l],    %[p4_r]    \n\t"            \
        "precr.qb.ph    %[p3],  %[p3_l],    %[p3_r]    \n\t"            \
        "precr.qb.ph    %[q3],  %[q3_l],    %[q3_r]    \n\t"            \
        "precr.qb.ph    %[q4],  %[q4_l],    %[q4_r]    \n\t"            \
        "precr.qb.ph    %[q5],  %[q5_l],    %[q5_r]    \n\t"            \
        "precr.qb.ph    %[q6],  %[q6_l],    %[q6_r]    \n\t"            \
                                                                        \
        : [p6] "=&r" (p6),[p5] "=&r" (p5),                              \
          [p4] "=&r" (p4),[p3] "=&r" (p3),                              \
          [q3] "=&r" (q3),[q4] "=&r" (q4),                              \
          [q5] "=&r" (q5),[q6] "=&r" (q6)                               \
        : [p6_l] "r" (p6_l), [p5_l] "r" (p5_l),                         \
          [p4_l] "r" (p4_l), [p3_l] "r" (p3_l),                         \
          [p6_r] "r" (p6_r), [p5_r] "r" (p5_r),                         \
          [p4_r] "r" (p4_r), [p3_r] "r" (p3_r),                         \
          [q3_l] "r" (q3_l), [q4_l] "r" (q4_l),                         \
          [q5_l] "r" (q5_l), [q6_l] "r" (q6_l),                         \
          [q3_r] "r" (q3_r), [q4_r] "r" (q4_r),                         \
          [q5_r] "r" (q5_r), [q6_r] "r" (q6_r)                          \
    );

/* processing 4 pixels at the same time
 * compute hev and mask in the same function */
static __inline void vp9_filter_hev_mask_dspr2(uint32_t limit, uint32_t flimit,
                                               uint32_t p1, uint32_t p0,
                                               uint32_t p3, uint32_t p2,
                                               uint32_t q0, uint32_t q1,
                                               uint32_t q2, uint32_t q3,
                                               uint32_t thresh, uint32_t *hev,
                                               uint32_t *mask) {
  uint32_t c, r, r3, r_k;
  uint32_t s1, s2, s3;
  uint32_t ones = 0xFFFFFFFF;
  uint32_t hev1;

  __asm__ __volatile__ (
      /* mask |= (abs(p3 - p2) > limit) */
      "subu_s.qb      %[c],   %[p3],     %[p2]        \n\t"
      "subu_s.qb      %[r_k], %[p2],     %[p3]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   $0,        %[c]         \n\t"

      /* mask |= (abs(p2 - p1) > limit) */
      "subu_s.qb      %[c],   %[p2],     %[p1]        \n\t"
      "subu_s.qb      %[r_k], %[p1],     %[p2]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      /* mask |= (abs(p1 - p0) > limit)
       * hev  |= (abs(p1 - p0) > thresh)
       */
      "subu_s.qb      %[c],   %[p1],     %[p0]        \n\t"
      "subu_s.qb      %[r_k], %[p0],     %[p1]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[thresh], %[r_k]       \n\t"
      "or             %[r3],  $0,        %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      /* mask |= (abs(q1 - q0) > limit)
       * hev  |= (abs(q1 - q0) > thresh)
       */
      "subu_s.qb      %[c],   %[q1],     %[q0]        \n\t"
      "subu_s.qb      %[r_k], %[q0],     %[q1]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[thresh], %[r_k]       \n\t"
      "or             %[r3],  %[r3],     %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      /* mask |= (abs(q2 - q1) > limit) */
      "subu_s.qb      %[c],   %[q2],     %[q1]        \n\t"
      "subu_s.qb      %[r_k], %[q1],     %[q2]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"
      "sll            %[r3],    %[r3],    24          \n\t"

      /* mask |= (abs(q3 - q2) > limit) */
      "subu_s.qb      %[c],   %[q3],     %[q2]        \n\t"
      "subu_s.qb      %[r_k], %[q2],     %[q3]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      : [c] "=&r" (c), [r_k] "=&r" (r_k),
        [r] "=&r" (r), [r3] "=&r" (r3)
      : [limit] "r" (limit), [p3] "r" (p3), [p2] "r" (p2),
        [p1] "r" (p1), [p0] "r" (p0), [q1] "r" (q1), [q0] "r" (q0),
        [q2] "r" (q2), [q3] "r" (q3), [thresh] "r" (thresh)
  );

  __asm__ __volatile__ (
      /* abs(p0 - q0) */
      "subu_s.qb      %[c],   %[p0],     %[q0]        \n\t"
      "subu_s.qb      %[r_k], %[q0],     %[p0]        \n\t"
      "wrdsp          %[r3]                           \n\t"
      "or             %[s1],  %[r_k],    %[c]         \n\t"

      /* abs(p1 - q1) */
      "subu_s.qb      %[c],    %[p1],    %[q1]        \n\t"
      "addu_s.qb      %[s3],   %[s1],    %[s1]        \n\t"
      "pick.qb        %[hev1], %[ones],  $0           \n\t"
      "subu_s.qb      %[r_k],  %[q1],    %[p1]        \n\t"
      "or             %[s2],   %[r_k],   %[c]         \n\t"

      /* abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > flimit * 2 + limit */
      "shrl.qb        %[s2],   %[s2],     1           \n\t"
      "addu_s.qb      %[s1],   %[s2],     %[s3]       \n\t"
      "cmpgu.lt.qb    %[c],    %[flimit], %[s1]       \n\t"
      "or             %[r],    %[r],      %[c]        \n\t"
      "sll            %[r],    %[r],      24          \n\t"

      "wrdsp          %[r]                            \n\t"
      "pick.qb        %[s2],  $0,         %[ones]     \n\t"

      : [c] "=&r" (c), [r_k] "=&r" (r_k), [s1] "=&r" (s1), [hev1] "=&r" (hev1),
        [s2] "=&r" (s2), [r] "+r" (r), [s3] "=&r" (s3)
      : [p0] "r" (p0), [q0] "r" (q0), [p1] "r" (p1), [r3] "r" (r3),
        [q1] "r" (q1), [ones] "r" (ones), [flimit] "r" (flimit)
  );

  *hev = hev1;
  *mask = s2;
}

static __inline void filter_hev_mask_flatmask4_dspr2(uint32_t limit, uint32_t flimit,
                                                     uint32_t thresh,
                                                     uint32_t p1, uint32_t p0,
                                                     uint32_t p3, uint32_t p2,
                                                     uint32_t q0, uint32_t q1,
                                                     uint32_t q2, uint32_t q3,
                                                     uint32_t *hev, uint32_t *mask,
                                                     uint32_t *flat) {
  uint32_t c, r, r3, r_k, r_flat;
  uint32_t s1, s2, s3;
  uint32_t ones = 0xFFFFFFFF;
  uint32_t flat_thresh = 0x01010101;
  uint32_t hev1;
  uint32_t flat1;

  __asm__ __volatile__ (
      /* mask |= (abs(p3 - p2) > limit) */
      "subu_s.qb      %[c],   %[p3],     %[p2]        \n\t"
      "subu_s.qb      %[r_k], %[p2],     %[p3]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   $0,        %[c]         \n\t"

      /* mask |= (abs(p2 - p1) > limit) */
      "subu_s.qb      %[c],   %[p2],     %[p1]        \n\t"
      "subu_s.qb      %[r_k], %[p1],     %[p2]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      /* mask |= (abs(p1 - p0) > limit)
       * hev  |= (abs(p1 - p0) > thresh)
       * flat |= (abs(p1 - p0) > thresh)
       */
      "subu_s.qb      %[c],   %[p1],     %[p0]        \n\t"
      "subu_s.qb      %[r_k], %[p0],     %[p1]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[thresh], %[r_k]       \n\t"
      "or             %[r3],  $0,        %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  $0,             %[c]         \n\t"

      /* mask |= (abs(q1 - q0) > limit)
       * hev  |= (abs(q1 - q0) > thresh)
       * flat |= (abs(q1 - q0) > thresh)
       */
      "subu_s.qb      %[c],   %[q1],     %[q0]        \n\t"
      "subu_s.qb      %[r_k], %[q0],     %[q1]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[thresh], %[r_k]       \n\t"
      "or             %[r3],  %[r3],     %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(p0 - p2) > thresh) */
      "subu_s.qb      %[c],       %[p0],          %[p2]        \n\t"
      "subu_s.qb      %[r_k],     %[p2],          %[p0]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(q0 - q2) > thresh) */
      "subu_s.qb      %[c],       %[q0],          %[q2]        \n\t"
      "subu_s.qb      %[r_k],     %[q2],          %[q0]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(p3 - p0) > thresh) */
      "subu_s.qb      %[c],       %[p3],          %[p0]        \n\t"
      "subu_s.qb      %[r_k],     %[p0],          %[p3]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(q3 - q0) > thresh) */
      "subu_s.qb      %[c],       %[q3],          %[q0]        \n\t"
      "subu_s.qb      %[r_k],     %[q0],          %[q3]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"
      "sll            %[r_flat],  %[r_flat],      24           \n\t"
      /* look at stall here */
      "wrdsp          %[r_flat]                                \n\t"
      "pick.qb        %[flat1],  $0,         %[ones]     \n\t"

      /* mask |= (abs(q2 - q1) > limit) */
      "subu_s.qb      %[c],   %[q2],     %[q1]        \n\t"
      "subu_s.qb      %[r_k], %[q1],     %[q2]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"
      "sll            %[r3],    %[r3],    24          \n\t"

      /* mask |= (abs(q3 - q2) > limit) */
      "subu_s.qb      %[c],   %[q3],     %[q2]        \n\t"
      "subu_s.qb      %[r_k], %[q2],     %[q3]        \n\t"
      "or             %[r_k], %[r_k],    %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[limit],  %[r_k]       \n\t"
      "or             %[r],   %[r],      %[c]         \n\t"

      : [c] "=&r" (c), [r_k] "=&r" (r_k),
        [r] "=&r" (r), [r3] "=&r" (r3), [r_flat] "=&r" (r_flat),
        [flat1] "=&r" (flat1)
      : [limit] "r" (limit), [p3] "r" (p3), [p2] "r" (p2),
        [p1] "r" (p1), [p0] "r" (p0), [q1] "r" (q1), [q0] "r" (q0),
        [q2] "r" (q2), [q3] "r" (q3), [thresh] "r" (thresh),
        [flat_thresh] "r" (flat_thresh), [ones] "r" (ones)
  );

  __asm__ __volatile__ (
      /* abs(p0 - q0) */
      "subu_s.qb      %[c],   %[p0],     %[q0]        \n\t"
      "subu_s.qb      %[r_k], %[q0],     %[p0]        \n\t"
      "wrdsp          %[r3]                           \n\t"
      "or             %[s1],  %[r_k],    %[c]         \n\t"

      /* abs(p1 - q1) */
      "subu_s.qb      %[c],    %[p1],    %[q1]        \n\t"
      "addu_s.qb      %[s3],   %[s1],    %[s1]        \n\t"
      "pick.qb        %[hev1], %[ones],  $0           \n\t"
      "subu_s.qb      %[r_k],  %[q1],    %[p1]        \n\t"
      "or             %[s2],   %[r_k],   %[c]         \n\t"

      /* abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > flimit * 2 + limit */
      "shrl.qb        %[s2],   %[s2],     1           \n\t"
      "addu_s.qb      %[s1],   %[s2],     %[s3]       \n\t"
      "cmpgu.lt.qb    %[c],    %[flimit], %[s1]       \n\t"
      "or             %[r],    %[r],      %[c]        \n\t"
      "sll            %[r],    %[r],      24          \n\t"

      "wrdsp          %[r]                            \n\t"
      "pick.qb        %[s2],  $0,         %[ones]     \n\t"

      : [c] "=&r" (c), [r_k] "=&r" (r_k), [s1] "=&r" (s1), [hev1] "=&r" (hev1),
        [s2] "=&r" (s2), [r] "+r" (r), [s3] "=&r" (s3)
      : [p0] "r" (p0), [q0] "r" (q0), [p1] "r" (p1), [r3] "r" (r3),
        [q1] "r" (q1), [ones] "r" (ones), [flimit] "r" (flimit)
  );

  *hev = hev1;
  *mask = s2;
  *flat = flat1;
}

/* inputs & outputs are quad-byte vectors */
static __inline void vp9_filter_dspr2(uint32_t mask, uint32_t hev,
                                      uint32_t *ps1, uint32_t *ps0,
                                      uint32_t *qs0, uint32_t *qs1) {
  int32_t vp8_filter_l, vp8_filter_r;
  int32_t Filter1_l, Filter1_r, Filter2_l, Filter2_r;
  int32_t subr_r, subr_l;
  uint32_t t1, t2, HWM, t3;
  uint32_t hev_l, hev_r, mask_l, mask_r, invhev_l, invhev_r;
  int32_t vps1, vps0, vqs0, vqs1;
  int32_t vps1_l, vps1_r, vps0_l, vps0_r, vqs0_l, vqs0_r, vqs1_l, vqs1_r;
  uint32_t N128;

  N128 = 0x80808080;
  t1  = 0x03000300;
  t2  = 0x04000400;
  t3  = 0x01000100;
  HWM = 0xFF00FF00;

  vps0 = (*ps0) ^ N128;
  vps1 = (*ps1) ^ N128;
  vqs0 = (*qs0) ^ N128;
  vqs1 = (*qs1) ^ N128;

  /* use halfword pairs instead quad-bytes because of accuracy */
  vps0_l = vps0 & HWM;
  vps0_r = vps0 << 8;
  vps0_r = vps0_r & HWM;

  vps1_l = vps1 & HWM;
  vps1_r = vps1 << 8;
  vps1_r = vps1_r & HWM;

  vqs0_l = vqs0 & HWM;
  vqs0_r = vqs0 << 8;
  vqs0_r = vqs0_r & HWM;

  vqs1_l = vqs1 & HWM;
  vqs1_r = vqs1 << 8;
  vqs1_r = vqs1_r & HWM;

  mask_l = mask & HWM;
  mask_r = mask << 8;
  mask_r = mask_r & HWM;

  hev_l = hev & HWM;
  hev_r = hev << 8;
  hev_r = hev_r & HWM;

  __asm__ __volatile__ (
      /* vp8_filter = vp8_signed_char_clamp(ps1 - qs1); */
      "subq_s.ph    %[vp8_filter_l], %[vps1_l],       %[vqs1_l]       \n\t"
      "subq_s.ph    %[vp8_filter_r], %[vps1_r],       %[vqs1_r]       \n\t"

      /* qs0 - ps0 */
      "subq_s.ph    %[subr_l],       %[vqs0_l],       %[vps0_l]       \n\t"
      "subq_s.ph    %[subr_r],       %[vqs0_r],       %[vps0_r]       \n\t"

      /* vp8_filter &= hev; */
      "and          %[vp8_filter_l], %[vp8_filter_l], %[hev_l]        \n\t"
      "and          %[vp8_filter_r], %[vp8_filter_r], %[hev_r]        \n\t"

      /* vp8_filter = vp8_signed_char_clamp(vp8_filter + 3 * (qs0 - ps0)); */
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"
      "xor          %[invhev_l],     %[hev_l],        %[HWM]          \n\t"
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"
      "xor          %[invhev_r],     %[hev_r],        %[HWM]          \n\t"
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"

      /* vp8_filter &= mask; */
      "and          %[vp8_filter_l], %[vp8_filter_l], %[mask_l]       \n\t"
      "and          %[vp8_filter_r], %[vp8_filter_r], %[mask_r]       \n\t"

      : [vp8_filter_l] "=&r" (vp8_filter_l), [vp8_filter_r] "=&r" (vp8_filter_r),
        [subr_l] "=&r" (subr_l), [subr_r] "=&r" (subr_r),
        [invhev_l] "=&r" (invhev_l), [invhev_r] "=&r" (invhev_r)

      : [vps0_l] "r" (vps0_l), [vps0_r] "r" (vps0_r), [vps1_l] "r" (vps1_l),
        [vps1_r] "r" (vps1_r), [vqs0_l] "r" (vqs0_l), [vqs0_r] "r" (vqs0_r),
        [vqs1_l] "r" (vqs1_l), [vqs1_r] "r" (vqs1_r),
        [mask_l] "r" (mask_l), [mask_r] "r" (mask_r),
        [hev_l] "r" (hev_l), [hev_r] "r" (hev_r),
        [HWM] "r" (HWM)
  );

  /* save bottom 3 bits so that we round one side +4 and the other +3 */
  __asm__ __volatile__ (
      /* Filter2 = vp8_signed_char_clamp(vp8_filter + 3) >>= 3; */
      "addq_s.ph    %[Filter1_l],    %[vp8_filter_l], %[t2]           \n\t"
      "addq_s.ph    %[Filter1_r],    %[vp8_filter_r], %[t2]           \n\t"

      /* Filter1 = vp8_signed_char_clamp(vp8_filter + 4) >>= 3; */
      "addq_s.ph    %[Filter2_l],    %[vp8_filter_l], %[t1]           \n\t"
      "addq_s.ph    %[Filter2_r],    %[vp8_filter_r], %[t1]           \n\t"
      "shra.ph      %[Filter1_r],    %[Filter1_r],    3               \n\t"
      "shra.ph      %[Filter1_l],    %[Filter1_l],    3               \n\t"

      "shra.ph      %[Filter2_l],    %[Filter2_l],    3               \n\t"
      "shra.ph      %[Filter2_r],    %[Filter2_r],    3               \n\t"

      "and          %[Filter1_l],    %[Filter1_l],    %[HWM]          \n\t"
      "and          %[Filter1_r],    %[Filter1_r],    %[HWM]          \n\t"

      /* vps0 = vp8_signed_char_clamp(ps0 + Filter2); */
      "addq_s.ph    %[vps0_l],       %[vps0_l],       %[Filter2_l]    \n\t"
      "addq_s.ph    %[vps0_r],       %[vps0_r],       %[Filter2_r]    \n\t"

      /* vqs0 = vp8_signed_char_clamp(qs0 - Filter1); */
      "subq_s.ph    %[vqs0_l],       %[vqs0_l],       %[Filter1_l]    \n\t"
      "subq_s.ph    %[vqs0_r],       %[vqs0_r],       %[Filter1_r]    \n\t"

      : [Filter1_l] "=&r" (Filter1_l), [Filter1_r] "=&r" (Filter1_r),
        [Filter2_l] "=&r" (Filter2_l), [Filter2_r] "=&r" (Filter2_r),
        [vps0_l] "+r" (vps0_l), [vps0_r] "+r" (vps0_r),
        [vqs0_l] "+r" (vqs0_l), [vqs0_r] "+r" (vqs0_r)

      : [t1] "r" (t1), [t2] "r" (t2),
        [vp8_filter_l] "r" (vp8_filter_l), [vp8_filter_r] "r" (vp8_filter_r),
        [HWM] "r" (HWM)
  );

  __asm__ __volatile__ (
      /* (vp8_filter += 1) >>= 1 */
      "addqh.ph    %[Filter1_l],    %[Filter1_l],     %[t3]           \n\t"
      "addqh.ph    %[Filter1_r],    %[Filter1_r],     %[t3]           \n\t"

      /* vp8_filter &= ~hev; */
      "and          %[Filter1_l],    %[Filter1_l],    %[invhev_l]     \n\t"
      "and          %[Filter1_r],    %[Filter1_r],    %[invhev_r]     \n\t"

      /* vps1 = vp8_signed_char_clamp(ps1 + vp8_filter); */
      "addq_s.ph    %[vps1_l],       %[vps1_l],       %[Filter1_l]    \n\t"
      "addq_s.ph    %[vps1_r],       %[vps1_r],       %[Filter1_r]    \n\t"

      /* vqs1 = vp8_signed_char_clamp(qs1 - vp8_filter); */
      "subq_s.ph    %[vqs1_l],       %[vqs1_l],       %[Filter1_l]    \n\t"
      "subq_s.ph    %[vqs1_r],       %[vqs1_r],       %[Filter1_r]    \n\t"

      : [Filter1_l] "+r" (Filter1_l), [Filter1_r] "+r" (Filter1_r),
        [vps1_l] "+r" (vps1_l), [vps1_r] "+r" (vps1_r),
        [vqs1_l] "+r" (vqs1_l), [vqs1_r] "+r" (vqs1_r)

      : [t3] "r" (t3), [invhev_l] "r" (invhev_l), [invhev_r] "r" (invhev_r)
  );

  /* Create quad-bytes from halfword pairs */
  vqs0_l = vqs0_l & HWM;
  vqs1_l = vqs1_l & HWM;
  vps0_l = vps0_l & HWM;
  vps1_l = vps1_l & HWM;

  __asm__ __volatile__ (
      "shrl.ph      %[vqs0_r],       %[vqs0_r],       8   \n\t"
      "shrl.ph      %[vps0_r],       %[vps0_r],       8   \n\t"
      "shrl.ph      %[vqs1_r],       %[vqs1_r],       8   \n\t"
      "shrl.ph      %[vps1_r],       %[vps1_r],       8   \n\t"

      : [vps1_r] "+r" (vps1_r), [vqs1_r] "+r" (vqs1_r),
        [vps0_r] "+r" (vps0_r), [vqs0_r] "+r" (vqs0_r)
      :
  );

  vqs0 = vqs0_l | vqs0_r;
  vqs1 = vqs1_l | vqs1_r;
  vps0 = vps0_l | vps0_r;
  vps1 = vps1_l | vps1_r;

  *ps0 = vps0 ^ N128;
  *ps1 = vps1 ^ N128;
  *qs0 = vqs0 ^ N128;
  *qs1 = vqs1 ^ N128;
}

static __inline void vp9_filter1_dspr2(uint32_t mask, uint32_t hev,
                                       uint32_t ps1, uint32_t ps0,
                                       uint32_t qs0, uint32_t qs1,
                                       uint32_t *p1_f0, uint32_t *p0_f0,
                                       uint32_t *q0_f0, uint32_t *q1_f0) {
  int32_t vp8_filter_l, vp8_filter_r;
  int32_t Filter1_l, Filter1_r, Filter2_l, Filter2_r;
  int32_t subr_r, subr_l;
  uint32_t t1, t2, HWM, t3;
  uint32_t hev_l, hev_r, mask_l, mask_r, invhev_l, invhev_r;
  int32_t vps1, vps0, vqs0, vqs1;
  int32_t vps1_l, vps1_r, vps0_l, vps0_r, vqs0_l, vqs0_r, vqs1_l, vqs1_r;
  uint32_t N128;

  N128 = 0x80808080;
  t1  = 0x03000300;
  t2  = 0x04000400;
  t3  = 0x01000100;
  HWM = 0xFF00FF00;

  vps0 = (ps0) ^ N128;
  vps1 = (ps1) ^ N128;
  vqs0 = (qs0) ^ N128;
  vqs1 = (qs1) ^ N128;

  /* use halfword pairs instead quad-bytes because of accuracy */
  vps0_l = vps0 & HWM;
  vps0_r = vps0 << 8;
  vps0_r = vps0_r & HWM;

  vps1_l = vps1 & HWM;
  vps1_r = vps1 << 8;
  vps1_r = vps1_r & HWM;

  vqs0_l = vqs0 & HWM;
  vqs0_r = vqs0 << 8;
  vqs0_r = vqs0_r & HWM;

  vqs1_l = vqs1 & HWM;
  vqs1_r = vqs1 << 8;
  vqs1_r = vqs1_r & HWM;

  mask_l = mask & HWM;
  mask_r = mask << 8;
  mask_r = mask_r & HWM;

  hev_l = hev & HWM;
  hev_r = hev << 8;
  hev_r = hev_r & HWM;

  __asm__ __volatile__ (
      /* vp8_filter = vp8_signed_char_clamp(ps1 - qs1); */
      "subq_s.ph    %[vp8_filter_l], %[vps1_l],       %[vqs1_l]       \n\t"
      "subq_s.ph    %[vp8_filter_r], %[vps1_r],       %[vqs1_r]       \n\t"

      /* qs0 - ps0 */
      "subq_s.ph    %[subr_l],       %[vqs0_l],       %[vps0_l]       \n\t"
      "subq_s.ph    %[subr_r],       %[vqs0_r],       %[vps0_r]       \n\t"

      /* vp8_filter &= hev; */
      "and          %[vp8_filter_l], %[vp8_filter_l], %[hev_l]        \n\t"
      "and          %[vp8_filter_r], %[vp8_filter_r], %[hev_r]        \n\t"

      /* vp8_filter = vp8_signed_char_clamp(vp8_filter + 3 * (qs0 - ps0)); */
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"
      "xor          %[invhev_l],     %[hev_l],        %[HWM]          \n\t"
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"
      "xor          %[invhev_r],     %[hev_r],        %[HWM]          \n\t"
      "addq_s.ph    %[vp8_filter_l], %[vp8_filter_l], %[subr_l]       \n\t"
      "addq_s.ph    %[vp8_filter_r], %[vp8_filter_r], %[subr_r]       \n\t"

      /* vp8_filter &= mask; */
      "and          %[vp8_filter_l], %[vp8_filter_l], %[mask_l]       \n\t"
      "and          %[vp8_filter_r], %[vp8_filter_r], %[mask_r]       \n\t"

      : [vp8_filter_l] "=&r" (vp8_filter_l), [vp8_filter_r] "=&r" (vp8_filter_r),
        [subr_l] "=&r" (subr_l), [subr_r] "=&r" (subr_r),
        [invhev_l] "=&r" (invhev_l), [invhev_r] "=&r" (invhev_r)

      : [vps0_l] "r" (vps0_l), [vps0_r] "r" (vps0_r), [vps1_l] "r" (vps1_l),
        [vps1_r] "r" (vps1_r), [vqs0_l] "r" (vqs0_l), [vqs0_r] "r" (vqs0_r),
        [vqs1_l] "r" (vqs1_l), [vqs1_r] "r" (vqs1_r),
        [mask_l] "r" (mask_l), [mask_r] "r" (mask_r),
        [hev_l] "r" (hev_l), [hev_r] "r" (hev_r),
        [HWM] "r" (HWM)
  );

  /* save bottom 3 bits so that we round one side +4 and the other +3 */
  __asm__ __volatile__ (
      /* Filter2 = vp8_signed_char_clamp(vp8_filter + 3) >>= 3; */
      "addq_s.ph    %[Filter1_l],    %[vp8_filter_l], %[t2]           \n\t"
      "addq_s.ph    %[Filter1_r],    %[vp8_filter_r], %[t2]           \n\t"

      /* Filter1 = vp8_signed_char_clamp(vp8_filter + 4) >>= 3; */
      "addq_s.ph    %[Filter2_l],    %[vp8_filter_l], %[t1]           \n\t"
      "addq_s.ph    %[Filter2_r],    %[vp8_filter_r], %[t1]           \n\t"
      "shra.ph      %[Filter1_r],    %[Filter1_r],    3               \n\t"
      "shra.ph      %[Filter1_l],    %[Filter1_l],    3               \n\t"

      "shra.ph      %[Filter2_l],    %[Filter2_l],    3               \n\t"
      "shra.ph      %[Filter2_r],    %[Filter2_r],    3               \n\t"

      "and          %[Filter1_l],    %[Filter1_l],    %[HWM]          \n\t"
      "and          %[Filter1_r],    %[Filter1_r],    %[HWM]          \n\t"

      /* vps0 = vp8_signed_char_clamp(ps0 + Filter2); */
      "addq_s.ph    %[vps0_l],       %[vps0_l],       %[Filter2_l]    \n\t"
      "addq_s.ph    %[vps0_r],       %[vps0_r],       %[Filter2_r]    \n\t"

      /* vqs0 = vp8_signed_char_clamp(qs0 - Filter1); */
      "subq_s.ph    %[vqs0_l],       %[vqs0_l],       %[Filter1_l]    \n\t"
      "subq_s.ph    %[vqs0_r],       %[vqs0_r],       %[Filter1_r]    \n\t"

      : [Filter1_l] "=&r" (Filter1_l), [Filter1_r] "=&r" (Filter1_r),
        [Filter2_l] "=&r" (Filter2_l), [Filter2_r] "=&r" (Filter2_r),
        [vps0_l] "+r" (vps0_l), [vps0_r] "+r" (vps0_r),
        [vqs0_l] "+r" (vqs0_l), [vqs0_r] "+r" (vqs0_r)

      : [t1] "r" (t1), [t2] "r" (t2),
        [vp8_filter_l] "r" (vp8_filter_l), [vp8_filter_r] "r" (vp8_filter_r),
        [HWM] "r" (HWM)
  );

  __asm__ __volatile__ (
      /* (vp8_filter += 1) >>= 1 */
      "addqh.ph    %[Filter1_l],    %[Filter1_l],     %[t3]           \n\t"
      "addqh.ph    %[Filter1_r],    %[Filter1_r],     %[t3]           \n\t"

      /* vp8_filter &= ~hev; */
      "and          %[Filter1_l],    %[Filter1_l],    %[invhev_l]     \n\t"
      "and          %[Filter1_r],    %[Filter1_r],    %[invhev_r]     \n\t"

      /* vps1 = vp8_signed_char_clamp(ps1 + vp8_filter); */
      "addq_s.ph    %[vps1_l],       %[vps1_l],       %[Filter1_l]    \n\t"
      "addq_s.ph    %[vps1_r],       %[vps1_r],       %[Filter1_r]    \n\t"

      /* vqs1 = vp8_signed_char_clamp(qs1 - vp8_filter); */
      "subq_s.ph    %[vqs1_l],       %[vqs1_l],       %[Filter1_l]    \n\t"
      "subq_s.ph    %[vqs1_r],       %[vqs1_r],       %[Filter1_r]    \n\t"

      : [Filter1_l] "+r" (Filter1_l), [Filter1_r] "+r" (Filter1_r),
        [vps1_l] "+r" (vps1_l), [vps1_r] "+r" (vps1_r),
        [vqs1_l] "+r" (vqs1_l), [vqs1_r] "+r" (vqs1_r)

      : [t3] "r" (t3), [invhev_l] "r" (invhev_l), [invhev_r] "r" (invhev_r)
  );

  /* Create quad-bytes from halfword pairs */
  vqs0_l = vqs0_l & HWM;
  vqs1_l = vqs1_l & HWM;
  vps0_l = vps0_l & HWM;
  vps1_l = vps1_l & HWM;

  __asm__ __volatile__ (
      "shrl.ph      %[vqs0_r],       %[vqs0_r],       8   \n\t"
      "shrl.ph      %[vps0_r],       %[vps0_r],       8   \n\t"
      "shrl.ph      %[vqs1_r],       %[vqs1_r],       8   \n\t"
      "shrl.ph      %[vps1_r],       %[vps1_r],       8   \n\t"

      : [vps1_r] "+r" (vps1_r), [vqs1_r] "+r" (vqs1_r),
        [vps0_r] "+r" (vps0_r), [vqs0_r] "+r" (vqs0_r)
      :
  );

  vqs0 = vqs0_l | vqs0_r;
  vqs1 = vqs1_l | vqs1_r;
  vps0 = vps0_l | vps0_r;
  vps1 = vps1_l | vps1_r;

  *p0_f0 = vps0 ^ N128;
  *p1_f0 = vps1 ^ N128;
  *q0_f0 = vqs0 ^ N128;
  *q1_f0 = vqs1 ^ N128;
}

static INLINE void mbfilter_dspr2(uint32_t *op3, uint32_t *op2,
                                  uint32_t *op1, uint32_t *op0,
                                  uint32_t *oq0, uint32_t *oq1,
                                  uint32_t *oq2, uint32_t *oq3) {
  /* use a 7 tap filter [1, 1, 1, 2, 1, 1, 1] for flat line */
  const uint32_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
  const uint32_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;
  uint32_t res_op2;
  uint32_t res_op1;
  uint32_t res_op0;
  uint32_t res_oq0;
  uint32_t res_oq1;
  uint32_t res_oq2;
  uint32_t tmp;
  uint32_t add_p210_q012;
  uint32_t u32Four = 0x00040004;

  /* *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + p2 + p2 + p1 + p0 + q0, 3)       1 */
  /* *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + p1 + p1 + p0 + q0 + q1, 3)       2 */
  /* *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 + p0 + q0 + q1 + q2, 3)       3 */
  /* *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + q0 + q0 + q1 + q2 + q3, 3)       4 */
  /* *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + q1 + q1 + q2 + q3 + q3, 3)       5 */
  /* *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + q2 + q2 + q3 + q3 + q3, 3)       6 */

  __asm__ __volatile__ (
      "addu.ph      %[add_p210_q012],   %[p2],              %[p1]           \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],   %[p0]           \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],   %[q0]           \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],   %[q1]           \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],   %[q2]           \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],   %[u32Four]      \n\t"

      "shll.ph      %[tmp],             %[p3],              1                \n\t" /* 1 */
      "addu.ph      %[res_op2],         %[tmp],             %[p3]            \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[p3],              %[p3]            \n\t" /* 2 */
      "addu.ph      %[res_op2],         %[res_op2],         %[p2]            \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[res_op1],         %[p1]            \n\t" /* 2 */
      "addu.ph      %[res_op2],         %[res_op2],         %[add_p210_q012] \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[res_op1],         %[add_p210_q012] \n\t" /* 2 */
      "subu.ph      %[res_op2],         %[res_op2],         %[q1]            \n\t" /* 1 */
      "subu.ph      %[res_op1],         %[res_op1],         %[q2]            \n\t" /* 2 */
      "subu.ph      %[res_op2],         %[res_op2],         %[q2]            \n\t" /* 1 */
      "shrl.ph      %[res_op1],         %[res_op1],         3                \n\t" /* 2 */
      "shrl.ph      %[res_op2],         %[res_op2],         3                \n\t" /* 1 */
      "addu.ph      %[res_op0],         %[p3],              %[p0]            \n\t" /* 3 */
      "addu.ph      %[res_oq0],         %[q0],              %[q3]            \n\t" /* 4 */
      "addu.ph      %[res_op0],         %[res_op0],         %[add_p210_q012] \n\t" /* 3 */
      "addu.ph      %[res_oq0],         %[res_oq0],         %[add_p210_q012] \n\t" /* 4 */
      "addu.ph      %[res_oq1],         %[q3],              %[q3]            \n\t" /* 5 */
      "shll.ph      %[tmp],             %[q3],              1                \n\t" /* 6 */
      "addu.ph      %[res_oq1],         %[res_oq1],         %[q1]            \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[tmp],             %[q3]            \n\t" /* 6 */
      "addu.ph      %[res_oq1],         %[res_oq1],         %[add_p210_q012] \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[res_oq2],         %[add_p210_q012] \n\t" /* 6 */
      "subu.ph      %[res_oq1],         %[res_oq1],         %[p2]            \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[res_oq2],         %[q2]            \n\t" /* 6 */
      "shrl.ph      %[res_oq1],         %[res_oq1],         3                \n\t" /* 5 */
      "subu.ph      %[res_oq2],         %[res_oq2],         %[p2]            \n\t" /* 6 */
      "shrl.ph      %[res_oq0],         %[res_oq0],         3                \n\t" /* 4 */
      "subu.ph      %[res_oq2],         %[res_oq2],         %[p1]            \n\t" /* 6 */
      "shrl.ph      %[res_op0],         %[res_op0],         3                \n\t" /* 3 */
      "shrl.ph      %[res_oq2],         %[res_oq2],         3                \n\t" /* 6 */

      : [add_p210_q012] "=&r" (add_p210_q012),
        [tmp] "=&r" (tmp), [res_op2] "=&r" (res_op2),
        [res_op1] "=&r" (res_op1), [res_op0] "=&r" (res_op0),
        [res_oq0] "=&r" (res_oq0), [res_oq1] "=&r" (res_oq1),
        [res_oq2] "=&r" (res_oq2)
      : [p0] "r" (p0), [q0] "r" (q0), [p1] "r" (p1), [q1] "r" (q1),
        [p2] "r" (p2), [q2] "r" (q2), [p3] "r" (p3), [q3] "r" (q3),
        [u32Four] "r" (u32Four)
  );

  *op2 = res_op2;
  *op1 = res_op1;
  *op0 = res_op0;
  *oq0 = res_oq0;
  *oq1 = res_oq1;
  *oq2 = res_oq2;
}

static INLINE void mbfilter1_dspr2(uint32_t p3, uint32_t p2,
                                   uint32_t p1, uint32_t p0,
                                   uint32_t q0, uint32_t q1,
                                   uint32_t q2, uint32_t q3,
                                   uint32_t *op2_f1,
                                   uint32_t *op1_f1, uint32_t *op0_f1,
                                   uint32_t *oq0_f1, uint32_t *oq1_f1,
                                   uint32_t *oq2_f1) {
  /* use a 7 tap filter [1, 1, 1, 2, 1, 1, 1] for flat line */
  uint32_t res_op2;
  uint32_t res_op1;
  uint32_t res_op0;
  uint32_t res_oq0;
  uint32_t res_oq1;
  uint32_t res_oq2;
  uint32_t tmp;
  uint32_t add_p210_q012;
  uint32_t u32Four = 0x00040004;

  /* *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + p2 + p2 + p1 + p0 + q0, 3)       1 */
  /* *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + p1 + p1 + p0 + q0 + q1, 3)       2 */
  /* *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 + p0 + q0 + q1 + q2, 3)       3 */
  /* *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + q0 + q0 + q1 + q2 + q3, 3)       4 */
  /* *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + q1 + q1 + q2 + q3 + q3, 3)       5 */
  /* *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + q2 + q2 + q3 + q3 + q3, 3)       6 */

  __asm__ __volatile__ (
      "addu.ph      %[add_p210_q012],   %[p2],                  %[p1]               \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],       %[p0]               \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],       %[q0]               \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],       %[q1]               \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],       %[q2]               \n\t"
      "addu.ph      %[add_p210_q012],   %[add_p210_q012],       %[u32Four]          \n\t"

      "shll.ph      %[tmp],             %[p3],                  1                   \n\t" /* 1 */
      "addu.ph      %[res_op2],         %[tmp],                 %[p3]               \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[p3],                  %[p3]               \n\t" /* 2 */
      "addu.ph      %[res_op2],         %[res_op2],             %[p2]               \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[res_op1],             %[p1]               \n\t" /* 2 */
      "addu.ph      %[res_op2],         %[res_op2],             %[add_p210_q012]    \n\t" /* 1 */
      "addu.ph      %[res_op1],         %[res_op1],             %[add_p210_q012]    \n\t" /* 2 */
      "subu.ph      %[res_op2],         %[res_op2],             %[q1]               \n\t" /* 1 */
      "subu.ph      %[res_op1],         %[res_op1],             %[q2]               \n\t" /* 2 */
      "subu.ph      %[res_op2],         %[res_op2],             %[q2]               \n\t" /* 1 */
      "shrl.ph      %[res_op1],         %[res_op1],             3                   \n\t" /* 2 */
      "shrl.ph      %[res_op2],         %[res_op2],             3                   \n\t" /* 1 */
      "addu.ph      %[res_op0],         %[p3],                  %[p0]               \n\t" /* 3 */
      "addu.ph      %[res_oq0],         %[q0],                  %[q3]               \n\t" /* 4 */
      "addu.ph      %[res_op0],         %[res_op0],             %[add_p210_q012]    \n\t" /* 3 */
      "addu.ph      %[res_oq0],         %[res_oq0],             %[add_p210_q012]    \n\t" /* 4 */
      "addu.ph      %[res_oq1],         %[q3],                  %[q3]               \n\t" /* 5 */
      "shll.ph      %[tmp],             %[q3],                  1                   \n\t" /* 6 */
      "addu.ph      %[res_oq1],         %[res_oq1],             %[q1]               \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[tmp],                 %[q3]               \n\t" /* 6 */
      "addu.ph      %[res_oq1],         %[res_oq1],             %[add_p210_q012]    \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[res_oq2],             %[add_p210_q012]    \n\t" /* 6 */
      "subu.ph      %[res_oq1],         %[res_oq1],             %[p2]               \n\t" /* 5 */
      "addu.ph      %[res_oq2],         %[res_oq2],             %[q2]               \n\t" /* 6 */
      "shrl.ph      %[res_oq1],         %[res_oq1],             3                   \n\t" /* 5 */
      "subu.ph      %[res_oq2],         %[res_oq2],             %[p2]               \n\t" /* 6 */
      "shrl.ph      %[res_oq0],         %[res_oq0],             3                   \n\t" /* 4 */
      "subu.ph      %[res_oq2],         %[res_oq2],             %[p1]               \n\t" /* 6 */
      "shrl.ph      %[res_op0],         %[res_op0],             3                   \n\t" /* 3 */
      "shrl.ph      %[res_oq2],         %[res_oq2],             3                   \n\t" /* 6 */

      : [add_p210_q012] "=&r" (add_p210_q012),
        [tmp] "=&r" (tmp), [res_op2] "=&r" (res_op2),
        [res_op1] "=&r" (res_op1), [res_op0] "=&r" (res_op0),
        [res_oq0] "=&r" (res_oq0), [res_oq1] "=&r" (res_oq1),
        [res_oq2] "=&r" (res_oq2)
      : [p0] "r" (p0), [q0] "r" (q0), [p1] "r" (p1), [q1] "r" (q1),
        [p2] "r" (p2), [q2] "r" (q2), [p3] "r" (p3), [q3] "r" (q3),
        [u32Four] "r" (u32Four)
  );

  *op2_f1 = res_op2;
  *op1_f1 = res_op1;
  *op0_f1 = res_op0;
  *oq0_f1 = res_oq0;
  *oq1_f1 = res_oq1;
  *oq2_f1 = res_oq2;
}

static __inline void flatmask5(uint32_t p4, uint32_t p3,
                               uint32_t p2, uint32_t p1,
                               uint32_t p0, uint32_t q0,
                               uint32_t q1, uint32_t q2,
                               uint32_t q3, uint32_t q4,
                               uint32_t *flat2) {

  uint32_t c, r, r_k, r_flat;
  uint32_t ones = 0xFFFFFFFF;
  uint32_t flat_thresh = 0x01010101;
  uint32_t flat1, flat3;

  __asm__ __volatile__ (
      /* flat |= (abs(p4 - p0) > thresh) */
      "subu_s.qb      %[c],   %[p4],           %[p0]        \n\t"
      "subu_s.qb      %[r_k], %[p0],           %[p4]        \n\t"
      "or             %[r_k], %[r_k],          %[c]         \n\t"
      "cmpgu.lt.qb    %[c],   %[flat_thresh],  %[r_k]       \n\t"
      "or             %[r],   $0,              %[c]         \n\t"

      /* flat |= (abs(q4 - q0) > thresh) */
      "subu_s.qb      %[c],     %[q4],           %[q0]     \n\t"
      "subu_s.qb      %[r_k],   %[q0],           %[q4]     \n\t"
      "or             %[r_k],   %[r_k],          %[c]      \n\t"
      "cmpgu.lt.qb    %[c],     %[flat_thresh],  %[r_k]    \n\t"
      "or             %[r],     %[r],            %[c]      \n\t"
      "sll            %[r],     %[r],            24        \n\t"
      "wrdsp          %[r]                                 \n\t"
      "pick.qb        %[flat3], $0,           %[ones]      \n\t"

      /* flat |= (abs(p1 - p0) > thresh) */
      "subu_s.qb      %[c],       %[p1],          %[p0]        \n\t"
      "subu_s.qb      %[r_k],     %[p0],          %[p1]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  $0,             %[c]         \n\t"

      /* flat |= (abs(q1 - q0) > thresh) */
      "subu_s.qb      %[c],      %[q1],           %[q0]        \n\t"
      "subu_s.qb      %[r_k],    %[q0],           %[q1]        \n\t"
      "or             %[r_k],    %[r_k],          %[c]         \n\t"
      "cmpgu.lt.qb    %[c],      %[flat_thresh],  %[r_k]       \n\t"
      "or             %[r_flat], %[r_flat],       %[c]         \n\t"

      /* flat |= (abs(p0 - p2) > thresh) */
      "subu_s.qb      %[c],       %[p0],          %[p2]        \n\t"
      "subu_s.qb      %[r_k],     %[p2],          %[p0]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(q0 - q2) > thresh) */
      "subu_s.qb      %[c],       %[q0],          %[q2]        \n\t"
      "subu_s.qb      %[r_k],     %[q2],          %[q0]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(p3 - p0) > thresh) */
      "subu_s.qb      %[c],       %[p3],          %[p0]        \n\t"
      "subu_s.qb      %[r_k],     %[p0],          %[p3]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"

      /* flat |= (abs(q3 - q0) > thresh) */
      "subu_s.qb      %[c],       %[q3],          %[q0]        \n\t"
      "subu_s.qb      %[r_k],     %[q0],          %[q3]        \n\t"
      "or             %[r_k],     %[r_k],         %[c]         \n\t"
      "cmpgu.lt.qb    %[c],       %[flat_thresh], %[r_k]       \n\t"
      "or             %[r_flat],  %[r_flat],      %[c]         \n\t"
      "sll            %[r_flat],  %[r_flat],      24           \n\t"
      "wrdsp          %[r_flat]                                \n\t"
      "pick.qb        %[flat1],   $0,             %[ones]      \n\t"
      /* flat & flatmask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3) */
      "and            %[flat1],  %[flat3],        %[flat1]     \n\t"

      : [c] "=&r" (c), [r_k] "=&r" (r_k),
        [r] "=&r" (r), [r_flat] "=&r" (r_flat),
        [flat1] "=&r" (flat1), [flat3] "=&r" (flat3)
      : [p4] "r" (p4), [p3] "r" (p3), [p2] "r" (p2),
        [p1] "r" (p1), [p0] "r" (p0), [q0] "r" (q0), [q1] "r" (q1),
        [q2] "r" (q2), [q3] "r" (q3), [q4] "r" (q4),
        [flat_thresh] "r" (flat_thresh), [ones] "r" (ones)
  );

  *flat2 = flat1;
}

static INLINE void wide_mbfilter_dspr2(uint32_t *op7, uint32_t *op6,
                                       uint32_t *op5, uint32_t *op4,
                                       uint32_t *op3, uint32_t *op2,
                                       uint32_t *op1, uint32_t *op0,
                                       uint32_t *oq0, uint32_t *oq1,
                                       uint32_t *oq2, uint32_t *oq3,
                                       uint32_t *oq4, uint32_t *oq5,
                                       uint32_t *oq6, uint32_t *oq7) {
  const uint32_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4;
  const uint32_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
  const uint32_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;
  const uint32_t q4 = *oq4, q5 = *oq5, q6 = *oq6, q7 = *oq7;
  uint32_t res_op6;
  uint32_t res_op5;
  uint32_t res_op4;
  uint32_t res_op3;
  uint32_t res_op2;
  uint32_t res_op1;
  uint32_t res_op0;
  uint32_t res_oq0;
  uint32_t res_oq1;
  uint32_t res_oq2;
  uint32_t res_oq3;
  uint32_t res_oq4;
  uint32_t res_oq5;
  uint32_t res_oq6;
  uint32_t tmp;
  uint32_t add_p6toq6;
  uint32_t u32Eight = 0x00080008;

  __asm__ __volatile__ (
      /* addition of p6,p5,p4,p3,p2,p1,p0,q0,q1,q2,q3,q4,q5,q6 which is used most of the time */
      "addu.ph      %[add_p6toq6],      %[p6],              %[p5]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[p4]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[p3]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[p2]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[p1]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[p0]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q0]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q1]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q2]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q3]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q4]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q5]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[q6]           \n\t"
      "addu.ph      %[add_p6toq6],      %[add_p6toq6],      %[u32Eight]     \n\t"

      : [add_p6toq6] "=&r" (add_p6toq6)
      : [p6] "r" (p6), [p5] "r" (p5), [p4] "r" (p4),
        [p3] "r" (p3), [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),
        [q0] "r" (q0), [q1] "r" (q1), [q2] "r" (q2), [q3] "r" (q3),
        [q4] "r" (q4), [q5] "r" (q5), [q6] "r" (q6),
        [u32Eight] "r" (u32Eight)
  );

  __asm__ __volatile__ (
      /* *op6 = ROUND_POWER_OF_TWO(p7 * 7 + p6 * 2 + p5 + p4 +
                                   p3 + p2 + p1 + p0 + q0, 4) */
      "shll.ph       %[tmp],            %[p7],              3               \n\t"
      "subu.ph       %[res_op6],        %[tmp],             %[p7]           \n\t"
      "addu.ph       %[res_op6],        %[res_op6],         %[p6]           \n\t"
      "addu.ph       %[res_op6],        %[res_op6],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q1]           \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q2]           \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q3]           \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q4]           \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q5]           \n\t"
      "subu.ph       %[res_op6],        %[res_op6],         %[q6]           \n\t"
      "shrl.ph       %[res_op6],        %[res_op6],         4               \n\t"

      /* *op5 = ROUND_POWER_OF_TWO(p7 * 6 + p6 + p5 * 2 + p4 + p3 +
                                   p2 + p1 + p0 + q0 + q1, 4) */
      "shll.ph       %[tmp],            %[p7],              2               \n\t"
      "addu.ph       %[res_op5],        %[tmp],             %[p7]           \n\t"
      "addu.ph       %[res_op5],        %[res_op5],         %[p7]           \n\t"
      "addu.ph       %[res_op5],        %[res_op5],         %[p5]           \n\t"
      "addu.ph       %[res_op5],        %[res_op5],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op5],        %[res_op5],         %[q2]           \n\t"
      "subu.ph       %[res_op5],        %[res_op5],         %[q3]           \n\t"
      "subu.ph       %[res_op5],        %[res_op5],         %[q4]           \n\t"
      "subu.ph       %[res_op5],        %[res_op5],         %[q5]           \n\t"
      "subu.ph       %[res_op5],        %[res_op5],         %[q6]           \n\t"
      "shrl.ph       %[res_op5],        %[res_op5],         4               \n\t"

      /* *op4 = ROUND_POWER_OF_TWO(p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 +
                                   p1 + p0 + q0 + q1 + q2, 4) */
      "shll.ph       %[tmp],            %[p7],              2               \n\t"
      "addu.ph       %[res_op4],        %[tmp],             %[p7]           \n\t"
      "addu.ph       %[res_op4],        %[res_op4],         %[p4]           \n\t"
      "addu.ph       %[res_op4],        %[res_op4],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op4],        %[res_op4],         %[q3]           \n\t"
      "subu.ph       %[res_op4],        %[res_op4],         %[q4]           \n\t"
      "subu.ph       %[res_op4],        %[res_op4],         %[q5]           \n\t"
      "subu.ph       %[res_op4],        %[res_op4],         %[q6]           \n\t"
      "shrl.ph       %[res_op4],        %[res_op4],         4               \n\t"

      /* *op3 = ROUND_POWER_OF_TWO(p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 +
                                   p1 + p0 + q0 + q1 + q2 + q3, 4) */
      "shll.ph       %[tmp],            %[p7],              2               \n\t"
      "addu.ph       %[res_op3],        %[tmp],             %[p3]           \n\t"
      "addu.ph       %[res_op3],        %[res_op3],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op3],        %[res_op3],         %[q4]           \n\t"
      "subu.ph       %[res_op3],        %[res_op3],         %[q5]           \n\t"
      "subu.ph       %[res_op3],        %[res_op3],         %[q6]           \n\t"
      "shrl.ph       %[res_op3],        %[res_op3],         4               \n\t"

      /* *op2 = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4, 4) */
      "shll.ph       %[tmp],            %[p7],              1               \n\t"
      "addu.ph       %[res_op2],        %[tmp],             %[p7]           \n\t"
      "addu.ph       %[res_op2],        %[res_op2],         %[p2]           \n\t"
      "addu.ph       %[res_op2],        %[res_op2],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op2],        %[res_op2],         %[q5]           \n\t"
      "subu.ph       %[res_op2],        %[res_op2],         %[q6]           \n\t"
      "shrl.ph       %[res_op2],        %[res_op2],         4               \n\t"

      /* *op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5, 4); */
      "shll.ph       %[tmp],            %[p7],              1               \n\t"
      "addu.ph       %[res_op1],        %[tmp],             %[p1]           \n\t"
      "addu.ph       %[res_op1],        %[res_op1],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_op1],        %[res_op1],         %[q6]           \n\t"
      "shrl.ph       %[res_op1],        %[res_op1],         4               \n\t"

      /* *op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                                  q0 + q1 + q2 + q3 + q4 + q5 + q6, 4) */
      "addu.ph       %[res_op0],        %[p7],          %[p0]               \n\t"
      "addu.ph       %[res_op0],        %[res_op0],     %[add_p6toq6]       \n\t"
      "shrl.ph       %[res_op0],        %[res_op0],     4                   \n\t"

      : [res_op6] "=&r" (res_op6), [res_op5] "=&r" (res_op5),
        [res_op4] "=&r" (res_op4), [res_op3] "=&r" (res_op3),
        [res_op2] "=&r" (res_op2), [res_op1] "=&r" (res_op1),
        [res_op0] "=&r" (res_op0), [tmp] "=&r" (tmp)
      : [p7] "r" (p7), [p6] "r" (p6), [p5] "r" (p5), [p4] "r" (p4),
        [p3] "r" (p3), [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),
        [q2] "r" (q2), [q1] "r" (q1),
        [q3] "r" (q3), [q4] "r" (q4), [q5] "r" (q5), [q6] "r" (q6),
        [add_p6toq6] "r" (add_p6toq6)
  );

  *op6 = res_op6;
  *op5 = res_op5;
  *op4 = res_op4;
  *op3 = res_op3;
  *op2 = res_op2;
  *op1 = res_op1;
  *op0 = res_op0;

  __asm__ __volatile__ (
      /* *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 + q7, 4); */
      "addu.ph       %[res_oq0],        %[q7],              %[q0]           \n\t"
      "addu.ph       %[res_oq0],        %[res_oq0],         %[add_p6toq6]   \n\t"
      "shrl.ph       %[res_oq0],        %[res_oq0],         4               \n\t"

      /* *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 + q0 + q1 * 2 +
                                   q2 + q3 + q4 + q5 + q6 + q7 * 2, 4) */
      "shll.ph       %[tmp],            %[q7],              1               \n\t"
      "addu.ph       %[res_oq1],        %[tmp],             %[q1]           \n\t"
      "addu.ph       %[res_oq1],        %[res_oq1],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq1],        %[res_oq1],         %[p6]           \n\t"
      "shrl.ph       %[res_oq1],        %[res_oq1],         4               \n\t"

      /* *oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 + q0 + q1 + q2 * 2 +
                                   q3 + q4 + q5 + q6 + q7 * 3, 4) */
      "shll.ph       %[tmp],            %[q7],              1               \n\t"
      "addu.ph       %[res_oq2],        %[tmp],             %[q7]           \n\t"
      "addu.ph       %[res_oq2],        %[res_oq2],         %[q2]           \n\t"
      "addu.ph       %[res_oq2],        %[res_oq2],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq2],        %[res_oq2],         %[p5]           \n\t"
      "subu.ph       %[res_oq2],        %[res_oq2],         %[p6]           \n\t"
      "shrl.ph       %[res_oq2],        %[res_oq2],         4               \n\t"

      /* *oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 + q0 + q1 + q2 +
                                   q3 * 2 + q4 + q5 + q6 + q7 * 4, 4) */
      "shll.ph       %[tmp],            %[q7],              2               \n\t"
      "addu.ph       %[res_oq3],        %[tmp],             %[q3]           \n\t"
      "addu.ph       %[res_oq3],        %[res_oq3],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq3],        %[res_oq3],         %[p4]           \n\t"
      "subu.ph       %[res_oq3],        %[res_oq3],         %[p5]           \n\t"
      "subu.ph       %[res_oq3],        %[res_oq3],         %[p6]           \n\t"
      "shrl.ph       %[res_oq3],        %[res_oq3],         4               \n\t"

      /* *oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + q0 + q1 + q2 + q3 +
                                   q4 * 2 + q5 + q6 + q7 * 5, 4) */
      "shll.ph       %[tmp],            %[q7],              2               \n\t"
      "addu.ph       %[res_oq4],        %[tmp],             %[q7]           \n\t"
      "addu.ph       %[res_oq4],        %[res_oq4],         %[q4]           \n\t"
      "addu.ph       %[res_oq4],        %[res_oq4],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq4],        %[res_oq4],         %[p3]           \n\t"
      "subu.ph       %[res_oq4],        %[res_oq4],         %[p4]           \n\t"
      "subu.ph       %[res_oq4],        %[res_oq4],         %[p5]           \n\t"
      "subu.ph       %[res_oq4],        %[res_oq4],         %[p6]           \n\t"
      "shrl.ph       %[res_oq4],        %[res_oq4],         4               \n\t"

      /* *oq5 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + q1 + q2 + q3 + q4 +
                                   q5 * 2 + q6 + q7 * 6, 4) */
      "shll.ph       %[tmp],            %[q7],              2               \n\t"
      "addu.ph       %[res_oq5],        %[tmp],             %[q7]           \n\t"
      "addu.ph       %[res_oq5],        %[res_oq5],         %[q7]           \n\t"
      "addu.ph       %[res_oq5],        %[res_oq5],         %[q5]           \n\t"
      "addu.ph       %[res_oq5],        %[res_oq5],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq5],        %[res_oq5],         %[p2]           \n\t"
      "subu.ph       %[res_oq5],        %[res_oq5],         %[p3]           \n\t"
      "subu.ph       %[res_oq5],        %[res_oq5],         %[p4]           \n\t"
      "subu.ph       %[res_oq5],        %[res_oq5],         %[p5]           \n\t"
      "subu.ph       %[res_oq5],        %[res_oq5],         %[p6]           \n\t"
      "shrl.ph       %[res_oq5],        %[res_oq5],         4               \n\t"

      /* *oq6 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + q2 + q3 +
                                   q4 + q5 + q6 * 2 + q7 * 7, 4) */
      "shll.ph       %[tmp],            %[q7],              3               \n\t"
      "subu.ph       %[res_oq6],        %[tmp],             %[q7]           \n\t"
      "addu.ph       %[res_oq6],        %[res_oq6],         %[q6]           \n\t"
      "addu.ph       %[res_oq6],        %[res_oq6],         %[add_p6toq6]   \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p1]           \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p2]           \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p3]           \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p4]           \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p5]           \n\t"
      "subu.ph       %[res_oq6],        %[res_oq6],         %[p6]           \n\t"
      "shrl.ph       %[res_oq6],        %[res_oq6],         4               \n\t"

      : [res_oq6] "=&r" (res_oq6), [res_oq5] "=&r" (res_oq5),
        [res_oq4] "=&r" (res_oq4), [res_oq3] "=&r" (res_oq3),
        [res_oq2] "=&r" (res_oq2), [res_oq1] "=&r" (res_oq1),
        [res_oq0] "=&r" (res_oq0), [tmp] "=&r" (tmp)
      : [q7] "r" (q7), [q6] "r" (q6), [q5] "r" (q5), [q4] "r" (q4),
        [q3] "r" (q3), [q2] "r" (q2), [q1] "r" (q1), [q0] "r" (q0),
        [p1] "r" (p1), [p2] "r" (p2),
        [p3] "r" (p3), [p4] "r" (p4), [p5] "r" (p5), [p6] "r" (p6),
        [add_p6toq6] "r" (add_p6toq6)
  );

  *oq0 = res_oq0;
  *oq1 = res_oq1;
  *oq2 = res_oq2;
  *oq3 = res_oq3;
  *oq4 = res_oq4;
  *oq5 = res_oq5;
  *oq6 = res_oq6;
}

void vp9_mb_lpf_horizontal_edge_w_dspr2(unsigned char *s,
                                        int pitch,
                                        const uint8_t *blimit,
                                        const uint8_t *limit,
                                        const uint8_t *thresh,
                                        int count) {
  uint32_t mask;
  uint32_t hev,flat,flat2;
  uint8_t  i;
  unsigned char *sp7, *sp6, *sp5, *sp4, *sp3, *sp2, *sp1, *sp0;
  unsigned char *sq0, *sq1, *sq2, *sq3, *sq4, *sq5, *sq6, *sq7;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;
  uint32_t p7, p6, p5, p4, p3, p2, p1, p0, q0, q1, q2, q3, q4, q5, q6, q7;
  uint32_t p1_f0, p0_f0, q0_f0, q1_f0;
  uint32_t p7_l, p6_l, p5_l, p4_l, p3_l, p2_l, p1_l, p0_l,
           q0_l, q1_l, q2_l, q3_l, q4_l, q5_l, q6_l, q7_l;
  uint32_t p7_r, p6_r, p5_r, p4_r, p3_r, p2_r, p1_r, p0_r,
           q0_r, q1_r, q2_r, q3_r, q4_r, q5_r, q6_r, q7_r;
  uint32_t p2_l_f1, p1_l_f1, p0_l_f1, p2_r_f1, p1_r_f1, p0_r_f1;
  uint32_t q0_l_f1, q1_l_f1, q2_l_f1, q0_r_f1, q1_r_f1, q2_r_f1;

  uflimit = *blimit;
  ulimit  = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb       %[thresh_vec],    %[uthresh]      \n\t"
      "replv.qb       %[flimit_vec],    %[uflimit]      \n\t"
      "replv.qb       %[limit_vec],     %[ulimit]       \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  /* prefetch data for store */
  vp9_prefetch_store(s);

  for(i = 0; i < (2 * count); i++) {
    sp7 = s - (pitch << 3);
    sp6 = sp7 + pitch;
    sp5 = sp6 + pitch;
    sp4 = sp5 + pitch;
    sp3 = sp4 + pitch;
    sp2 = sp3 + pitch;
    sp1 = sp2 + pitch;
    sp0 = sp1 + pitch;
    sq0 = s;
    sq1 = s + pitch;
    sq2 = sq1 + pitch;
    sq3 = sq2 + pitch;
    sq4 = sq3 + pitch;
    sq5 = sq4 + pitch;
    sq6 = sq5 + pitch;
    sq7 = sq6 + pitch;

    __asm__ __volatile__ (
        "lw     %[p7],      (%[sp7])            \n\t"
        "lw     %[p6],      (%[sp6])            \n\t"
        "lw     %[p5],      (%[sp5])            \n\t"
        "lw     %[p4],      (%[sp4])            \n\t"
        "lw     %[p3],      (%[sp3])            \n\t"
        "lw     %[p2],      (%[sp2])            \n\t"
        "lw     %[p1],      (%[sp1])            \n\t"
        "lw     %[p0],      (%[sp0])            \n\t"

        : [p3] "=&r" (p3), [p2] "=&r" (p2), [p1] "=&r" (p1), [p0] "=&r" (p0),
          [p7] "=&r" (p7), [p6] "=&r" (p6), [p5] "=&r" (p5), [p4] "=&r" (p4)
        : [sp3] "r" (sp3), [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
          [sp4] "r" (sp4), [sp5] "r" (sp5), [sp6] "r" (sp6), [sp7] "r" (sp7)
    );

    __asm__ __volatile__ (
        "lw     %[q0],      (%[sq0])            \n\t"
        "lw     %[q1],      (%[sq1])            \n\t"
        "lw     %[q2],      (%[sq2])            \n\t"
        "lw     %[q3],      (%[sq3])            \n\t"
        "lw     %[q4],      (%[sq4])            \n\t"
        "lw     %[q5],      (%[sq5])            \n\t"
        "lw     %[q6],      (%[sq6])            \n\t"
        "lw     %[q7],      (%[sq7])            \n\t"

        : [q3] "=&r" (q3), [q2] "=&r" (q2), [q1] "=&r" (q1), [q0] "=&r" (q0),
          [q7] "=&r" (q7), [q6] "=&r" (q6), [q5] "=&r" (q5), [q4] "=&r" (q4)
        : [sq3] "r" (sq3), [sq2] "r" (sq2), [sq1] "r" (sq1), [sq0] "r" (sq0),
          [sq4] "r" (sq4), [sq5] "r" (sq5), [sq6] "r" (sq6), [sq7] "r" (sq7)
    );

    filter_hev_mask_flatmask4_dspr2(limit_vec, flimit_vec, thresh_vec,
                                    p1, p0, p3, p2, q0, q1, q2, q3,
                                    &hev, &mask, &flat);

    flatmask5(p7, p6, p5, p4, p0, q0, q4, q5 ,q6, q7, &flat2);

    /* f0 */
    if(((flat2 == 0) && (flat == 0) && (mask != 0)) ||
       ((flat2 != 0) && (flat == 0) && (mask != 0))) {

      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      __asm__ __volatile__ (
          "sw       %[p1_f0],   (%[sp1])            \n\t"
          "sw       %[p0_f0],   (%[sp0])            \n\t"
          "sw       %[q0_f0],   (%[sq0])            \n\t"
          "sw       %[q1_f0],   (%[sq1])            \n\t"

          :
          : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
            [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
            [sp1] "r" (sp1), [sp0] "r" (sp0),
            [sq0] "r" (sq0), [sq1] "r" (sq1)
      );
    } else if((flat2 == 0XFFFFFFFF) && (flat == 0xFFFFFFFF) && (mask == 0xFFFFFFFF)) {
      /* f2 */
      PACK_LEFT_0TO3
      PACK_LEFT_4TO7
      wide_mbfilter_dspr2(&p7_l, &p6_l, &p5_l, &p4_l,
                          &p3_l, &p2_l, &p1_l, &p0_l,
                          &q0_l, &q1_l, &q2_l, &q3_l,
                          &q4_l, &q5_l, &q6_l, &q7_l);

      PACK_RIGHT_0TO3
      PACK_RIGHT_4TO7
      wide_mbfilter_dspr2(&p7_r, &p6_r, &p5_r, &p4_r,
                          &p3_r, &p2_r, &p1_r, &p0_r,
                          &q0_r, &q1_r, &q2_r, &q3_r,
                          &q4_r, &q5_r, &q6_r, &q7_r);

      COMBINE_LEFT_RIGHT_0TO2
      COMBINE_LEFT_RIGHT_3TO6

      __asm__ __volatile__ (
          "sw         %[p6], (%[sp6])    \n\t"
          "sw         %[p5], (%[sp5])    \n\t"
          "sw         %[p4], (%[sp4])    \n\t"
          "sw         %[p3], (%[sp3])    \n\t"
          "sw         %[p2], (%[sp2])    \n\t"
          "sw         %[p1], (%[sp1])    \n\t"
          "sw         %[p0], (%[sp0])    \n\t"

          :
          : [p6] "r" (p6), [p5] "r" (p5), [p4] "r" (p4), [p3] "r" (p3),
            [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),
            [sp6] "r" (sp6), [sp5] "r" (sp5), [sp4] "r" (sp4), [sp3] "r" (sp3),
            [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0)
      );

      __asm__ __volatile__ (
          "sw         %[q6], (%[sq6])    \n\t"
          "sw         %[q5], (%[sq5])    \n\t"
          "sw         %[q4], (%[sq4])    \n\t"
          "sw         %[q3], (%[sq3])    \n\t"
          "sw         %[q2], (%[sq2])    \n\t"
          "sw         %[q1], (%[sq1])    \n\t"
          "sw         %[q0], (%[sq0])    \n\t"

          :
          : [q6] "r" (q6), [q5] "r" (q5), [q4] "r" (q4), [q3] "r" (q3),
            [q2] "r" (q2), [q1] "r" (q1), [q0] "r" (q0),
            [sq6] "r" (sq6), [sq5] "r" (sq5), [sq4] "r" (sq4), [sq3] "r" (sq3),
            [sq2] "r" (sq2), [sq1] "r" (sq1), [sq0] "r" (sq0)
      );
    } else if((flat2 == 0) && (flat == 0xFFFFFFFF) && (mask == 0xFFFFFFFF)) {
      /* f1 */
      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      COMBINE_LEFT_RIGHT_0TO2

      __asm__ __volatile__ (
          "sw         %[p2], (%[sp2])    \n\t"
          "sw         %[p1], (%[sp1])    \n\t"
          "sw         %[p0], (%[sp0])    \n\t"
          "sw         %[q0], (%[sq0])    \n\t"
          "sw         %[q1], (%[sq1])    \n\t"
          "sw         %[q2], (%[sq2])    \n\t"

          :
          : [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),
            [q0] "r" (q0), [q1] "r" (q1), [q2] "r" (q2),
            [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
            [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
      );
    } else if((flat2 == 0) && (flat != 0) && (mask != 0)) {
      /* f0+f1 */
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      if(mask & flat & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p2_r],  (%[sp2])    \n\t"
            "sb         %[p1_r],  (%[sp1])    \n\t"
            "sb         %[p0_r],  (%[sp0])    \n\t"
            "sb         %[q0_r],  (%[sq0])    \n\t"
            "sb         %[q1_r],  (%[sq1])    \n\t"
            "sb         %[q2_r],  (%[sq2])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),

              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)

        );
      } else if(mask & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  (%[sp1])    \n\t"
            "sb         %[p0_f0],  (%[sp0])    \n\t"
            "sb         %[q0_f0],  (%[sq0])    \n\t"
            "sb         %[q1_f0],  (%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_r],    %[p2_r],    16      \n\t"
          "srl      %[p1_r],    %[p1_r],    16      \n\t"
          "srl      %[p0_r],    %[p0_r],    16      \n\t"
          "srl      %[q0_r],    %[q0_r],    16      \n\t"
          "srl      %[q1_r],    %[q1_r],    16      \n\t"
          "srl      %[q2_r],    %[q2_r],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_r] "+r" (p2_r), [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r),
            [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r), [q2_r] "+r" (q2_r),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p2_r],  +1(%[sp2])    \n\t"
            "sb         %[p1_r],  +1(%[sp1])    \n\t"
            "sb         %[p0_r],  +1(%[sp0])    \n\t"
            "sb         %[q0_r],  +1(%[sq0])    \n\t"
            "sb         %[q1_r],  +1(%[sq1])    \n\t"
            "sb         %[q2_r],  +1(%[sq2])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  +1(%[sp1])    \n\t"
            "sb         %[p0_f0],  +1(%[sp0])    \n\t"
            "sb         %[q0_f0],  +1(%[sq0])    \n\t"
            "sb         %[q1_f0],  +1(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p1_f0],   %[p1_f0],   8     \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8     \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8     \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8     \n\t"

          : [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p2_l],  +2(%[sp2])    \n\t"
            "sb         %[p1_l],  +2(%[sp1])    \n\t"
            "sb         %[p0_l],  +2(%[sp0])    \n\t"
            "sb         %[q0_l],  +2(%[sq0])    \n\t"
            "sb         %[q1_l],  +2(%[sq1])    \n\t"
            "sb         %[q2_l],  +2(%[sq2])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  +2(%[sp1])    \n\t"
            "sb         %[p0_f0],  +2(%[sp0])    \n\t"
            "sb         %[q0_f0],  +2(%[sq0])    \n\t"
            "sb         %[q1_f0],  +2(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_l],    %[p2_l],    16      \n\t"
          "srl      %[p1_l],    %[p1_l],    16      \n\t"
          "srl      %[p0_l],    %[p0_l],    16      \n\t"
          "srl      %[q0_l],    %[q0_l],    16      \n\t"
          "srl      %[q1_l],    %[q1_l],    16      \n\t"
          "srl      %[q2_l],    %[q2_l],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_l] "+r" (p2_l), [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l),
            [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb         %[p2_l],  +3(%[sp2])    \n\t"
            "sb         %[p1_l],  +3(%[sp1])    \n\t"
            "sb         %[p0_l],  +3(%[sp0])    \n\t"
            "sb         %[q0_l],  +3(%[sq0])    \n\t"
            "sb         %[q1_l],  +3(%[sq1])    \n\t"
            "sb         %[q2_l],  +3(%[sq2])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  +3(%[sp1])    \n\t"
            "sb         %[p0_f0],  +3(%[sp0])    \n\t"
            "sb         %[q0_f0],  +3(%[sq0])    \n\t"
            "sb         %[q1_f0],  +3(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }
    } else if((flat2 != 0) && (flat != 0) && (mask != 0)) {
      /* f0+f1+f2 */
      /* f0  function */
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      /* f1  function */
      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter1_dspr2(p3_l, p2_l, p1_l, p0_l,
                       q0_l, q1_l, q2_l, q3_l,
                       &p2_l_f1, &p1_l_f1, &p0_l_f1,
                       &q0_l_f1, &q1_l_f1, &q2_l_f1);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter1_dspr2(p3_r, p2_r, p1_r, p0_r,
                       q0_r, q1_r, q2_r, q3_r,
                       &p2_r_f1, &p1_r_f1, &p0_r_f1,
                       &q0_r_f1, &q1_r_f1, &q2_r_f1);

      /* f2  function */
      PACK_LEFT_4TO7
      wide_mbfilter_dspr2(&p7_l, &p6_l, &p5_l, &p4_l,
                          &p3_l, &p2_l, &p1_l, &p0_l,
                          &q0_l, &q1_l, &q2_l, &q3_l,
                          &q4_l, &q5_l, &q6_l, &q7_l);

      PACK_RIGHT_4TO7
      wide_mbfilter_dspr2(&p7_r, &p6_r, &p5_r, &p4_r,
                          &p3_r, &p2_r, &p1_r, &p0_r,
                          &q0_r, &q1_r, &q2_r, &q3_r,
                          &q4_r, &q5_r, &q6_r, &q7_r);

      if(mask&flat&flat2&0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p6_r],  (%[sp6])    \n\t"
            "sb         %[p5_r],  (%[sp5])    \n\t"
            "sb         %[p4_r],  (%[sp4])    \n\t"
            "sb         %[p3_r],  (%[sp3])    \n\t"
            "sb         %[p2_r],  (%[sp2])    \n\t"
            "sb         %[p1_r],  (%[sp1])    \n\t"
            "sb         %[p0_r],  (%[sp0])    \n\t"

            :
            : [p6_r] "r" (p6_r), [p5_r] "r" (p5_r), [p4_r] "r" (p4_r), [p3_r] "r" (p3_r),
              [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [sp6] "r" (sp6), [sp5] "r" (sp5), [sp4] "r" (sp4), [sp3] "r" (sp3),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0)
        );

        __asm__ __volatile__ (
            "sb         %[q0_r],  (%[sq0])    \n\t"
            "sb         %[q1_r],  (%[sq1])    \n\t"
            "sb         %[q2_r],  (%[sq2])    \n\t"
            "sb         %[q3_r],  (%[sq3])    \n\t"
            "sb         %[q4_r],  (%[sq4])    \n\t"
            "sb         %[q5_r],  (%[sq5])    \n\t"
            "sb         %[q6_r],  (%[sq6])    \n\t"

            :
            : [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r), [q3_r] "r" (q3_r),
              [q4_r] "r" (q4_r), [q5_r] "r" (q5_r), [q6_r] "r" (q6_r),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2), [sq3] "r" (sq3),
              [sq4] "r" (sq4), [sq5] "r" (sq5), [sq6] "r" (sq6)
        );
      } else if(mask&flat&0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p2_r_f1],  (%[sp2])    \n\t"
            "sb         %[p1_r_f1],  (%[sp1])    \n\t"
            "sb         %[p0_r_f1],  (%[sp0])    \n\t"
            "sb         %[q0_r_f1],  (%[sq0])    \n\t"
            "sb         %[q1_r_f1],  (%[sq1])    \n\t"
            "sb         %[q2_r_f1],  (%[sq2])    \n\t"

            :
            : [p2_r_f1] "r" (p2_r_f1), [p1_r_f1] "r" (p1_r_f1), [p0_r_f1] "r" (p0_r_f1),
              [q0_r_f1] "r" (q0_r_f1), [q1_r_f1] "r" (q1_r_f1), [q2_r_f1] "r" (q2_r_f1),

              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)

        );
      } else if(mask&0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  (%[sp1])    \n\t"
            "sb         %[p0_f0],  (%[sp0])    \n\t"
            "sb         %[q0_f0],  (%[sq0])    \n\t"
            "sb         %[q1_f0],  (%[sq1])    \n\t"
            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0), [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0), [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }


      __asm__ __volatile__ (
          "srl        %[p6_r], %[p6_r], 16     \n\t"
          "srl        %[p5_r], %[p5_r], 16     \n\t"
          "srl        %[p4_r], %[p4_r], 16     \n\t"
          "srl        %[p3_r], %[p3_r], 16     \n\t"
          "srl        %[p2_r], %[p2_r], 16     \n\t"
          "srl        %[p1_r], %[p1_r], 16     \n\t"
          "srl        %[p0_r], %[p0_r], 16     \n\t"

          "srl        %[q0_r], %[q0_r], 16     \n\t"
          "srl        %[q1_r], %[q1_r], 16     \n\t"
          "srl        %[q2_r], %[q2_r], 16     \n\t"
          "srl        %[q3_r], %[q3_r], 16     \n\t"
          "srl        %[q4_r], %[q4_r], 16     \n\t"
          "srl        %[q5_r], %[q5_r], 16     \n\t"
          "srl        %[q6_r], %[q6_r], 16     \n\t"

          : [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r), [q2_r] "+r" (q2_r), [q3_r] "+r" (q3_r),
            [q4_r] "+r" (q4_r), [q5_r] "+r" (q5_r), [q6_r] "+r" (q6_r),
            [p6_r] "+r" (p6_r), [p5_r] "+r" (p5_r), [p4_r] "+r" (p4_r), [p3_r] "+r" (p3_r),
            [p2_r] "+r" (p2_r), [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r)
          :
      );

      __asm__ __volatile__ (
          "srl        %[p2_r_f1], %[p2_r_f1], 16     \n\t"
          "srl        %[p1_r_f1], %[p1_r_f1], 16     \n\t"
          "srl        %[p0_r_f1], %[p0_r_f1], 16     \n\t"
          "srl        %[q0_r_f1], %[q0_r_f1], 16     \n\t"
          "srl        %[q1_r_f1], %[q1_r_f1], 16     \n\t"
          "srl        %[q2_r_f1], %[q2_r_f1], 16     \n\t"

          "srl        %[p1_f0], %[p1_f0], 8     \n\t"
          "srl        %[p0_f0], %[p0_f0], 8     \n\t"
          "srl        %[q0_f0], %[q0_f0], 8     \n\t"
          "srl        %[q1_f0], %[q1_f0], 8     \n\t"
          : [p2_r_f1] "+r" (p2_r_f1), [p1_r_f1] "+r" (p1_r_f1), [p0_r_f1] "+r" (p0_r_f1),
            [q0_r_f1] "+r" (q0_r_f1), [q1_r_f1] "+r" (q1_r_f1), [q2_r_f1] "+r" (q2_r_f1),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0), [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask&flat&flat2&0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p6_r],  +1(%[sp6])    \n\t"
            "sb         %[p5_r],  +1(%[sp5])    \n\t"
            "sb         %[p4_r],  +1(%[sp4])    \n\t"
            "sb         %[p3_r],  +1(%[sp3])    \n\t"
            "sb         %[p2_r],  +1(%[sp2])    \n\t"
            "sb         %[p1_r],  +1(%[sp1])    \n\t"
            "sb         %[p0_r],  +1(%[sp0])    \n\t"

            :
            : [p6_r] "r" (p6_r), [p5_r] "r" (p5_r), [p4_r] "r" (p4_r), [p3_r] "r" (p3_r),
              [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [sp6] "r" (sp6), [sp5] "r" (sp5), [sp4] "r" (sp4), [sp3] "r" (sp3),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0)
        );
        __asm__ __volatile__ (
            "sb         %[q0_r],  +1(%[sq0])    \n\t"
            "sb         %[q1_r],  +1(%[sq1])    \n\t"
            "sb         %[q2_r],  +1(%[sq2])    \n\t"
            "sb         %[q3_r],  +1(%[sq3])    \n\t"
            "sb         %[q4_r],  +1(%[sq4])    \n\t"
            "sb         %[q5_r],  +1(%[sq5])    \n\t"
            "sb         %[q6_r],  +1(%[sq6])    \n\t"

            :
            : [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r), [q3_r] "r" (q3_r),
              [q4_r] "r" (q4_r), [q5_r] "r" (q5_r), [q6_r] "r" (q6_r),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2), [sq3] "r" (sq3),
              [sq4] "r" (sq4), [sq5] "r" (sq5), [sq6] "r" (sq6)

        );
      } else if(mask&flat&0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p2_r_f1],  +1(%[sp2])    \n\t"
            "sb         %[p1_r_f1],  +1(%[sp1])    \n\t"
            "sb         %[p0_r_f1],  +1(%[sp0])    \n\t"
            "sb         %[q0_r_f1],  +1(%[sq0])    \n\t"
            "sb         %[q1_r_f1],  +1(%[sq1])    \n\t"
            "sb         %[q2_r_f1],  +1(%[sq2])    \n\t"

            :
            : [p2_r_f1] "r" (p2_r_f1), [p1_r_f1] "r" (p1_r_f1), [p0_r_f1] "r" (p0_r_f1),
              [q0_r_f1] "r" (q0_r_f1), [q1_r_f1] "r" (q1_r_f1), [q2_r_f1] "r" (q2_r_f1),

              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)

        );
      } else if(mask&0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  +1(%[sp1])    \n\t"
            "sb         %[p0_f0],  +1(%[sp0])    \n\t"
            "sb         %[q0_f0],  +1(%[sq0])    \n\t"
            "sb         %[q1_f0],  +1(%[sq1])    \n\t"
            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0), [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0), [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (

          "srl        %[p1_f0], %[p1_f0], 8     \n\t"
          "srl        %[p0_f0], %[p0_f0], 8     \n\t"
          "srl        %[q0_f0], %[q0_f0], 8     \n\t"
          "srl        %[q1_f0], %[q1_f0], 8     \n\t"
          : [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0), [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask&flat&flat2&0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p6_l],  +2(%[sp6])    \n\t"
            "sb         %[p5_l],  +2(%[sp5])    \n\t"
            "sb         %[p4_l],  +2(%[sp4])    \n\t"
            "sb         %[p3_l],  +2(%[sp3])    \n\t"
            "sb         %[p2_l],  +2(%[sp2])    \n\t"
            "sb         %[p1_l],  +2(%[sp1])    \n\t"
            "sb         %[p0_l],  +2(%[sp0])    \n\t"

            :
            : [p6_l] "r" (p6_l), [p5_l] "r" (p5_l), [p4_l] "r" (p4_l), [p3_l] "r" (p3_l),
              [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [sp6] "r" (sp6), [sp5] "r" (sp5), [sp4] "r" (sp4), [sp3] "r" (sp3),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0)
        );
        __asm__ __volatile__ (
            "sb         %[q0_l],  +2(%[sq0])    \n\t"
            "sb         %[q1_l],  +2(%[sq1])    \n\t"
            "sb         %[q2_l],  +2(%[sq2])    \n\t"
            "sb         %[q3_l],  +2(%[sq3])    \n\t"
            "sb         %[q4_l],  +2(%[sq4])    \n\t"
            "sb         %[q5_l],  +2(%[sq5])    \n\t"
            "sb         %[q6_l],  +2(%[sq6])    \n\t"

            :
            : [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l), [q3_l] "r" (q3_l),
              [q4_l] "r" (q4_l), [q5_l] "r" (q5_l), [q6_l] "r" (q6_l),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2), [sq3] "r" (sq3),
              [sq4] "r" (sq4), [sq5] "r" (sq5), [sq6] "r" (sq6)

        );
      } else if(mask&flat&0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p2_l_f1],  +2(%[sp2])    \n\t"
            "sb         %[p1_l_f1],  +2(%[sp1])    \n\t"
            "sb         %[p0_l_f1],  +2(%[sp0])    \n\t"
            "sb         %[q0_l_f1],  +2(%[sq0])    \n\t"
            "sb         %[q1_l_f1],  +2(%[sq1])    \n\t"
            "sb         %[q2_l_f1],  +2(%[sq2])    \n\t"

            :
            : [p2_l_f1] "r" (p2_l_f1), [p1_l_f1] "r" (p1_l_f1), [p0_l_f1] "r" (p0_l_f1),
              [q0_l_f1] "r" (q0_l_f1), [q1_l_f1] "r" (q1_l_f1), [q2_l_f1] "r" (q2_l_f1),

              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)

        );
      } else if(mask&0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  +2(%[sp1])    \n\t"
            "sb         %[p0_f0],  +2(%[sp0])    \n\t"
            "sb         %[q0_f0],  +2(%[sq0])    \n\t"
            "sb         %[q1_f0],  +2(%[sq1])    \n\t"
            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0), [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0), [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl        %[p6_l], %[p6_l], 16     \n\t"
          "srl        %[p5_l], %[p5_l], 16     \n\t"
          "srl        %[p4_l], %[p4_l], 16     \n\t"
          "srl        %[p3_l], %[p3_l], 16     \n\t"
          "srl        %[p2_l], %[p2_l], 16     \n\t"
          "srl        %[p1_l], %[p1_l], 16     \n\t"
          "srl        %[p0_l], %[p0_l], 16     \n\t"

          "srl        %[q0_l], %[q0_l], 16     \n\t"
          "srl        %[q1_l], %[q1_l], 16     \n\t"
          "srl        %[q2_l], %[q2_l], 16     \n\t"
          "srl        %[q3_l], %[q3_l], 16     \n\t"
          "srl        %[q4_l], %[q4_l], 16     \n\t"
          "srl        %[q5_l], %[q5_l], 16     \n\t"
          "srl        %[q6_l], %[q6_l], 16     \n\t"

          : [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l), [q3_l] "+r" (q3_l),
            [q4_l] "+r" (q4_l), [q5_l] "+r" (q5_l), [q6_l] "+r" (q6_l),
            [p6_l] "+r" (p6_l), [p5_l] "+r" (p5_l), [p4_l] "+r" (p4_l), [p3_l] "+r" (p3_l),
            [p2_l] "+r" (p2_l), [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l)
          :
      );

      __asm__ __volatile__ (
          "srl        %[p2_l_f1], %[p2_l_f1], 16     \n\t"
          "srl        %[p1_l_f1], %[p1_l_f1], 16     \n\t"
          "srl        %[p0_l_f1], %[p0_l_f1], 16     \n\t"
          "srl        %[q0_l_f1], %[q0_l_f1], 16     \n\t"
          "srl        %[q1_l_f1], %[q1_l_f1], 16     \n\t"
          "srl        %[q2_l_f1], %[q2_l_f1], 16     \n\t"

          "srl        %[p1_f0], %[p1_f0], 8     \n\t"
          "srl        %[p0_f0], %[p0_f0], 8     \n\t"
          "srl        %[q0_f0], %[q0_f0], 8     \n\t"
          "srl        %[q1_f0], %[q1_f0], 8     \n\t"
          : [p2_l_f1] "+r" (p2_l_f1), [p1_l_f1] "+r" (p1_l_f1), [p0_l_f1] "+r" (p0_l_f1),
            [q0_l_f1] "+r" (q0_l_f1), [q1_l_f1] "+r" (q1_l_f1), [q2_l_f1] "+r" (q2_l_f1),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0), [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & flat2 & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p6_l],    +3(%[sp6])    \n\t"
            "sb     %[p5_l],    +3(%[sp5])    \n\t"
            "sb     %[p4_l],    +3(%[sp4])    \n\t"
            "sb     %[p3_l],    +3(%[sp3])    \n\t"
            "sb     %[p2_l],    +3(%[sp2])    \n\t"
            "sb     %[p1_l],    +3(%[sp1])    \n\t"
            "sb     %[p0_l],    +3(%[sp0])    \n\t"

            :
            : [p6_l] "r" (p6_l), [p5_l] "r" (p5_l), [p4_l] "r" (p4_l), [p3_l] "r" (p3_l),
              [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [sp6] "r" (sp6), [sp5] "r" (sp5), [sp4] "r" (sp4), [sp3] "r" (sp3),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0)
        );

        __asm__ __volatile__ (
            "sb     %[q0_l],    +3(%[sq0])    \n\t"
            "sb     %[q1_l],    +3(%[sq1])    \n\t"
            "sb     %[q2_l],    +3(%[sq2])    \n\t"
            "sb     %[q3_l],    +3(%[sq3])    \n\t"
            "sb     %[q4_l],    +3(%[sq4])    \n\t"
            "sb     %[q5_l],    +3(%[sq5])    \n\t"
            "sb     %[q6_l],    +3(%[sq6])    \n\t"

            :
            : [q0_l] "r" (q0_l), [q1_l] "r" (q1_l),
              [q2_l] "r" (q2_l), [q3_l] "r" (q3_l),
              [q4_l] "r" (q4_l), [q5_l] "r" (q5_l),
              [q6_l] "r" (q6_l),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2),
              [sq3] "r" (sq3), [sq4] "r" (sq4), [sq5] "r" (sq5),
              [sq6] "r" (sq6)
        );
      } else if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p2_l_f1],     +3(%[sp2])    \n\t"
            "sb     %[p1_l_f1],     +3(%[sp1])    \n\t"
            "sb     %[p0_l_f1],     +3(%[sp0])    \n\t"
            "sb     %[q0_l_f1],     +3(%[sq0])    \n\t"
            "sb     %[q1_l_f1],     +3(%[sq1])    \n\t"
            "sb     %[q2_l_f1],     +3(%[sq2])    \n\t"

            :
            : [p2_l_f1] "r" (p2_l_f1), [p1_l_f1] "r" (p1_l_f1),
              [p0_l_f1] "r" (p0_l_f1), [q0_l_f1] "r" (q0_l_f1),
              [q1_l_f1] "r" (q1_l_f1), [q2_l_f1] "r" (q2_l_f1),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   +3(%[sp1])    \n\t"
            "sb     %[p0_f0],   +3(%[sp0])    \n\t"
            "sb     %[q0_f0],   +3(%[sq0])    \n\t"
            "sb     %[q1_f0],   +3(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }
    }

    s = s + 4;
  }
}

void vp9_mb_lpf_vertical_edge_w_dspr2(uint8_t *s,
                                      int pitch,
                                      const uint8_t *blimit,
                                      const uint8_t *limit,
                                      const uint8_t *thresh) {
  uint8_t i;
  uint32_t mask, hev, flat, flat2;
  unsigned char *s1, *s2, *s3, *s4;
  uint32_t prim1, prim2, sec3, sec4, prim3, prim4;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;
  uint32_t p7, p6, p5, p4, p3, p2, p1, p0, q0, q1, q2, q3, q4, q5, q6, q7;
  uint32_t p1_f0, p0_f0, q0_f0, q1_f0;
  uint32_t p7_l, p6_l, p5_l, p4_l, p3_l, p2_l, p1_l, p0_l,
           q0_l, q1_l, q2_l, q3_l, q4_l, q5_l, q6_l, q7_l;
  uint32_t p7_r, p6_r, p5_r, p4_r, p3_r, p2_r, p1_r, p0_r,
           q0_r, q1_r, q2_r, q3_r, q4_r, q5_r, q6_r, q7_r;
  uint32_t p2_l_f1, p1_l_f1, p0_l_f1, p2_r_f1, p1_r_f1, p0_r_f1;
  uint32_t q0_l_f1, q1_l_f1, q2_l_f1, q0_r_f1, q1_r_f1, q2_r_f1;

  uflimit = *blimit;
  ulimit = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb     %[thresh_vec],     %[uthresh]    \n\t"
      "replv.qb     %[flimit_vec],     %[uflimit]    \n\t"
      "replv.qb     %[limit_vec],      %[ulimit]     \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  vp9_prefetch_store(s + pitch);

  for(i = 0; i < 2; i++) {
    s1 = s;
    s2 = s + pitch;
    s3 = s2 + pitch;
    s4 = s3 + pitch;
    s  = s4 + pitch;

    __asm__ __volatile__ (
        "lw     %[p0],  -4(%[s1])    \n\t"
        "lw     %[p1],  -4(%[s2])    \n\t"
        "lw     %[p2],  -4(%[s3])    \n\t"
        "lw     %[p3],  -4(%[s4])    \n\t"
        "lw     %[p4],  -8(%[s1])    \n\t"
        "lw     %[p5],  -8(%[s2])    \n\t"
        "lw     %[p6],  -8(%[s3])    \n\t"
        "lw     %[p7],  -8(%[s4])    \n\t"

        : [p3] "=&r" (p3), [p2] "=&r" (p2), [p1] "=&r" (p1),
          [p0] "=&r" (p0), [p7] "=&r" (p7), [p6] "=&r" (p6),
          [p5] "=&r" (p5), [p4] "=&r" (p4)
        : [s1] "r" (s1), [s2] "r" (s2), [s3] "r" (s3), [s4] "r" (s4)
    );

    __asm__ __volatile__ (
        "lw     %[q3],  (%[s1])     \n\t"
        "lw     %[q2],  (%[s2])     \n\t"
        "lw     %[q1],  (%[s3])     \n\t"
        "lw     %[q0],  (%[s4])     \n\t"
        "lw     %[q7],  +4(%[s1])   \n\t"
        "lw     %[q6],  +4(%[s2])   \n\t"
        "lw     %[q5],  +4(%[s3])   \n\t"
        "lw     %[q4],  +4(%[s4])   \n\t"

        : [q3] "=&r" (q3), [q2] "=&r" (q2), [q1] "=&r" (q1),
          [q0] "=&r" (q0), [q7] "=&r" (q7), [q6] "=&r" (q6),
          [q5] "=&r" (q5), [q4] "=&r" (q4)
        : [s1] "r" (s1), [s2] "r" (s2), [s3] "r" (s3), [s4] "r" (s4)
    );

    /* implemented transpose technique

 original (when loaded from memory)

 -8    -7   -6     -5   register    -4    -3   -2     -1   register  memory   register      +1    +2    +3    +4      register   +5    +6    +7    +8
p4_0  p4_1  p4_2  p4_3    p4       p0_0  p0_1  p0_2  p0_3      p0      s1        q3        q3_0  q3_1  q3_2  q3_3       q7      q7_0  q7_1  q7_2  q7_3
p5_0  p5_1  p5_2  p5_3    p5       p1_0  p1_1  p1_2  p1_3      p1      s2        q2        q2_0  q2_1  q2_2  q2_3       q6      q6_0  q6_1  q6_2  q6_3
p6_0  p6_1  p6_2  p6_3    p6       p2_0  p2_1  p2_2  p2_3      p2      s3        q1        q1_0  q1_1  q1_2  q1_3       q5      q5_0  q5_1  q5_2  q5_3
p7_0  p7_1  p7_2  p7_3    p7       p3_0  p3_1  p3_2  p3_3      p3      s4        q0        q0_0  q0_1  q0_2  q0_3       q4      q4_0  q4_1  q4_2  q4_3

 after transpose

                          register                               register    register                            register
p7_3  p6_3  p5_3  p4_3       p4        p3_3  p2_3  p1_3  p0_3       p0         q3       q0_3  q1_3  q2_3  q3_3     q7       q4_3  q5_3  q26_3  q7_3
p7_2  p6_2  p5_2  p4_2       p5        p3_2  p2_2  p1_2  p0_2       p1         q2       q0_2  q1_2  q2_2  q3_2     q6       q4_2  q5_2  q26_2  q7_2
p7_1  p6_1  p5_1  p4_1       p6        p3_1  p2_1  p1_1  p0_1       p2         q1       q0_1  q1_1  q2_1  q3_1     q5       q4_1  q5_1  q26_1  q7_1
p7_0  p6_0  p5_0  p4_0       p7        p3_0  p2_0  p1_0  p0_0       p3         q0       q0_0  q1_0  q2_0  q3_0     q4       q4_0  q5_0  q26_0  q7_0
    */

    /* transpose p3, p2, p1, p0 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[p0],      %[p1]       \n\t"
        "precr.qb.ph    %[prim2],   %[p0],      %[p1]       \n\t"
        "precrq.qb.ph   %[prim3],   %[p2],      %[p3]       \n\t"
        "precr.qb.ph    %[prim4],   %[p2],      %[p3]       \n\t"

        "precrq.qb.ph   %[p1],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[p3],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[p0],      %[p1],      %[sec3]     \n\t"
        "precrq.ph.w    %[p2],      %[p3],      %[sec4]     \n\t"
        "append         %[p1],      %[sec3],    16          \n\t"
        "append         %[p3],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [p0] "+r" (p0), [p1] "+r" (p1), [p2] "+r" (p2), [p3] "+r" (p3),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* transpose q0, q1, q2, q3 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[q3],      %[q2]       \n\t"
        "precr.qb.ph    %[prim2],   %[q3],      %[q2]       \n\t"
        "precrq.qb.ph   %[prim3],   %[q1],      %[q0]       \n\t"
        "precr.qb.ph    %[prim4],   %[q1],      %[q0]       \n\t"

        "precrq.qb.ph   %[q2],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[q0],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[q3],      %[q2],      %[sec3]     \n\t"
        "precrq.ph.w    %[q1],      %[q0],      %[sec4]     \n\t"
        "append         %[q2],      %[sec3],    16          \n\t"
        "append         %[q0],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [q3] "+r" (q3), [q2] "+r" (q2), [q1] "+r" (q1), [q0] "+r" (q0),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* transpose p7, p6, p5, p4 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[p4],      %[p5]       \n\t"
        "precr.qb.ph    %[prim2],   %[p4],      %[p5]       \n\t"
        "precrq.qb.ph   %[prim3],   %[p6],      %[p7]       \n\t"
        "precr.qb.ph    %[prim4],   %[p6],      %[p7]       \n\t"

        "precrq.qb.ph   %[p5],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[p7],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[p4],      %[p5],      %[sec3]     \n\t"
        "precrq.ph.w    %[p6],      %[p7],      %[sec4]     \n\t"
        "append         %[p5],      %[sec3],    16          \n\t"
        "append         %[p7],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [p4] "+r" (p4), [p5] "+r" (p5), [p6] "+r" (p6), [p7] "+r" (p7),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* transpose q4, q5, q6, q7 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[q7],      %[q6]       \n\t"
        "precr.qb.ph    %[prim2],   %[q7],      %[q6]       \n\t"
        "precrq.qb.ph   %[prim3],   %[q5],      %[q4]       \n\t"
        "precr.qb.ph    %[prim4],   %[q5],      %[q4]       \n\t"

        "precrq.qb.ph   %[q6],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[q4],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[q7],      %[q6],      %[sec3]     \n\t"
        "precrq.ph.w    %[q5],      %[q4],      %[sec4]     \n\t"
        "append         %[q6],      %[sec3],    16          \n\t"
        "append         %[q4],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [q7] "+r" (q7), [q6] "+r" (q6), [q5] "+r" (q5), [q4] "+r" (q4),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    filter_hev_mask_flatmask4_dspr2(limit_vec, flimit_vec, thresh_vec,
                                    p1, p0, p3, p2, q0, q1, q2, q3,
                                    &hev, &mask, &flat);

    flatmask5(p7, p6, p5, p4, p0, q0, q4, q5 ,q6, q7, &flat2);

    /* f0 */
    if(((flat2 == 0) && (flat == 0) && (mask != 0)) ||
       ((flat2 != 0) && (flat == 0) && (mask != 0))) {

      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);
      STORE_F0
    } else if((flat2 == 0XFFFFFFFF) && (flat == 0xFFFFFFFF) && (mask == 0xFFFFFFFF)) {
      /* f2 */
      PACK_LEFT_0TO3
      PACK_LEFT_4TO7
      wide_mbfilter_dspr2(&p7_l, &p6_l, &p5_l, &p4_l,
                          &p3_l, &p2_l, &p1_l, &p0_l,
                          &q0_l, &q1_l, &q2_l, &q3_l,
                          &q4_l, &q5_l, &q6_l, &q7_l);

      PACK_RIGHT_0TO3
      PACK_RIGHT_4TO7
      wide_mbfilter_dspr2(&p7_r, &p6_r, &p5_r, &p4_r,
                          &p3_r, &p2_r, &p1_r, &p0_r,
                          &q0_r, &q1_r, &q2_r, &q3_r,
                          &q4_r, &q5_r, &q6_r, &q7_r);

      STORE_F2
    } else if((flat2 == 0) && (flat == 0xFFFFFFFF) && (mask == 0xFFFFFFFF)) {
      /* f1 */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      STORE_F1
    } else if((flat2 == 0) && (flat != 0) && (mask != 0)) {
      /* f0+f1 */
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);


      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);


      if(mask & flat & 0x000000FF) {
        __asm__ __volatile__ (
            "sb     %[p2_r],    -3(%[s4])    \n\t"
            "sb     %[p1_r],    -2(%[s4])    \n\t"
            "sb     %[p0_r],    -1(%[s4])    \n\t"
            "sb     %[q0_r],      (%[s4])    \n\t"
            "sb     %[q1_r],    +1(%[s4])    \n\t"
            "sb     %[q2_r],    +2(%[s4])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [s4] "r" (s4)
        );
      } else if(mask & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  -2(%[s4])    \n\t"
            "sb         %[p0_f0],  -1(%[s4])    \n\t"
            "sb         %[q0_f0],    (%[s4])    \n\t"
            "sb         %[q1_f0],  +1(%[s4])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s4] "r" (s4)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_r],    %[p2_r],    16      \n\t"
          "srl      %[p1_r],    %[p1_r],    16      \n\t"
          "srl      %[p0_r],    %[p0_r],    16      \n\t"
          "srl      %[q0_r],    %[q0_r],    16      \n\t"
          "srl      %[q1_r],    %[q1_r],    16      \n\t"
          "srl      %[q2_r],    %[q2_r],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_r] "+r" (p2_r), [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r),
            [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r), [q2_r] "+r" (q2_r),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p2_r],    -3(%[s3])    \n\t"
            "sb     %[p1_r],    -2(%[s3])    \n\t"
            "sb     %[p0_r],    -1(%[s3])    \n\t"
            "sb     %[q0_r],      (%[s3])    \n\t"
            "sb     %[q1_r],    +1(%[s3])    \n\t"
            "sb     %[q2_r],    +2(%[s3])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [s3] "r" (s3)
        );
      } else if(mask & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s3])    \n\t"
            "sb     %[p0_f0],   -1(%[s3])    \n\t"
            "sb     %[q0_f0],     (%[s3])    \n\t"
            "sb     %[q1_f0],   +1(%[s3])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s3] "r" (s3)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p1_f0],   %[p1_f0],   8     \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8     \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8     \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8     \n\t"

          : [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x00FF0000) {
        __asm__ __volatile__ (
          "sb       %[p2_l],    -3(%[s2])    \n\t"
          "sb       %[p1_l],    -2(%[s2])    \n\t"
          "sb       %[p0_l],    -1(%[s2])    \n\t"
          "sb       %[q0_l],      (%[s2])    \n\t"
          "sb       %[q1_l],    +1(%[s2])    \n\t"
          "sb       %[q2_l],    +2(%[s2])    \n\t"

          :
          : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
            [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
            [s2] "r" (s2)
        );
      } else if(mask & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s2])    \n\t"
            "sb     %[p0_f0],   -1(%[s2])    \n\t"
            "sb     %[q0_f0],     (%[s2])    \n\t"
            "sb     %[q1_f0],   +1(%[s2])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s2] "r" (s2)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_l],    %[p2_l],    16      \n\t"
          "srl      %[p1_l],    %[p1_l],    16      \n\t"
          "srl      %[p0_l],    %[p0_l],    16      \n\t"
          "srl      %[q0_l],    %[q0_l],    16      \n\t"
          "srl      %[q1_l],    %[q1_l],    16      \n\t"
          "srl      %[q2_l],    %[q2_l],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_l] "+r" (p2_l), [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l),
            [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p2_l],    -3(%[s1])    \n\t"
            "sb     %[p1_l],    -2(%[s1])    \n\t"
            "sb     %[p0_l],    -1(%[s1])    \n\t"
            "sb     %[q0_l],      (%[s1])    \n\t"
            "sb     %[q1_l],    +1(%[s1])    \n\t"
            "sb     %[q2_l],    +2(%[s1])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [s1] "r" (s1)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s1])    \n\t"
            "sb     %[p0_f0],   -1(%[s1])    \n\t"
            "sb     %[q0_f0],     (%[s1])    \n\t"
            "sb     %[q1_f0],   +1(%[s1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s1] "r" (s1)
        );
      }
    } else if((flat2 != 0) && (flat != 0) && (mask != 0)) {
      /* f0+f1+f2 */
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      PACK_LEFT_0TO3
      mbfilter1_dspr2(p3_l, p2_l, p1_l, p0_l,
                      q0_l, q1_l, q2_l, q3_l,
                      &p2_l_f1, &p1_l_f1, &p0_l_f1,
                      &q0_l_f1, &q1_l_f1, &q2_l_f1);

      PACK_RIGHT_0TO3
      mbfilter1_dspr2(p3_r, p2_r, p1_r, p0_r,
                      q0_r, q1_r, q2_r, q3_r,
                      &p2_r_f1, &p1_r_f1, &p0_r_f1,
                      &q0_r_f1, &q1_r_f1, &q2_r_f1);

      PACK_LEFT_4TO7
      wide_mbfilter_dspr2(&p7_l, &p6_l, &p5_l, &p4_l,
                          &p3_l, &p2_l, &p1_l, &p0_l,
                          &q0_l, &q1_l, &q2_l, &q3_l,
                          &q4_l, &q5_l, &q6_l, &q7_l);

      PACK_RIGHT_4TO7
      wide_mbfilter_dspr2(&p7_r, &p6_r, &p5_r, &p4_r,
                          &p3_r, &p2_r, &p1_r, &p0_r,
                          &q0_r, &q1_r, &q2_r, &q3_r,
                          &q4_r, &q5_r, &q6_r, &q7_r);

      if(mask & flat & flat2 & 0x000000FF) {
        __asm__ __volatile__ (
            "sb     %[p6_r],    -7(%[s4])    \n\t"
            "sb     %[p5_r],    -6(%[s4])    \n\t"
            "sb     %[p4_r],    -5(%[s4])    \n\t"
            "sb     %[p3_r],    -4(%[s4])    \n\t"
            "sb     %[p2_r],    -3(%[s4])    \n\t"
            "sb     %[p1_r],    -2(%[s4])    \n\t"
            "sb     %[p0_r],    -1(%[s4])    \n\t"

            :
            : [p6_r] "r" (p6_r), [p5_r] "r" (p5_r),
              [p4_r] "r" (p4_r), [p3_r] "r" (p3_r),
              [p2_r] "r" (p2_r), [p1_r] "r" (p1_r),
              [p0_r] "r" (p0_r), [s4] "r" (s4)
        );

        __asm__ __volatile__ (
            "sb     %[q0_r],      (%[s4])    \n\t"
            "sb     %[q1_r],    +1(%[s4])    \n\t"
            "sb     %[q2_r],    +2(%[s4])    \n\t"
            "sb     %[q3_r],    +3(%[s4])    \n\t"
            "sb     %[q4_r],    +4(%[s4])    \n\t"
            "sb     %[q5_r],    +5(%[s4])    \n\t"
            "sb     %[q6_r],    +6(%[s4])    \n\t"

            :
            : [q0_r] "r" (q0_r), [q1_r] "r" (q1_r),
              [q2_r] "r" (q2_r), [q3_r] "r" (q3_r),
              [q4_r] "r" (q4_r), [q5_r] "r" (q5_r),
              [q6_r] "r" (q6_r), [s4] "r" (s4)
        );
      } else if(mask & flat & 0x000000FF) {
        __asm__ __volatile__ (
            "sb     %[p2_r_f1],     -3(%[s4])    \n\t"
            "sb     %[p1_r_f1],     -2(%[s4])    \n\t"
            "sb     %[p0_r_f1],     -1(%[s4])    \n\t"
            "sb     %[q0_r_f1],       (%[s4])    \n\t"
            "sb     %[q1_r_f1],     +1(%[s4])    \n\t"
            "sb     %[q2_r_f1],     +2(%[s4])    \n\t"

            :
            : [p2_r_f1] "r" (p2_r_f1), [p1_r_f1] "r" (p1_r_f1),
              [p0_r_f1] "r" (p0_r_f1), [q0_r_f1] "r" (q0_r_f1),
              [q1_r_f1] "r" (q1_r_f1), [q2_r_f1] "r" (q2_r_f1),
              [s4] "r" (s4)
        );
      } else if(mask & 0x000000FF) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s4])    \n\t"
            "sb     %[p0_f0],   -1(%[s4])    \n\t"
            "sb     %[q0_f0],     (%[s4])    \n\t"
            "sb     %[q1_f0],   +1(%[s4])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s4] "r" (s4)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p6_r],        %[p6_r],        16     \n\t"
          "srl      %[p5_r],        %[p5_r],        16     \n\t"
          "srl      %[p4_r],        %[p4_r],        16     \n\t"
          "srl      %[p3_r],        %[p3_r],        16     \n\t"
          "srl      %[p2_r],        %[p2_r],        16     \n\t"
          "srl      %[p1_r],        %[p1_r],        16     \n\t"
          "srl      %[p0_r],        %[p0_r],        16     \n\t"
          "srl      %[q0_r],        %[q0_r],        16     \n\t"
          "srl      %[q1_r],        %[q1_r],        16     \n\t"
          "srl      %[q2_r],        %[q2_r],        16     \n\t"
          "srl      %[q3_r],        %[q3_r],        16     \n\t"
          "srl      %[q4_r],        %[q4_r],        16     \n\t"
          "srl      %[q5_r],        %[q5_r],        16     \n\t"
          "srl      %[q6_r],        %[q6_r],        16     \n\t"

          : [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r),
            [q2_r] "+r" (q2_r), [q3_r] "+r" (q3_r),
            [q4_r] "+r" (q4_r), [q5_r] "+r" (q5_r),
            [q6_r] "+r" (q6_r), [p6_r] "+r" (p6_r),
            [p5_r] "+r" (p5_r), [p4_r] "+r" (p4_r),
            [p3_r] "+r" (p3_r), [p2_r] "+r" (p2_r),
            [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r)
          :
      );

      __asm__ __volatile__ (
          "srl      %[p2_r_f1],     %[p2_r_f1],     16      \n\t"
          "srl      %[p1_r_f1],     %[p1_r_f1],     16      \n\t"
          "srl      %[p0_r_f1],     %[p0_r_f1],     16      \n\t"
          "srl      %[q0_r_f1],     %[q0_r_f1],     16      \n\t"
          "srl      %[q1_r_f1],     %[q1_r_f1],     16      \n\t"
          "srl      %[q2_r_f1],     %[q2_r_f1],     16      \n\t"
          "srl      %[p1_f0],       %[p1_f0],       8       \n\t"
          "srl      %[p0_f0],       %[p0_f0],       8       \n\t"
          "srl      %[q0_f0],       %[q0_f0],       8       \n\t"
          "srl      %[q1_f0],       %[q1_f0],       8       \n\t"

          : [p2_r_f1] "+r" (p2_r_f1), [p1_r_f1] "+r" (p1_r_f1),
            [p0_r_f1] "+r" (p0_r_f1), [q0_r_f1] "+r" (q0_r_f1),
            [q1_r_f1] "+r" (q1_r_f1), [q2_r_f1] "+r" (q2_r_f1),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & flat2 & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p6_r],    -7(%[s3])    \n\t"
            "sb     %[p5_r],    -6(%[s3])    \n\t"
            "sb     %[p4_r],    -5(%[s3])    \n\t"
            "sb     %[p3_r],    -4(%[s3])    \n\t"
            "sb     %[p2_r],    -3(%[s3])    \n\t"
            "sb     %[p1_r],    -2(%[s3])    \n\t"
            "sb     %[p0_r],    -1(%[s3])    \n\t"

            :
            : [p6_r] "r" (p6_r), [p5_r] "r" (p5_r), [p4_r] "r" (p4_r),
              [p3_r] "r" (p3_r), [p2_r] "r" (p2_r), [p1_r] "r" (p1_r),
              [p0_r] "r" (p0_r), [s3] "r" (s3)
        );

        __asm__ __volatile__ (
            "sb     %[q0_r],      (%[s3])    \n\t"
            "sb     %[q1_r],    +1(%[s3])    \n\t"
            "sb     %[q2_r],    +2(%[s3])    \n\t"
            "sb     %[q3_r],    +3(%[s3])    \n\t"
            "sb     %[q4_r],    +4(%[s3])    \n\t"
            "sb     %[q5_r],    +5(%[s3])    \n\t"
            "sb     %[q6_r],    +6(%[s3])    \n\t"

            :
            : [q0_r] "r" (q0_r), [q1_r] "r" (q1_r),
              [q2_r] "r" (q2_r), [q3_r] "r" (q3_r),
              [q4_r] "r" (q4_r), [q5_r] "r" (q5_r),
              [q6_r] "r" (q6_r), [s3] "r" (s3)
        );
      } else if(mask & flat & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p2_r_f1],     -3(%[s3])    \n\t"
            "sb     %[p1_r_f1],     -2(%[s3])    \n\t"
            "sb     %[p0_r_f1],     -1(%[s3])    \n\t"
            "sb     %[q0_r_f1],       (%[s3])    \n\t"
            "sb     %[q1_r_f1],     +1(%[s3])    \n\t"
            "sb     %[q2_r_f1],     +2(%[s3])    \n\t"

            :
            : [p2_r_f1] "r" (p2_r_f1), [p1_r_f1] "r" (p1_r_f1),
              [p0_r_f1] "r" (p0_r_f1), [q0_r_f1] "r" (q0_r_f1),
              [q1_r_f1] "r" (q1_r_f1), [q2_r_f1] "r" (q2_r_f1),
              [s3] "r" (s3)
        );
      } else if(mask & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s3])    \n\t"
            "sb     %[p0_f0],   -1(%[s3])    \n\t"
            "sb     %[q0_f0],     (%[s3])    \n\t"
            "sb     %[q1_f0],   +1(%[s3])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s3] "r" (s3)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p1_f0],   %[p1_f0],   8     \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8     \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8     \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8     \n\t"

          : [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & flat2 & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p6_l],    -7(%[s2])    \n\t"
            "sb     %[p5_l],    -6(%[s2])    \n\t"
            "sb     %[p4_l],    -5(%[s2])    \n\t"
            "sb     %[p3_l],    -4(%[s2])    \n\t"
            "sb     %[p2_l],    -3(%[s2])    \n\t"
            "sb     %[p1_l],    -2(%[s2])    \n\t"
            "sb     %[p0_l],    -1(%[s2])    \n\t"

            :
            : [p6_l] "r" (p6_l), [p5_l] "r" (p5_l), [p4_l] "r" (p4_l),
              [p3_l] "r" (p3_l), [p2_l] "r" (p2_l), [p1_l] "r" (p1_l),
              [p0_l] "r" (p0_l), [s2] "r" (s2)
        );

        __asm__ __volatile__ (
            "sb     %[q0_l],      (%[s2])    \n\t"
            "sb     %[q1_l],    +1(%[s2])    \n\t"
            "sb     %[q2_l],    +2(%[s2])    \n\t"
            "sb     %[q3_l],    +3(%[s2])    \n\t"
            "sb     %[q4_l],    +4(%[s2])    \n\t"
            "sb     %[q5_l],    +5(%[s2])    \n\t"
            "sb     %[q6_l],    +6(%[s2])    \n\t"

            :
            : [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [q3_l] "r" (q3_l), [q4_l] "r" (q4_l), [q5_l] "r" (q5_l),
              [q6_l] "r" (q6_l), [s2] "r" (s2)
        );
      } else if(mask & flat & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p2_l_f1],     -3(%[s2])    \n\t"
            "sb     %[p1_l_f1],     -2(%[s2])    \n\t"
            "sb     %[p0_l_f1],     -1(%[s2])    \n\t"
            "sb     %[q0_l_f1],       (%[s2])    \n\t"
            "sb     %[q1_l_f1],     +1(%[s2])    \n\t"
            "sb     %[q2_l_f1],     +2(%[s2])    \n\t"

            :
            : [p2_l_f1] "r" (p2_l_f1), [p1_l_f1] "r" (p1_l_f1),
              [p0_l_f1] "r" (p0_l_f1), [q0_l_f1] "r" (q0_l_f1),
              [q1_l_f1] "r" (q1_l_f1), [q2_l_f1] "r" (q2_l_f1),
              [s2] "r" (s2)
        );
      } else if(mask & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s2])    \n\t"
            "sb     %[p0_f0],   -1(%[s2])    \n\t"
            "sb     %[q0_f0],     (%[s2])    \n\t"
            "sb     %[q1_f0],   +1(%[s2])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s2] "r" (s2)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p6_l],        %[p6_l],        16     \n\t"
          "srl      %[p5_l],        %[p5_l],        16     \n\t"
          "srl      %[p4_l],        %[p4_l],        16     \n\t"
          "srl      %[p3_l],        %[p3_l],        16     \n\t"
          "srl      %[p2_l],        %[p2_l],        16     \n\t"
          "srl      %[p1_l],        %[p1_l],        16     \n\t"
          "srl      %[p0_l],        %[p0_l],        16     \n\t"
          "srl      %[q0_l],        %[q0_l],        16     \n\t"
          "srl      %[q1_l],        %[q1_l],        16     \n\t"
          "srl      %[q2_l],        %[q2_l],        16     \n\t"
          "srl      %[q3_l],        %[q3_l],        16     \n\t"
          "srl      %[q4_l],        %[q4_l],        16     \n\t"
          "srl      %[q5_l],        %[q5_l],        16     \n\t"
          "srl      %[q6_l],        %[q6_l],        16     \n\t"

          : [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l),
            [q3_l] "+r" (q3_l), [q4_l] "+r" (q4_l), [q5_l] "+r" (q5_l),
            [q6_l] "+r" (q6_l), [p6_l] "+r" (p6_l), [p5_l] "+r" (p5_l),
            [p4_l] "+r" (p4_l), [p3_l] "+r" (p3_l), [p2_l] "+r" (p2_l),
            [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l)
          :
      );

      __asm__ __volatile__ (
          "srl      %[p2_l_f1],     %[p2_l_f1],     16      \n\t"
          "srl      %[p1_l_f1],     %[p1_l_f1],     16      \n\t"
          "srl      %[p0_l_f1],     %[p0_l_f1],     16      \n\t"
          "srl      %[q0_l_f1],     %[q0_l_f1],     16      \n\t"
          "srl      %[q1_l_f1],     %[q1_l_f1],     16      \n\t"
          "srl      %[q2_l_f1],     %[q2_l_f1],     16      \n\t"
          "srl      %[p1_f0],       %[p1_f0],       8       \n\t"
          "srl      %[p0_f0],       %[p0_f0],       8       \n\t"
          "srl      %[q0_f0],       %[q0_f0],       8       \n\t"
          "srl      %[q1_f0],       %[q1_f0],       8       \n\t"

          : [p2_l_f1] "+r" (p2_l_f1), [p1_l_f1] "+r" (p1_l_f1),
            [p0_l_f1] "+r" (p0_l_f1), [q0_l_f1] "+r" (q0_l_f1),
            [q1_l_f1] "+r" (q1_l_f1), [q2_l_f1] "+r" (q2_l_f1),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & flat2 & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p6_l],    -7(%[s1])    \n\t"
            "sb     %[p5_l],    -6(%[s1])    \n\t"
            "sb     %[p4_l],    -5(%[s1])    \n\t"
            "sb     %[p3_l],    -4(%[s1])    \n\t"
            "sb     %[p2_l],    -3(%[s1])    \n\t"
            "sb     %[p1_l],    -2(%[s1])    \n\t"
            "sb     %[p0_l],    -1(%[s1])    \n\t"

            :
            : [p6_l] "r" (p6_l), [p5_l] "r" (p5_l), [p4_l] "r" (p4_l),
              [p3_l] "r" (p3_l), [p2_l] "r" (p2_l), [p1_l] "r" (p1_l),
              [p0_l] "r" (p0_l),
              [s1] "r" (s1)
        );

        __asm__ __volatile__ (
            "sb     %[q0_l],     (%[s1])    \n\t"
            "sb     %[q1_l],    1(%[s1])    \n\t"
            "sb     %[q2_l],    2(%[s1])    \n\t"
            "sb     %[q3_l],    3(%[s1])    \n\t"
            "sb     %[q4_l],    4(%[s1])    \n\t"
            "sb     %[q5_l],    5(%[s1])    \n\t"
            "sb     %[q6_l],    6(%[s1])    \n\t"

            :
            : [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [q3_l] "r" (q3_l), [q4_l] "r" (q4_l), [q5_l] "r" (q5_l),
              [q6_l] "r" (q6_l),
              [s1] "r" (s1)
        );
      } else if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p2_l_f1],     -3(%[s1])    \n\t"
            "sb     %[p1_l_f1],     -2(%[s1])    \n\t"
            "sb     %[p0_l_f1],     -1(%[s1])    \n\t"
            "sb     %[q0_l_f1],       (%[s1])    \n\t"
            "sb     %[q1_l_f1],     +1(%[s1])    \n\t"
            "sb     %[q2_l_f1],     +2(%[s1])    \n\t"

            :
            : [p2_l_f1] "r" (p2_l_f1), [p1_l_f1] "r" (p1_l_f1),
              [p0_l_f1] "r" (p0_l_f1),
              [q0_l_f1] "r" (q0_l_f1), [q1_l_f1] "r" (q1_l_f1),
              [q2_l_f1] "r" (q2_l_f1),
              [s1] "r" (s1)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   -2(%[s1])    \n\t"
            "sb     %[p0_f0],   -1(%[s1])    \n\t"
            "sb     %[q0_f0],     (%[s1])    \n\t"
            "sb     %[q1_f0],   +1(%[s1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s1] "r" (s1)
        );
      }
    }
  }
}

void vp9_mbloop_filter_horizontal_edge_dspr2(unsigned char *s,
                                             int pitch,
                                             const uint8_t *blimit,
                                             const uint8_t *limit,
                                             const uint8_t *thresh,
                                             int count) {
  uint32_t mask;
  uint32_t hev, flat;
  uint8_t i;
  unsigned char *sp3, *sp2, *sp1, *sp0, *sq0, *sq1, *sq2, *sq3;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;
  uint32_t p1_f0, p0_f0, q0_f0, q1_f0;
  uint32_t p3, p2, p1, p0, q0, q1, q2, q3;
  uint32_t p0_l, p1_l, p2_l, p3_l, q0_l, q1_l, q2_l, q3_l;
  uint32_t p0_r, p1_r, p2_r, p3_r, q0_r, q1_r, q2_r, q3_r;

  uflimit = *blimit;
  ulimit  = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb       %[thresh_vec],    %[uthresh]    \n\t"
      "replv.qb       %[flimit_vec],    %[uflimit]    \n\t"
      "replv.qb       %[limit_vec],     %[ulimit]     \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  /* prefetch data for store */
  vp9_prefetch_store(s);

  for(i = 0; i < 2; i++) {
    sp3 = s - (pitch << 2);
    sp2 = sp3 + pitch;
    sp1 = sp2 + pitch;
    sp0 = sp1 + pitch;
    sq0 = s;
    sq1 = s + pitch;
    sq2 = sq1 + pitch;
    sq3 = sq2 + pitch;

    __asm__ __volatile__ (
        "lw     %[p3],      (%[sp3])    \n\t"
        "lw     %[p2],      (%[sp2])    \n\t"
        "lw     %[p1],      (%[sp1])    \n\t"
        "lw     %[p0],      (%[sp0])    \n\t"
        "lw     %[q0],      (%[sq0])    \n\t"
        "lw     %[q1],      (%[sq1])    \n\t"
        "lw     %[q2],      (%[sq2])    \n\t"
        "lw     %[q3],      (%[sq3])    \n\t"

        : [p3] "=&r" (p3), [p2] "=&r" (p2), [p1] "=&r" (p1), [p0] "=&r" (p0),
          [q3] "=&r" (q3), [q2] "=&r" (q2), [q1] "=&r" (q1), [q0] "=&r" (q0)
        : [sp3] "r" (sp3), [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
          [sq3] "r" (sq3), [sq2] "r" (sq2), [sq1] "r" (sq1), [sq0] "r" (sq0)
    );


    filter_hev_mask_flatmask4_dspr2(limit_vec, flimit_vec, thresh_vec,
                                    p1, p0, p3, p2, q0, q1, q2, q3,
                                    &hev, &mask, &flat);

    if((flat == 0) && (mask != 0)) {
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      __asm__ __volatile__ (
          "sw       %[p1_f0],   (%[sp1])    \n\t"
          "sw       %[p0_f0],   (%[sp0])    \n\t"
          "sw       %[q0_f0],   (%[sq0])    \n\t"
          "sw       %[q1_f0],   (%[sq1])    \n\t"

          :
          : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
            [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
            [sp1] "r" (sp1), [sp0] "r" (sp0),
            [sq0] "r" (sq0), [sq1] "r" (sq1)
      );
    } else if((mask & flat) == 0xFFFFFFFF) {
      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      COMBINE_LEFT_RIGHT_0TO2

      __asm__ __volatile__ (
          "sw       %[p2],      (%[sp2])    \n\t"
          "sw       %[p1],      (%[sp1])    \n\t"
          "sw       %[p0],      (%[sp0])    \n\t"
          "sw       %[q0],      (%[sq0])    \n\t"
          "sw       %[q1],      (%[sq1])    \n\t"
          "sw       %[q2],      (%[sq2])    \n\t"

          :
          : [p2] "r" (p2), [p1] "r" (p1), [p0] "r" (p0),
            [q0] "r" (q0), [q1] "r" (q1), [q2] "r" (q2),
            [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
            [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
      );
    } else if((flat != 0) && (mask != 0)) {
      /* filtering */
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);


      if(mask & flat & 0x000000FF) {
        __asm__ __volatile__ (
            "sb     %[p2_r],    (%[sp2])    \n\t"
            "sb     %[p1_r],    (%[sp1])    \n\t"
            "sb     %[p0_r],    (%[sp0])    \n\t"
            "sb     %[q0_r],    (%[sq0])    \n\t"
            "sb     %[q1_r],    (%[sq1])    \n\t"
            "sb     %[q2_r],    (%[sq2])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  (%[sp1])    \n\t"
            "sb         %[p0_f0],  (%[sp0])    \n\t"
            "sb         %[q0_f0],  (%[sq0])    \n\t"
            "sb         %[q1_f0],  (%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_r],    %[p2_r],    16      \n\t"
          "srl      %[p1_r],    %[p1_r],    16      \n\t"
          "srl      %[p0_r],    %[p0_r],    16      \n\t"
          "srl      %[q0_r],    %[q0_r],    16      \n\t"
          "srl      %[q1_r],    %[q1_r],    16      \n\t"
          "srl      %[q2_r],    %[q2_r],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_r] "+r" (p2_r), [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r),
            [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r), [q2_r] "+r" (q2_r),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p2_r],    +1(%[sp2])    \n\t"
            "sb     %[p1_r],    +1(%[sp1])    \n\t"
            "sb     %[p0_r],    +1(%[sp0])    \n\t"
            "sb     %[q0_r],    +1(%[sq0])    \n\t"
            "sb     %[q1_r],    +1(%[sq1])    \n\t"
            "sb     %[q2_r],    +1(%[sq2])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   +1(%[sp1])    \n\t"
            "sb     %[p0_f0],   +1(%[sp0])    \n\t"
            "sb     %[q0_f0],   +1(%[sq0])    \n\t"
            "sb     %[q1_f0],   +1(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p1_f0],   %[p1_f0],   8     \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8     \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8     \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8     \n\t"

          : [p2] "+r" (p2), [p1] "+r" (p1), [p0] "+r" (p0),
            [q0] "+r" (q0), [q1] "+r" (q1), [q2] "+r" (q2),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p2_l],    +2(%[sp2])    \n\t"
            "sb     %[p1_l],    +2(%[sp1])    \n\t"
            "sb     %[p0_l],    +2(%[sp0])    \n\t"
            "sb     %[q0_l],    +2(%[sq0])    \n\t"
            "sb     %[q1_l],    +2(%[sq1])    \n\t"
            "sb     %[q2_l],    +2(%[sq2])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   +2(%[sp1])    \n\t"
            "sb     %[p0_f0],   +2(%[sp0])    \n\t"
            "sb     %[q0_f0],   +2(%[sq0])    \n\t"
            "sb     %[q1_f0],   +2(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_l],    %[p2_l],    16      \n\t"
          "srl      %[p1_l],    %[p1_l],    16      \n\t"
          "srl      %[p0_l],    %[p0_l],    16      \n\t"
          "srl      %[q0_l],    %[q0_l],    16      \n\t"
          "srl      %[q1_l],    %[q1_l],    16      \n\t"
          "srl      %[q2_l],    %[q2_l],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_l] "+r" (p2_l), [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l),
            [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p2_l],    +3(%[sp2])    \n\t"
            "sb     %[p1_l],    +3(%[sp1])    \n\t"
            "sb     %[p0_l],    +3(%[sp0])    \n\t"
            "sb     %[q0_l],    +3(%[sq0])    \n\t"
            "sb     %[q1_l],    +3(%[sq1])    \n\t"
            "sb     %[q2_l],    +3(%[sq2])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [sp2] "r" (sp2), [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1), [sq2] "r" (sq2)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb     %[p1_f0],   +3(%[sp1])    \n\t"
            "sb     %[p0_f0],   +3(%[sp0])    \n\t"
            "sb     %[q0_f0],   +3(%[sq0])    \n\t"
            "sb     %[q1_f0],   +3(%[sq1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [sp1] "r" (sp1), [sp0] "r" (sp0),
              [sq0] "r" (sq0), [sq1] "r" (sq1)
        );
      }
    }

    s = s + 4;
  }
}

void vp9_loop_filter_horizontal_edge_dspr2(unsigned char *s,
                                           int pitch,
                                           const uint8_t *blimit,
                                           const uint8_t *limit,
                                           const uint8_t *thresh,
                                           int count) {
  uint8_t i;
  uint32_t mask;
  uint32_t hev;
  uint32_t pm1, p0, p1, p2, p3, p4, p5, p6;
  unsigned char *sm1, *s0, *s1, *s2, *s3, *s4, *s5, *s6;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;

  uflimit = *blimit;
  ulimit = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb       %[thresh_vec],    %[uthresh]    \n\t"
      "replv.qb       %[flimit_vec],    %[uflimit]    \n\t"
      "replv.qb       %[limit_vec],     %[ulimit]     \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  /* prefetch data for store */
  vp9_prefetch_store(s);

  /* loop filter designed to work using chars so that we can make maximum use
   * of 8 bit simd instructions.
   */
  for(i = 0; i < 2; i++) {
    sm1 = s - (pitch << 2);
    s0 = sm1 + pitch;
    s1 = s0 + pitch ;
    s2 = s - pitch;
    s3 = s;
    s4 = s + pitch;
    s5 = s4 + pitch;
    s6 = s5 + pitch;

    __asm__ __volatile__ (
        "lw     %[p1],  (%[s1])    \n\t"
        "lw     %[p2],  (%[s2])    \n\t"
        "lw     %[p3],  (%[s3])    \n\t"
        "lw     %[p4],  (%[s4])    \n\t"

        : [p1] "=&r" (p1), [p2] "=&r" (p2),
          [p3] "=&r" (p3), [p4] "=&r" (p4)
        : [s1] "r" (s1), [s2] "r" (s2),
          [s3] "r" (s3), [s4] "r" (s4)
    );

    /* if (p1 - p4 == 0) and (p2 - p3 == 0)
     * mask will be zero and filtering is not needed
     */
    if (!(((p1 - p4) == 0) && ((p2 - p3) == 0))) {
      __asm__ __volatile__ (
          "lw       %[pm1], (%[sm1])   \n\t"
          "lw       %[p0],  (%[s0])    \n\t"
          "lw       %[p5],  (%[s5])    \n\t"
          "lw       %[p6],  (%[s6])    \n\t"

          : [pm1] "=&r" (pm1), [p0] "=&r" (p0),
            [p5] "=&r" (p5), [p6] "=&r" (p6)
          : [sm1] "r" (sm1), [s0] "r" (s0),
            [s5] "r" (s5), [s6] "r" (s6)
      );

      vp9_filter_hev_mask_dspr2(limit_vec, flimit_vec, p1, p2,
                                pm1, p0, p3, p4, p5, p6,
                               thresh_vec, &hev, &mask);

      /* if mask == 0 do filtering is not needed */
      if (mask) {
        /* filtering */
        vp9_filter_dspr2(mask, hev, &p1, &p2, &p3, &p4);

        __asm__ __volatile__ (
            "sw     %[p1],  (%[s1])    \n\t"
            "sw     %[p2],  (%[s2])    \n\t"
            "sw     %[p3],  (%[s3])    \n\t"
            "sw     %[p4],  (%[s4])    \n\t"

            :
            : [p1] "r" (p1), [p2] "r" (p2), [p3] "r" (p3), [p4] "r" (p4),
              [s1] "r" (s1), [s2] "r" (s2), [s3] "r" (s3), [s4] "r" (s4)
        );
      }
    }
    s=s+4;
  }
}

void vp9_loop_filter_vertical_edge_dspr2(unsigned char *s,
                                         int pitch,
                                         const uint8_t *blimit,
                                         const uint8_t *limit,
                                         const uint8_t *thresh,
                                         int count) {
  uint8_t i;
  uint32_t mask, hev;
  uint32_t pm1, p0, p1, p2, p3, p4, p5, p6;
  unsigned char *s1, *s2, *s3, *s4;
  uint32_t prim1, prim2, sec3, sec4, prim3, prim4;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;

  uflimit = *blimit;
  ulimit = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb       %[thresh_vec],    %[uthresh]    \n\t"
      "replv.qb       %[flimit_vec],    %[uflimit]    \n\t"
      "replv.qb       %[limit_vec],     %[ulimit]     \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  /* prefetch data for store */
  vp9_prefetch_store(s + pitch);

  for(i = 0; i < 2; i++) {
    s1 = s;
    s2 = s + pitch;
    s3 = s2 + pitch;
    s4 = s3 + pitch;
    s  = s4 + pitch;

    /* load quad-byte vectors
     * memory is 4 byte aligned
     */
    p2  = *((uint32_t *)(s1 - 4));
    p6  = *((uint32_t *)(s1));
    p1  = *((uint32_t *)(s2 - 4));
    p5  = *((uint32_t *)(s2));
    p0  = *((uint32_t *)(s3 - 4));
    p4  = *((uint32_t *)(s3));
    pm1 = *((uint32_t *)(s4 - 4));
    p3  = *((uint32_t *)(s4));

    /* transpose pm1, p0, p1, p2 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[p2],      %[p1]       \n\t"
        "precr.qb.ph    %[prim2],   %[p2],      %[p1]       \n\t"
        "precrq.qb.ph   %[prim3],   %[p0],      %[pm1]      \n\t"
        "precr.qb.ph    %[prim4],   %[p0],      %[pm1]      \n\t"

        "precrq.qb.ph   %[p1],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[pm1],     %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[p2],      %[p1],      %[sec3]     \n\t"
        "precrq.ph.w    %[p0],      %[pm1],     %[sec4]     \n\t"
        "append         %[p1],      %[sec3],    16          \n\t"
        "append         %[pm1],     %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [p2] "+r" (p2), [p1] "+r" (p1), [p0] "+r" (p0), [pm1] "+r" (pm1),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* transpose p3, p4, p5, p6 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[p6],      %[p5]       \n\t"
        "precr.qb.ph    %[prim2],   %[p6],      %[p5]       \n\t"
        "precrq.qb.ph   %[prim3],   %[p4],      %[p3]       \n\t"
        "precr.qb.ph    %[prim4],   %[p4],      %[p3]       \n\t"

        "precrq.qb.ph   %[p5],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[p3],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[p6],      %[p5],      %[sec3]     \n\t"
        "precrq.ph.w    %[p4],      %[p3],      %[sec4]     \n\t"
        "append         %[p5],      %[sec3],    16          \n\t"
        "append         %[p3],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [p6] "+r" (p6), [p5] "+r" (p5), [p4] "+r" (p4), [p3] "+r" (p3),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* if (p1 - p4 == 0) and (p2 - p3 == 0)
     * mask will be zero and filtering is not needed
     */
    if (!(((p1 - p4) == 0) && ((p2 - p3) == 0))) {

      vp9_filter_hev_mask_dspr2(limit_vec, flimit_vec, p1, p2, pm1,
                                p0, p3, p4, p5, p6, thresh_vec,
                                &hev, &mask);

      /* if mask == 0 do filtering is not needed */
      if (mask) {
        /* filtering */
        vp9_filter_dspr2(mask, hev, &p1, &p2, &p3, &p4);

        /* unpack processed 4x4 neighborhood
         * don't use transpose on output data
         * because memory isn't aligned
         */
        __asm__ __volatile__ (
            "sb     %[p4],   1(%[s4])    \n\t"
            "sb     %[p3],   0(%[s4])    \n\t"
            "sb     %[p2],  -1(%[s4])    \n\t"
            "sb     %[p1],  -2(%[s4])    \n\t"

            :
            : [p4] "r" (p4), [p3] "r" (p3),
              [p2] "r" (p2), [p1] "r" (p1),
              [s4] "r" (s4)
        );

        __asm__ __volatile__ (
            "srl    %[p4],  %[p4],  8     \n\t"
            "srl    %[p3],  %[p3],  8     \n\t"
            "srl    %[p2],  %[p2],  8     \n\t"
            "srl    %[p1],  %[p1],  8     \n\t"

            : [p4] "+r" (p4), [p3] "+r" (p3),
              [p2] "+r" (p2), [p1] "+r" (p1)
            :
        );

        __asm__ __volatile__ (
            "sb     %[p4],   1(%[s3])    \n\t"
            "sb     %[p3],   0(%[s3])    \n\t"
            "sb     %[p2],  -1(%[s3])    \n\t"
            "sb     %[p1],  -2(%[s3])    \n\t"

            : [p1] "+r" (p1)
            : [p4] "r" (p4), [p3] "r" (p3),
              [p2] "r" (p2),
              [s3] "r" (s3)
        );

        __asm__ __volatile__ (
            "srl    %[p4],  %[p4],  8     \n\t"
            "srl    %[p3],  %[p3],  8     \n\t"
            "srl    %[p2],  %[p2],  8     \n\t"
            "srl    %[p1],  %[p1],  8     \n\t"

            : [p4] "+r" (p4), [p3] "+r" (p3),
              [p2] "+r" (p2), [p1] "+r" (p1)
            :
        );

        __asm__ __volatile__ (
            "sb     %[p4],   1(%[s2])    \n\t"
            "sb     %[p3],   0(%[s2])    \n\t"
            "sb     %[p2],  -1(%[s2])    \n\t"
            "sb     %[p1],  -2(%[s2])    \n\t"

            :
            : [p4] "r" (p4), [p3] "r" (p3),
              [p2] "r" (p2), [p1] "r" (p1),
              [s2] "r" (s2)
        );

        __asm__ __volatile__ (
            "srl    %[p4],  %[p4],  8     \n\t"
            "srl    %[p3],  %[p3],  8     \n\t"
            "srl    %[p2],  %[p2],  8     \n\t"
            "srl    %[p1],  %[p1],  8     \n\t"

            : [p4] "+r" (p4), [p3] "+r" (p3),
              [p2] "+r" (p2), [p1] "+r" (p1)
            :
        );

        __asm__ __volatile__ (
            "sb     %[p4],   1(%[s1])    \n\t"
            "sb     %[p3],   0(%[s1])    \n\t"
            "sb     %[p2],  -1(%[s1])    \n\t"
            "sb     %[p1],  -2(%[s1])    \n\t"

            :
            : [p4] "r" (p4), [p3] "r" (p3),
              [p2] "r" (p2), [p1] "r" (p1),
              [s1] "r" (s1)
        );
      }
    }
  }
}

void vp9_mbloop_filter_vertical_edge_dspr2(unsigned char *s,
                                           int pitch,
                                           const uint8_t *blimit,
                                           const uint8_t *limit,
                                           const uint8_t *thresh,
                                           int count) {
  uint8_t i;
  uint32_t mask, hev,flat;
  unsigned char *s1, *s2, *s3, *s4;
  uint32_t prim1, prim2, sec3, sec4, prim3, prim4;
  unsigned int thresh_vec, flimit_vec, limit_vec;
  unsigned int uflimit, ulimit, uthresh;
  uint32_t p3, p2, p1, p0, q3, q2, q1, q0;
  uint32_t p1_f0, p0_f0, q0_f0, q1_f0;
  uint32_t p0_l, p1_l, p2_l, p3_l, q0_l, q1_l, q2_l, q3_l;
  uint32_t p0_r, p1_r, p2_r, p3_r, q0_r, q1_r, q2_r, q3_r;

  uflimit = *blimit;
  ulimit  = *limit;
  uthresh = *thresh;

  /* create quad-byte */
  __asm__ __volatile__ (
      "replv.qb     %[thresh_vec],  %[uthresh]    \n\t"
      "replv.qb     %[flimit_vec],  %[uflimit]    \n\t"
      "replv.qb     %[limit_vec],   %[ulimit]     \n\t"

      : [thresh_vec] "=&r" (thresh_vec), [flimit_vec] "=&r" (flimit_vec),
        [limit_vec] "=r" (limit_vec)
      : [uthresh] "r" (uthresh), [uflimit] "r" (uflimit), [ulimit] "r" (ulimit)
  );

  vp9_prefetch_store(s + pitch);

  for(i = 0; i < 2; i++) {
    s1 = s;
    s2 = s + pitch;
    s3 = s2 + pitch;
    s4 = s3 + pitch;
    s  = s4 + pitch;

    __asm__ __volatile__ (
        "lw     %[p0],  -4(%[s1])    \n\t"
        "lw     %[p1],  -4(%[s2])    \n\t"
        "lw     %[p2],  -4(%[s3])    \n\t"
        "lw     %[p3],  -4(%[s4])    \n\t"
        "lw     %[q3],    (%[s1])    \n\t"
        "lw     %[q2],    (%[s2])    \n\t"
        "lw     %[q1],    (%[s3])    \n\t"
        "lw     %[q0],    (%[s4])    \n\t"

        : [p3] "=&r" (p3), [p2] "=&r" (p2), [p1] "=&r" (p1), [p0] "=&r" (p0),
          [q0] "=&r" (q0), [q1] "=&r" (q1), [q2] "=&r" (q2), [q3] "=&r" (q3)
        : [s1] "r" (s1), [s2] "r" (s2), [s3] "r" (s3), [s4] "r" (s4)
    );

    /* implemented transpose technique

    original (when loaded from memory)

     -4    -3   -2     -1    memory   register     +1    +2    +3    +4
    p0_0  p0_1  p0_2  p0_3     s1        p0        q3_0  q3_1  q3_2  q3_3
    p1_0  p1_1  p1_2  p1_3     s2        p1        q2_0  q2_1  q2_2  q2_3
    p2_0  p2_1  p2_2  p2_3     s3        p2        q1_0  q1_1  q1_2  q1_3
    p3_0  p3_1  p3_2  p3_3     s4        p3        q0_0  q0_1  q0_2  q0_3

    after transpose

     -4    -3   -2     -1       register      +1    +2    +3    +4
    p3_3  p2_3  p1_3  p0_3         p0         q0_3  q1_3  q2_3  q3_3
    p3_2  p2_2  p1_2  p0_2         p1         q0_2  q1_2  q2_2  q3_2
    p3_1  p2_1  p1_1  p0_1         p2         q0_1  q1_1  q2_1  q3_1
    p3_0  p2_0  p1_0  p0_0         p3         q0_0  q1_0  q2_0  q3_0

    */

    /* transpose p3, p2, p1, p0 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[p0],      %[p1]       \n\t"
        "precr.qb.ph    %[prim2],   %[p0],      %[p1]       \n\t"
        "precrq.qb.ph   %[prim3],   %[p2],      %[p3]       \n\t"
        "precr.qb.ph    %[prim4],   %[p2],      %[p3]       \n\t"

        "precrq.qb.ph   %[p1],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[p3],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[p0],      %[p1],      %[sec3]     \n\t"
        "precrq.ph.w    %[p2],      %[p3],      %[sec4]     \n\t"
        "append         %[p1],      %[sec3],    16          \n\t"
        "append         %[p3],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [p0] "+r" (p0), [p1] "+r" (p1), [p2] "+r" (p2), [p3] "+r" (p3),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    /* transpose q0, q1, q2, q3 */
    __asm__ __volatile__ (
        "precrq.qb.ph   %[prim1],   %[q3],      %[q2]       \n\t"
        "precr.qb.ph    %[prim2],   %[q3],      %[q2]       \n\t"
        "precrq.qb.ph   %[prim3],   %[q1],      %[q0]       \n\t"
        "precr.qb.ph    %[prim4],   %[q1],      %[q0]       \n\t"

        "precrq.qb.ph   %[q2],      %[prim1],   %[prim2]    \n\t"
        "precr.qb.ph    %[q0],      %[prim1],   %[prim2]    \n\t"
        "precrq.qb.ph   %[sec3],    %[prim3],   %[prim4]    \n\t"
        "precr.qb.ph    %[sec4],    %[prim3],   %[prim4]    \n\t"

        "precrq.ph.w    %[q3],      %[q2],      %[sec3]     \n\t"
        "precrq.ph.w    %[q1],      %[q0],      %[sec4]     \n\t"
        "append         %[q2],      %[sec3],    16          \n\t"
        "append         %[q0],      %[sec4],    16          \n\t"

        : [prim1] "=&r" (prim1), [prim2] "=&r" (prim2),
          [prim3] "=&r" (prim3), [prim4] "=&r" (prim4),
          [q3] "+r" (q3), [q2] "+r" (q2), [q1] "+r" (q1), [q0] "+r" (q0),
          [sec3] "=&r" (sec3), [sec4] "=&r" (sec4)
        :
    );

    filter_hev_mask_flatmask4_dspr2(limit_vec, flimit_vec, thresh_vec,
                                    p1, p0, p3, p2, q0, q1, q2, q3,
                                    &hev, &mask, &flat);

    if((flat == 0) && (mask != 0)) {
      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);
      STORE_F0
    } else if((mask & flat) == 0xFFFFFFFF) {
      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      STORE_F1
    } else if((flat != 0) && (mask != 0)) {

      vp9_filter1_dspr2(mask, hev, p1, p0, q0, q1,
                        &p1_f0, &p0_f0, &q0_f0, &q1_f0);

      /* left 2 element operation */
      PACK_LEFT_0TO3
      mbfilter_dspr2(&p3_l, &p2_l, &p1_l, &p0_l,
                     &q0_l, &q1_l, &q2_l, &q3_l);

      /* right 2 element operation */
      PACK_RIGHT_0TO3
      mbfilter_dspr2(&p3_r, &p2_r, &p1_r, &p0_r,
                     &q0_r, &q1_r, &q2_r, &q3_r);

      if(mask & flat & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p2_r],  -3(%[s4])    \n\t"
            "sb         %[p1_r],  -2(%[s4])    \n\t"
            "sb         %[p0_r],  -1(%[s4])    \n\t"
            "sb         %[q0_r],    (%[s4])    \n\t"
            "sb         %[q1_r],  +1(%[s4])    \n\t"
            "sb         %[q2_r],  +2(%[s4])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [s4] "r" (s4)
        );
      } else if(mask & 0x000000FF) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  -2(%[s4])    \n\t"
            "sb         %[p0_f0],  -1(%[s4])    \n\t"
            "sb         %[q0_f0],    (%[s4])    \n\t"
            "sb         %[q1_f0],  +1(%[s4])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s4] "r" (s4)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_r],    %[p2_r],    16      \n\t"
          "srl      %[p1_r],    %[p1_r],    16      \n\t"
          "srl      %[p0_r],    %[p0_r],    16      \n\t"
          "srl      %[q0_r],    %[q0_r],    16      \n\t"
          "srl      %[q1_r],    %[q1_r],    16      \n\t"
          "srl      %[q2_r],    %[q2_r],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_r] "+r" (p2_r), [p1_r] "+r" (p1_r), [p0_r] "+r" (p0_r),
            [q0_r] "+r" (q0_r), [q1_r] "+r" (q1_r), [q2_r] "+r" (q2_r),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p2_r],  -3(%[s3])    \n\t"
            "sb         %[p1_r],  -2(%[s3])    \n\t"
            "sb         %[p0_r],  -1(%[s3])    \n\t"
            "sb         %[q0_r],    (%[s3])    \n\t"
            "sb         %[q1_r],  +1(%[s3])    \n\t"
            "sb         %[q2_r],  +2(%[s3])    \n\t"

            :
            : [p2_r] "r" (p2_r), [p1_r] "r" (p1_r), [p0_r] "r" (p0_r),
              [q0_r] "r" (q0_r), [q1_r] "r" (q1_r), [q2_r] "r" (q2_r),
              [s3] "r" (s3)
        );
      } else if(mask & 0x0000FF00) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  -2(%[s3])    \n\t"
            "sb         %[p0_f0],  -1(%[s3])    \n\t"
            "sb         %[q0_f0],    (%[s3])    \n\t"
            "sb         %[q1_f0],  +1(%[s3])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s3] "r" (s3)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p1_f0],   %[p1_f0],   8     \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8     \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8     \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8     \n\t"

          : [p2] "+r" (p2), [p1] "+r" (p1), [p0] "+r" (p0),
            [q0] "+r" (q0), [q1] "+r" (q1), [q2] "+r" (q2),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0x00FF0000) {
        __asm__ __volatile__ (
          "sb         %[p2_l],  -3(%[s2])    \n\t"
          "sb         %[p1_l],  -2(%[s2])    \n\t"
          "sb         %[p0_l],  -1(%[s2])    \n\t"
          "sb         %[q0_l],    (%[s2])    \n\t"
          "sb         %[q1_l],  +1(%[s2])    \n\t"
          "sb         %[q2_l],  +2(%[s2])    \n\t"

          :
          : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
            [s2] "r" (s2)
        );
      } else if(mask & 0x00FF0000) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  -2(%[s2])    \n\t"
            "sb         %[p0_f0],  -1(%[s2])    \n\t"
            "sb         %[q0_f0],    (%[s2])    \n\t"
            "sb         %[q1_f0],  +1(%[s2])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s2] "r" (s2)
        );
      }

      __asm__ __volatile__ (
          "srl      %[p2_l],    %[p2_l],    16      \n\t"
          "srl      %[p1_l],    %[p1_l],    16      \n\t"
          "srl      %[p0_l],    %[p0_l],    16      \n\t"
          "srl      %[q0_l],    %[q0_l],    16      \n\t"
          "srl      %[q1_l],    %[q1_l],    16      \n\t"
          "srl      %[q2_l],    %[q2_l],    16      \n\t"
          "srl      %[p1_f0],   %[p1_f0],   8       \n\t"
          "srl      %[p0_f0],   %[p0_f0],   8       \n\t"
          "srl      %[q0_f0],   %[q0_f0],   8       \n\t"
          "srl      %[q1_f0],   %[q1_f0],   8       \n\t"

          : [p2_l] "+r" (p2_l), [p1_l] "+r" (p1_l), [p0_l] "+r" (p0_l),
            [q0_l] "+r" (q0_l), [q1_l] "+r" (q1_l), [q2_l] "+r" (q2_l),
            [p1_f0] "+r" (p1_f0), [p0_f0] "+r" (p0_f0),
            [q0_f0] "+r" (q0_f0), [q1_f0] "+r" (q1_f0)
          :
      );

      if(mask & flat & 0xFF000000) {
        __asm__ __volatile__ (
            "sb         %[p2_l],  -3(%[s1])    \n\t"
            "sb         %[p1_l],  -2(%[s1])    \n\t"
            "sb         %[p0_l],  -1(%[s1])    \n\t"
            "sb         %[q0_l],    (%[s1])    \n\t"
            "sb         %[q1_l],  +1(%[s1])    \n\t"
            "sb         %[q2_l],  +2(%[s1])    \n\t"

            :
            : [p2_l] "r" (p2_l), [p1_l] "r" (p1_l), [p0_l] "r" (p0_l),
              [q0_l] "r" (q0_l), [q1_l] "r" (q1_l), [q2_l] "r" (q2_l),
              [s1] "r" (s1)
        );
      } else if(mask & 0xFF000000) {
        __asm__ __volatile__ (
            "sb         %[p1_f0],  -2(%[s1])    \n\t"
            "sb         %[p0_f0],  -1(%[s1])    \n\t"
            "sb         %[q0_f0],    (%[s1])    \n\t"
            "sb         %[q1_f0],  +1(%[s1])    \n\t"

            :
            : [p1_f0] "r" (p1_f0), [p0_f0] "r" (p0_f0),
              [q0_f0] "r" (q0_f0), [q1_f0] "r" (q1_f0),
              [s1] "r" (s1)
        );
      }
    }
  }
}
#endif
