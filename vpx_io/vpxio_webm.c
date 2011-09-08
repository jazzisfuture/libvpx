#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "vpx_io/vpxio.h"
#include "vpx_io/vpxio_intrn.h"
#include "libmkv/EbmlWriter.h"
#include "libmkv/EbmlIDs.h"
#include "nestegg/include/nestegg/nestegg.h"
#include "vpx_version.h"
#include "vpx_ports/mem_ops.h"
#include "limits.h"
#include "tools_common.h"

#define VP8_FOURCC (0x00385056)

struct cue_entry
{
    unsigned int time;
    uint64_t     loc;
};
int webm_guess_framerate(struct vpxio_ctx *ctx)
{
    unsigned int i;
    uint64_t     tstamp = 0;

    for (i = 0; tstamp < 1000000000 && i < 50;)
    {
        nestegg_packet *pkt;
        unsigned int track;

        if (nestegg_read_packet(ctx->decompression_ctx.webm_ctx.nestegg_ctx,
                                &pkt) <= 0)
            break;

        nestegg_packet_track(pkt, &track);

        if (track == ctx->decompression_ctx.webm_ctx.video_track)
        {
            nestegg_packet_tstamp(pkt, &tstamp);
            i++;
        }

        nestegg_free_packet(pkt);
    }

    if (nestegg_track_seek(ctx->decompression_ctx.webm_ctx.nestegg_ctx,
                           ctx->decompression_ctx.webm_ctx.video_track, 0))
        goto fail;

    ctx->framerate.num = (i - 1) * 1000000;
    ctx->framerate.den = tstamp / 1000;
    return 0;
fail:
    nestegg_destroy(ctx->decompression_ctx.webm_ctx.nestegg_ctx);
    ctx->decompression_ctx.webm_ctx.nestegg_ctx = NULL;
    rewind(ctx->decompression_ctx.webm_ctx.infile);
    return 1;
}
int nestegg_read_cb(void *buffer, size_t length, void *userdata)
{
    FILE *f = (FILE *)userdata;

    if (fread(buffer, 1, length, f) < length)
    {
        if (ferror(f))
            return -1;

        if (feof(f))
            return 0;
    }

    return 1;
}
int nestegg_seek_cb(int64_t offset, int whence, void *userdata)
{
    switch (whence)
    {
    case NESTEGG_SEEK_SET:
        whence = SEEK_SET;
        break;
    case NESTEGG_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case NESTEGG_SEEK_END:
        whence = SEEK_END;
        break;
    }

    return fseek((FILE *)userdata, offset, whence) ? -1 : 0;
}
int64_t nestegg_tell_cb(void *userdata)
{
    return ftell((FILE *)userdata);
}
void Ebml_Write(EbmlGlobal *glob, const void *buffer_in, unsigned long len)
{
    if (fwrite(buffer_in, 1, len, glob->stream));
}

#define WRITE_BUFFER(s) \
    for(i = len-1; i>=0; i--)\
    { \
        x = *(const s *)buffer_in >> (i * CHAR_BIT); \
        Ebml_Write(glob, &x, 1); \
    }
void Ebml_Serialize(EbmlGlobal *glob, const void *buffer_in, int buffer_size,
                    unsigned long len)
{
    char x;
    int i;

    /* buffer_size:
     * 1 - int8_t;
     * 2 - int16_t;
     * 3 - int32_t;
     * 4 - int64_t;
     */
    switch (buffer_size)
    {
    case 1:
        WRITE_BUFFER(int8_t)
        break;
    case 2:
        WRITE_BUFFER(int16_t)
        break;
    case 4:
        WRITE_BUFFER(int32_t)
        break;
    case 8:
        WRITE_BUFFER(int64_t)
        break;
    default:
        break;
    }
}
#undef WRITE_BUFFER

/* Need a fixed size serializer for the track ID. libmkv provdes a 64 bit
 * one, but not a 32 bit one.
 */
static void Ebml_SerializeUnsigned32(EbmlGlobal *glob,
                                     unsigned long class_id,
                                     uint64_t ui)
{
    unsigned char sizeSerialized = 4 | 0x80;
    Ebml_WriteID(glob, class_id);
    Ebml_Serialize(glob, &sizeSerialized, sizeof(sizeSerialized), 1);
    Ebml_Serialize(glob, &ui, sizeof(ui), 4);
}


static void
Ebml_StartSubElement(EbmlGlobal *glob,
                     EbmlLoc *ebmlLoc,
                     unsigned long class_id)
{
    //todo this is always taking 8 bytes, this may need later optimization
    //this is a key that says lenght unknown
    unsigned long long unknownLen =  LITERALU64(0x01FFFFFFFFFFFFFF);

    Ebml_WriteID(glob, class_id);
    *ebmlLoc = ftello(glob->stream);
    Ebml_Serialize(glob, &unknownLen, sizeof(unknownLen), 8);
}

static void
Ebml_EndSubElement(EbmlGlobal *glob, EbmlLoc *ebmlLoc)
{
    off_t pos;
    uint64_t size;

    /* Save the current stream pointer */
    pos = ftello(glob->stream);

    /* Calculate the size of this element */
    size = pos - *ebmlLoc - 8;
    size |=  LITERALU64(0x0100000000000000);

    /* Seek back to the beginning of the element and write the new size */
    fseeko(glob->stream, *ebmlLoc, SEEK_SET);
    Ebml_Serialize(glob, &size, sizeof(size), 8);

    /* Reset the stream pointer */
    fseeko(glob->stream, pos, SEEK_SET);
}


static void
write_webm_seek_element(EbmlGlobal *ebml, unsigned long id, off_t pos)
{
    uint64_t offset = pos - ebml->position_reference;
    EbmlLoc start;
    Ebml_StartSubElement(ebml, &start, Seek);
    Ebml_SerializeBinary(ebml, SeekID, id);
    Ebml_SerializeUnsigned64(ebml, SeekPosition, offset);
    Ebml_EndSubElement(ebml, &start);
}


static void
write_webm_seek_info(EbmlGlobal *ebml)
{

    off_t pos;

    /* Save the current stream pointer */
    pos = ftello(ebml->stream);

    if (ebml->seek_info_pos)
        fseeko(ebml->stream, ebml->seek_info_pos, SEEK_SET);
    else
        ebml->seek_info_pos = pos;

    {
        EbmlLoc start;

        Ebml_StartSubElement(ebml, &start, SeekHead);
        write_webm_seek_element(ebml, Tracks, ebml->track_pos);
        write_webm_seek_element(ebml, Cues,   ebml->cue_pos);
        write_webm_seek_element(ebml, Info,   ebml->segment_info_pos);
        Ebml_EndSubElement(ebml, &start);
    }
    {
        //segment info
        EbmlLoc startInfo;
        uint64_t frame_time;

        frame_time = (uint64_t)1000 * ebml->framerate.den
                     / ebml->framerate.num;
        ebml->segment_info_pos = ftello(ebml->stream);
        Ebml_StartSubElement(ebml, &startInfo, Info);
        Ebml_SerializeUnsigned(ebml, TimecodeScale, 1000000);
        Ebml_SerializeFloat(ebml, Segment_Duration,
                            ebml->last_pts_ms + frame_time);
        Ebml_SerializeString(ebml, 0x4D80,
                             ebml->debug ? "vpxenc" : "vpxenc" VERSION_STRING);
        Ebml_SerializeString(ebml, 0x5741,
                             ebml->debug ? "vpxenc" : "vpxenc" VERSION_STRING);
        Ebml_EndSubElement(ebml, &startInfo);
    }
}


static void
write_webm_file_header_intrn(EbmlGlobal                *glob,
                             unsigned int width,
                             unsigned int height,
                             const struct vpx_rational *fps,
                             stereo_format_t            stereo_fmt)
{
    {
        EbmlLoc start;
        Ebml_StartSubElement(glob, &start, EBML);
        Ebml_SerializeUnsigned(glob, EBMLVersion, 1);
        Ebml_SerializeUnsigned(glob, EBMLReadVersion, 1); //EBML Read Version
        Ebml_SerializeUnsigned(glob, EBMLMaxIDLength, 4); //EBML Max ID Length
        Ebml_SerializeUnsigned(glob, EBMLMaxSizeLength, 8); //EBML Max Size Length
        Ebml_SerializeString(glob, DocType, "webm"); //Doc Type
        Ebml_SerializeUnsigned(glob, DocTypeVersion, 2); //Doc Type Version
        Ebml_SerializeUnsigned(glob, DocTypeReadVersion, 2); //Doc Type Read Version
        Ebml_EndSubElement(glob, &start);
    }
    {
        Ebml_StartSubElement(glob, &glob->startSegment, Segment); //segment
        glob->position_reference = ftello(glob->stream);
        glob->framerate = *fps;

        write_webm_seek_info(glob);

        {
            EbmlLoc trackStart;
            glob->track_pos = ftello(glob->stream);
            Ebml_StartSubElement(glob, &trackStart, Tracks);
            {
                unsigned int trackNumber = 1;
                uint64_t     trackID = 0;

                EbmlLoc start;
                Ebml_StartSubElement(glob, &start, TrackEntry);
                Ebml_SerializeUnsigned(glob, TrackNumber, trackNumber);
                glob->track_id_pos = ftello(glob->stream);
                Ebml_SerializeUnsigned32(glob, TrackUID, trackID);
                Ebml_SerializeUnsigned(glob, TrackType, 1); //video is always 1
                Ebml_SerializeString(glob, CodecID, "V_VP8");
                {
                    unsigned int pixelWidth = width;
                    unsigned int pixelHeight = height;
                    float        frameRate  = (float)fps->num / (float)fps->den;

                    EbmlLoc videoStart;
                    Ebml_StartSubElement(glob, &videoStart, Video);
                    Ebml_SerializeUnsigned(glob, PixelWidth, pixelWidth);
                    Ebml_SerializeUnsigned(glob, PixelHeight, pixelHeight);
                    Ebml_SerializeUnsigned(glob, StereoMode, stereo_fmt);
                    Ebml_SerializeFloat(glob, FrameRate, frameRate);
                    Ebml_EndSubElement(glob, &videoStart); //Video
                }
                Ebml_EndSubElement(glob, &start); //Track Entry
            }
            Ebml_EndSubElement(glob, &trackStart);
        }
        // segment element is open
    }
}
static void
write_webm_block(EbmlGlobal                *glob,
                 int timebase_num,
                 int timebase_den,
                 const vpx_codec_cx_pkt_t  *pkt)
{
    unsigned long  block_length;
    unsigned char  track_number;
    unsigned short block_timecode = 0;
    unsigned char  flags;
    int64_t        pts_ms;
    int            start_cluster = 0, is_keyframe;

    /* Calculate the PTS of this frame in milliseconds */
    pts_ms = pkt->data.frame.pts * 1000
             * timebase_num / timebase_den;

    if (pts_ms <= glob->last_pts_ms)
        pts_ms = glob->last_pts_ms + 1;

    glob->last_pts_ms = pts_ms;

    /* Calculate the relative time of this block */
    if (pts_ms - glob->cluster_timecode > SHRT_MAX)
        start_cluster = 1;
    else
        block_timecode = pts_ms - glob->cluster_timecode;

    is_keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY);

    if (start_cluster || is_keyframe)
    {
        if (glob->cluster_open)
            Ebml_EndSubElement(glob, &glob->startCluster);

        /* Open the new cluster */
        block_timecode = 0;
        glob->cluster_open = 1;
        glob->cluster_timecode = pts_ms;
        glob->cluster_pos = ftello(glob->stream);
        Ebml_StartSubElement(glob, &glob->startCluster, Cluster); //cluster
        Ebml_SerializeUnsigned(glob, Timecode, glob->cluster_timecode);

        /* Save a cue point if this is a keyframe. */
        if (is_keyframe)
        {
            struct cue_entry *cue, *new_cue_list;

            new_cue_list = realloc(glob->cue_list,
                                   (glob->cues + 1) * sizeof(struct cue_entry));

            if (new_cue_list)
                glob->cue_list = new_cue_list;
            else
            {
                fprintf(stderr, "\nFailed to realloc cue list.\n");
                exit(EXIT_FAILURE);
            }

            cue = &glob->cue_list[glob->cues];
            cue->time = glob->cluster_timecode;
            cue->loc = glob->cluster_pos;
            glob->cues++;
        }
    }

    /* Write the Simple Block */
    Ebml_WriteID(glob, SimpleBlock);

    block_length = pkt->data.frame.sz + 4;
    block_length |= 0x10000000;
    Ebml_Serialize(glob, &block_length, sizeof(block_length), 4);

    track_number = 1;
    track_number |= 0x80;
    Ebml_Write(glob, &track_number, 1);

    Ebml_Serialize(glob, &block_timecode, sizeof(block_timecode), 2);

    flags = 0;

    if (is_keyframe)
        flags |= 0x80;

    if (pkt->data.frame.flags & VPX_FRAME_IS_INVISIBLE)
        flags |= 0x08;

    Ebml_Write(glob, &flags, 1);

    Ebml_Write(glob, pkt->data.frame.buf, pkt->data.frame.sz);
}
static void write_webm_file_footer(EbmlGlobal *glob, long hash)
{
    if (glob->cluster_open)
        Ebml_EndSubElement(glob, &glob->startCluster);

    {
        EbmlLoc start;
        int i;

        glob->cue_pos = ftello(glob->stream);
        Ebml_StartSubElement(glob, &start, Cues);

        for (i = 0; i < glob->cues; i++)
        {
            struct cue_entry *cue = &glob->cue_list[i];
            EbmlLoc start;

            Ebml_StartSubElement(glob, &start, CuePoint);
            {
                EbmlLoc start;

                Ebml_SerializeUnsigned(glob, CueTime, cue->time);

                Ebml_StartSubElement(glob, &start, CueTrackPositions);
                Ebml_SerializeUnsigned(glob, CueTrack, 1);
                Ebml_SerializeUnsigned64(glob, CueClusterPosition,
                                         cue->loc - glob->position_reference);
               //Ebml_SerializeUnsigned(glob, CueBlockNumber, cue->blockNumber);
                Ebml_EndSubElement(glob, &start);
            }
            Ebml_EndSubElement(glob, &start);
        }

        Ebml_EndSubElement(glob, &start);
    }

    Ebml_EndSubElement(glob, &glob->startSegment);

    /* Patch up the seek info block */
    write_webm_seek_info(glob);

    /* Patch up the track id */
    fseeko(glob->stream, glob->track_id_pos, SEEK_SET);
    Ebml_SerializeUnsigned32(glob, TrackUID, glob->debug ? 0xDEADBEEF : hash);

    fseeko(glob->stream, 0, SEEK_END);
}



/* Murmur hash derived from public domain reference implementation at
 *   http://sites.google.com/site/murmurhash/
 */
struct vpxio_ctx* vpxio_open_webm_dst(struct vpxio_ctx *ctx,
                        const char *file_name,
                        int width,
                        int height,
                        int framerate_den,
                        int framerate_num)
{
    ctx = vpxio_init_ctx(ctx);

    ctx->file_name = file_name;
    ctx->file = strcmp(ctx->file_name, "-") ?
                       fopen(ctx->file_name, "wb") :
                             set_binary_mode(stdout);

    if (!ctx->file)
    {
        printf("Failed to open output file: %s", ctx->file_name);
        free(ctx);
        return NULL;
    }

    ctx->mode = DST;
    ctx->fourcc = MAKEFOURCC('V', 'P', '8', '0');
    ctx->file_type = FILE_TYPE_WEBM;
    ctx->width = width;
    ctx->height = height;
    ctx->framerate.den = framerate_den;
    ctx->framerate.num = framerate_num;

    return ctx;
}
int file_is_webm(struct vpxio_ctx *ctx)
{
    unsigned int i, n;
    int          track_type = -1;
    nestegg_io io = {nestegg_read_cb, nestegg_seek_cb, nestegg_tell_cb,
                     ctx->decompression_ctx.webm_ctx.infile
                    };
    nestegg_video_params params;

    if (nestegg_init(&ctx->decompression_ctx.webm_ctx.nestegg_ctx,
                     io, NULL))
        goto fail;

    if (nestegg_track_count(ctx->decompression_ctx.webm_ctx.nestegg_ctx, &n))
        goto fail;

    for (i = 0; i < n; i++)
    {
        track_type = nestegg_track_type(
            ctx->decompression_ctx.webm_ctx.nestegg_ctx, i);

        if (track_type == NESTEGG_TRACK_VIDEO)
            break;
        else if (track_type < 0)
            goto fail;
    }

    if (nestegg_track_codec_id(
            ctx->decompression_ctx.webm_ctx.nestegg_ctx, i)
            != NESTEGG_CODEC_VP8)
    {
        fprintf(stderr, "Not VP8 video, quitting.\n");
        exit(1);
    }

    ctx->decompression_ctx.webm_ctx.video_track = i;

    if (nestegg_track_video_params(
            ctx->decompression_ctx.webm_ctx.nestegg_ctx,i, &params))
        goto fail;

    ctx->framerate.den = 0;
    ctx->framerate.num = 0;
    ctx->fourcc = VP8_FOURCC;
    ctx->width  = params.width;
    ctx->height = params.height;

    return 1;
fail:
    ctx->decompression_ctx.webm_ctx.nestegg_ctx = NULL;
    rewind(ctx->decompression_ctx.webm_ctx.infile);
    return 0;
}
static unsigned int murmur(const void *key, int len, unsigned int seed)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    unsigned int h = seed ^ len;
    const unsigned char *data = (const unsigned char *)key;

    while (len >= 4)
    {
        unsigned int k;
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }

    switch (len)
    {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= m;
    };

    h ^= h >> 13;

    h *= m;

    h ^= h >> 15;

    return h;
}
int write_webm_file_header(struct vpxio_ctx *ctx)
{
    if (fseek(ctx->file, 0, SEEK_CUR))
    {
        fprintf(stderr, "WebM output to pipes not supported.\n");
        fclose(ctx->file);
        return -1;
    }

    ctx->compression_ctx.ebml.stream = ctx->file;
    write_webm_file_header_intrn(&ctx->compression_ctx.ebml,
                                 ctx->width, ctx->height,
                                 &ctx->framerate, 0);

    return 0;
}
int vpxio_write_pkt_webm(struct vpxio_ctx *ctx,
                         const vpx_codec_cx_pkt_t *pkt)
{
    if (!ctx->compression_ctx.ebml.debug)
        ctx->compression_ctx.hash = murmur(pkt->data.frame.buf,
                                           pkt->data.frame.sz,
                                           ctx->compression_ctx.hash);

    write_webm_block(&ctx->compression_ctx.ebml,
                     ctx->timebase.num, ctx->timebase.den, pkt);

    return 0;
}
int vpxio_write_close_enc_file_webm(struct vpxio_ctx *ctx)
{
    write_webm_file_footer(&ctx->compression_ctx.ebml,
                           ctx->compression_ctx.hash);
    free(ctx->compression_ctx.ebml.cue_list);
    ctx->compression_ctx.ebml.cue_list = NULL;

    return 0;
}

int vpxio_read_pkt_webm(struct vpxio_ctx *ctx, uint8_t **buf,
                        size_t *buf_sz, size_t *buf_alloc_sz)
{
    if (ctx->decompression_ctx.webm_ctx.chunk >=
            ctx->decompression_ctx.webm_ctx.chunks)
    {
        unsigned int track;

        do
        {
            if (ctx->decompression_ctx.webm_ctx.pkt)
                nestegg_free_packet(ctx->decompression_ctx.webm_ctx.pkt);

            if (nestegg_read_packet(ctx->decompression_ctx.webm_ctx.nestegg_ctx,
                                    &ctx->decompression_ctx.webm_ctx.pkt) <= 0
                    || nestegg_packet_track(ctx->decompression_ctx.webm_ctx.pkt,
                                            &track))
                return 1;
        }
        while (track != ctx->decompression_ctx.webm_ctx.video_track);

        if (nestegg_packet_count(ctx->decompression_ctx.webm_ctx.pkt,
                                 &ctx->decompression_ctx.webm_ctx.chunks))
            return 1;

        ctx->decompression_ctx.webm_ctx.chunk = 0;
    }

    if (nestegg_packet_data(ctx->decompression_ctx.webm_ctx.pkt,
                            ctx->decompression_ctx.webm_ctx.chunk,
                            buf, buf_sz))
        return 1;

    ctx->decompression_ctx.webm_ctx.chunk++;
    return 0;
}
