PROJECT(HandDetection)
INCLUDE_DIRECTORIES(${QT_INCLUDES})
#ADD_LIBRARY(image SHARED Image.cpp)
ADD_LIBRARY(image STATIC Image.cpp ConnectedRegion.cpp SubImage.cpp)