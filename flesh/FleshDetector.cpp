#include "FleshDetector.h"
#include <stdio.h>

#define MAX_FEATURES 512

FleshDetector::FleshDetector()
{
}

FleshDetector::~FleshDetector()
{
}

bool FleshDetector::Load(const char* filename)
{
  FILE* filePtr;
  bool retCode;
  char buffer[MAX_FEATURES];

  if ( !filename || !*filename )
    return false;

  filePtr = fopen(filename, "r");
  if ( !filePtr )
    return false;

  retCode = fgets(buffer, MAX_FEATURES, filePtr);

  if ( retCode )
    retCode = mClassifier.Load(filePtr);
  fclose(filePtr);

  return retCode;
}

bool FleshDetector::Process(Image* imagePtr)
{
  int i;
  int x, y;
  double* featureBuffer;
  double* pixel;
  int numFeatures, classIndex, width, height;
  double confidence;
  Matrix input;

  if ( !imagePtr )
    return false;

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();
  numFeatures = mFeatureList.size();
  input.SetSize(numFeatures, 1);
  featureBuffer = imagePtr->GetCustomBuffer(mFeatureList);
  pixel = featureBuffer;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      for (i = 0; i < numFeatures; i++)
        input.SetValue(i, 0, pixel[i]);

      mClassifier.Classify(input, classIndex, confidence);
      if ( classIndex == 0 )
      {
        // Pixel is flesh colored
//        fleshImage->setPixel(u, v, qRgb(r, g, b));
      }
      else
      {
        // Pixel is not flesh colored
//        fleshImage->setPixel(u, v, background.rgb());
      }

      pixel += numFeatures;
    }
  }

  return true;
}
