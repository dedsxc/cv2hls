#include "cvToHls.hpp"

using namespace cv;

CvToHls::CvToHls(const String id, const String uri, const int fps)
{
    this->id = id;
    this->uri = uri;
    this->fps = fps;
    timeout = 3000;

    //  ffmpeg
    filename = "hls/stream.m3u8";
    video_st = {0};
}


void CvToHls::process(void)
{
    int ret;
    VideoCapture video;
    Mat blob;

    // capture Size of frame
    video.open(uri);
    video.read(frame);
    CvToHls::getSize(frame);
    
    // Initialize ffmpeg
    ret = CvToHls::setContextFFmpeg(filename);
    if (ret < 0){
        std::cout << "[-] Error during init context ffmpeg" << std::endl;
        abort();
    }

    // Run
    while (video.read(frame))
    {
        // Display frame
        imshow(uri, frame);

        // output to hls
        ret = CvToHls::encode(frame);
        if (ret < 0){
            std::cout << "[-] Error during encoding" << std::endl;
            abort();
        }
        ret = CvToHls::muxing();
        if (ret < 0){
            std::cout << "[-] Error during muxing" << std::endl;
            abort();
        }
    }
    video.release();
    CvToHls::release();
}

void CvToHls::getSize(const Mat &frame)
{
    frameWidth  = frame.cols;
    frameHeight = frame.rows;
}

int CvToHls::setContextFFmpeg(const char* filename)
{
    int ret;
    AVCodecContext *c;
    AVRational opencvFps = {fps, 1};
    AVDictionary *opt = NULL;

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, nullptr, nullptr, filename);
    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. 
    * i.E Add stream
    */
    const AVCodec *vcodec = avcodec_find_encoder(oc->oformat->video_codec);
    video_st.st = avformat_new_stream(oc, vcodec);
    video_st.st->id = oc->nb_streams - 1;
    c = avcodec_alloc_context3(vcodec);
    video_st.enc = c;
    c->width = frameWidth;
    c->height = frameHeight;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    video_st.st->time_base = av_inv_q(opencvFps);
    c->time_base = video_st.st->time_base;

    /* Now that all the parameters are set, we can open the audio and
            * video codecs and allocate the necessary encode buffers. */
    avcodec_open2(video_st.enc, vcodec, nullptr);
    avcodec_parameters_from_context(video_st.st->codecpar, c);
    /* open the output file, if needed */
    avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);

    // initialize scaler context
    video_st.sws_ctx = sws_getCachedContext(
        nullptr,
        frameWidth, frameHeight, AV_PIX_FMT_BGR24,      // Input source
        frameWidth, frameHeight, video_st.enc->pix_fmt, // Output
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!video_st.sws_ctx)
        return -1;

    video_st.frame = av_frame_alloc();
    video_st.frame->width = frameWidth;
    video_st.frame->height = frameHeight;
    video_st.frame->format = c->pix_fmt;
    ret = av_frame_get_buffer(video_st.frame, 0);

    av_dump_format(oc, 0, filename, 1);
    
    /* Write the stream header, if any. */
    av_dict_set_int(&opt, "hls_list_size", 4, 0);
    av_dict_set(&opt, "hls_flags", "delete_segments", 0);
    ret = avformat_write_header(oc, &opt);
    if (ret < 0)
        return -1;
    
    return 0;
}

int CvToHls::encode(const Mat &frame)
{
    if (av_frame_make_writable(video_st.frame) < 0)
        return -1;

    // Convert cv::Mat(OpenCV) to AVFrame(FFmpeg)
    const int stride[] = {static_cast<int>(frame.step[0])};
    int ret;
    ret = sws_scale(
        video_st.sws_ctx,
        &frame.data, stride, 0, frame.rows,
        video_st.frame->data, video_st.frame->linesize);
    if (ret < 0)
        return -1;
    video_st.frame->pts = video_st.next_pts++;
    
    return 0;
}

int CvToHls::muxing(void)
{
    int ret;
    ret = write_frame(oc, &video_st);
    if (ret < 0){
        return -1;
    }
    return 0;
}

void CvToHls::release()
{
    close_stream(oc, &video_st);
    /* free the stream */
    avformat_free_context(oc);
}
