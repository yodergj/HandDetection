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
    string GetFeatureString() const;
    bool SetFeatureString(const string& featureString);
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr classifierNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr classifierNode);
  private:
    void Clear();
    bool TrainLevel(const vector<double>& weights, int levelNum, int& chosenFeature);
    bool FillData(int featureNum, vector<double>& data, vector<int>& truth);
    int mNumDimensions;
    string mFeatureString;
    int mNumClasses;
    vector< vector<Matrix> > mTrainingData;
    vector< vector<int> > mTrainingDataFreq;
    vector<WeakClassifier*> mWeakClassifiers;
    vector<double> mClassifierWeights;
    vector<int> mClassifierFeatures;
};

#endif
