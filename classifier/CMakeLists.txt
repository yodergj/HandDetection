PROJECT(HandDetection)
INCLUDE_DIRECTORIES(/usr/include/libxml2)
#ADD_LIBRARY(gmm SHARED Matrix.cpp Gaussian.cpp GaussianMixtureModel.cpp BayesianClassifier.cpp ThresholdClassifier.cpp AdaboostClassifier.cpp)
ADD_LIBRARY(gmm STATIC Matrix.cpp Gaussian.cpp GaussianMixtureModel.cpp BayesianClassifier.cpp ThresholdClassifier.cpp RangeClassifier.cpp WeakClassifier.cpp AdaboostClassifier.cpp CompositeClassifier.cpp)
