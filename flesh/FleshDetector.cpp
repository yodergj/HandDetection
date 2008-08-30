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
    return false;

  filePtr = fopen(filename, "r");
  if ( !filePtr )
    return false;

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
  unsigned char outlineColor[] = {0, 255, 0};

  if ( !imagePtr || (!outlineImageOut && !fleshImageOut && !confidenceImageOut) )
    return false;

  // TODO : Make sure this gets cached and is made available if we just want to call one of the other public functions
  TimingAnalyzer_Start(4);
  if ( !CalcConfidence(imagePtr, 8, 8) )
    return false;
  TimingAnalyzer_Stop(4);

  TimingAnalyzer_Start(1);
  if ( !GetFleshImage(imagePtr, backgroundColor, &fleshImage) )
    return false;
  TimingAnalyzer_Stop(1);

  if ( fleshImageOut )
    *fleshImageOut = fleshImage;

  if ( outlineImageOut )
  {
    TimingAnalyzer_Start(2);
    if ( !GetOutlineImage(backgroundColor, outlineColor, imagePtr, fleshImage, outlineImageOut) )
      return false;
    TimingAnalyzer_Stop(2);
  }

  if ( confidenceImageOut )
  {
    TimingAnalyzer_Start(3);
    if ( !GetFleshConfidenceImage(imagePtr, &confidenceImage) )
      return false;
    TimingAnalyzer_Stop(3);
    *confidenceImageOut = confidenceImage;
  }

  return true;
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
    return false;

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer )
    return false;

  xScale = width / bufferWidth;
  yScale = height / bufferHeight;

  srcPixel = imagePtr->GetRGBBuffer();

  if ( !mFleshImage.Create(width, height) )
    return false;

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
    return false;

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer )
    return false;

  xScale = width / bufferWidth;
  yScale = height / bufferHeight;

  if ( !mConfidenceImage.Create(width, height) )
    return false;

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

int FleshDetector::GetBlocks(unsigned char* ignoreColor, Image* imagePtr, std::vector<BlockType*> &blockList)
{
  int i, j;
  int x, y;
  int width, height;
  int numClusters = 0;
  bool blockFound;
  unsigned char* pixel;
  BlockType* rect;
  int numHits = 0;
  int numBlocks;

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();

  pixel = imagePtr->GetRGBBuffer();
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++, pixel += 3)
    {
      if ( (pixel[0] != ignoreColor[0]) ||
           (pixel[1] != ignoreColor[1]) ||
           (pixel[2] != ignoreColor[2]) )
      {
#if DEBUG_BLOCKS
        printf("Hit (%d,%d)\n",x,y);
#endif
        numHits++;
        blockFound = false;
        for (i = 0; (i < numClusters) && !blockFound; i++)
        {
          if ( (x >= blockList[i]->left - PROXIMITY_THRESH) &&
               (x <= blockList[i]->right + PROXIMITY_THRESH) &&
               (y >= blockList[i]->top - PROXIMITY_THRESH) &&
               (y <= blockList[i]->bottom + PROXIMITY_THRESH) )
          {
            if ( x < blockList[i]->left )
              blockList[i]->left = x;
            if ( x > blockList[i]->right )
              blockList[i]->right = x;
            if ( y < blockList[i]->top )
              blockList[i]->top = y;
            if ( y > blockList[i]->bottom )
              blockList[i]->bottom = y;
            blockFound = true;
          }
        }
        if ( !blockFound )
        {
#if DEBUG_BLOCKS
          printf("New block\n");
#endif
          rect = (BlockType*) malloc(sizeof(BlockType));
          if ( !rect )
            return -1;
          rect->left = x;
          rect->right = x;
          rect->top = y;
          rect->bottom = y;
          blockList.push_back(rect);
          numClusters++;
        }
      }
    }
  }

  numBlocks = blockList.size();
  for (i = 0; i < numBlocks; i++)
  {
    if ( (blockList[i]->right - blockList[i]->left + 1 < 5) ||
         (blockList[i]->bottom - blockList[i]->top + 1 < 5) )
      continue;
    j = i + 1;
    while (j < numBlocks)
    {
      if ( (blockList[j]->right - blockList[j]->left + 1 < 5) ||
           (blockList[j]->bottom - blockList[j]->top + 1 < 5) )
      {
        j++;
        continue;
      }
#if DEBUG_BLOCKS
      printf("Block (%d %d %d %d) vs (%d %d %d %d)\n",
          blockList[i]->left,
          blockList[i]->right,
          blockList[i]->top,
          blockList[i]->bottom,
          blockList[j]->left,
          blockList[j]->right,
          blockList[j]->top,
          blockList[j]->bottom);
#endif
      if ( ( ( (blockList[i]->left >= blockList[j]->left - PROXIMITY_THRESH) &&
               (blockList[i]->left <= blockList[j]->right + PROXIMITY_THRESH) ) ||
             ( (blockList[i]->right >= blockList[j]->left - PROXIMITY_THRESH) &&
               (blockList[i]->right <= blockList[j]->right + PROXIMITY_THRESH) ) ) &&
           ( ( (blockList[i]->top >= blockList[j]->top - PROXIMITY_THRESH) &&
               (blockList[i]->top <= blockList[j]->bottom + PROXIMITY_THRESH) ) ||
             ( (blockList[i]->bottom >= blockList[j]->top - PROXIMITY_THRESH) &&
               (blockList[i]->bottom <= blockList[j]->bottom + PROXIMITY_THRESH) ) ) )
      {
        blockList[i]->left = MIN(blockList[i]->left, blockList[j]->left);
        blockList[i]->right = MAX(blockList[i]->right, blockList[j]->right);
        blockList[i]->top = MIN(blockList[i]->top, blockList[j]->top);
        blockList[i]->bottom = MAX(blockList[i]->bottom, blockList[j]->bottom);
        free(blockList[j]);
        blockList.erase(blockList.begin() + j);
        numBlocks--;
        numClusters--;
#if DEBUG_BLOCKS
        printf("Merge\n");
#endif
      }
      else
      {
#if DEBUG_BLOCKS
        printf("Skip\n");
#endif
        j++;
      }
    }
  }
#if DEBUG_BLOCKS
  printf("%d hits %d clusters\n",numHits,numClusters);
#endif

  return numClusters;
}

bool FleshDetector::GetOutlineImage(unsigned char* backgroundColor, unsigned char* outlineColor, Image* imagePtr, Image* fleshImagePtr, Image** outlineImage)
{
  int i, j;
  int x, y;
  int numBlocks;
  int left, right, top, bottom;
  int width, height;
  std::vector<BlockType*> blocklist;
  unsigned char* buffer;

  if ( !backgroundColor || !outlineColor || !imagePtr || !fleshImagePtr || !outlineImage )
    return false;

  width = imagePtr->GetWidth();
  height = imagePtr->GetHeight();
  if ( !mOutlineImage.CopyRGBBuffer(width, height, imagePtr->GetRGBBuffer(), 3 * width) )
    return false;

  buffer = mOutlineImage.GetRGBBuffer();

  numBlocks = GetBlocks(backgroundColor, fleshImagePtr, blocklist);
  for (i = 0; i < numBlocks; i++)
  {
    if ( (blocklist[i]->right - blocklist[i]->left + 1 < 20) ||
         (blocklist[i]->bottom - blocklist[i]->top + 1 < 20) )
      continue;

    left = blocklist[i]->left;
    right = blocklist[i]->right;
    top = blocklist[i]->top;
    bottom = blocklist[i]->bottom;

    for (x = left; x <= right; x++)
    {
      for (j = 0; j < 3; j++)
      {
        buffer[3 * width * top + 3 * x + j] = outlineColor[j];
        buffer[3 * width * bottom + 3 * x + j] = outlineColor[j];
      }
    }
    for (y = top + 1; y < bottom; y++)
    {
      for (j = 0; j < 3; j++)
      {
        buffer[3 * width * y + 3 * left + j] = outlineColor[j];
        buffer[3 * width * y + 3 * right + j] = outlineColor[j];
      }
    }
  }

  for (i = 0; i < numBlocks; i++)
    free(blocklist[i]);

  *outlineImage = &mOutlineImage;

  return true;
}

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

  if ( !imagePtr || (xScale <= 0) || (yScale <= 0) )
    return false;

  imageWidth = imagePtr->GetWidth();
  imageHeight = imagePtr->GetHeight();
  pixelsPerPoint = xScale * yScale;
  featureRatio = 1.0 / pixelsPerPoint;

  if ( (imageWidth % xScale) || (imageHeight % yScale) )
    return false;

  scaledWidth = imageWidth / xScale;
  scaledHeight = imageHeight / yScale;
  numConfidencePoints = scaledWidth * scaledHeight;

  imagePtr->GetConfidenceBuffer(confidenceBuffer, bufferWidth, bufferHeight, bufferAlloc);
  if ( !confidenceBuffer || (numConfidencePoints > bufferAlloc) )
  {
    confidenceBuffer = (double*)malloc(numConfidencePoints * sizeof(double));
    if ( !confidenceBuffer )
      return false;
    bufferAlloc = numConfidencePoints;
  }
  bufferWidth = scaledWidth;
  bufferHeight = scaledHeight;

  numFeatures = mFeatureList.size();
  if ( !pixelFeatures.SetSize(numFeatures, 1) )
    return false;

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

  return true;
}
