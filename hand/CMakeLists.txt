PROJECT(HandDetection)
INCLUDE_DIRECTORIES(/usr/include/libxml2)  
#ADD_LIBRARY(hand SHARED HandDetector.cpp Hand.cpp)
ADD_LIBRARY(hand STATIC HandDetector.cpp Hand.cpp)
