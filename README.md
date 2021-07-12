# cv2hls

Example to convert cv::Mat to HLS using ffmpeg lib

## Install

https://github.com/dedsxc/cv2hls

```sh
mkdir build
cd build
cmake ..
make
```

## Usage

```sh
./cv2hls --target=rtsp://user:pass@ip --fps=15
```
The HLS output will be generated in hls/stream.m3u8

- To view the result in a webpage :

```sh
(hls) $ python3 -m http.server
```

- Open chrome in http://localhost:8000
