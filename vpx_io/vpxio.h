#ifdef __cplusplus
extern "C" {
#endif

#ifndef VPXIO_H
#define VPXIO_H

#include "vpx/vpx_image.h"
#include "vpx/vpx_encoder.h"
struct vpxio_ctx;
typedef enum do_md5
{
    NO_MD5,              //No md5
    FILE_MD5,            //Full file md5
    FRAME_MD5,           //Frame by frame md5
    FRAME_FILE_MD5,      //Frame by frame and full file md5
    FRAME_OUT_MD5        //Frame by frame and full file md5 written to file
} do_md5_t;
typedef enum stereo_format
{
    STEREO_FORMAT_MONO       = 0,
    STEREO_FORMAT_LEFT_RIGHT = 1,
    STEREO_FORMAT_BOTTOM_TOP = 2,
    STEREO_FORMAT_TOP_BOTTOM = 3,
    STEREO_FORMAT_RIGHT_LEFT = 11
} stereo_format_t;
typedef enum video_file_type
{
    NO_FILE,
    FILE_TYPE_I420,
    FILE_TYPE_YV12,
    FILE_TYPE_RAW_PKT,
    FILE_TYPE_IVF,
    FILE_TYPE_Y4M,
    FILE_TYPE_WEBM
} video_file_type_t;

int vpxio_get_width(struct vpxio_ctx *ctx);
int vpxio_get_height(struct vpxio_ctx *ctx);
const char *vpxio_get_file_name(struct vpxio_ctx *ctx);
video_file_type_t vpxio_get_file_type(struct vpxio_ctx *ctx);
int vpxio_get_use_i420(struct vpxio_ctx *ctx);
unsigned int vpxio_get_fourcc(struct vpxio_ctx *ctx);
struct vpx_rational vpxio_get_framerate(struct vpxio_ctx *ctx);
int vpxio_set_outfile_pattern(struct vpxio_ctx *ctx, const char *pattern);
int vpxio_set_file_name(struct vpxio_ctx *ctx, char *file_name);
int vpxio_set_debug(struct vpxio_ctx *ctx, int debug);
int vpxio_set_md5(struct vpxio_ctx *ctx, enum do_md5 md5_value);
struct vpxio_ctx* vpxio_open_src(struct vpxio_ctx *ctx,
                                 const char *input_file_name);
struct vpxio_ctx* vpxio_open_raw_src(struct vpxio_ctx *ctx,
                        const char *input_file_name,
                        int width,
                        int height,
                        vpx_img_fmt_t img_fmt);
struct vpxio_ctx* vpxio_open_ivf_dst(struct vpxio_ctx *ctx,
                       const char *file_name,
                       int width, int height,
                       int framerate_den,
                       int framerate_num);
struct vpxio_ctx* vpxio_open_webm_dst(struct vpxio_ctx *ctx,
                        const char *file_name,
                        int width,
                        int height,
                        int framerate_den,
                        int framerate_num);
struct vpxio_ctx* vpxio_open_raw_dst(struct vpxio_ctx *ctx,
                        const char *outfile_pattern,
                        int width,
                        int height,
                        vpx_img_fmt_t img_fmt);
struct vpxio_ctx* vpxio_open_y4m_dst(struct vpxio_ctx *ctx,
                       const char *outfile_pattern,
                       int width,
                       int height,
                       struct vpx_rational framerate);
int vpxio_close(struct vpxio_ctx *ctx);
int vpxio_read_img(struct vpxio_ctx *ctx, vpx_image_t *img_raw);
int vpxio_write_pkt(struct vpxio_ctx *ctx,
                    const vpx_codec_cx_pkt_t *pkt);
int vpxio_read_pkt(struct vpxio_ctx *ctx,
                   uint8_t **buf,
                   size_t *buf_sz,
                   size_t *buf_alloc_sz);
int vpxio_write_img(struct vpxio_ctx *ctx,
                    vpx_image_t *img);

#endif

#ifdef __cplusplus
}
#endif