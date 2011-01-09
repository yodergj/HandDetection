#ifndef THRESHOLD_CLASSIFIER_H
#define THRESHOLD_CLASSIFIER_H

#include "WeakClassifier.h"

#define THRESHOLD_CLASSIFIER_STR "ThresholdClassifier"

class ThresholdClassifier : public WeakClassifier
{
  public:
    ThresholdClassifier();
    ThresholdClassifier(const ThresholdClassifier& ref);
    virtual ~ThresholdClassifier();
    ThresholdClassifier& operator=(const ThresholdClassifier& ref);
    virtual int Classify(double value);
    virtual bool Train(const vector<double>& samples, const vector<double>& weights, const vector<int>& classes, double* trainingError = 0);
    virtual bool Print(FILE* file);
    virtual xmlNodePtr Save(xmlDocPtr document);
  protected:
    virtual bool LoadClassifier(xmlNodePtr classifierNode);
  private:
    double mThreshold;
    int mLowerClass;
    int mUpperClass;
};

#endif
