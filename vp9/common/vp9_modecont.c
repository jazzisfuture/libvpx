/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp9/common/vp9_entropy.h"

const int vp9_default_mode_contexts[INTER_MODE_CONTEXTS][6] = {
#if CONFIG_SBSEGMENT
    {/* 0 */         2,   118,     1,   194,     1,     1,   },
    {/* 1 */         3,    88,   101,   184,     1,     1,   },
    {/* 2 */         2,    62,    66,   152,     1,     1,   },
    {/* 3 */         4,   147,    88,   185,     1,     1,   },
    {/* 4 */        25,    99,    62,   231,     1,     1,   },
    {/* 5 */         4,   127,    71,   154,     1,     1,   },
    {/* 6 */        16,    69,    64,   228,     1,     1,   },
    {/* 7 */         2,    48,     8,    98,   190,   253,   },
    {/* 8 */         2,   107,    49,    79,    97,   255,   },
    {/* 9 */         3,   101,    39,    71,   152,   252,   },
    {/* 10 */         2,   132,    46,   120,   131,   255,   },
    {/* 11 */         2,    84,    36,   183,   102,   253,   },
    {/* 12 */         2,   109,    43,    91,   157,   254,   },
    {/* 13 */         6,    84,    27,   127,   140,   252,   },
    {/* 14 */         3,   211,   150,   255,   250,   250,   },
    {/* 15 */         2,   168,   217,   255,   250,   250,   },
    {/* 16 */         7,   150,   149,   255,   250,   250,   },
    {/* 17 */         2,   146,   217,   255,   250,   250,   },
    {/* 18 */         4,   171,   179,   255,   250,   250,   },
    {/* 19 */         2,   151,   207,   255,   250,   250,   },
    {/* 20 */        11,   121,    35,   255,   250,   250,   },

#else
    // macroblock
    {2,    63,     1,   221,     1,     1},
    {2,    93,   111,   207,     1,     1},
    {42,   13,    70,   177,     1,     1},
    {4,   127,    87,   219,     1,     1},
    {21,   57,    50,   251,     1,     1},
    {5,    92,    55,   197,     1,     1},
    {37,   81,    66,   227,     1,     1},

    // super macroblock
    {1,   101,     1,   127,   127,   252},
    {1,    80,    77,   127,   127,   252},
    {103,  25,    52,   100,   127,   252},
    {2,   120,    62,   127,   127,   252},
    {6,    47,     4,   127,   127,   252},
    {2,    65,    46,   127,   127,   252},
    {13,   61,    13,   150,   127,   252},
#endif
};
