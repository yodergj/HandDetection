#ifndef RANGE_CLASSIFIER_H
#define RANGE_CLASSIFIER_H

#include "WeakClassifier.h"

#define RANGE_CLASSIFIER_STR "RangeClassifier"

class RangeClassifier : public WeakClassifier
{
  public:
    RangeClassifier();
    RangeClassifier(const RangeClassifier& ref);
    virtual ~RangeClassifier();
    RangeClassifier& operator=(const RangeClassifier& ref);
    virtual int Classify(double value);
    virtual bool Train(const vector<double>& samples, const vector<double>& weights, const vector<int>& classes, double* trainingError = 0);
    virtual bool Print(FILE* file);
    virtual xercesc::DOMElement* Save(xercesc::DOMDocument* document, bool toRootElem);
  protected:
    virtual bool LoadClassifier(xercesc::DOMElement* classifierNode);
  private:
    double mLowerThreshold;
    double mUpperThreshold;
    int mInnerClass;
    int mOuterClass;
};

#endif
