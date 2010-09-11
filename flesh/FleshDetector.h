#ifndef FLESH_DETECTOR_H
#define FLESH_DETECTOR_H

#include "BayesianClassifier.h"
#include "Image.h"
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

class ConfidenceRevision
{
  public:
    int imageRevision;
    int xScale;
    int yScale;
    bool operator==(const ConfidenceRevision& ref) {return (imageRevision == ref.imageRevision) && (xScale = ref.xScale) && (yScale = ref.yScale);};
};

class FleshDetector
{
  public:
    FleshDetector();
    ~FleshDetector();
    virtual bool Load(const char* filename);
    bool Process(Image* imagePtr, Image** outlineImageOut, Image** fleshImageOut, Image** confidenceImageOut);
    vector<ConnectedRegion*>* GetFleshRegions(Image* imagePtr, int &xScale, int &yScale);
  protected:
    bool GetFleshImage(Image* imagePtr, unsigned char* backgroundColor, Image** fleshImage);
    bool GetFleshConfidenceImage(Image* imagePtr, Image** outputImage);
    bool GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image** outlineImage);
    virtual bool CalcConfidence(Image* imagePtr, int xScale, int yScale);
    string mFeatureList;
    BayesianClassifier mClassifier;
    Image mConfidenceImage;
    Image mFleshImage;
    Image mOutlineImage;
    map<Image*,ConfidenceRevision> mConfidenceRevisions;
};

#endif
