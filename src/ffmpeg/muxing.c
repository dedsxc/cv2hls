#include "muxing.h"

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
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