PROJECT(HandDetection)
INCLUDE_DIRECTORIES(/usr/include/libxml2)
#ADD_LIBRARY(flesh SHARED FleshDetector.cpp)
ADD_LIBRARY(flesh STATIC FleshDetector.cpp)
