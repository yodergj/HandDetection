PROJECT(HandDetection)
INCLUDE_DIRECTORIES(/usr/include/avifile-0.7)
ADD_EXECUTABLE(imgtoavi imgtoavi.cpp)
TARGET_LINK_LIBRARIES(imgtoavi image geometry ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} aviplay)
