#ifndef WEAK_CLASSIFIER_H
#define WEAK_CLASSIFIER_H

#include <vector>
using std::vector;
#include "XMLUtils.h"

#define WEAK_CLASSIFIER_STR "WeakClassifier"
#define CLASSIFIER_TYPE_STR "ClassifierType"

class WeakClassifier
{
  public:
    WeakClassifier();
    WeakClassifier(const WeakClassifier& ref);
    virtual ~WeakClassifier();
    WeakClassifier& operator=(const WeakClassifier& ref);
    string GetFeatureString() const;
    bool SetFeatureString(const string& featureString);
    bool SetFeatureString(const char featureLetter);

    virtual int Classify(double value);
    virtual bool Train(const vector<double>& samples, const vector<double>& weights, const vector<int>& classes, double* trainingError = 0);
    virtual bool Print(FILE* file);
    virtual bool Save(const char* filename);
    virtual xmlNodePtr Save(xmlDocPtr document);
    static WeakClassifier* Load(const char* filename);
    static WeakClassifier* Load(xmlNodePtr classifierNode);
  protected:
    virtual bool LoadClassifier(xmlNodePtr classifierNode);

    string mFeatureString;
};

#endif