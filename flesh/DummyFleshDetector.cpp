#include "DummyFleshDetector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FEATURES 512
#define PROXIMITY_THRESH  30
#define DEBUG_BLOCKS 0

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

DummyFleshDetector::DummyFleshDetector()
{
}

DummyFleshDetector::~DummyFleshDetector()
{
}

bool DummyFleshDetector::Load(const char* filename)
{
  mFeatureList = "rgb";

  return true;
}

/*! Calculates the confidence buffer of the image.  This keeps track of whether the image has been modified and only recomputes when necessary. */
bool DummyFleshDetector::CalcConfidence(Image* imagePtr, int xScale, int yScale)
{
  int x, y;
  int imageWidth, imageHeight, numConfidencePoints, numFeatures, pixelsPerPoint;
  int xIncrement, yIncrement, leftOffset, upOffset, diagOffset;
  double* integralBuffer;
  double* integralPixel;
  double* confPoint;
  Matrix pixelFeatures;
  double featureRatio;
  double* confidenceBuffer;
  int bufferAlloc, bufferWidth, bufferHeight, scaledWidth, scaledHeight;
  ConfidenceRevision confRevision;
  map<Image*,ConfidenceRevision>::iterator mapPos;

  if ( !imagePtr || (xScale <= 0) || (yScale <= 0) )
  {
    fprintf(stderr, "DummyFleshDetector::CalcConfidence - Invalid parameter\n");
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
    fprintf(stderr, "DummyFleshDetector::CalcConfidence - Scale doesn't evenly divide image\n");
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
      fprintf(stderr, "DummyFleshDetector::CalcConfidence - Failed getting confidence buffer\n");
      return false;
    }
    bufferAlloc = numConfidencePoints;
  }
  bufferWidth = scaledWidth;
  bufferHeight = scaledHeight;

  numFeatures = mFeatureList.size();
  if ( !pixelFeatures.SetSize(numFeatures, 1) )
  {
    fprintf(stderr, "DummyFleshDetector::CalcConfidence - Failed setting feature size\n");
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

      if ( (pixelFeatures.GetValue(0, 0) <= .15) || (pixelFeatures.GetValue(2, 0) >= pixelFeatures.GetValue(0, 0) - .08) || (pixelFeatures.GetValue(1, 0) >= pixelFeatures.GetValue(0, 0) - .08) )
        *confPoint = 0;
      else
        *confPoint = 1;
    }
  }
  imagePtr->SetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);

  if ( mapPos != mConfidenceRevisions.end() )
    mapPos->second = confRevision;
  else
    mConfidenceRevisions[imagePtr] = confRevision;

  return true;
}