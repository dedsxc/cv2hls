#include "cvToHls.hpp"

using namespace cv;

CvToHls::CvToHls(const String id, const String uri, const int fps, const char *video_codec)
{
    this->id = id;
    this->uri = uri;
    this->fps = fps;
    this->video_codec = video_codec;

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
    CvToHls::setContextFFmpeg(filename, video_codec);

    // Run
    while (video.read(frame))
    {
        // Display frame
        imshow(uri, frame);

        // output to hls
        ret = CvToHls::encode(frame);
        if (ret < 0)
        {
            std::cout << "[-] Error during encoding" << std::endl;
            abort();
        }
        ret = CvToHls::muxing();
        if (ret < 0)
        {
            std::cout << "[-] Error during muxing" << std::endl;
            abort();
        }
    }
    video.release();
    CvToHls::release();
}

void CvToHls::getSize(const Mat &frame)
{
    frameWidth = frame.cols;
    frameHeight = frame.rows;
}

void CvToHls::setContextFFmpeg(const char *filename, const char *video_codec)
{
    oc = set_context(fps, filename, frameWidth, frameHeight, video_codec, &video_st, oc, fmt);

    std::cout << "-----------------------"
              << "\n"
              << "target  : " << uri << "\n"
              << "outfile : " << filename << "\n"
              << "format  : " << oc->oformat->name << "\n"
              << "vcodec  : " << avcodec_get_name(oc->oformat->video_codec) << "\n"
              << "size    : " << frameWidth << 'x' << frameHeight << "\n"
              << "fps     : " << fps << "\n"
              << "-----------------------"
              << "\n"
              << std::flush;
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
    if (ret < 0)
    {
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