#ifndef FLESH_DETECTOR_H
#define FLESH_DETECTOR_H

#include "BayesianClassifier.h"
#include "Image.h"
#include <string>
using std::string;

typedef struct
{
  int left, right, top, bottom;
} BlockType;

class FleshDetector
{
  public:
    FleshDetector();
    ~FleshDetector();
    bool Load(const char* filename);
    bool Process(Image* imagePtr, Image** outlineImageOut, Image** fleshImageOut, Image** confidenceImageOut);
  private:
    bool GetFleshImage(Image* imagePtr, unsigned char* backgroundColor, Image** fleshImage);
    bool GetFleshConfidenceImage(Image* imagePtr, Image** outputImage);
    bool GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image** outlineImage);
    bool CalcConfidence(Image* imagePtr, int xScale, int yScale);
    string mFeatureList;
    BayesianClassifier mClassifier;
    Image mConfidenceImage;
    Image mFleshImage;
    Image mOutlineImage;
};

#endif
