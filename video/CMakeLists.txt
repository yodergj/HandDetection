PROJECT(HandDetection)
INCLUDE_DIRECTORIES(/usr/include/avifile-0.7)
ADD_LIBRARY(video STATIC VideoDecoder.cpp VideoEncoder.cpp)
