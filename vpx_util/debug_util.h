#include <stdio.h>
#include "./vpx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_BITSTREAM_DEBUG
int bitstream_queue_get_write();
int bitstream_queue_get_read();
void bitstream_queue_record_write();
void bitstream_queue_reset_write();
void bitstream_queue_pop(int* result, int* prob);
void bitstream_queue_push(int result, int prob);
void bitstream_queue_skip_write_start();
void bitstream_queue_skip_write_end();
void bitstream_queue_skip_read_start();
void bitstream_queue_skip_read_end();
#endif

#ifdef __cplusplus
}  // extern "C"
#endif
