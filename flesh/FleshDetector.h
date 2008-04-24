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
    bool Process(Image* imagePtr, Image** outlineImageOut, Image** fleshImageOut);
    bool GetFleshImage(Image* imagePtr, unsigned char* backgroundColor, Image** fleshImage, Image** nonfleshImage);
    int GetBlocks(unsigned char* ignoreColor, Image* imagePtr, std::vector<BlockType*> &blockList);
    bool GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image* fleshImagePtr, Image** outlineImage);
  private:
    std::string mFeatureList;
    BayesianClassifier mClassifier;
    Image mFleshImage;
    Image mNonFleshImage;
    Image mOutlineImage;
};

#endif
