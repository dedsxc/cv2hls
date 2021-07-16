#include <unistd.h>

#include "opencv/cvToHls.hpp"

const cv::String keys =
    "{help h         |      | Usage: ./cv2hls --target=rtsp://user:passwd@ip/path                               }"
    "{target t       |<none>| uri or path input video                                                           }"
    "{fps f          |10    | frame per second                                                                  }"
    "{codec c        |libx264| video codec ffmpeg                                                               }";
;

int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    const cv::String target = parser.get<cv::String>("target");
    const int fps = parser.get<int>("fps");
    const cv::String vcodec = parser.get<cv::String>("codec");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    if (target.size() > 0)
    {
        CvToHls c(target, target, fps, vcodec.c_str());
        c.process();
    }
    else
    {
        parser.printErrors();
    }
    return 0;
}
