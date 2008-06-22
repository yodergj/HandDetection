PROJECT(HandDetection)
#ADD_LIBRARY(gmm SHARED Matrix.cpp Gaussian.cpp GaussianMixtureModel.cpp BayesianClassifier.cpp)
ADD_LIBRARY(gmm STATIC Matrix.cpp Gaussian.cpp GaussianMixtureModel.cpp BayesianClassifier.cpp)
