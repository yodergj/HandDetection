#ifndef FLESH_DETECTOR_H
#define FLESH_DETECTOR_H

#include "BayesianClassifier.h"
#include "Image.h"
#include <string>

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
    bool GetFleshImage(Image* imagePtr, unsigned char* backgroundColor, Image** fleshImage, Image** nonfleshImage);
    bool GetFleshConfidenceImage(Image* imagePtr, Image** outputImage);
    int GetBlocks(unsigned char* ignoreColor, Image* imagePtr, std::vector<BlockType*> &blockList);
    bool GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image* fleshImagePtr, Image** outlineImage);
  private:
    bool CalcConfidence(Image* imagePtr, int xScale, int yScale);
    std::string mFeatureList;
    BayesianClassifier mClassifier;
    Image mConfidenceImage;
    Image mFleshImage;
    Image mNonFleshImage;
    Image mOutlineImage;
    double* mConfidenceBuffer;
    int mConfidenceBufferAlloc;
    int mConfidenceBufferWidth;
    int mConfidenceBufferHeight;
};

#endif
