cmake_minimum_required(VERSION 3.0)
project( OpenCV )

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS})

add_library(OpenCV INTERFACE IMPORTED GLOBAL)

target_link_libraries(OpenCV INTERFACE
    ${OpenCV_LIBS}
)