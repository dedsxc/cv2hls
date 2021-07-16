#ifndef MUXING_H_INCLUDED
#define MUXING_H_INCLUDED

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>


// a wrapper around a single output AVStream
typedef struct OutputStream
{
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

void close_stream(AVFormatContext *oc, OutputStream *ost);
AVFormatContext *set_context(const int fps, const char *filename, const int frameWidth, const int frameHeight, const char* videoCodec, OutputStream *ost, AVFormatContext *ctx, const AVOutputFormat *fmt);
int write_frame(AVFormatContext *oc, OutputStream *ost);

#endif
