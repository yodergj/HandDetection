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
    int Classifiy(double value);
    bool Train(vector<double>& samples, vector<double>& weights, vector<int>& classes, double* trainingError = 0);
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr classifierNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr classifierNode);
  private:
    double mThreshold;
    int mLowerClass;
    int mUpperClass;
};

#endif
