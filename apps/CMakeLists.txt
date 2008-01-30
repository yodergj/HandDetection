PROJECT(GMM)
FIND_LIBRARY(QT_LIBRARY
             NAMES qt-mt
             PATHS /usr/qt/3/lib
            )
INCLUDE_DIRECTORIES(/usr/qt/3/include)
ADD_EXECUTABLE(trainer trainer.cpp)
ADD_EXECUTABLE(extractor extractor.cpp)
ADD_EXECUTABLE(dataAnalyzer dataAnalyzer.cpp)
TARGET_LINK_LIBRARIES(trainer gmm qt-mt)
TARGET_LINK_LIBRARIES(extractor gmm qt-mt)
TARGET_LINK_LIBRARIES(dataAnalyzer gmm qt-mt)
