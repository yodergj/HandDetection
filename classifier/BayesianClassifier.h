#ifndef BAYESIAN_CLASSIFIER_H
#define BAYESIAN_CLASSIFIER_H

#include "GaussianMixtureModel.h"

class BayesianClassifier
{
  public:
    BayesianClassifier();
    ~BayesianClassifier();
    bool Create(int numDimensions, int numClasses, int* classComponents = NULL);
    bool AddTrainingData(Matrix& data, int classIndex);
    int* Get2dDataHistogram(int classIndex, int binsPerSide, double scaleFactor);
    bool Classify(Matrix& data, int& classIndex, double& confidence);
    bool Train();
    bool Save(FILE* file);
    bool Load(FILE* file);
  private:
    int mNumDimensions;
    int mNumClasses;
    GaussianMixtureModel* mModels;
    int* mClassCounts;
    double* mClassWeights;
};

#endif
