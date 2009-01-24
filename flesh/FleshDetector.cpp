#include "FleshDetector.h"
#include "TimingAnalyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FEATURES 512
#define PROXIMITY_THRESH  30
#define DEBUG_BLOCKS 0

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

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
  int numFeatures;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "FleshDetector::Load - Invalid filename\n");
    return false;
  }

  filePtr = fopen(filename, "r");
  if ( !filePtr )
  {
    fprintf(stderr, "FleshDetector::Load - Unable to open %s\n", filename);
    return false;
  }

  retCode = fgets(buffer, MAX_FEATURES, filePtr);

  if ( retCode )
  {
    numFeatures = strlen(buffer);
    if ( buffer[numFeatures - 1] == '\n' )
      buffer[numFeatures - 1] = 0;

    mFeatureList = buffer;

    retCode = mClassifier.Load(filePtr);
  }
  fclose(filePtr);

  return retCode;
}

bool FleshDetector::Process(Image* imagePtr, Image** outlineImageOut, Image** fleshImageOut, Image** confidenceImageOut)
{
  Image* fleshImage;
  Image* confidenceImage;
  unsigned char backgroundColor[] = {255, 255, 255};
  unsigned char outlineColor[] = {0, 0, 0};

  if ( !imagePtr || (!outlineImageOut && !fleshImageOut && !confidenceImageOut) )
  {
    fprintf(stderr, "FleshDetector::Process - Invalid parameter\n");
    return false;
  }

  TimingAnalyzer_Start(4);
  if ( !CalcConfidence(imagePtr, 8, 8) )
  {
    fprintf(stderr, "FleshDetector::Process - CalcConfidence failed\n");
    return false;
  }
  TimingAnalyzer_Stop(4);

  TimingAnalyzer_Start(1);
  if ( !GetFleshImage(imagePtr, backgroundColor, &fleshImage) )
  {
    fprintf(stderr, "FleshDetector::Process - GetFleshImage failed\n");
    return false;
  }
  TimingAnalyzer_Stop(1);

  if ( fleshImageOut )
    *fleshImageOut = fleshImage;

  if ( outlineImageOut )
  {
    TimingAnalyzer_Start(2);
    if ( !GetOutlineImage(backgroundColor, outlineColor, imagePtr, outlineImageOut) )
    {
      fprintf(stderr, "FleshDetector::Process - GetOutlineImage failed\n");
      return false;
    }
    TimingAnalyzer_Stop(2);
  }

  if ( confidenceImageOut )
  {
    TimingAnalyzer_Start(3);
    if ( !GetFleshConfidenceImage(imagePtr, &confidenceImage) )
    {
      fprintf(stderr, "FleshDetector::Process - GetFleshConfidenceImage failed\n");
      return false;
    }
    TimingAnalyzer_Stop(3);
    *confidenceImageOut = confidenceImage;
  }

  return true;
}

vector<ConnectedRegion*>* FleshDetector::GetFleshRegions(Image* imagePtr, int &xScale, int &yScale)
{
  double* confidenceBuffer;
  int bufferAlloc, bufferWidth, bufferHeight;

  if ( !imagePtr )
  {
    fprintf(stderr, "FleshDetector::GetFleshRegions - Invalid parameter\n");
    return false;
  }

  TimingAnalyzer_Start(4);
  if ( !CalcConfidence(imagePtr, 8, 8) )
  {
    fprintf(stderr, "FleshDetector::GetFleshRegions - CalcConfidence failed\n");
    return false;
  }
  TimingAnalyzer_Stop(4);

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer )
  {
    fprintf(stderr, "FleshDetector::GetFleshRegions - Failed getting confidence buffer\n");
    return false;
  }

  xScale = imagePtr->GetWidth() / bufferWidth;
  yScale = imagePtr->GetHeight() / bufferHeight;

  return imagePtr->GetRegionsFromConfidenceBuffer();
}

bool FleshDetector::GetFleshImage(Image* imagePtr, unsigned char* backgroundColor, Image** fleshImage)
{
  int x, y;
  int xScale, yScale, width, height;
  unsigned char* srcPixel;
  unsigned char* fleshDestPixel;
  int yOffset;
  double* confidenceBuffer;
  int bufferAlloc, bufferWidth, bufferHeight;

  if ( !imagePtr || !fleshImage )
  {
    fprintf(stderr, "FleshDetector::GetFleshImage - Invalid parameter\n");
    return false;
  }

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer )
  {
    fprintf(stderr, "FleshDetector::GetFleshImage - Failed getting confidence buffer\n");
    return false;
  }

  xScale = width / bufferWidth;
  yScale = height / bufferHeight;

  srcPixel = imagePtr->GetRGBBuffer();

  if ( !mFleshImage.Create(width, height) )
  {
    fprintf(stderr, "FleshDetector::GetFleshImage - Failed creating flesh image\n");
    return false;
  }

  fleshDestPixel = mFleshImage.GetRGBBuffer();

  for (y = 0; y < height; y++)
  {
    yOffset = (y / yScale) * bufferWidth;
    for (x = 0; x < width; x++, srcPixel += 3, fleshDestPixel += 3)
    {
      if ( confidenceBuffer[yOffset + x / xScale] >= .50 )
      {
        // Pixel is flesh colored
        fleshDestPixel[0] = srcPixel[0];
        fleshDestPixel[1] = srcPixel[1];
        fleshDestPixel[2] = srcPixel[2];
      }
      else
      {
        // Pixel is not flesh colored
        fleshDestPixel[0] = backgroundColor[0];
        fleshDestPixel[1] = backgroundColor[1];
        fleshDestPixel[2] = backgroundColor[2];
      }
    }
  }

  if ( fleshImage )
    *fleshImage = &mFleshImage;

  return true;
}

bool FleshDetector::GetFleshConfidenceImage(Image* imagePtr, Image** outputImage)
{
  int x, y;
  int width, height, xScale, yScale;
  double confidence;
  unsigned char* destPixel;
  int yOffset;
  double* confidenceBuffer;
  int bufferAlloc, bufferWidth, bufferHeight;

  if ( !imagePtr || !outputImage )
  {
    fprintf(stderr, "FleshDetector::GetFleshConfidenceImage - Invalid parameter\n");
    return false;
  }

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer )
  {
    fprintf(stderr, "FleshDetector::GetFleshConfidenceImage - Failed getting confidence buffer\n");
    return false;
  }

  xScale = width / bufferWidth;
  yScale = height / bufferHeight;

  if ( !mConfidenceImage.Create(width, height) )
  {
    fprintf(stderr, "FleshDetector::GetFleshConfidenceImage - Failed creating confidence image\n");
    return false;
  }

  destPixel = mConfidenceImage.GetRGBBuffer();

  for (y = 0; y < height; y++)
  {
    yOffset = (y / yScale) * bufferWidth;
    for (x = 0; x < width; x++, destPixel += 3)
    {
      confidence = confidenceBuffer[yOffset + x / xScale];

      if ( confidence >= .50 )
      {
        destPixel[0] = 0;
        destPixel[1] = (int)(255 * (confidence - .5) / .5);
      }
      else
      {
        destPixel[0] = (int)(255 * (confidence - .5) / .5);
        destPixel[1] = 0;
      }
      destPixel[2] = 0;
    }
  }

  if ( outputImage )
    *outputImage = &mConfidenceImage;

  return true;
}

bool FleshDetector::GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image** outlineImage)
{
  int i;
  int numRegions, width, height;
  int left, right, top, bottom;
  double* confidenceBuffer;
  int bufferWidth, bufferHeight, bufferAlloc, xScale, yScale;
  vector<ConnectedRegion*>* regionList;

  if ( !backgroundColor || !outlineColor || !imagePtr || !outlineImage )
  {
    fprintf(stderr, "FleshDetector::GetOutlineImage - Invalid parameter\n");
    return false;
  }

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();
  if ( !mOutlineImage.CopyRGBBuffer(width, height, imagePtr->GetRGBBuffer(), 3 * width) )
  {
    fprintf(stderr, "FleshDetector::GetOutlineImage - Failed copying image\n");
    return false;
  }

  if ( !imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc) )
  {
    fprintf(stderr, "FleshDetector::GetOutlineImage - Failed getting confidence buffer\n");
    return false;
  }

  xScale = width / bufferWidth;
  yScale = height / bufferHeight;
  regionList = imagePtr->GetRegionsFromConfidenceBuffer();

  if ( !regionList )
  {
    fprintf(stderr, "FleshDetector::GetOutlineImage - Failed getting regions\n");
    return false;
  }

  numRegions = regionList->size();
  for (i = 0; i < numRegions; i++)
  {
    (*regionList)[i]->GetBounds(left, right, top, bottom);
    left *= xScale;
    right = (right + 1) * xScale - 1;
    top *= yScale;
    bottom = (bottom + 1) * yScale - 1;
#if 0
    if ( (right - left + 1 < 20) || (bottom - top + 1 < 20) )
      continue;
#else
    if ( (right - left + 1 < 20) || (bottom - top + 1 < 40) )
      continue;
#endif

    mOutlineImage.DrawBox(outlineColor, 3, left, top, right, bottom);
  }

  *outlineImage = &mOutlineImage;

  return true;
}

/*! Calculates the confidence buffer of the image.  This keeps track of whether the image has been modified and only recomputes when necessary. */
bool FleshDetector::CalcConfidence(Image* imagePtr, int xScale, int yScale)
{
  int x, y;
  int imageWidth, imageHeight, numConfidencePoints, numFeatures, pixelsPerPoint;
  int xIncrement, yIncrement, leftOffset, upOffset, diagOffset, classIndex;
  double* integralBuffer;
  double* integralPixel;
  double* confPoint;
  Matrix pixelFeatures;
  double featureRatio, confidence;
  double* confidenceBuffer;
  int bufferAlloc, bufferWidth, bufferHeight, scaledWidth, scaledHeight;
  ConfidenceRevision confRevision;
  map<Image*,ConfidenceRevision>::iterator mapPos;

  if ( !imagePtr || (xScale <= 0) || (yScale <= 0) )
  {
    fprintf(stderr, "FleshDetector::CalcConfidence - Invalid parameter\n");
    return false;
  }

  confRevision.imageRevision = imagePtr->GetBufferUpdateIndex();
  confRevision.xScale = xScale;
  confRevision.yScale = yScale;
  mapPos = mConfidenceRevisions.find(imagePtr);
  if ( (mapPos != mConfidenceRevisions.end()) && (confRevision == mapPos->second) )
    return true;

  imageWidth = imagePtr->GetWidth();
  imageHeight = imagePtr->GetHeight();
  pixelsPerPoint = xScale * yScale;
  featureRatio = 1.0 / pixelsPerPoint;

  if ( (imageWidth % xScale) || (imageHeight % yScale) )
  {
    fprintf(stderr, "FleshDetector::CalcConfidence - Scale doesn't evenly divide image\n");
    return false;
  }

  scaledWidth = imageWidth / xScale;
  scaledHeight = imageHeight / yScale;
  numConfidencePoints = scaledWidth * scaledHeight;

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer || (numConfidencePoints > bufferAlloc) )
  {
    confidenceBuffer = (double*)malloc(numConfidencePoints * sizeof(double));
    if ( !confidenceBuffer )
    {
      fprintf(stderr, "FleshDetector::CalcConfidence - Failed getting confidence buffer\n");
      return false;
    }
    bufferAlloc = numConfidencePoints;
  }
  bufferWidth = scaledWidth;
  bufferHeight = scaledHeight;

  numFeatures = mFeatureList.size();
  if ( !pixelFeatures.SetSize(numFeatures, 1) )
  {
    fprintf(stderr, "FleshDetector::CalcConfidence - Failed setting feature size\n");
    return false;
  }

  integralBuffer = imagePtr->GetCustomIntegralBuffer(mFeatureList);
  integralPixel = integralBuffer + (imageWidth * (yScale - 1) + xScale - 1) * numFeatures;
  xIncrement = numFeatures * xScale;
  yIncrement = numFeatures * imageWidth * (yScale - 1);
  leftOffset = -xIncrement;
  upOffset = -numFeatures * imageWidth * yScale;
  diagOffset = leftOffset + upOffset;
  confPoint = confidenceBuffer;
  for (y = 0; y < bufferHeight; y++, integralPixel += yIncrement)
  {
    for (x = 0; x < bufferWidth; x++, integralPixel += xIncrement, confPoint++)
    {
      /* Get the feature sum for the rectangle */
      pixelFeatures.Set(integralPixel);
      if ( x > 0 )
        pixelFeatures -= integralPixel + leftOffset;
      if ( y > 0 )
      {
        pixelFeatures -= integralPixel + upOffset;
        if ( x > 0 )
          pixelFeatures += integralPixel + diagOffset;
      }

      /* Use the average feature values for classification */
      pixelFeatures *= featureRatio;
      mClassifier.Classify(pixelFeatures, classIndex, confidence);

      if ( classIndex == 0 )
      {
        // Pixel is flesh colored
        *confPoint = confidence;
      }
      else
        *confPoint = 1 - confidence;
    }
  }
  imagePtr->SetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);

  if ( mapPos != mConfidenceRevisions.end() )
    mapPos->second = confRevision;
  else
    mConfidenceRevisions[imagePtr] = confRevision;

  return true;
}
