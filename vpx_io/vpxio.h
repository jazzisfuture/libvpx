#include "vpx_io/vpxio_intrn.h"

int vpxio_init_ctx(struct vpxio_ctx *input_ctx);
int vpxio_update_enc_ctx(struct vpxio_ctx *input_ctx, struct vpxio_ctx *output_ctx, unsigned int fourcc);
int vpxio_update_dec_ctx(struct vpxio_ctx *input_ctx, struct vpxio_ctx *output_ctx, int do_md5, int flipuv);

int vpxio_get_width(struct vpxio_ctx *input_ctx);
int vpxio_get_height(struct vpxio_ctx *input_ctx);
const char *vpxio_get_file_name(struct vpxio_ctx *input_ctx);
video_file_type_t vpxio_get_file_type(struct vpxio_ctx *input_ctx);
int vpxio_get_use_i420(struct vpxio_ctx *input_ctx);
unsigned int vpxio_get_fourcc(struct vpxio_ctx *input_ctx);
file_kind_t vpxio_get_webm_ctx_kind(struct vpxio_ctx *input_ctx);
struct vpx_rational vpxio_get_framerate(struct vpxio_ctx *input_ctx);

int vpxio_set_outfile_pattern(struct vpxio_ctx *input_ctx, const char *pattern);
int vpxio_set_file_name(struct vpxio_ctx *input_ctx, char *file_name);
int vpxio_set_file_type(struct vpxio_ctx *input_ctx, video_file_type_t file_type);
int vpxio_set_fourcc(struct vpxio_ctx *input_ctx, unsigned int fourcc);
int vpxio_set_debug(struct vpxio_ctx *input_ctx, int debug);
int vpxio_set_md5(struct vpxio_ctx *input_ctx, enum do_md5 md5_value);

int vpxio_open_src(struct vpxio_ctx *input_ctx, const char *input_file_name);
int vpxio_open_i420_src(struct vpxio_ctx *input_ctx, const char *input_file_name, int width, int height);
int vpxio_open_ivf_dst(struct vpxio_ctx *output_ctx, const char *file_name, int width, int height, int framerate_den, int framerate_num);
int vpxio_open_webm_dst(struct vpxio_ctx *output_ctx, const char *file_name, int width, int height, int framerate_den, int framerate_num);
int vpxio_open_i420_dst(struct vpxio_ctx *output_ctx, const char *outfile_pattern, int width, int height);
int vpxio_open_y4m_dst(struct vpxio_ctx *output_ctx, const char *outfile_pattern, int width, int height, int framerate_den, int framerate_num);
int vpxio_close(struct vpxio_ctx *input_ctx);

int vpxio_read_img(struct vpxio_ctx *input_ctx, vpx_image_t *img_raw);
int vpxio_write_pkt(struct vpxio_ctx *output_ctx, const vpx_codec_cx_pkt_t *pkt);
int vpxio_read_pkt(struct vpxio_ctx *input_ctx, uint8_t **buf, size_t *buf_sz, size_t *buf_alloc_sz);
int vpxio_write_img(struct vpxio_ctx *output_ctx, vpx_image_t *img);
