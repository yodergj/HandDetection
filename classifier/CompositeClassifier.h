#ifndef COMPOSITE_CLASSIFIER_H
#define COMPOSITE_CLASSIFIER_H

#include <string>
#include <vector>
using std::string;
using std::vector;
#include "XMLUtils2.h"
class Matrix;
class AdaboostClassifier;

#define COMPOSITE_CLASSIFIER_STR "CompositeClassifier"

class CompositeClassifier
{
  public:
    CompositeClassifier();
    ~CompositeClassifier();
    int Classify(const Matrix& data);
    bool AddClassifier(AdaboostClassifier* classifier, const string& className);
    string GetFeatureString() const;
    int GetNumClasses() const;
    string GetClassName(int i) const;
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xercesc::DOMDocument* doc, xercesc::DOMElement* classifierNode);
    bool Load(const char* filename);
    bool Load(xercesc::DOMElement* classifierNode);
  private:
    void Clear();
    string mFeatureString;
    vector<AdaboostClassifier*> mClassifiers;
    vector<string> mClassNames;
};

#endif