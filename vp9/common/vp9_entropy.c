/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdio.h>

#include "vp9/common/vp9_entropy.h"
#include "string.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_entropymode.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_coefupdateprobs.h"

const int vp9_i8x8_block[4] = {0, 2, 8, 10};

DECLARE_ALIGNED(16, const uint8_t, vp9_norm[256]) = {
  0, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Unified coefficient band structure used by all block sizes
DECLARE_ALIGNED(16, const int, vp9_coef_bands8x8[64]) = {
  0, 1, 2, 3, 4, 4, 5, 5,
  1, 2, 3, 4, 4, 5, 5, 5,
  2, 3, 4, 4, 5, 5, 5, 5,
  3, 4, 4, 5, 5, 5, 5, 5,
  4, 4, 5, 5, 5, 5, 5, 5,
  4, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5
};
DECLARE_ALIGNED(16, const int, vp9_coef_bands4x4[16]) = {
  0, 1, 2, 3,
  1, 2, 3, 4,
  2, 3, 4, 5,
  3, 4, 5, 5
};

DECLARE_ALIGNED(16, const uint8_t, vp9_pt_energy_class[MAX_ENTROPY_TOKENS]) = {
  0, 1, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5
};

DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_4x4[16]) = {
   0,  4,  1,  5,
   8,  2, 12,  9,
   3,  6, 13, 10,
   7, 14, 11, 15,
};

DECLARE_ALIGNED(16, const int, vp9_col_scan_4x4[16]) = {
   0,  4,  8,  1,
  12,  5,  9,  2,
  13,  6, 10,  3,
   7, 14, 11, 15,
};

DECLARE_ALIGNED(16, const int, vp9_row_scan_4x4[16]) = {
   0,  1,  4,  2,
   5,  3,  6,  8,
   9,  7, 12, 10,
  13, 11, 14, 15,
};

DECLARE_ALIGNED(64, const int, vp9_default_zig_zag1d_8x8[64]) = {
   0,  8,  1, 16,  9,  2, 17, 24,
  10,  3, 18, 25, 32, 11,  4, 26,
  33, 19, 40, 12, 34, 27,  5, 41,
  20, 48, 13, 35, 42, 28, 21,  6,
  49, 56, 36, 43, 29,  7, 14, 50,
  57, 44, 22, 37, 15, 51, 58, 30,
  45, 23, 52, 59, 38, 31, 60, 53,
  46, 39, 61, 54, 47, 62, 55, 63,
};

DECLARE_ALIGNED(16, const int, vp9_col_scan_8x8[64]) = {
   0,  8, 16,  1, 24,  9, 32, 17,
   2, 40, 25, 10, 33, 18, 48,  3,
  26, 41, 11, 56, 19, 34,  4, 49,
  27, 42, 12, 35, 20, 57, 50, 28,
   5, 43, 13, 36, 58, 51, 21, 44,
   6, 29, 59, 37, 14, 52, 22,  7,
  45, 60, 30, 15, 38, 53, 23, 46,
  31, 61, 39, 54, 47, 62, 55, 63,
};

DECLARE_ALIGNED(16, const int, vp9_row_scan_8x8[64]) = {
   0,  1,  2,  8,  9,  3, 16, 10,
   4, 17, 11, 24,  5, 18, 25, 12,
  19, 26, 32,  6, 13, 20, 33, 27,
   7, 34, 40, 21, 28, 41, 14, 35,
  48, 42, 29, 36, 49, 22, 43, 15,
  56, 37, 50, 44, 30, 57, 23, 51,
  58, 45, 38, 52, 31, 59, 53, 46,
  60, 39, 61, 47, 54, 55, 62, 63,
};

DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_16x16[256]) = {
    0,  16,   1,  32,  17,   2,  48,  33,  18,   3,  64,  34,  49,  19,  65,  80,
   50,   4,  35,  66,  20,  81,  96,  51,   5,  36,  82,  97,  67, 112,  21,  52,
   98,  37,  83, 113,   6,  68, 128,  53,  22,  99, 114,  84,   7, 129,  38,  69,
  100, 115, 144, 130,  85,  54,  23,   8, 145,  39,  70, 116, 101, 131, 160, 146,
   55,  86,  24,  71, 132, 117, 161,  40,   9, 102, 147, 176, 162,  87,  56,  25,
  133, 118, 177, 148,  72, 103,  41, 163,  10, 192, 178,  88,  57, 134, 149, 119,
   26, 164,  73, 104, 193,  42, 179, 208,  11, 135,  89, 165, 120, 150,  58, 194,
  180,  27,  74, 209, 105, 151, 136,  43,  90, 224, 166, 195, 181, 121, 210,  59,
   12, 152, 106, 167, 196,  75, 137, 225, 211, 240, 182, 122,  91,  28, 197,  13,
  226, 168, 183, 153,  44, 212, 138, 107, 241,  60,  29, 123, 198, 184, 227, 169,
  242,  76, 213, 154,  45,  92,  14, 199, 139,  61, 228, 214, 170, 185, 243, 108,
   77, 155,  30,  15, 200, 229, 124, 215, 244,  93,  46, 186, 171, 201, 109, 140,
  230,  62, 216, 245,  31, 125,  78, 156, 231,  47, 187, 202, 217,  94, 246, 141,
   63, 232, 172, 110, 247, 157,  79, 218, 203, 126, 233, 188, 248,  95, 173, 142,
  219, 111, 249, 234, 158, 127, 189, 204, 250, 235, 143, 174, 220, 205, 159, 251,
  190, 221, 175, 236, 237, 191, 206, 252, 222, 253, 207, 238, 223, 254, 239, 255,
};

DECLARE_ALIGNED(16, const int, vp9_col_scan_16x16[256]) = {
    0,  16,  32,  48,   1,  64,  17,  80,  33,  96,  49,   2,  65, 112,  18,  81,
   34, 128,  50,  97,   3,  66, 144,  19, 113,  35,  82, 160,  98,  51, 129,   4,
   67, 176,  20, 114, 145,  83,  36,  99, 130,  52, 192,   5, 161,  68, 115,  21,
  146,  84, 208, 177,  37, 131, 100,  53, 162, 224,  69,   6, 116, 193, 147,  85,
   22, 240, 132,  38, 178, 101, 163,  54, 209, 117,  70,   7, 148, 194,  86, 179,
  225,  23, 133,  39, 164,   8, 102, 210, 241,  55, 195, 118, 149,  71, 180,  24,
   87, 226, 134, 165, 211,  40, 103,  56,  72, 150, 196, 242, 119,   9, 181, 227,
   88, 166,  25, 135,  41, 104, 212,  57, 151, 197, 120,  73, 243, 182, 136, 167,
  213,  89,  10, 228, 105, 152, 198,  26,  42, 121, 183, 244, 168,  58, 137, 229,
   74, 214,  90, 153, 199, 184,  11, 106, 245,  27, 122, 230, 169,  43, 215,  59,
  200, 138, 185, 246,  75,  12,  91, 154, 216, 231, 107,  28,  44, 201, 123, 170,
   60, 247, 232,  76, 139,  13,  92, 217, 186, 248, 155, 108,  29, 124,  45, 202,
  233, 171,  61,  14,  77, 140,  15, 249,  93,  30, 187, 156, 218,  46, 109, 125,
   62, 172,  78, 203,  31, 141, 234,  94,  47, 188,  63, 157, 110, 250, 219,  79,
  126, 204, 173, 142,  95, 189, 111, 235, 158, 220, 251, 127, 174, 143, 205, 236,
  159, 190, 221, 252, 175, 206, 237, 191, 253, 222, 238, 207, 254, 223, 239, 255,
};

DECLARE_ALIGNED(16, const int, vp9_row_scan_16x16[256]) = {
    0,   1,   2,  16,   3,  17,   4,  18,  32,   5,  33,  19,   6,  34,  48,  20,
   49,   7,  35,  21,  50,  64,   8,  36,  65,  22,  51,  37,  80,   9,  66,  52,
   23,  38,  81,  67,  10,  53,  24,  82,  68,  96,  39,  11,  54,  83,  97,  69,
   25,  98,  84,  40, 112,  55,  12,  70,  99, 113,  85,  26,  41,  56, 114, 100,
   13,  71, 128,  86,  27, 115, 101, 129,  42,  57,  72, 116,  14,  87, 130, 102,
  144,  73, 131, 117,  28,  58,  15,  88,  43, 145, 103, 132, 146, 118,  74, 160,
   89, 133, 104,  29,  59, 147, 119,  44, 161, 148,  90, 105, 134, 162, 120, 176,
   75, 135, 149,  30,  60, 163, 177,  45, 121,  91, 106, 164, 178, 150, 192, 136,
  165, 179,  31, 151, 193,  76, 122,  61, 137, 194, 107, 152, 180, 208,  46, 166,
  167, 195,  92, 181, 138, 209, 123, 153, 224, 196,  77, 168, 210, 182, 240, 108,
  197,  62, 154, 225, 183, 169, 211,  47, 139,  93, 184, 226, 212, 241, 198, 170,
  124, 155, 199,  78, 213, 185, 109, 227, 200,  63, 228, 242, 140, 214, 171, 186,
  156, 229, 243, 125,  94, 201, 244, 215, 216, 230, 141, 187, 202,  79, 172, 110,
  157, 245, 217, 231,  95, 246, 232, 126, 203, 247, 233, 173, 218, 142, 111, 158,
  188, 248, 127, 234, 219, 249, 189, 204, 143, 174, 159, 250, 235, 205, 220, 175,
  190, 251, 221, 191, 206, 236, 207, 237, 252, 222, 253, 223, 238, 239, 254, 255,
};

DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_32x32[1024]) = {
     0,   32,    1,   64,   33,    2,   96,   65,   34,  128,    3,   97,   66,  160,  129,   35,   98,    4,   67,  130,  161,  192,   36,   99,  224,    5,  162,  193,   68,  131,   37,  100,
   225,  194,  256,  163,   69,  132,    6,  226,  257,  288,  195,  101,  164,   38,  258,    7,  227,  289,  133,  320,   70,  196,  165,  290,  259,  228,   39,  321,  102,  352,    8,  197,
    71,  134,  322,  291,  260,  353,  384,  229,  166,  103,   40,  354,  323,  292,  135,  385,  198,  261,   72,    9,  416,  167,  386,  355,  230,  324,  104,  293,   41,  417,  199,  136,
   262,  387,  448,  325,  356,   10,   73,  418,  231,  168,  449,  294,  388,  105,  419,  263,   42,  200,  357,  450,  137,  480,   74,  326,  232,   11,  389,  169,  295,  420,  106,  451,
   481,  358,  264,  327,  201,   43,  138,  512,  482,  390,  296,  233,  170,  421,   75,  452,  359,   12,  513,  265,  483,  328,  107,  202,  514,  544,  422,  391,  453,  139,   44,  234,
   484,  297,  360,  171,   76,  515,  545,  266,  329,  454,   13,  423,  392,  203,  108,  546,  485,  576,  298,  235,  140,  361,  516,  330,  172,  547,   45,  424,  455,  267,  393,  577,
   486,   77,  204,  517,  362,  548,  608,   14,  456,  299,  578,  109,  236,  425,  394,  487,  609,  331,  141,  579,  518,   46,  268,   15,  173,  549,  610,  640,  363,   78,  519,  488,
   300,  205,   16,  457,  580,  426,  550,  395,  110,  237,  611,  641,  332,  672,  142,  642,  269,  458,   47,  581,  427,  489,  174,  364,  520,  612,  551,  673,   79,  206,  301,  643,
   704,   17,  111,  490,  674,  238,  582,   48,  521,  613,  333,  396,  459,  143,  270,  552,  644,  705,  736,  365,   80,  675,  583,  175,  428,  706,  112,  302,  207,  614,  553,   49,
   645,  522,  737,  397,  768,  144,  334,   18,  676,  491,  239,  615,  707,  584,   81,  460,  176,  271,  738,  429,  113,  800,  366,  208,  523,  708,  646,  554,  677,  769,   19,  145,
   585,  739,  240,  303,   50,  461,  616,  398,  647,  335,  492,  177,   82,  770,  832,  555,  272,  430,  678,  209,  709,  114,  740,  801,  617,   51,  304,  679,  524,  367,  586,  241,
    20,  146,  771,  864,   83,  802,  648,  493,  399,  273,  336,  710,  178,  462,  833,  587,  741,  115,  305,  711,  368,  525,  618,  803,  210,  896,  680,  834,  772,   52,  649,  147,
   431,  494,  556,  242,  400,  865,  337,   21,  928,  179,  742,   84,  463,  274,  369,  804,  650,  557,  743,  960,  835,  619,  773,  306,  211,  526,  432,  992,  588,  712,  116,  243,
   866,  495,  681,  558,  805,  589,  401,  897,   53,  338,  148,  682,  867,  464,  275,   22,  370,  433,  307,  620,  527,  836,  774,  651,  713,  744,   85,  180,  621,  465,  929,  775,
   496,  898,  212,  339,  244,  402,  590,  117,  559,  714,  434,   23,  868,  930,  806,  683,  528,  652,  371,  961,  149,  837,   54,  899,  745,  276,  993,  497,  403,  622,  181,  776,
   746,  529,  560,  435,   86,  684,  466,  308,  591,  653,  715,  807,  340,  869,  213,  962,  245,  838,  561,  931,  808,  592,  118,  498,  372,  623,  685,  994,  467,  654,  747,  900,
   716,  277,  150,   55,   24,  404,  530,  839,  777,  655,  182,  963,  840,  686,  778,  309,  870,  341,   87,  499,  809,  624,  593,  436,  717,  932,  214,  246,  995,  718,  625,  373,
   562,   25,  119,  901,  531,  468,  964,  748,  810,  278,  779,  500,  563,  656,  405,  687,  871,  872,  594,  151,  933,  749,  841,  310,  657,  626,  595,  437,  688,  183,  996,  965,
   902,  811,  342,  750,  689,  719,  532,   56,  215,  469,  934,  374,  247,  720,  780,  564,  781,  842,  406,   26,  751,  903,  873,   57,  279,  627,  501,  658,  843,  997,  812,  904,
    88,  813,  438,  752,  935,  936,  311,  596,  533,  690,  343,  966,  874,   89,  120,  470,  721,  875,  659,  782,  565,  998,  375,  844,  845,   27,  628,  967,  121,  905,  968,  152,
   937,  814,  753,  502,  691,  783,  184,  153,  722,  407,   58,  815,  999,  660,  597,  723,  534,  906,  216,  439,  907,  248,  185,  876,  846,  692,  784,  629,   90,  969,  280,  754,
   938,  939,  217,  847,  566,  471,  785,  816,  877, 1000,  249,  878,  661,  503,  312,  970,  755,  122,  817,  281,  344,  786,  598,  724,   28,   59,   29,  154,  535,  630,  376, 1001,
   313,  908,  186,   91,  848,  849,  345,  909,  940,  879,  408,  818,  693, 1002,  971,  941,  567,  377,  218,  756,  910,  787,  440,  123,  880,  725,  662,  250,  819, 1003,  282,  972,
   850,  599,  472,  409,  155,  441,  942,  757,  788,  694,  911,  881,  314,  631,  973,  504,  187, 1004,  346,  473,  851,  943,  820,  726,   60,  505,  219,  378,  912,  974,   30,   31,
   536,  882, 1005,   92,  251,  663,  944,  913,  283,  695,  883,  568, 1006,  975,  410,  442,  945,  789,  852,  537, 1007,  124,  315,   61,  758,  821,  600,  914,  976,  569,  474,  347,
   156, 1008,  915,   93,  977,  506,  946,  727,  379,  884,  188,  632,  601, 1009,  790,  853,  978,  947,  220,  411,  125,  633,  664,  759,  252,  443,  916,  538,  157,  822,   62,  570,
   979,  284, 1010,  885,  948,  189,  475,   94,  316,  665,  696, 1011,  854,  791,  980,  221,  348,   63,  917,  602,  380,  507,  253,  126,  697,  823,  634,  285,  728,  949,  886,   95,
   158,  539, 1012,  317,  412,  444,  760,  571,  190,  981,  729,  918,  127,  666,  349,  381,  476,  855,  761, 1013,  603,  222,  159,  698,  950,  508,  254,  792,  286,  635,  887,  793,
   413,  191,  982,  445,  540,  318,  730,  667,  223,  824,  919, 1014,  350,  477,  572,  255,  825,  951,  762,  509,  604,  856,  382,  699,  287,  319,  636,  983,  794,  414,  541,  731,
   857,  888,  351,  446,  573, 1015,  668,  889,  478,  826,  383,  763,  605,  920,  510,  637,  415,  700,  921,  858,  447,  952,  542,  795,  479,  953,  732,  890,  669,  574,  511,  984,
   827,  985,  922, 1016,  764,  606,  543,  701,  859,  638, 1017,  575,  796,  954,  733,  891,  670,  607,  828,  986,  765,  923,  639, 1018,  702,  860,  955,  671,  892,  734,  797,  703,
   987,  829, 1019,  766,  924,  735,  861,  956,  988,  893,  767,  798,  830, 1020,  925,  957,  799,  862,  831,  989,  894, 1021,  863,  926,  895,  958,  990, 1022,  927,  959,  991, 1023,
};

/* Array indices are identical to previously-existing CONTEXT_NODE indices */

const vp9_tree_index vp9_coef_tree[ 22] =     /* corresponding _CONTEXT_NODEs */
{
  -DCT_EOB_TOKEN, 2,                             /* 0 = EOB */
  -ZERO_TOKEN, 4,                               /* 1 = ZERO */
  -ONE_TOKEN, 6,                               /* 2 = ONE */
  8, 12,                                      /* 3 = LOW_VAL */
  -TWO_TOKEN, 10,                            /* 4 = TWO */
  -THREE_TOKEN, -FOUR_TOKEN,                /* 5 = THREE */
  14, 16,                                    /* 6 = HIGH_LOW */
  -DCT_VAL_CATEGORY1, -DCT_VAL_CATEGORY2,   /* 7 = CAT_ONE */
  18, 20,                                   /* 8 = CAT_THREEFOUR */
  -DCT_VAL_CATEGORY3, -DCT_VAL_CATEGORY4,  /* 9 = CAT_THREE */
  -DCT_VAL_CATEGORY5, -DCT_VAL_CATEGORY6   /* 10 = CAT_FIVE */
};

struct vp9_token_struct vp9_coef_encodings[MAX_ENTROPY_TOKENS];

/* Trees for extra bits.  Probabilities are constant and
   do not depend on previously encoded bits */

static const vp9_prob Pcat1[] = { 159};
static const vp9_prob Pcat2[] = { 165, 145};
static const vp9_prob Pcat3[] = { 173, 148, 140};
static const vp9_prob Pcat4[] = { 176, 155, 140, 135};
static const vp9_prob Pcat5[] = { 180, 157, 141, 134, 130};
static const vp9_prob Pcat6[] = {
  254, 254, 254, 252, 249, 243, 230, 196, 177, 153, 140, 133, 130, 129
};

#if CONFIG_CODE_NONZEROCOUNT
const vp9_tree_index vp9_nzc4x4_tree[2 * NZC4X4_NODES] = {
  -NZC_0, 2,
  4, 6,
  -NZC_1, -NZC_2,
  -NZC_3TO4, 8,
  -NZC_5TO8, -NZC_9TO16,
};
struct vp9_token_struct vp9_nzc4x4_encodings[NZC4X4_TOKENS];

const vp9_tree_index vp9_nzc8x8_tree[2 * NZC8X8_NODES] = {
  -NZC_0, 2,
  4, 6,
  -NZC_1, -NZC_2,
  8, 10,
  -NZC_3TO4, -NZC_5TO8,
  -NZC_9TO16, 12,
  -NZC_17TO32, -NZC_33TO64,
};
struct vp9_token_struct vp9_nzc8x8_encodings[NZC8X8_TOKENS];

const vp9_tree_index vp9_nzc16x16_tree[2 * NZC16X16_NODES] = {
  -NZC_0, 2,
  4, 6,
  -NZC_1, -NZC_2,
  8, 10,
  -NZC_3TO4, -NZC_5TO8,
  12, 14,
  -NZC_9TO16, -NZC_17TO32,
  -NZC_33TO64, 16,
  -NZC_65TO128, -NZC_129TO256,
};
struct vp9_token_struct vp9_nzc16x16_encodings[NZC16X16_TOKENS];

const vp9_tree_index vp9_nzc32x32_tree[2 * NZC32X32_NODES] = {
  -NZC_0, 2,
  4, 6,
  -NZC_1, -NZC_2,
  8, 10,
  -NZC_3TO4, -NZC_5TO8,
  12, 14,
  -NZC_9TO16, -NZC_17TO32,
  16, 18,
  -NZC_33TO64, -NZC_65TO128,
  -NZC_129TO256, 20,
  -NZC_257TO512, -NZC_513TO1024,
};
struct vp9_token_struct vp9_nzc32x32_encodings[NZC32X32_TOKENS];

const int vp9_extranzcbits[NZC32X32_TOKENS] = {
  0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

const int vp9_basenzcvalue[NZC32X32_TOKENS] = {
  0, 1, 2, 3, 5, 9, 17, 33, 65, 129, 257, 513
};

#endif  // CONFIG_CODE_NONZEROCOUNT

static vp9_tree_index cat1[2], cat2[4], cat3[6], cat4[8], cat5[10], cat6[28];

static void init_bit_tree(vp9_tree_index *p, int n) {
  int i = 0;

  while (++i < n) {
    p[0] = p[1] = i << 1;
    p += 2;
  }

  p[0] = p[1] = 0;
}

static void init_bit_trees() {
  init_bit_tree(cat1, 1);
  init_bit_tree(cat2, 2);
  init_bit_tree(cat3, 3);
  init_bit_tree(cat4, 4);
  init_bit_tree(cat5, 5);
  init_bit_tree(cat6, 14);
}

vp9_extra_bit_struct vp9_extra_bits[12] = {
  { 0, 0, 0, 0},
  { 0, 0, 0, 1},
  { 0, 0, 0, 2},
  { 0, 0, 0, 3},
  { 0, 0, 0, 4},
  { cat1, Pcat1, 1, 5},
  { cat2, Pcat2, 2, 7},
  { cat3, Pcat3, 3, 11},
  { cat4, Pcat4, 4, 19},
  { cat5, Pcat5, 5, 35},
  { cat6, Pcat6, 14, 67},
  { 0, 0, 0, 0}
};

#include "vp9/common/vp9_default_coef_probs.h"

// This function updates and then returns n AC coefficient context
// This is currently a placeholder function to allow experimentation
// using various context models based on the energy earlier tokens
// within the current block.
//
// For now it just returns the previously used context.
int vp9_get_coef_context(const int *scan, const int *neighbors,
                         int nb_pad, uint8_t *token_cache, int c, int l) {
  int eob = l;
  assert(nb_pad == 2);
  if (c == eob - 1) {
    return 0;
  } else {
    int ctx;
    c++;
    assert(neighbors[2 * c + 0] >= 0);
    if (neighbors[2 * c + 1] >= 0) {
      ctx = (1 + token_cache[neighbors[2 * c + 0]] +
             token_cache[neighbors[2 * c + 1]]) >> 1;
    } else {
      ctx = token_cache[neighbors[2 * c + 0]];
    }
    return vp9_pt_energy_class[ctx];
  }
};

void vp9_default_coef_probs(VP9_COMMON *pc) {
#if CONFIG_CODE_NONZEROCOUNT
#ifdef NZC_DEFAULT_COUNTS
  int h, g;
  for (h = 0; h < MAX_NZC_CONTEXTS; ++h) {
    for (g = 0; g < REF_TYPES; ++g) {
      int i;
      unsigned int branch_ct4x4[NZC4X4_NODES][2];
      unsigned int branch_ct8x8[NZC8X8_NODES][2];
      unsigned int branch_ct16x16[NZC16X16_NODES][2];
      unsigned int branch_ct32x32[NZC32X32_NODES][2];
      for (i = 0; i < BLOCK_TYPES; ++i) {
        vp9_tree_probs_from_distribution(
          vp9_nzc4x4_tree,
          pc->fc.nzc_probs_4x4[h][g][i], branch_ct4x4,
          default_nzc_counts_4x4[h][g][i], 0);
      }
      for (i = 0; i < BLOCK_TYPES; ++i) {
        vp9_tree_probs_from_distribution(
          vp9_nzc8x8_tree,
          pc->fc.nzc_probs_8x8[h][g][i], branch_ct8x8,
          default_nzc_counts_8x8[h][g][i], 0);
      }
      for (i = 0; i < BLOCK_TYPES; ++i) {
        vp9_tree_probs_from_distribution(
          vp9_nzc16x16_tree,
          pc->fc.nzc_probs_16x16[h][g][i], branch_ct16x16,
          default_nzc_counts_16x16[h][g][i], 0);
      }
      for (i = 0; i < BLOCK_TYPES; ++i) {
        vp9_tree_probs_from_distribution(
          vp9_nzc32x32_tree,
          pc->fc.nzc_probs_32x32[h][g][i], branch_ct32x32,
          default_nzc_counts_32x32[h][g][i], 0);
      }
    }
  }
#else
  vpx_memcpy(pc->fc.nzc_probs_4x4, default_nzc_probs_4x4,
             sizeof(pc->fc.nzc_probs_4x4));
  vpx_memcpy(pc->fc.nzc_probs_8x8, default_nzc_probs_8x8,
             sizeof(pc->fc.nzc_probs_8x8));
  vpx_memcpy(pc->fc.nzc_probs_16x16, default_nzc_probs_16x16,
             sizeof(pc->fc.nzc_probs_16x16));
  vpx_memcpy(pc->fc.nzc_probs_32x32, default_nzc_probs_32x32,
             sizeof(pc->fc.nzc_probs_32x32));
#endif
  vpx_memcpy(pc->fc.nzc_pcat_probs, default_nzc_pcat_probs,
             sizeof(pc->fc.nzc_pcat_probs));
#endif  // CONFIG_CODE_NONZEROCOUNT
  vpx_memcpy(pc->fc.coef_probs_4x4, default_coef_probs_4x4,
             sizeof(pc->fc.coef_probs_4x4));
  vpx_memcpy(pc->fc.coef_probs_8x8, default_coef_probs_8x8,
             sizeof(pc->fc.coef_probs_8x8));
  vpx_memcpy(pc->fc.coef_probs_16x16, default_coef_probs_16x16,
             sizeof(pc->fc.coef_probs_16x16));
  vpx_memcpy(pc->fc.coef_probs_32x32, default_coef_probs_32x32,
             sizeof(pc->fc.coef_probs_32x32));
}

// Neighborhood 5-tuples for various scans and blocksizes,
// in {top, left, topleft, topright, bottomleft} order
// for each position in raster scan order.
// -1 indicates the neighbor does not exist.
DECLARE_ALIGNED(16, int,
                vp9_default_zig_zag1d_4x4_neighbors[16 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_col_scan_4x4_neighbors[16 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_row_scan_4x4_neighbors[16 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_default_zig_zag1d_8x8_neighbors[64 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_col_scan_8x8_neighbors[64 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_row_scan_8x8_neighbors[64 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_default_zig_zag1d_16x16_neighbors[256 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_col_scan_16x16_neighbors[256 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_row_scan_16x16_neighbors[256 * 2]);
DECLARE_ALIGNED(16, int,
                vp9_default_zig_zag1d_32x32_neighbors[1024 * 2]);

static int find_in_scan(const int *scan, int l, int idx) {
  int n, l2 = l * l;
  for (n = 0; n < l2; n++) {
    int rc = scan[n];
    if (rc == idx)
      return  n;
  }
  assert(0);
  return -1;
}
static void init_scan_neighbors(const int *scan, int l, int *neighbors,
                                int MAX_NEIGHBORS) {
  int l2 = l * l;
  int n, i, j;

  for (n = 0; n < l2; n++) {
    int rc = scan[n];
    assert(MAX_NEIGHBORS == 2);
    i = rc / l;
    j = rc % l;
    if (i > 0 && j > 0) {
      int a = find_in_scan(scan, l, (i - 1) * l + j);
      int b = find_in_scan(scan, l,  i      * l + j - 1);
      if (scan == vp9_col_scan_4x4 || scan == vp9_col_scan_8x8 ||
          scan == vp9_col_scan_16x16) {
        neighbors[2 * n + 0] = a;
        neighbors[2 * n + 1] = -1;
      } else if (scan == vp9_row_scan_4x4 || scan == vp9_row_scan_8x8 ||
                 scan == vp9_row_scan_16x16) {
        neighbors[2 * n + 0] = b;
        neighbors[2 * n + 1] = -1;
      } else {
        neighbors[2 * n + 0] = a;
        neighbors[2 * n + 1] = b;
      }
    } else if (i > 0) {
      neighbors[2 * n + 0] = find_in_scan(scan, l, (i - 1) * l + j);
      neighbors[2 * n + 1] = -1;
    } else if (j > 0) {
      neighbors[2 * n + 0] = find_in_scan(scan, l,  i      * l + j - 1);
      neighbors[2 * n + 1] = -1;
    } else {
      assert(n == 0);
      // dc predictor doesn't use previous tokens
      neighbors[n] = -1;
    }
    assert(neighbors[MAX_NEIGHBORS * n + 0] < n);
  }
}

void vp9_init_neighbors() {
  init_scan_neighbors(vp9_default_zig_zag1d_4x4, 4,
                      vp9_default_zig_zag1d_4x4_neighbors, 2);
  init_scan_neighbors(vp9_row_scan_4x4, 4,
                      vp9_row_scan_4x4_neighbors, 2);
  init_scan_neighbors(vp9_col_scan_4x4, 4,
                      vp9_col_scan_4x4_neighbors, 2);
  init_scan_neighbors(vp9_default_zig_zag1d_8x8, 8,
                      vp9_default_zig_zag1d_8x8_neighbors, 2);
  init_scan_neighbors(vp9_row_scan_8x8, 8,
                      vp9_row_scan_8x8_neighbors, 2);
  init_scan_neighbors(vp9_col_scan_8x8, 8,
                      vp9_col_scan_8x8_neighbors, 2);
  init_scan_neighbors(vp9_default_zig_zag1d_16x16, 16,
                      vp9_default_zig_zag1d_16x16_neighbors, 2);
  init_scan_neighbors(vp9_row_scan_16x16, 16,
                      vp9_row_scan_16x16_neighbors, 2);
  init_scan_neighbors(vp9_col_scan_16x16, 16,
                      vp9_col_scan_16x16_neighbors, 2);
  init_scan_neighbors(vp9_default_zig_zag1d_32x32, 32,
                      vp9_default_zig_zag1d_32x32_neighbors, 2);
}

const int *vp9_get_coef_neighbors_handle(const int *scan, int *pad) {
  if (scan == vp9_default_zig_zag1d_4x4) {
    *pad = 2;
    return vp9_default_zig_zag1d_4x4_neighbors;
  } else if (scan == vp9_row_scan_4x4) {
    *pad = 2;
    return vp9_row_scan_4x4_neighbors;
  } else if (scan == vp9_col_scan_4x4) {
    *pad = 2;
    return vp9_col_scan_4x4_neighbors;
  } else if (scan == vp9_default_zig_zag1d_8x8) {
    *pad = 2;
    return vp9_default_zig_zag1d_8x8_neighbors;
  } else if (scan == vp9_row_scan_8x8) {
    *pad = 2;
    return vp9_row_scan_8x8_neighbors;
  } else if (scan == vp9_col_scan_8x8) {
    *pad = 2;
    return vp9_col_scan_8x8_neighbors;
  } else if (scan == vp9_default_zig_zag1d_16x16) {
    *pad = 2;
    return vp9_default_zig_zag1d_16x16_neighbors;
  } else if (scan == vp9_row_scan_16x16) {
    *pad = 2;
    return vp9_row_scan_16x16_neighbors;
  } else if (scan == vp9_col_scan_16x16) {
    *pad = 2;
    return vp9_col_scan_16x16_neighbors;
  } else if (scan == vp9_default_zig_zag1d_32x32) {
    *pad = 2;
    return vp9_default_zig_zag1d_32x32_neighbors;
  } else {
    assert(0);
    return NULL;
  }
}

void vp9_coef_tree_initialize() {
  vp9_init_neighbors();
  init_bit_trees();
  vp9_tokens_from_tree(vp9_coef_encodings, vp9_coef_tree);
#if CONFIG_CODE_NONZEROCOUNT
  vp9_tokens_from_tree(vp9_nzc4x4_encodings, vp9_nzc4x4_tree);
  vp9_tokens_from_tree(vp9_nzc8x8_encodings, vp9_nzc8x8_tree);
  vp9_tokens_from_tree(vp9_nzc16x16_encodings, vp9_nzc16x16_tree);
  vp9_tokens_from_tree(vp9_nzc32x32_encodings, vp9_nzc32x32_tree);
#endif
}

#if CONFIG_CODE_NONZEROCOUNT

#define mb_in_cur_tile(cm, mb_row, mb_col)      \
    ((mb_col) >= (cm)->cur_tile_mb_col_start && \
     (mb_col) <= (cm)->cur_tile_mb_col_end   && \
     (mb_row) >= 0)

#define choose_nzc_context(nzc_exp, t2, t1)     \
    ((nzc_exp) >= (t2) ? 2 : (nzc_exp) >= (t1) ? 1 : 0)

#define NZC_T2_32X32    (16 << 6)
#define NZC_T1_32X32     (4 << 6)

#define NZC_T2_16X16    (12 << 6)
#define NZC_T1_16X16     (3 << 6)

#define NZC_T2_8X8       (8 << 6)
#define NZC_T1_8X8       (2 << 6)

#define NZC_T2_4X4       (4 << 6)
#define NZC_T1_4X4       (1 << 6)

// Transforms a mb16 block index to a sb64 block index
static inline int mb16_to_sb64_index(int mb_row, int mb_col, int block) {
  int r = (mb_row & 3);
  int c = (mb_col & 3);
  int b;
  if (block < 16) {  // Y
    int ib = block >> 2;
    int jb = block & 3;
    ib += r * 4;
    jb += c * 4;
    b = ib * 16 + jb;
    assert(b < 256);
    return b;
  } else {  // UV
    int base = block - (block & 3);
    int ib = (block - base) >> 1;
    int jb = (block - base) & 1;
    ib += r * 2;
    jb += c * 2;
    b = base * 16 + ib * 8 + jb;
    assert(b >= 256 && b < 384);
    return b;
  }
}

// Transforms a mb16 block index to a sb32 block index
static inline int mb16_to_sb32_index(int mb_row, int mb_col, int block) {
  int r = (mb_row & 1);
  int c = (mb_col & 1);
  int b;
  if (block < 16) {  // Y
    int ib = block >> 2;
    int jb = block & 3;
    ib += r * 4;
    jb += c * 4;
    b = ib * 8 + jb;
    assert(b < 64);
    return b;
  } else {  // UV
    int base = block - (block & 3);
    int ib = (block - base) >> 1;
    int jb = (block - base) & 1;
    ib += r * 2;
    jb += c * 2;
    b = base * 4 + ib * 4 + jb;
    assert(b >= 64 && b < 96);
    return b;
  }
}

static inline int block_to_txfm_index(int block, TX_SIZE tx_size, int s) {
  // s is the log of the number of 4x4 blocks in each row/col of larger block
  int b, ib, jb, nb;
  ib = block >> s;
  jb = block - (ib << s);
  ib >>= tx_size;
  jb >>= tx_size;
  nb = 1 << (s - tx_size);
  b = (ib * nb + jb) << (2 * tx_size);
  return b;
}

/* BEGIN - Helper functions to get the y nzcs */
static unsigned int get_nzc_4x4_y_sb64(MB_MODE_INFO *mi, int block) {
  int b;
  assert(block < 256);
  b = block_to_txfm_index(block, mi->txfm_size, 4);
  assert(b < 256);
  return mi->nzcs[b] << (6 - 2 * mi->txfm_size);
}

static unsigned int get_nzc_4x4_y_sb32(MB_MODE_INFO *mi, int block) {
  int b;
  assert(block < 64);
  b = block_to_txfm_index(block, mi->txfm_size, 3);
  assert(b < 64);
  return mi->nzcs[b] << (6 - 2 * mi->txfm_size);
}

static unsigned int get_nzc_4x4_y_mb16(MB_MODE_INFO *mi, int block) {
  int b;
  assert(block < 16);
  b = block_to_txfm_index(block, mi->txfm_size, 2);
  assert(b < 16);
  return mi->nzcs[b] << (6 - 2 * mi->txfm_size);
}
/* END - Helper functions to get the y nzcs */

/* Function to get y nzc where block index is in mb16 terms */
static unsigned int get_nzc_4x4_y(VP9_COMMON *cm, MODE_INFO *m,
                                  int mb_row, int mb_col, int block) {
  // NOTE: All values returned are at 64 times the true value at 4x4 scale
  MB_MODE_INFO *const mi = &m->mbmi;
  const int mis = cm->mode_info_stride;
  if (mi->mb_skip_coeff || !mb_in_cur_tile(cm, mb_row, mb_col))
    return 0;
  if (mi->sb_type == BLOCK_SIZE_SB64X64) {
    int r = mb_row & 3;
    int c = mb_col & 3;
    m -= c + r * mis;
    if (m->mbmi.mb_skip_coeff || !mb_in_cur_tile(cm, mb_row - r, mb_col - c))
      return 0;
    else
      return get_nzc_4x4_y_sb64(
          &m->mbmi, mb16_to_sb64_index(mb_row, mb_col, block));
  } else if (mi->sb_type == BLOCK_SIZE_SB32X32) {
    int r = mb_row & 1;
    int c = mb_col & 1;
    m -= c + r * mis;
    if (m->mbmi.mb_skip_coeff || !mb_in_cur_tile(cm, mb_row - r, mb_col - c))
      return 0;
    else
      return get_nzc_4x4_y_sb32(
          &m->mbmi, mb16_to_sb32_index(mb_row, mb_col, block));
  } else {
    if (m->mbmi.mb_skip_coeff || !mb_in_cur_tile(cm, mb_row, mb_col))
      return 0;
    return get_nzc_4x4_y_mb16(mi, block);
  }
}

/* BEGIN - Helper functions to get the uv nzcs */
static unsigned int get_nzc_4x4_uv_sb64(MB_MODE_INFO *mi, int block) {
  int b;
  int base, uvtxfm_size;
  assert(block >= 256 && block < 384);
  uvtxfm_size = mi->txfm_size;
  base = 256 + (block & 64);
  block -= base;
  b = base + block_to_txfm_index(block, uvtxfm_size, 3);
  assert(b >= 256 && b < 384);
  return mi->nzcs[b] << (6 - 2 * uvtxfm_size);
}

static unsigned int get_nzc_4x4_uv_sb32(MB_MODE_INFO *mi, int block) {
  int b;
  int base, uvtxfm_size;
  assert(block >= 64 && block < 96);
  if (mi->txfm_size == TX_32X32)
    uvtxfm_size = TX_16X16;
  else
    uvtxfm_size = mi->txfm_size;
  base = 64 + (block & 16);
  block -= base;
  b = base + block_to_txfm_index(block, uvtxfm_size, 2);
  assert(b >= 64 && b < 96);
  return mi->nzcs[b] << (6 - 2 * uvtxfm_size);
}

static unsigned int get_nzc_4x4_uv_mb16(MB_MODE_INFO *mi, int block) {
  int b;
  int base, uvtxfm_size;
  assert(block >= 16 && block < 24);
  if (mi->txfm_size == TX_8X8 &&
      (mi->mode == SPLITMV || mi->mode == I8X8_PRED))
    uvtxfm_size = TX_4X4;
  else if (mi->txfm_size == TX_16X16)
    uvtxfm_size = TX_8X8;
  else
    uvtxfm_size = mi->txfm_size;
  base = 16 + (block & 4);
  block -= base;
  b = base + block_to_txfm_index(block, uvtxfm_size, 1);
  assert(b >= 16 && b < 24);
  return mi->nzcs[b] << (6 - 2 * uvtxfm_size);
}
/* END - Helper functions to get the uv nzcs */

/* Function to get uv nzc where block index is in mb16 terms */
static unsigned int get_nzc_4x4_uv(VP9_COMMON *cm, MODE_INFO *m,
                                   int mb_row, int mb_col, int block) {
  // NOTE: All values returned are at 64 times the true value at 4x4 scale
  MB_MODE_INFO *const mi = &m->mbmi;
  const int mis = cm->mode_info_stride;
  if (mi->mb_skip_coeff || !mb_in_cur_tile(cm, mb_row, mb_col))
    return 0;
  if (mi->sb_type == BLOCK_SIZE_SB64X64) {
    int r = mb_row & 3;
    int c = mb_col & 3;
    m -= c + r * mis;
    if (m->mbmi.mb_skip_coeff || !mb_in_cur_tile(cm, mb_row - r, mb_col - c))
      return 0;
    else
      return get_nzc_4x4_uv_sb64(
          &m->mbmi, mb16_to_sb64_index(mb_row, mb_col, block));
  } else if (mi->sb_type == BLOCK_SIZE_SB32X32) {
    int r = mb_row & 1;
    int c = mb_col & 1;
    m -= c + r * mis;
    if (m->mbmi.mb_skip_coeff || !mb_in_cur_tile(cm, mb_row - r, mb_col - c))
      return 0;
    else
    return get_nzc_4x4_uv_sb32(
        &m->mbmi, mb16_to_sb32_index(mb_row, mb_col, block));
  } else {
    return get_nzc_4x4_uv_mb16(mi, block);
  }
}

int vp9_get_nzc_context_y_sb64(VP9_COMMON *cm, MODE_INFO *cur,
                               int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  assert(block < 256);
  switch (txfm_size) {
    case TX_32X32:
      assert((block & 63) == 0);
      if (block < 128) {
        int o = (block >> 6) * 2;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 12) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 13) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 14) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 15) +
            get_nzc_4x4_y(cm, cur - mis + o + 1,
                          mb_row - 1, mb_col + o + 1, 12) +
            get_nzc_4x4_y(cm, cur - mis + o + 1,
                          mb_row - 1, mb_col + o + 1, 13) +
            get_nzc_4x4_y(cm, cur - mis + o + 1,
                          mb_row - 1, mb_col + o + 1, 14) +
            get_nzc_4x4_y(cm, cur - mis + o + 1,
                          mb_row - 1, mb_col + o + 1, 15);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 128] << 3;
      }
      if ((block & 127) == 0) {
        int o = (block >> 7) * 2;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 3) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 7) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 11) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 15) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis + mis,
                          mb_row + o + 1, mb_col - 1, 3) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis + mis,
                          mb_row + o + 1, mb_col - 1, 7) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis + mis,
                          mb_row + o + 1, mb_col - 1, 11) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis + mis,
                          mb_row + o + 1, mb_col - 1, 15);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 64] << 3;
      }
      nzc_exp <<= 2;
      // Note nzc_exp is 64 times the average value expected at 32x32 scale
      return choose_nzc_context(nzc_exp, NZC_T2_32X32, NZC_T1_32X32);
      break;

    case TX_16X16:
      assert((block & 15) == 0);
      if (block < 64) {
        int o = block >> 4;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 12) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 13) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 14) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 15);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 64] << 4;
      }
      if ((block & 63) == 0) {
        int o = block >> 6;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 3) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 7) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 11) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 15);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 16] << 4;
      }
      nzc_exp <<= 1;
      // Note nzc_exp is 64 times the average value expected at 16x16 scale
      return choose_nzc_context(nzc_exp, NZC_T2_16X16, NZC_T1_16X16);
      break;

    case TX_8X8:
      assert((block & 3) == 0);
      if (block < 32) {
        int o = block >> 3;
        int p = ((block >> 2) & 1) ? 14 : 12;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, p) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, p + 1);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 32] << 5;
      }
      if ((block & 31) == 0) {
        int o = block >> 6;
        int p = ((block >> 5) & 1) ? 11 : 3;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, p) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, p + 4);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 4] << 5;
      }
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);
      break;

    case TX_4X4:
      if (block < 16) {
        int o = block >> 2;
        int p = block & 3;
        nzc_exp = get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o,
                                12 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 16] << 6);
      }
      if ((block & 15) == 0) {
        int o = block >> 6;
        int p = (block >> 4) & 3;
        nzc_exp += get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                                 3 + 4 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);
      break;

    default:
      return 0;
  }
}

int vp9_get_nzc_context_y_sb32(VP9_COMMON *cm, MODE_INFO *cur,
                               int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  assert(block < 64);
  switch (txfm_size) {
    case TX_32X32:
      assert(block == 0);
      nzc_exp =
          (get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 12) +
           get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 13) +
           get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 14) +
           get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 15) +
           get_nzc_4x4_y(cm, cur - mis + 1, mb_row - 1, mb_col + 1, 12) +
           get_nzc_4x4_y(cm, cur - mis + 1, mb_row - 1, mb_col + 1, 13) +
           get_nzc_4x4_y(cm, cur - mis + 1, mb_row - 1, mb_col + 1, 14) +
           get_nzc_4x4_y(cm, cur - mis + 1, mb_row - 1, mb_col + 1, 15) +
           get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 3) +
           get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 7) +
           get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 11) +
           get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 15) +
           get_nzc_4x4_y(cm, cur - 1 + mis, mb_row + 1, mb_col - 1, 3) +
           get_nzc_4x4_y(cm, cur - 1 + mis, mb_row + 1, mb_col - 1, 7) +
           get_nzc_4x4_y(cm, cur - 1 + mis, mb_row + 1, mb_col - 1, 11) +
           get_nzc_4x4_y(cm, cur - 1 + mis, mb_row + 1, mb_col - 1, 15)) << 2;
      // Note nzc_exp is 64 times the average value expected at 32x32 scale
      return choose_nzc_context(nzc_exp, NZC_T2_32X32, NZC_T1_32X32);
      break;

    case TX_16X16:
      assert((block & 15) == 0);
      if (block < 32) {
        int o = (block >> 4) & 1;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 12) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 13) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 14) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, 15);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 32] << 4;
      }
      if ((block & 31) == 0) {
        int o = block >> 5;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 3) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 7) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 11) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, 15);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 16] << 4;
      }
      nzc_exp <<= 1;
      // Note nzc_exp is 64 times the average value expected at 16x16 scale
      return choose_nzc_context(nzc_exp, NZC_T2_16X16, NZC_T1_16X16);
      break;

    case TX_8X8:
      assert((block & 3) == 0);
      if (block < 16) {
        int o = block >> 3;
        int p = ((block >> 2) & 1) ? 14 : 12;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, p) +
            get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o, p + 1);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 16] << 5;
      }
      if ((block & 15) == 0) {
        int o = block >> 5;
        int p = ((block >> 4) & 1) ? 11 : 3;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, p) +
            get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1, p + 4);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 4] << 5;
      }
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);
      break;

    case TX_4X4:
      if (block < 8) {
        int o = block >> 2;
        int p = block & 3;
        nzc_exp = get_nzc_4x4_y(cm, cur - mis + o, mb_row - 1, mb_col + o,
                                12 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 8] << 6);
      }
      if ((block & 7) == 0) {
        int o = block >> 5;
        int p = (block >> 3) & 3;
        nzc_exp += get_nzc_4x4_y(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                                 3 + 4 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);
      break;

    default:
      return 0;
      break;
  }
}

int vp9_get_nzc_context_y_mb16(VP9_COMMON *cm, MODE_INFO *cur,
                               int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  assert(block < 16);
  switch (txfm_size) {
    case TX_16X16:
      assert(block == 0);
      nzc_exp =
          get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 12) +
          get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 13) +
          get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 14) +
          get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, 15) +
          get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 3) +
          get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 7) +
          get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 11) +
          get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, 15);
      nzc_exp <<= 1;
      // Note nzc_exp is 64 times the average value expected at 16x16 scale
      return choose_nzc_context(nzc_exp, NZC_T2_16X16, NZC_T1_16X16);

    case TX_8X8:
      assert((block & 3) == 0);
      if (block < 8) {
        int p = ((block >> 2) & 1) ? 14 : 12;
        nzc_exp =
            get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, p) +
            get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col, p + 1);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 8] << 5;
      }
      if ((block & 7) == 0) {
        int p = ((block >> 3) & 1) ? 11 : 3;
        nzc_exp +=
            get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, p) +
            get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1, p + 4);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 4] << 5;
      }
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);

    case TX_4X4:
      if (block < 4) {
        int p = block & 3;
        nzc_exp = get_nzc_4x4_y(cm, cur - mis, mb_row - 1, mb_col,
                                12 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 4] << 6);
      }
      if ((block & 3) == 0) {
        int p = (block >> 2) & 3;
        nzc_exp += get_nzc_4x4_y(cm, cur - 1, mb_row, mb_col - 1,
                                 3 + 4 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);

    default:
      return 0;
      break;
  }
}

int vp9_get_nzc_context_uv_sb64(VP9_COMMON *cm, MODE_INFO *cur,
                                int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  const int base = block - (block & 63);
  const int boff = (block & 63);
  const int base_mb16 = base >> 4;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  TX_SIZE txfm_size_uv;

  assert(block >= 256 && block < 384);
  txfm_size_uv = txfm_size;

  switch (txfm_size_uv) {
    case TX_32X32:
      assert(block == 256 || block == 320);
      nzc_exp =
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - mis + 1, mb_row - 1, mb_col + 1,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis + 1, mb_row - 1, mb_col + 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - mis + 2, mb_row - 1, mb_col + 2,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis + 2, mb_row - 1, mb_col + 2,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - mis + 3, mb_row - 1, mb_col + 3,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis + 3, mb_row - 1, mb_col + 3,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1, mb_row, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1, mb_row, mb_col - 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row + 1, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row + 1, mb_col - 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1 + 2 * mis, mb_row + 2, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1 + 2 * mis, mb_row + 2, mb_col - 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1 + 3 * mis, mb_row + 3, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1 + 3 * mis, mb_row + 3, mb_col - 1,
                         base_mb16 + 3);
      nzc_exp <<= 2;
      // Note nzc_exp is 64 times the average value expected at 32x32 scale
      return choose_nzc_context(nzc_exp, NZC_T2_32X32, NZC_T1_32X32);

    case TX_16X16:
      // uv txfm_size 16x16
      assert((block & 15) == 0);
      if (boff < 32) {
        int o = (boff >> 4) & 1;
        nzc_exp =
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 2) +
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 3) +
            get_nzc_4x4_uv(cm, cur - mis + o + 1, mb_row - 1, mb_col + o + 1,
                           base_mb16 + 2) +
            get_nzc_4x4_uv(cm, cur - mis + o + 1, mb_row - 1, mb_col + o + 1,
                           base_mb16 + 3);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 32] << 4;
      }
      if ((boff & 31) == 0) {
        int o = boff >> 5;
        nzc_exp +=
            get_nzc_4x4_uv(cm, cur - 1 + o * mis,
                           mb_row + o, mb_col - 1, base_mb16 + 1) +
            get_nzc_4x4_uv(cm, cur - 1 + o * mis,
                           mb_row + o, mb_col - 1, base_mb16 + 3) +
            get_nzc_4x4_uv(cm, cur - 1 + o * mis + mis,
                           mb_row + o + 1, mb_col - 1, base_mb16 + 1) +
            get_nzc_4x4_uv(cm, cur - 1 + o * mis + mis,
                           mb_row + o + 1, mb_col - 1, base_mb16 + 3);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 16] << 4;
      }
      nzc_exp <<= 1;
      // Note nzc_exp is 64 times the average value expected at 16x16 scale
      return choose_nzc_context(nzc_exp, NZC_T2_16X16, NZC_T1_16X16);

    case TX_8X8:
      assert((block & 3) == 0);
      if (boff < 16) {
        int o = boff >> 2;
        nzc_exp =
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 2) +
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 3);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 16] << 5;
      }
      if ((boff & 15) == 0) {
        int o = boff >> 4;
        nzc_exp +=
            get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                           base_mb16 + 1) +
            get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                           base_mb16 + 3);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 4] << 5;
      }
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);

    case TX_4X4:
      if (boff < 8) {
        int o = boff >> 1;
        int p = boff & 1;
        nzc_exp = get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                                 base_mb16 + 2 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 8] << 6);
      }
      if ((boff & 7) == 0) {
        int o = boff >> 4;
        int p = (boff >> 3) & 1;
        nzc_exp += get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                                  base_mb16 + 1 + 2 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);

    default:
      return 0;
  }
}

int vp9_get_nzc_context_uv_sb32(VP9_COMMON *cm, MODE_INFO *cur,
                                int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  const int base = block - (block & 15);
  const int boff = (block & 15);
  const int base_mb16 = base >> 2;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  TX_SIZE txfm_size_uv;

  assert(block >= 64 && block < 96);
  if (txfm_size == TX_32X32)
    txfm_size_uv = TX_16X16;
  else
    txfm_size_uv = txfm_size;

  switch (txfm_size_uv) {
    case TX_16X16:
      // uv txfm_size 16x16
      assert(block == 64 || block == 80);
      nzc_exp =
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - mis + 1, mb_row - 1, mb_col + 1,
                         base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis + 1, mb_row - 1, mb_col + 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row, mb_col - 1,
                         base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row + 1, mb_col - 1,
                         base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1 + mis, mb_row + 1, mb_col - 1,
                         base_mb16 + 3);
      nzc_exp <<= 1;
      // Note nzc_exp is 64 times the average value expected at 16x16 scale
      return choose_nzc_context(nzc_exp, NZC_T2_16X16, NZC_T1_16X16);
      break;

    case TX_8X8:
      assert((block & 3) == 0);
      if (boff < 8) {
        int o = boff >> 2;
        nzc_exp =
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 2) +
            get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                           base_mb16 + 3);
      } else {
        nzc_exp = cur->mbmi.nzcs[block - 8] << 5;
      }
      if ((boff & 7) == 0) {
        int o = boff >> 3;
        nzc_exp +=
            get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                           base_mb16 + 1) +
            get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                           base_mb16 + 3);
      } else {
        nzc_exp += cur->mbmi.nzcs[block - 4] << 5;
      }
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);

    case TX_4X4:
      if (boff < 4) {
        int o = boff >> 1;
        int p = boff & 1;
        nzc_exp = get_nzc_4x4_uv(cm, cur - mis + o, mb_row - 1, mb_col + o,
                                 base_mb16 + 2 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 4] << 6);
      }
      if ((boff & 3) == 0) {
        int o = boff >> 3;
        int p = (boff >> 2) & 1;
        nzc_exp += get_nzc_4x4_uv(cm, cur - 1 + o * mis, mb_row + o, mb_col - 1,
                                  base_mb16 + 1 + 2 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);

    default:
      return 0;
  }
}

int vp9_get_nzc_context_uv_mb16(VP9_COMMON *cm, MODE_INFO *cur,
                                int mb_row, int mb_col, int block) {
  // returns an index in [0, MAX_NZC_CONTEXTS - 1] to reflect how busy
  // neighboring blocks are
  int mis = cm->mode_info_stride;
  int nzc_exp = 0;
  const int base = block - (block & 3);
  const int boff = (block & 3);
  const int base_mb16 = base;
  TX_SIZE txfm_size = cur->mbmi.txfm_size;
  TX_SIZE txfm_size_uv;

  assert(block >= 16 && block < 24);
  if (txfm_size == TX_16X16)
    txfm_size_uv = TX_8X8;
  else if (txfm_size == TX_8X8 &&
           (cur->mbmi.mode == I8X8_PRED || cur->mbmi.mode == SPLITMV))
    txfm_size_uv = TX_4X4;
  else
    txfm_size_uv = txfm_size;

  switch (txfm_size_uv) {
    case TX_8X8:
      assert((block & 3) == 0);
      nzc_exp =
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col, base_mb16 + 2) +
          get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col, base_mb16 + 3) +
          get_nzc_4x4_uv(cm, cur - 1, mb_row, mb_col - 1, base_mb16 + 1) +
          get_nzc_4x4_uv(cm, cur - 1, mb_row, mb_col - 1, base_mb16 + 3);
      // Note nzc_exp is 64 times the average value expected at 8x8 scale
      return choose_nzc_context(nzc_exp, NZC_T2_8X8, NZC_T1_8X8);

    case TX_4X4:
      if (boff < 2) {
        int p = boff & 1;
        nzc_exp = get_nzc_4x4_uv(cm, cur - mis, mb_row - 1, mb_col,
                                 base_mb16 + 2 + p);
      } else {
        nzc_exp = (cur->mbmi.nzcs[block - 2] << 6);
      }
      if ((boff & 1) == 0) {
        int p = (boff >> 1) & 1;
        nzc_exp += get_nzc_4x4_uv(cm, cur - 1, mb_row, mb_col - 1,
                                  base_mb16 + 1 + 2 * p);
      } else {
        nzc_exp += (cur->mbmi.nzcs[block - 1] << 6);
      }
      nzc_exp >>= 1;
      // Note nzc_exp is 64 times the average value expected at 4x4 scale
      return choose_nzc_context(nzc_exp, NZC_T2_4X4, NZC_T1_4X4);

    default:
      return 0;
  }
}

int vp9_get_nzc_context(VP9_COMMON *cm, MACROBLOCKD *xd, int block) {
  if (xd->mode_info_context->mbmi.sb_type == BLOCK_SIZE_SB64X64) {
    assert(block < 384);
    if (block < 256)
      return vp9_get_nzc_context_y_sb64(cm, xd->mode_info_context,
                                        get_mb_row(xd), get_mb_col(xd), block);
    else
      return vp9_get_nzc_context_uv_sb64(cm, xd->mode_info_context,
                                         get_mb_row(xd), get_mb_col(xd), block);
  } else if (xd->mode_info_context->mbmi.sb_type == BLOCK_SIZE_SB32X32) {
    assert(block < 96);
    if (block < 64)
      return vp9_get_nzc_context_y_sb32(cm, xd->mode_info_context,
                                        get_mb_row(xd), get_mb_col(xd), block);
    else
      return vp9_get_nzc_context_uv_sb32(cm, xd->mode_info_context,
                                         get_mb_row(xd), get_mb_col(xd), block);
  } else {
    assert(block < 64);
    if (block < 16)
      return vp9_get_nzc_context_y_mb16(cm, xd->mode_info_context,
                                        get_mb_row(xd), get_mb_col(xd), block);
    else
      return vp9_get_nzc_context_uv_mb16(cm, xd->mode_info_context,
                                         get_mb_row(xd), get_mb_col(xd), block);
  }
}

static void update_nzc(VP9_COMMON *cm,
                       uint16_t nzc,
                       int nzc_context,
                       TX_SIZE tx_size,
                       int ref,
                       int type) {
  int e, c;
  c = codenzc(nzc);
  if (tx_size == TX_32X32)
    cm->fc.nzc_counts_32x32[nzc_context][ref][type][c]++;
  else if (tx_size == TX_16X16)
    cm->fc.nzc_counts_16x16[nzc_context][ref][type][c]++;
  else if (tx_size == TX_8X8)
    cm->fc.nzc_counts_8x8[nzc_context][ref][type][c]++;
  else if (tx_size == TX_4X4)
    cm->fc.nzc_counts_4x4[nzc_context][ref][type][c]++;
  else
    assert(0);

  if ((e = vp9_extranzcbits[c])) {
    int x = nzc - vp9_basenzcvalue[c];
    while (e--) {
      int b = (x >> e) & 1;
      cm->fc.nzc_pcat_counts[nzc_context][c - NZC_TOKENS_NOEXTRA][e][b]++;
    }
  }
}

static void update_nzcs_sb64(VP9_COMMON *cm,
                             MACROBLOCKD *xd,
                             int mb_row,
                             int mb_col) {
  MODE_INFO *m = xd->mode_info_context;
  MB_MODE_INFO *const mi = &m->mbmi;
  int j, nzc_context;
  const int ref = m->mbmi.ref_frame != INTRA_FRAME;

  assert(mb_col == get_mb_col(xd));
  assert(mb_row == get_mb_row(xd));

  if (mi->mb_skip_coeff)
    return;

  switch (mi->txfm_size) {
    case TX_32X32:
      for (j = 0; j < 256; j += 64) {
        nzc_context = vp9_get_nzc_context_y_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_32X32, ref, 0);
      }
      for (j = 256; j < 384; j += 64) {
        nzc_context = vp9_get_nzc_context_uv_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_32X32, ref, 1);
      }
      break;

    case TX_16X16:
      for (j = 0; j < 256; j += 16) {
        nzc_context = vp9_get_nzc_context_y_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 0);
      }
      for (j = 256; j < 384; j += 16) {
        nzc_context = vp9_get_nzc_context_uv_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 1);
      }
      break;

    case TX_8X8:
      for (j = 0; j < 256; j += 4) {
        nzc_context = vp9_get_nzc_context_y_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 0);
      }
      for (j = 256; j < 384; j += 4) {
        nzc_context = vp9_get_nzc_context_uv_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 1);
      }
      break;

    case TX_4X4:
      for (j = 0; j < 256; ++j) {
        nzc_context = vp9_get_nzc_context_y_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 0);
      }
      for (j = 256; j < 384; ++j) {
        nzc_context = vp9_get_nzc_context_uv_sb64(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 1);
      }
      break;

    default:
      break;
  }
}

static void update_nzcs_sb32(VP9_COMMON *cm,
                            MACROBLOCKD *xd,
                            int mb_row,
                            int mb_col) {
  MODE_INFO *m = xd->mode_info_context;
  MB_MODE_INFO *const mi = &m->mbmi;
  int j, nzc_context;
  const int ref = m->mbmi.ref_frame != INTRA_FRAME;

  assert(mb_col == get_mb_col(xd));
  assert(mb_row == get_mb_row(xd));

  if (mi->mb_skip_coeff)
    return;

  switch (mi->txfm_size) {
    case TX_32X32:
      for (j = 0; j < 64; j += 64) {
        nzc_context = vp9_get_nzc_context_y_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_32X32, ref, 0);
      }
      for (j = 64; j < 96; j += 16) {
        nzc_context = vp9_get_nzc_context_uv_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 1);
      }
      break;

    case TX_16X16:
      for (j = 0; j < 64; j += 16) {
        nzc_context = vp9_get_nzc_context_y_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 0);
      }
      for (j = 64; j < 96; j += 16) {
        nzc_context = vp9_get_nzc_context_uv_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 1);
      }
      break;

    case TX_8X8:
      for (j = 0; j < 64; j += 4) {
        nzc_context = vp9_get_nzc_context_y_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 0);
      }
      for (j = 64; j < 96; j += 4) {
        nzc_context = vp9_get_nzc_context_uv_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 1);
      }
      break;

    case TX_4X4:
      for (j = 0; j < 64; ++j) {
        nzc_context = vp9_get_nzc_context_y_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 0);
      }
      for (j = 64; j < 96; ++j) {
        nzc_context = vp9_get_nzc_context_uv_sb32(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 1);
      }
      break;

    default:
      break;
  }
}

static void update_nzcs_mb16(VP9_COMMON *cm,
                             MACROBLOCKD *xd,
                             int mb_row,
                             int mb_col) {
  MODE_INFO *m = xd->mode_info_context;
  MB_MODE_INFO *const mi = &m->mbmi;
  int j, nzc_context;
  const int ref = m->mbmi.ref_frame != INTRA_FRAME;

  assert(mb_col == get_mb_col(xd));
  assert(mb_row == get_mb_row(xd));

  if (mi->mb_skip_coeff)
    return;

  switch (mi->txfm_size) {
    case TX_16X16:
      for (j = 0; j < 16; j += 16) {
        nzc_context = vp9_get_nzc_context_y_mb16(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_16X16, ref, 0);
      }
      for (j = 16; j < 24; j += 4) {
        nzc_context = vp9_get_nzc_context_uv_mb16(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 1);
      }
      break;

    case TX_8X8:
      for (j = 0; j < 16; j += 4) {
        nzc_context = vp9_get_nzc_context_y_mb16(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 0);
      }
      if (mi->mode == I8X8_PRED || mi->mode == SPLITMV) {
        for (j = 16; j < 24; ++j) {
          nzc_context = vp9_get_nzc_context_uv_mb16(cm, m, mb_row, mb_col, j);
          update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 1);
        }
      } else {
        for (j = 16; j < 24; j += 4) {
          nzc_context = vp9_get_nzc_context_uv_mb16(cm, m, mb_row, mb_col, j);
          update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_8X8, ref, 1);
        }
      }
      break;

    case TX_4X4:
      for (j = 0; j < 16; ++j) {
        nzc_context = vp9_get_nzc_context_y_mb16(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 0);
      }
      for (j = 16; j < 24; ++j) {
        nzc_context = vp9_get_nzc_context_uv_mb16(cm, m, mb_row, mb_col, j);
        update_nzc(cm, m->mbmi.nzcs[j], nzc_context, TX_4X4, ref, 1);
      }
      break;

    default:
      break;
  }
}

void vp9_update_nzc_counts(VP9_COMMON *cm,
                           MACROBLOCKD *xd,
                           int mb_row,
                           int mb_col) {
  if (xd->mode_info_context->mbmi.sb_type == BLOCK_SIZE_SB64X64)
    update_nzcs_sb64(cm, xd, mb_row, mb_col);
  else if (xd->mode_info_context->mbmi.sb_type == BLOCK_SIZE_SB32X32)
    update_nzcs_sb32(cm, xd, mb_row, mb_col);
  else
    update_nzcs_mb16(cm, xd, mb_row, mb_col);
}
#endif  // CONFIG_CODE_NONZEROCOUNT

// #define COEF_COUNT_TESTING

#define COEF_COUNT_SAT 24
#define COEF_MAX_UPDATE_FACTOR 112
#define COEF_COUNT_SAT_KEY 24
#define COEF_MAX_UPDATE_FACTOR_KEY 112
#define COEF_COUNT_SAT_AFTER_KEY 24
#define COEF_MAX_UPDATE_FACTOR_AFTER_KEY 128

static void adapt_coef_probs(vp9_coeff_probs *dst_coef_probs,
                             vp9_coeff_probs *pre_coef_probs,
                             int block_types, vp9_coeff_count *coef_counts,
                             unsigned int (*eob_branch_count)[REF_TYPES]
                                                             [COEF_BANDS]
                                                      [PREV_COEF_CONTEXTS],
                             int count_sat, int update_factor) {
  int t, i, j, k, l, count;
  unsigned int branch_ct[ENTROPY_NODES][2];
  vp9_prob coef_probs[ENTROPY_NODES];
  int factor;

  for (i = 0; i < block_types; ++i)
    for (j = 0; j < REF_TYPES; ++j)
      for (k = 0; k < COEF_BANDS; ++k)
        for (l = 0; l < PREV_COEF_CONTEXTS; ++l) {
          if (l >= 3 && k == 0)
            continue;
          vp9_tree_probs_from_distribution(vp9_coef_tree,
                                           coef_probs, branch_ct,
                                           coef_counts[i][j][k][l], 0);
          branch_ct[0][1] = eob_branch_count[i][j][k][l] - branch_ct[0][0];
          coef_probs[0] = get_binary_prob(branch_ct[0][0], branch_ct[0][1]);
          for (t = 0; t < ENTROPY_NODES; ++t) {
            count = branch_ct[t][0] + branch_ct[t][1];
            count = count > count_sat ? count_sat : count;
            factor = (update_factor * count / count_sat);
            dst_coef_probs[i][j][k][l][t] =
                weighted_prob(pre_coef_probs[i][j][k][l][t],
                              coef_probs[t], factor);
          }
        }
}

void vp9_adapt_coef_probs(VP9_COMMON *cm) {
  int count_sat;
  int update_factor; /* denominator 256 */

  // printf("Frame type: %d\n", cm->frame_type);
  if (cm->frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_KEY;
    count_sat = COEF_COUNT_SAT_KEY;
  } else if (cm->last_frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_AFTER_KEY;  /* adapt quickly */
    count_sat = COEF_COUNT_SAT_AFTER_KEY;
  } else {
    update_factor = COEF_MAX_UPDATE_FACTOR;
    count_sat = COEF_COUNT_SAT;
  }

  adapt_coef_probs(cm->fc.coef_probs_4x4, cm->fc.pre_coef_probs_4x4,
                   BLOCK_TYPES, cm->fc.coef_counts_4x4,
                   cm->fc.eob_branch_counts[TX_4X4],
                   count_sat, update_factor);
  adapt_coef_probs(cm->fc.coef_probs_8x8, cm->fc.pre_coef_probs_8x8,
                   BLOCK_TYPES, cm->fc.coef_counts_8x8,
                   cm->fc.eob_branch_counts[TX_8X8],
                   count_sat, update_factor);
  adapt_coef_probs(cm->fc.coef_probs_16x16, cm->fc.pre_coef_probs_16x16,
                   BLOCK_TYPES, cm->fc.coef_counts_16x16,
                   cm->fc.eob_branch_counts[TX_16X16],
                   count_sat, update_factor);
  adapt_coef_probs(cm->fc.coef_probs_32x32, cm->fc.pre_coef_probs_32x32,
                   BLOCK_TYPES, cm->fc.coef_counts_32x32,
                   cm->fc.eob_branch_counts[TX_32X32],
                   count_sat, update_factor);
}

#if CONFIG_CODE_NONZEROCOUNT
static void adapt_nzc_probs(VP9_COMMON *cm,
                            int block_size,
                            int count_sat,
                            int update_factor) {
  int c, r, b, n;
  int count, factor;
  unsigned int nzc_branch_ct[NZC32X32_NODES][2];
  vp9_prob nzc_probs[NZC32X32_NODES];
  int tokens, nodes;
  const vp9_tree_index *nzc_tree;
  vp9_prob *dst_nzc_probs;
  vp9_prob *pre_nzc_probs;
  unsigned int *nzc_counts;

  if (block_size == 32) {
    tokens = NZC32X32_TOKENS;
    nzc_tree = vp9_nzc32x32_tree;
    dst_nzc_probs = cm->fc.nzc_probs_32x32[0][0][0];
    pre_nzc_probs = cm->fc.pre_nzc_probs_32x32[0][0][0];
    nzc_counts = cm->fc.nzc_counts_32x32[0][0][0];
  } else if (block_size == 16) {
    tokens = NZC16X16_TOKENS;
    nzc_tree = vp9_nzc16x16_tree;
    dst_nzc_probs = cm->fc.nzc_probs_16x16[0][0][0];
    pre_nzc_probs = cm->fc.pre_nzc_probs_16x16[0][0][0];
    nzc_counts = cm->fc.nzc_counts_16x16[0][0][0];
  } else if (block_size == 8) {
    tokens = NZC8X8_TOKENS;
    nzc_tree = vp9_nzc8x8_tree;
    dst_nzc_probs = cm->fc.nzc_probs_8x8[0][0][0];
    pre_nzc_probs = cm->fc.pre_nzc_probs_8x8[0][0][0];
    nzc_counts = cm->fc.nzc_counts_8x8[0][0][0];
  } else {
    nzc_tree = vp9_nzc4x4_tree;
    tokens = NZC4X4_TOKENS;
    dst_nzc_probs = cm->fc.nzc_probs_4x4[0][0][0];
    pre_nzc_probs = cm->fc.pre_nzc_probs_4x4[0][0][0];
    nzc_counts = cm->fc.nzc_counts_4x4[0][0][0];
  }
  nodes = tokens - 1;
  for (c = 0; c < MAX_NZC_CONTEXTS; ++c)
    for (r = 0; r < REF_TYPES; ++r)
      for (b = 0; b < BLOCK_TYPES; ++b) {
        int offset = c * REF_TYPES * BLOCK_TYPES + r * BLOCK_TYPES + b;
        int offset_nodes = offset * nodes;
        int offset_tokens = offset * tokens;
        vp9_tree_probs_from_distribution(nzc_tree,
                                         nzc_probs, nzc_branch_ct,
                                         nzc_counts + offset_tokens, 0);
        for (n = 0; n < nodes; ++n) {
          count = nzc_branch_ct[n][0] + nzc_branch_ct[n][1];
          count = count > count_sat ? count_sat : count;
          factor = (update_factor * count / count_sat);
          dst_nzc_probs[offset_nodes + n] =
              weighted_prob(pre_nzc_probs[offset_nodes + n],
                            nzc_probs[n], factor);
        }
      }
}

static void adapt_nzc_pcat(VP9_COMMON *cm, int count_sat, int update_factor) {
  int c, t;
  int count, factor;
  for (c = 0; c < MAX_NZC_CONTEXTS; ++c) {
    for (t = 0; t < NZC_TOKENS_EXTRA; ++t) {
      int bits = vp9_extranzcbits[t + NZC_TOKENS_NOEXTRA];
      int b;
      for (b = 0; b < bits; ++b) {
        vp9_prob prob = get_binary_prob(cm->fc.nzc_pcat_counts[c][t][b][0],
                                        cm->fc.nzc_pcat_counts[c][t][b][1]);
        count = cm->fc.nzc_pcat_counts[c][t][b][0] +
                cm->fc.nzc_pcat_counts[c][t][b][1];
        count = count > count_sat ? count_sat : count;
        factor = (update_factor * count / count_sat);
        cm->fc.nzc_pcat_probs[c][t][b] = weighted_prob(
            cm->fc.pre_nzc_pcat_probs[c][t][b], prob, factor);
      }
    }
  }
}

// #define NZC_COUNT_TESTING
void vp9_adapt_nzc_probs(VP9_COMMON *cm) {
  int count_sat;
  int update_factor; /* denominator 256 */
#ifdef NZC_COUNT_TESTING
  int c, r, b, t;
  printf("\n");
  for (c = 0; c < MAX_NZC_CONTEXTS; ++c)
    for (r = 0; r < REF_TYPES; ++r) {
      for (b = 0; b < BLOCK_TYPES; ++b) {
        printf("    {");
        for (t = 0; t < NZC4X4_TOKENS; ++t) {
          printf(" %d,", cm->fc.nzc_counts_4x4[c][r][b][t]);
        }
        printf("}\n");
      }
      printf("\n");
    }
#endif

  if (cm->frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_KEY;
    count_sat = COEF_COUNT_SAT_KEY;
  } else if (cm->last_frame_type == KEY_FRAME) {
    update_factor = COEF_MAX_UPDATE_FACTOR_AFTER_KEY;  /* adapt quickly */
    count_sat = COEF_COUNT_SAT_AFTER_KEY;
  } else {
    update_factor = COEF_MAX_UPDATE_FACTOR;
    count_sat = COEF_COUNT_SAT;
  }

  adapt_nzc_probs(cm, 4, count_sat, update_factor);
  adapt_nzc_probs(cm, 8, count_sat, update_factor);
  adapt_nzc_probs(cm, 16, count_sat, update_factor);
  adapt_nzc_probs(cm, 32, count_sat, update_factor);
  adapt_nzc_pcat(cm, count_sat, update_factor);
}
#endif  // CONFIG_CODE_NONZEROCOUNT
