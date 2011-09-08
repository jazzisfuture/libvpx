#include <wchar.h>
#include "y4minput.h"
#include "libmkv/EbmlWriter.h"
#include "libmkv/EbmlIDs.h"
#include "nestegg/include/nestegg/nestegg.h"
#include "vpx/vpx_encoder.h"

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#if !defined(_WIN32)
#include <sys/types.h>
#endif

typedef off_t EbmlLoc;

typedef enum file_kind
{
    NO_FILE,
    RAW_FILE,
    IVF_FILE,
    WEBM_FILE
} file_kind_t;
typedef enum do_md5
{
    NO_MD5,              //No md5
    FILE_MD5,            //Full file md5
    FRAME_MD5,           //Frame by frame md5
    FRAME_FILE_MD5,      //Frame by frame and full file md5
    FRAME_OUT_MD5   //Frame by frame and full file md5 written to file
} do_md5_t;
typedef struct webm_input_ctx
{
    enum file_kind  kind;
    FILE           *infile;
    nestegg        *nestegg_ctx;
    nestegg_packet *pkt;
    unsigned int    chunk;
    unsigned int    chunks;
    unsigned int    video_track;
} webm_input_ctx_t;
typedef struct detect_buffer
{
    char buf[4];
    size_t buf_read;
    size_t position;
} detect_buffer_t;
typedef struct EbmlGlobal
{
    int debug;
    FILE    *stream;
    int64_t last_pts_ms;
    vpx_rational_t  framerate;
    off_t    position_reference;
    off_t    seek_info_pos;
    off_t    segment_info_pos;
    off_t    track_pos;
    off_t    cue_pos;
    off_t    cluster_pos;
    off_t    track_id_pos;
    EbmlLoc  startSegment;
    EbmlLoc  startCluster;
    uint32_t cluster_timecode;
    int      cluster_open;
    struct cue_entry *cue_list;
    unsigned int      cues;
} EbmlGlobal_t;
typedef enum video_file_type
{
    FILE_TYPE_I420,
    FILE_TYPE_RAW,
    FILE_TYPE_IVF,
    FILE_TYPE_Y4M,
    FILE_TYPE_WEBM
} video_file_type_t;
typedef enum vpxio_mode
{
    NONE,
    SRC,
    DST
} vpxio_mode_t;
typedef struct vpxio_comp_ctx
{

    EbmlGlobal    ebml;
    uint32_t      hash;
    struct detect_buffer detect;
    vpx_image_t  *img_raw_ptr;
    y4m_input     y4m_in;
} vpxio_comp_ctx_t;
typedef struct vpxio_decomp_ctx
{

    struct webm_input_ctx webm_ctx;
    const char           *outfile_pattern;
    int                   single_file;
    int                   noblit;
    int                   do_md5;
    int                   flipuv;
    uint8_t               *buf_ptr;
} vpxio_decomp_ctx_t;
typedef struct vpxio_ctx
{

    FILE                    *file;
    const char              *file_name;
    enum video_file_type     file_type;
    unsigned int             fourcc;
    unsigned int             width;
    unsigned int             height;
    unsigned int             frame_cnt;
    enum vpxio_mode          mode;
    struct vpx_rational      framerate;
    struct vpx_rational      timebase;
    struct vpxio_comp_ctx   compression_ctx;
    struct vpxio_decomp_ctx decompression_ctx;
} vpxio_ctx_t;

#define IVF_FRAME_HDR_SZ (sizeof(uint32_t) + sizeof(uint64_t))
#define RAW_FRAME_HDR_SZ (sizeof(uint32_t))

#if !defined(_MSC_VER)
#define LITERALU64(n) n##LLU
#endif

#if defined(_MSC_VER)
#define fseeko _fseeki64
#define ftello _ftelli64
#define LITERALU64(n) n
#elif defined(_WIN32)
#define fseeko fseeko64
#define ftello ftello64
#endif

#if CONFIG_OS_SUPPORT
#if defined(_WIN32)
#include <io.h>
#define snprintf _snprintf
#define isatty   _isatty
#define fileno   _fileno
#else
#include <unistd.h>
#endif
#endif
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#if defined(_WIN32)
typedef unsigned long DWORD;
#else
typedef unsigned int  DWORD;
#endif
typedef unsigned char BYTE;

#ifdef __POWERPC__
# define make_endian_16(a) \
    (((unsigned int)(a & 0xff)) << 8) | (((unsigned int)(a & 0xff00)) >> 8)
# define make_endian_32(a)                                                                  \
    (((unsigned int)(a & 0xff)) << 24)    | (((unsigned int)(a & 0xff00)) << 8) |   \
    (((unsigned int)(a & 0xff0000)) >> 8) | (((unsigned int)(a & 0xff000000)) >> 24)
# define make_endian_64(a)  \
    ((a & 0xff) << 56           |   \
     ((a >>  8) & 0xff) << 48   |   \
     ((a >> 16) & 0xff) << 40   |   \
     ((a >> 24) & 0xff) << 32   |   \
     ((a >> 32) & 0xff) << 24   |   \
     ((a >> 40) & 0xff) << 16   |   \
     ((a >> 48) & 0xff) <<  8   |   \
     ((a >> 56) & 0xff))
# define MAKEFOURCC(ch0, ch1, ch2, ch3)                                 \
    ((DWORD)(BYTE)(ch0) << 24 | ((DWORD)(BYTE)(ch1) << 16) |    \
     ((DWORD)(BYTE)(ch2) << 8) | ((DWORD)(BYTE)(ch3)))
# define swap4(d)\
    ((d&0x000000ff)<<24) |  \
    ((d&0x0000ff00)<<8)  |  \
    ((d&0x00ff0000)>>8)  |  \
    ((d&0xff000000)>>24)
#else
# define make_endian_16(a)  a
# define make_endian_32(a)  a
# define make_endian_64(a)  a
# define MAKEFOURCC(ch0, ch1, ch2, ch3)                                 \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |           \
     ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
# define swap4(d) d
#endif

typedef enum stereo_format
{
    STEREO_FORMAT_MONO       = 0,
    STEREO_FORMAT_LEFT_RIGHT = 1,
    STEREO_FORMAT_BOTTOM_TOP = 2,
    STEREO_FORMAT_TOP_BOTTOM = 3,
    STEREO_FORMAT_RIGHT_LEFT = 11
} stereo_format_t;

void *out_open(const char *out_fn, int do_md5);
void out_put(void *out, const uint8_t *buf, unsigned int len, int do_md5);

