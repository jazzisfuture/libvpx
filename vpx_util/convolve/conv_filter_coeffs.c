/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Note:
// Workflow for x86 SIMD version of subpel filters
//
//  1) Utility, "conv_filter_coeffs" is built from this file by:
//     $ make
//  2) Usage:
//     Based on vp10/common/filter.c, we need to run this utility to
//     generate a SIMD filter coefficient file, i.e.,
//     vp10/common/x86/vp10_convolve_filters_ssse3.c by running:
//     $ conv_filter_coeffs
//     File vp10_convolve_filter_ssse3.c is generated in currect directory.
//  3) Copy generated file "vp10_convolve_filters_ssse3.c" to vp10/common/x86
//  4) Test and make a patch for code review.
//
//  The only assumption is the filter array in filter.c:
//    int16_t filter_10[16][10] or int16_t filter_12[16][12];
//  For any changes on filter.c
//  1) Already covered filter coefficient change:
//     We need to run this utility again.
//  2) New filter are added in filter.c:
//     We need to follow the example in this main() function.
//     Add function call by the API provided by this utility to generate new
//     filters.  API provided by this utility is:
//     set_10tap_filter()/set_12tap_filter(),
//     print_filter_coefficients().
//     See the main() function as example.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_EXT_INTERP         (1)
#define USE_TEMPORALFILTER_12TAP  (1)

// Note:
//  The following declarations are from libvpx. We copy them here to
//  make compiler pass vp10/common/filter.c only.
#if (defined(__GNUC__) && __GNUC__) || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n, typ, val)  typ val __attribute__ ((aligned (n)))
#endif

#define SUBPEL_TAPS (8)

typedef int16_t InterpKernel[SUBPEL_TAPS];
typedef struct InterpFilterParams {
  const int16_t* filter_ptr;
  uint16_t taps;
  uint16_t subpel_shifts;
} InterpFilterParams;

typedef uint8_t INTERP_FILTER;
typedef const int8_t (*SubpelFilterCoeffs)[16];

#define SUBPEL_BITS   (4)
#define SUBPEL_SHIFTS (1 << SUBPEL_BITS)
#define SWITCHABLE_FILTERS  (5)

#if USE_TEMPORALFILTER_12TAP
#define TEMPORALFILTER_12TAP (SWITCHABLE_FILTERS + 1)
#endif
// end of declaration from libvpx

#define GENERATE_SIMD_FILTER (1)
#include "../../vp10/common/filter.c"

enum DefineMacro {
  use_temporal_filter,
  config_ext_interp,
  no_define,
};

typedef struct FilterInfo_ {
  FILE *fptr;
  int tapsNum;
  const int16_t (*filter10)[10];
  const char *filter10_symbol;
  const int16_t (*filter12)[12];
  const char *filter12_symbol;
  bool signalDir;
  enum DefineMacro dm;
} FilterInfo;

static const char source_file_name[32] = "vp10_convolve_filters_ssse3.c";

static void print_copyright_notice(const FilterInfo *fInfo) {
  FILE *f = fInfo->fptr;
  fprintf(f, "%s", "/*\n");
  fprintf(f, "%s",
  " *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.\n");
  fprintf(f, "%s", " *\n");
  fprintf(f, "%s",
  " *  Use of this source code is governed by a BSD-style license\n");
  fprintf(f, "%s",
  " *  that can be found in the LICENSE file in the root of the source\n");
  fprintf(f, "%s",
  " *  tree. An additional intellectual property rights grant can be found\n");
  fprintf(f, "%s",
  " *  in the file PATENTS.  All contributing project authors may\n");
  fprintf(f, "%s",
  " *  be found in the AUTHORS file in the root of the source tree.\n");
  fprintf(f, "%s", " */\n");
}

static void print_include_headers(const FilterInfo *fInfo) {
  FILE *f = fInfo->fptr;
  fprintf(f, "%s", "#include \"./vpx_config.h\"\n");
  fprintf(f, "%s", "#include \"vp10/common/filter.h\"\n\n");
}

void init_state(FilterInfo *fInfo) {
  memset(fInfo, 0, sizeof(*fInfo));

  fInfo->fptr = fopen(source_file_name, "w");
  if (!fInfo->fptr) {
    printf("Failed to open output file!\n");
    exit(-1);
  }

  print_copyright_notice(fInfo);
  print_include_headers(fInfo);
}

void exit_state(FilterInfo *fInfo) {
  fclose(fInfo->fptr);
}

static const char white_space[] = "                ";
static const char print_fmt[] = "%3d,";

static void print_array_name(const FilterInfo *fInfo) {
  FILE *f = fInfo->fptr;

  fprintf(f, "%s", "DECLARE_ALIGNED(16, const int8_t,\n");
  if (10 == fInfo->tapsNum) {
    fprintf(f, "%s%s", white_space, fInfo->filter10_symbol);
  }
  if (12 == fInfo->tapsNum) {
    fprintf(f, "%s%s", white_space, fInfo->filter12_symbol);
  }
  if (fInfo->signalDir) {
    fprintf(f, "%s", "_signal_dir[15][2][16]) = {\n");
  } else {
    fprintf(f, "%s", "_ver_signal_dir[15][6][16]) = {\n");
  }
}

static void print_zero(FILE *fptr, int num, const char *format) {
  int i;
  for (i = 0; i < num; i++) {
    fprintf(fptr, format, 0);
  }
}

static void print_coeffs(FILE *fptr, const int16_t *p, int num,
                         const char *format) {
  int i;
  for (i = 0; i < num; i++) {
    fprintf(fptr, format, *p++);
  }
}

static void print_coeff_pair(FILE *fptr, int16_t h1, int16_t h2,
                             const char *format) {
  int i = 8;
  do {
    fprintf(fptr, format, h1);
    fprintf(fptr, format, h2);
    i -= 1;
  } while (i > 0);
}

static void print_10tap_ver_signal_dir(FILE *fptr, const int16_t *p,
                                       const char *format, int i) {
  if (0 == i) {
    print_coeff_pair(fptr, 0, p[0], format);
  } else if (5 == i) {
    print_coeff_pair(fptr, p[9], 0, format);
  } else {
    print_coeff_pair(fptr, p[2 * i - 1], p[2 * i], format);
  }
}

static void print_12tap_ver_signal_dir(FILE *fptr, const int16_t *p,
                                       const char *format, int i) {
  print_coeff_pair(fptr, p[2 * i], p[2 * i + 1], format);
}

static void print_10tap_coeffs(FilterInfo *fInfo, int i) {
  const int16_t *p = (const int16_t *)fInfo->filter10;
  FILE *f = fInfo->fptr;

  if (fInfo->signalDir) {
    if (0 == i) {
      print_zero(f, 1, print_fmt);
      print_coeffs(f, p, 10, print_fmt);
      print_zero(f, 5, print_fmt);
    } else {
      print_zero(f, 3, print_fmt);
      print_coeffs(f, p, 10, print_fmt);
      print_zero(f, 3, print_fmt);
    }
  } else {
    print_10tap_ver_signal_dir(f, p, print_fmt, i);
  }
}

static void print_12tap_coeffs(FilterInfo *fInfo, int i) {
  const int16_t *p = (const int16_t *)fInfo->filter12;
  FILE *f = fInfo->fptr;

  if (fInfo->signalDir) {
    if (0 == i) {
      print_coeffs(f, p, 12, print_fmt);
      print_zero(f, 4, print_fmt);
    } else {
      print_zero(f, 2, print_fmt);
      print_coeffs(f, p, 12, print_fmt);
      print_zero(f, 2, print_fmt);
    }
  } else {
    print_12tap_ver_signal_dir(f, p, print_fmt, i);
  }
}

static void print_filter(FilterInfo *fInfo) {
  const int filtersNum = 15;
  const int filterCoeffsNum = 16;
  int filterPatterns;
  FILE *f = fInfo->fptr;
  int i, j;

  // We ignore the first filter, interpolation filter
  if (fInfo->tapsNum == 12) {
    fInfo->filter12 += 1;
  }
  if (fInfo->tapsNum == 10) {
    fInfo->filter10 += 1;
  }

  if (fInfo->signalDir) {
    filterPatterns = 2;
  } else {
    filterPatterns = 6;
  }

  for (i = 0; i < filtersNum; i++) {
    fprintf(f, "%s", "  {\n");
    for (j = 0; j < filterPatterns; j++) {
      fprintf(f, "%s", "    {");
      if (fInfo->tapsNum == 10) {
        print_10tap_coeffs(fInfo, j);
      }
      if (fInfo->tapsNum == 12) {
        print_12tap_coeffs(fInfo, j);
      }
      fprintf(f, "%s", "},\n");
    }
    fprintf(f, "%s", "  },\n");
    if (fInfo->tapsNum == 10) {
      fInfo->filter10 += 1;
    }
    if (fInfo->tapsNum == 12) {
      fInfo->filter12 += 1;
    }
  }

  fprintf(f, "%s", "};\n");

  // We move pointer back to our filter coefficents
  if (fInfo->tapsNum == 10) {
    fInfo->filter10 -= SUBPEL_SHIFTS;
  }
  if (fInfo->tapsNum == 12) {
    fInfo->filter12 -= SUBPEL_SHIFTS;
  }
}

static void print_filter_coeffs(FilterInfo *fInfo) {
  FILE *f = fInfo->fptr;

  if (fInfo->dm == use_temporal_filter) {
    fprintf(f, "%s", "#if USE_TEMPORALFILTER_12TAP\n");
  }
  if (fInfo->dm == config_ext_interp) {
    fprintf(f, "%s", "#if CONFIG_EXT_INTERP\n");
  }

  print_array_name(fInfo);
  print_filter(fInfo);

  if (fInfo->dm == config_ext_interp) {
    fprintf(f, "%s", "#endif\n");
  }
  if (fInfo->dm == use_temporal_filter) {
    fprintf(f, "%s", "#endif\n");
  }
}

static void print_signal_direction_filter_coeffs(FilterInfo *fInfo) {
  fInfo->signalDir = true;
  print_filter_coeffs(fInfo);
}

static void print_vert_signal_direction_filter_coeffs(
    FilterInfo *fInfo) {
  fInfo->signalDir = false;
  print_filter_coeffs(fInfo);
}

void set_10tap_filter(const int16_t (*filter)[10], const char *symbol,
                      FilterInfo *fInfo, enum DefineMacro dm) {
  fInfo->tapsNum = 10;
  fInfo->filter10 = filter;
  fInfo->filter10_symbol = symbol;
  fInfo->dm = dm;
}

void set_12tap_filter(const int16_t (*filter)[12], const char *symbol,
                      FilterInfo *fInfo, enum DefineMacro dm) {
  fInfo->tapsNum = 12;
  fInfo->filter12 = filter;
  fInfo->filter12_symbol = symbol;
  fInfo->dm = dm;
}

void print_filter_coefficients(FilterInfo *fInfo) {
  print_signal_direction_filter_coeffs(fInfo);
  print_vert_signal_direction_filter_coeffs(fInfo);
}

int main() {
  FilterInfo filter;

  init_state(&filter);

  set_10tap_filter(sub_pel_filters_10sharp, "sub_pel_filters_10sharp",
                   &filter, config_ext_interp);
  print_filter_coefficients(&filter);

  set_12tap_filter(sub_pel_filters_12sharp, "sub_pel_filters_12sharp",
                   &filter, config_ext_interp);
  print_filter_coefficients(&filter);

  set_12tap_filter(sub_pel_filters_temporalfilter_12,
                   "sub_pel_filters_temporalfilter_12",
                   &filter, use_temporal_filter);
  print_filter_coefficients(&filter);

  exit_state(&filter);
  return 0;
}
