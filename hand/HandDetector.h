#ifndef HAND_DETECTOR_H
#define HAND_DETECTOR_H

#include <string>
#include <vector>
using std::string;
using std::vector;
#include <opencv/ml.h>
#include "BayesianClassifier.h"
#include "Image.h"
#include "Hand.h"

class HandDetector
{
  public:
    HandDetector();
    ~HandDetector();
    bool Load(const char* filename);
    bool Save(const char* filename);
    bool Process(Image* imagePtr, int left, int right, int top, int bottom,
                 vector<Hand*> &results);
    bool Create(string featureList, int xResolution, int yResolution, int numHandGaussians, int numNonHandGaussians);
    bool Create(string haarFilename);
    bool AddTrainingSample(Image* imagePtr, int left, int right, int top, int bottom, bool isHand);
    bool Train();
  private:
    bool FillFeatureVector(Image* imagePtr, int left, int right, int top, int bottom,
                           Matrix& featureVector);

    string mFeatureList;
    BayesianClassifier mClassifier;
    int mXResolution;
    int mYResolution;

    string mHaarFilename;
    CvHaarClassifierCascade* mHaarClassifier;
    CvMemStorage* mStorage;
};

#endif
