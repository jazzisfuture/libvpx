#include <stdio.h>
#include <stdlib.h>
#include "vp9_loopfilter.h"
#include "loopfilter.h"
#include "rpcmem.h"
#include "dspCV.h"

#include <time.h>

#define ION_ADSP_HEAP_ID 22

extern void vp9_loop_filter_rows_work(int start, int stop, int num_planes,
    int mi_rows, int mi_cols, uint8_t *buffer_alloc,
    BUF_INFO * const buf_infos, const loop_filter_info_n *lf_info,
    LOOP_FILTER_MASK *lfms);

extern void *vpx_memalign(size_t align, size_t size);

int main() {
  FILE *file_pointer1 = fopen("/sdcard/loopfilter_input.bin", "r");
  FILE *file_pointer2 = fopen("/sdcard/loopfilter_output.bin", "w");

  clock_t start_t, end_t;
  double total_t;
  int retVal = 0;

  if (file_pointer1 && file_pointer2) {
    //vp9_loop_filter_rows_serialized(NULL, NULL, NULL, 1, 1, 1);
    int start = 0, stop = 0, num_planes = 0, mi_rows = 0, mi_cols = 0;

    fread(&start, sizeof(int), 1, file_pointer1);
    fread(&stop, sizeof(int), 1, file_pointer1);
    fread(&num_planes, sizeof(int), 1, file_pointer1);
    fread(&mi_rows, sizeof(int), 1, file_pointer1);
    fread(&mi_cols, sizeof(int), 1, file_pointer1);
    printf("Start: %d\n", start);
    printf("Stop: %d\n", stop);
    printf("NoPlanes: %d\n", num_planes);
    printf("Rows: %d\n", mi_rows);
    printf("Columns: %d\n", mi_cols);


    rpcmem_init();

       // call dspCV_initQ6() to bump up Q6 clock frequency
    retVal = dspCV_initQ6();
    printf("return value from dspCV_initQ6() : %d \n", retVal);

    int buffer_alloc_sz = 4704000;
    uint8_t *buffer_alloc = rpcmem_alloc(ION_ADSP_HEAP_ID, 0,
        buffer_alloc_sz);
    fread(buffer_alloc, 16, buffer_alloc_sz >> 4, file_pointer1);

    int size_bufo_infos = 25296;
    BUF_INFO *buf_infos = rpcmem_alloc(ION_ADSP_HEAP_ID, 0,
        size_bufo_infos);
    fread(buf_infos, sizeof(BUF_INFO),
        size_bufo_infos / sizeof(BUF_INFO), file_pointer1);

    int size_lf_info = 3152;
    loop_filter_info_n *lf_info = rpcmem_alloc(ION_ADSP_HEAP_ID, 0,
        size_lf_info);
    fread(lf_info, sizeof(loop_filter_info_n), 1, file_pointer1);

    int size_lfms = 92752;
    LOOP_FILTER_MASK *lfms = rpcmem_alloc(ION_ADSP_HEAP_ID, 0, size_lfms);
    fread(lfms, sizeof(LOOP_FILTER_MASK),
        size_lfms / sizeof(LOOP_FILTER_MASK), file_pointer1);

    int dsp_result = 0;

    printf("Loopfilter_rows_work_dsp: Entry\n");

    /* Dummy call to measure the Dynamic loading delay */
    start_t = clock();
    dsp_result = loopfilter_rows_work_dsp(start, stop, num_planes, mi_rows,
        mi_cols, buffer_alloc, buffer_alloc_sz, buf_infos,
        num_planes, (const uint8_t *) lf_info,
        size_lf_info, lfms, size_lfms / sizeof(LOOP_FILTER_MASK));
    end_t = clock();

    total_t = (double) (end_t - start_t) * 1000 / CLOCKS_PER_SEC;
    printf("Dynamic loading Time: %f millisec \n", total_t);

    /* Dummy call to measure the RPC round trip delay */
    start_t = clock();
    dsp_result = loopfilter_rows_work_dsp(start, stop, num_planes, mi_rows,
        mi_cols, buffer_alloc, buffer_alloc_sz, buf_infos,
        num_planes, (const uint8_t *) lf_info,
        size_lf_info, lfms, size_lfms / sizeof(LOOP_FILTER_MASK));
    end_t = clock();

    total_t = (double) (end_t - start_t) * 1000 / CLOCKS_PER_SEC;
    printf("RPC round trip Time: %f millisec \n", total_t);

    /* Call to Loop filter one frame of data */
    start_t = clock();
    dsp_result = loopfilter_rows_work_dsp(start, stop, num_planes, mi_rows,
        mi_cols, buffer_alloc, buffer_alloc_sz, buf_infos,
        num_planes, (const uint8_t *) lf_info,
        size_lf_info, lfms, size_lfms / sizeof(LOOP_FILTER_MASK));
    end_t = clock();

    total_t = (double) (end_t - start_t) * 1000 / CLOCKS_PER_SEC;
    printf("Loop filter: %d, frame Time : %f millisec \n", dsp_result, total_t);

    printf("Loopfilter_rows_work_dsp: Exit\n");

    fwrite(buffer_alloc, 16, buffer_alloc_sz >> 4, file_pointer2);

    fclose(file_pointer1);
    fclose(file_pointer2);

    if (buffer_alloc) {
      rpcmem_free(buffer_alloc);
    }
    if (buf_infos) {
      rpcmem_free(buf_infos);
    }
    if (lf_info) {
      rpcmem_free(lf_info);
    }
    if (lfms) {
      rpcmem_free(lfms);
    }

    printf("calling dspCV_deinitQ6()... \n");
    retVal = dspCV_deinitQ6();
    printf("return value from dspCV_deinitQ6(): %d \n", retVal);
    
    rpcmem_deinit();
  } else {
    printf("failed to open loopfilter DSP input file!\n");
  }


  return 0;
}
