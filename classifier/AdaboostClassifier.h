#ifndef ADABOOST_CLASSIFIER_H
#define ADABOOST_CLASSIFIER_H

#include <string>
#include <vector>
using std::string;
using std::vector;
#include "ThresholdClassifier.h"
#include "Matrix.h"
#include "XMLUtils.h"

#define ADABOOST_CLASSIFIER_STR "AdaboostClassifier"

class AdaboostClassifier
{
  public:
    AdaboostClassifier();
    ~AdaboostClassifier();
    bool Create(int numDimensions, int numClasses, int numWeakClassifiers);
    bool AddTrainingData(const Matrix& data, int classIndex);
    int Classify(const Matrix& data);
    bool Train();
    string GetFeatureString();
    bool SetFeatureString(const string& featureString);
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr classifierNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr classifierNode);
  private:
    void Clear();
    int mNumDimensions;
    string mFeatureString;
    int mNumClasses;
    vector< vector<Matrix> > mTrainingData;
    vector< vector<int> > mTrainingDataFreq;
    vector<ThresholdClassifier> mWeakClassifiers;
    vector<double> mClassifierWeights;
};

#endif
