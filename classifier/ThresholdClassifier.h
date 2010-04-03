#ifndef THRESHOLD_CLASSIFIER_H
#define THRESHOLD_CLASSIFIER_H

#include <vector>
using std::vector;
#include "XMLUtils.h"

#define THRESHOLD_CLASSIFIER_STR "ThresholdClassifier"

class ThresholdClassifier
{
  public:
    ThresholdClassifier();
    ~ThresholdClassifier();
    int Classify(double value);
    bool Train(const vector<double>& samples, const vector<double>& weights, const vector<int>& classes, double* trainingError = 0);
    string GetFeatureString() const;
    bool SetFeatureString(const string& featureString);
    bool SetFeatureString(const char featureLetter);
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr classifierNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr classifierNode);
  private:
    double mThreshold;
    int mLowerClass;
    int mUpperClass;
    string mFeatureString;
};

#endif
