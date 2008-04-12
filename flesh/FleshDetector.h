#ifndef FLESH_DETECTOR_H
#define FLESH_DETECTOR_H

#include "BayesianClassifier.h"
#include "Image.h"
#include <string>

class FleshDetector
{
  public:
    FleshDetector();
    ~FleshDetector();
    bool Load(const char* filename);
    bool Process(Image* imagePtr);
  private:
    std::string mFeatureList;
    BayesianClassifier mClassifier;
};

#endif
