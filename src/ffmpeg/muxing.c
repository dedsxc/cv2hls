#include "muxing.h"

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

AVFormatContext *set_context(const int fps, const char *filename, const int frameWidth, const int frameHeight, const char* videoCodec, OutputStream *ost, AVFormatContext *oc, const AVOutputFormat *fmt)
{
    int ret;
    AVCodecContext *c;
    AVRational opencvFps = {fps, 1};
    AVDictionary *opt = NULL;

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
    * and initialize the codecs. 
    * Add stream
    */
    const AVCodec *vcodec = avcodec_find_encoder_by_name(videoCodec);
    ost->st = avformat_new_stream(oc, vcodec);
    ost->st->id = oc->nb_streams - 1;
    c = avcodec_alloc_context3(vcodec);
    ost->enc = c;
    c->width = frameWidth;
    c->height = frameHeight;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    ost->st->time_base = av_inv_q(opencvFps);
    c->time_base = ost->st->time_base;
    
    /* Now that all the parameters are set, we can open the audio and
            * video codecs and allocate the necessary encode buffers. */
    avcodec_open2(ost->enc, vcodec, NULL);
    avcodec_parameters_from_context(ost->st->codecpar, c);
    /* open the output file, if needed */
    avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);

    // initialize scaler context
    ost->sws_ctx = sws_getCachedContext(
        NULL,
        frameWidth, frameHeight, AV_PIX_FMT_BGR24,  // Input source
        frameWidth, frameHeight, ost->enc->pix_fmt, // Output
        SWS_BICUBIC, NULL, NULL, NULL);

    ost->frame = av_frame_alloc();
    ost->frame->width = frameWidth;
    ost->frame->height = frameHeight;
    ost->frame->format = c->pix_fmt;
    ret = av_frame_get_buffer(ost->frame, 0);

    av_dump_format(oc, 0, filename, 1);

    /* Write the stream header, if any. */
    av_dict_set_int(&opt, "hls_list_size", 4, 0);
    av_dict_set(&opt, "hls_flags", "delete_segments", 0);
    ret = avformat_write_header(oc, &opt);

    return oc;
}

int write_frame(AVFormatContext *oc, OutputStream *ost){
    int ret;
    ret = avcodec_send_frame(ost->enc, ost->frame);
    if (ret < 0)
        return -1;
    while (ret >= 0)
    {
        AVPacket pkt = {0};
        ret = avcodec_receive_packet(ost->enc, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
            return -1;

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, ost->enc->time_base, ost->st->time_base);
        pkt.stream_index = ost->st->index;

        /* Write the compressed frame to the media file. */
        // log_packet(avFormatCtx, &pkt);
        ret = av_interleaved_write_frame(oc, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0)
            return -1;
    }
    return 0; 
}
