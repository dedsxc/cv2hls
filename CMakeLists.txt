cmake_minimum_required(VERSION 3.0)
project( cv2hls )

SET(name cv2hls)

add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/opencv)
get_directory_property(_vars VARIABLES)

list(APPEND SOURCES
    src/main.cpp
    src/opencv/cvToHls.cpp
    src/ffmpeg/muxing.c
)

add_executable( ${name} ${SOURCES})

target_link_libraries( ${name} FFmpeg OpenCV)