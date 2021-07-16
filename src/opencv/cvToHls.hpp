#ifndef CVTOHLS_HPP_INCLUDED
#define CVTOHLS_HPP_INCLUDED

#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

extern "C"
{
#include "../ffmpeg/muxing.h"
}

class CvToHls
{
private:
    cv::String id;
    cv::String uri;
    int timeout;
    int frameWidth;
    int frameHeight;
    cv::Mat frame;

    // ffmpeg
    OutputStream video_st;
    const char *filename;
    const AVOutputFormat *fmt;
    AVFormatContext *oc;
    const char *video_codec;
    int fps;

public:
    CvToHls(const cv::String id, const cv::String uri, const int fps, const char* video_codec);
    void process(void);
    void getSize(const cv::Mat &frame);
    void setContextFFmpeg(const char *filename, const char *video_codec);
    int encode(const cv::Mat &frame);
    int muxing(void);
    void release(void);
};

#endif