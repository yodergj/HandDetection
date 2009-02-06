#ifndef BAYESIAN_CLASSIFIER_H
#define BAYESIAN_CLASSIFIER_H

#include "GaussianMixtureModel.h"
#include "XMLUtils.h"
#include <string>
using std::string;

#define BAYESIAN_CLASSIFIER_STR "BayesianClassifier"

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
    string GetFeatureString();
    bool SetFeatureString(const string& featureString);
    bool Save(const char* filename);
    bool Save(xmlNodePtr classifierNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr classifierNode);
  private:
    int mNumDimensions;
    string mFeatureString;
    int mNumClasses;
    GaussianMixtureModel* mModels;
    int* mClassCounts;
    double* mClassWeights;
};

#endif
