#include "Image.h"
#include "ConnectedRegion.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

Image::Image()
{
  mWidth = 0;
  mHeight = 0;
  mBuffer = NULL;
  mBufferSize = 0;
  mYIQBuffer = NULL;
  mYIQAlloc = 0;
  mYIQValid = false;
  mScaledRGBBuffer = NULL;
  mScaledRGBAlloc = 0;
  mScaledRGBValid = false;
  mCustomBuffer = NULL;
  mCustomAlloc = 0;
  mCustomValid = false;
  mCustomIntegralBuffer = NULL;
  mCustomIntegralAlloc = 0;
  mCustomIntegralValid = false;
  mConfidenceBuffer = NULL;
  mConfidenceBufferAlloc = 0;
  mConfidenceBufferWidth = 0;
  mConfidenceBufferHeight = 0;
  mConfidenceRegionsValid = false;
}

Image::~Image()
{
  if ( mBuffer )
    free(mBuffer);
  if ( mYIQBuffer )
    free(mYIQBuffer);
  if ( mScaledRGBBuffer )
    free(mScaledRGBBuffer);
  if ( mCustomBuffer )
    free(mCustomBuffer);
  if ( mCustomIntegralBuffer )
    free(mCustomIntegralBuffer);
  if ( mConfidenceBuffer )
    free(mConfidenceBuffer);
  ClearRegions();
}

bool Image::Create(int width, int height)
{
  if ( (width <= 0) || (height <= 0) )
    return false;

  return SetSize(width, height);
}

int Image::GetWidth()
{
  return mWidth;
}

int Image::GetHeight()
{
  return mHeight;
}

unsigned char* Image::GetRGBBuffer()
{
  return mBuffer;
}

double* Image::GetYIQBuffer()
{
  int x, y;
  unsigned char* srcPixel;
  double* destPixel;

  if ( mYIQValid )
    return mYIQBuffer;

  if ( !ResizeBuffer(&mYIQBuffer, &mYIQAlloc, 3) )
    return NULL;

  srcPixel = mBuffer;
  destPixel = mYIQBuffer;
  for (y = 0; y < mHeight; y++)
  {
    for (x = 0; x < mWidth; x++)
    {
      destPixel[0] =   srcPixel[0] * .299 + srcPixel[1] *  .587 + srcPixel[2] *  .114;
      destPixel[1] = ((srcPixel[0] * .596 + srcPixel[1] * -.275 + srcPixel[2] * -.321)/.596 + 255)/2;
      destPixel[2] = ((srcPixel[0] * .212 + srcPixel[1] * -.523 + srcPixel[2] *  .311)/.523 + 255)/2;

      srcPixel += 3;
      destPixel += 3;
    }
  }
  mYIQValid = true;

  return mYIQBuffer;
}

double* Image::GetScaledRGBBuffer()
{
  int x, y;
  unsigned char* srcPixel;
  double* destPixel;
  double maxVal;

  if ( mScaledRGBValid )
    return mScaledRGBBuffer;

  if ( !ResizeBuffer(&mScaledRGBBuffer, &mScaledRGBAlloc, 3) )
    return NULL;

  srcPixel = mBuffer;
  destPixel = mScaledRGBBuffer;
  for (y = 0; y < mHeight; y++)
  {
    for (x = 0; x < mWidth; x++)
    {
      maxVal = MAX(srcPixel[0], MAX(srcPixel[1], srcPixel[2]));
      if ( maxVal == 0 )
      {
        destPixel[0] = 1;
        destPixel[1] = 1;
        destPixel[2] = 1;
      }
      else
      {
        destPixel[0] = srcPixel[0] / maxVal;
        destPixel[1] = srcPixel[1] / maxVal;
        destPixel[2] = srcPixel[2] / maxVal;
      }
      srcPixel += 3;
      destPixel += 3;
    }
  }
  mScaledRGBValid = true;

  return mScaledRGBBuffer;
}

double* Image::GetCustomBuffer(string &featureList)
{
  int i, j;
  unsigned char* srcPixel;
  double* destPixel;
  double maxVal, minVal;
  double r, g, b;
  int numFeatures, numPixels;
  double colorX, colorY, colorZ;

  if ( mCustomValid && (mCustomString == featureList) )
    return mCustomBuffer;

  mCustomValid = false;
  numFeatures = featureList.size();
  numPixels = mWidth * mHeight;

  if ( !ResizeBuffer(&mCustomBuffer, &mCustomAlloc, numFeatures) )
    return NULL;

  srcPixel = mBuffer;
  destPixel = mCustomBuffer;
  for (j = 0; j < numPixels; j++)
  {
    r = srcPixel[0] / 255.0;
    g = srcPixel[1] / 255.0;
    b = srcPixel[2] / 255.0;

    /* CIE XYZ Colorspace */
    colorX = (r * .49 + g * .31 + b * .20) / .17697;
    colorY = (r * .17697 + g * .81240 + b * .01063) / .17697;
    colorZ = (g * .01 + b * .99) / .17697;

    maxVal = MAX(r, MAX(g, b));
    minVal = MIN(r, MIN(g, b));
    for (i = 0; i < numFeatures; i++)
    {
      switch (featureList[i])
      {
        /* Normal RGB */
        case 'r':
          destPixel[i] = r;
          break;
        case 'g':
          destPixel[i] = g;
          break;
        case 'b':
          destPixel[i] = b;
          break;
          /* RGB scaled relative to maximum component */
        case 'R':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = r / maxVal;
          break;
        case 'G':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = g / maxVal;
          break;
        case 'B':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = b / maxVal;
          break;
          /* rg chromaticity space */
        case 'm':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = r / (r + g + b);
          break;
        case 'n':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = g / (r + g + b);
          break;
        case 'o':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = b / (r + g + b);
          break;
          /* YIQ Colorspace */
        case 'Y':
          destPixel[i] = r * .299 + g *  .587 + b *  .114;
          break;
        case 'I':
          destPixel[i] = ((r * .596 + g * -.275 + b * -.321) / .596 + 1) / 2;
          break;
        case 'Q':
          destPixel[i] = ((r * .212 + g * -.523 + b *  .311) / .523 + 1) / 2;
          break;
          /* HSL aka HSI Colorspace */
        case 'H':
          if ( maxVal == minVal )
            destPixel[i] = 0;
          else if ( r == maxVal )
          {
            if ( g >= b )
              destPixel[i] = (g - b) / (6 * (maxVal - minVal));
            else
              destPixel[i] = (g - b) / (6 * (maxVal - minVal)) + 1;
          }
          else if ( g == maxVal )
            destPixel[i] = (b - r) / (6 * (maxVal - minVal)) + (1 / 3.0);
          else
            destPixel[i] = (r - g) / (6 * (maxVal - minVal)) + (2 / 3.0);
          break;
        case 'S':
          if ( maxVal == minVal )
            destPixel[i] = 0;
          else if ( maxVal + minVal <= 1 )
            destPixel[i] = (maxVal - minVal) / (maxVal + minVal);
          else
            destPixel[i] = (maxVal - minVal) / (2 - maxVal + minVal);
          break;
        case 'L':
          destPixel[i] = (maxVal + minVal) / 2;
          break;
          /* CIE xy chromaticity space */
        case 'x':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = colorX / (colorX + colorY + colorZ);
          break;
        case 'y':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = colorY / (colorX + colorY + colorZ);
          break;
        case 'z':
          if ( maxVal == 0 )
            destPixel[i] = 1;
          else
            destPixel[i] = colorZ / (colorX + colorY + colorZ);
          break;
          /* Hunter Lab Color space */
        case 'l':
          destPixel[i] = sqrt(colorY);
          break;
        case 'a':
          destPixel[i] = 1.723 * (colorX - colorY) / sqrt(colorY);
          break;
        case 'c':
          destPixel[i] = .672 * (colorY - colorZ) / sqrt(colorY);
          break;
      }
    }

    srcPixel += 3;
    destPixel += numFeatures;
  }
  mCustomValid = true;
  mCustomString = featureList;

  return mCustomBuffer;
}

double* Image::GetCustomIntegralBuffer(string &featureList)
{
  int i, y;
  double* feature;
  int numFeatures, leftOffset, upOffset, diagOffset;
  int stepsPerRow;

  if ( mCustomIntegralValid && (mCustomString == featureList) )
    return mCustomIntegralBuffer;

  mCustomIntegralValid = false;
  numFeatures = featureList.size();
  leftOffset = -numFeatures;
  upOffset = mWidth * leftOffset;
  diagOffset = leftOffset + upOffset;

  if ( !ResizeBuffer(&mCustomIntegralBuffer, &mCustomIntegralAlloc, numFeatures) )
    return NULL;

  if ( !GetCustomBuffer(featureList) )
    return NULL;

  memcpy(mCustomIntegralBuffer, mCustomBuffer, mWidth * mHeight * numFeatures * sizeof(double));
  stepsPerRow = mWidth * numFeatures;

  feature = mCustomIntegralBuffer + numFeatures;
  for (i = numFeatures; i < stepsPerRow; i++, feature++)
    feature[0] += feature[leftOffset];
  for (y = 1; y < mHeight; y++)
  {
    for (i = 0; i < numFeatures; i++, feature++)
      feature[0] += feature[upOffset];
    for (i = numFeatures; i < stepsPerRow; i++, feature++)
      feature[0] += feature[upOffset] + feature[leftOffset] - feature[diagOffset];
  }

  mCustomIntegralValid = true;

  return mCustomIntegralBuffer;
}

bool Image::GetConfidenceBuffer(double* &buffer, int &bufferWidth, int &bufferHeight, int &bufferAlloc)
{
  buffer = mConfidenceBuffer;
  bufferWidth = mConfidenceBufferWidth;
  bufferHeight = mConfidenceBufferHeight;
  bufferAlloc = mConfidenceBufferAlloc;

  return true;
}

bool Image::SetConfidenceBuffer(double* buffer, int bufferWidth, int bufferHeight, int bufferAlloc)
{
  if ( buffer != mConfidenceBuffer )
    free(mConfidenceBuffer);

  mConfidenceBuffer = buffer;
  mConfidenceBufferWidth = bufferWidth;
  mConfidenceBufferHeight = bufferHeight;
  mConfidenceBufferAlloc = bufferAlloc;

  mConfidenceRegionsValid = false;

  return true;
}

vector<ConnectedRegion*>* Image::GetRegionsFromConfidenceBuffer()
{
  int i, j, x, y, xStart, numRegions;
  double* confidence;
  ConnectedRegion* region = NULL;
  bool regionsCombined;

  if ( mConfidenceRegionsValid )
    return &mConfidenceRegions;

  if ( !mConfidenceBuffer )
    return NULL;

  ClearRegions();

  /* Create regions out of each horizontal run of connected confidences */
  confidence = mConfidenceBuffer;
  for (y = 0; y < mConfidenceBufferHeight; y++)
  {
    xStart = -1;
    for (x = 0; x < mConfidenceBufferWidth; x++, confidence++)
    {
      if ( *confidence >= .50 )
      {
        if ( xStart == -1 )
        {
          xStart = x;
          region = new ConnectedRegion;
        }
      }
      else
      {
        if ( xStart != -1 )
        {
          region->AddRun(xStart, y, x - xStart);
          xStart = -1;
          mConfidenceRegions.push_back(region);
        }
      }
    }
    if ( xStart != -1 )
    {
      region->AddRun(xStart, y, x - xStart);
      mConfidenceRegions.push_back(region);
    }
  }

  /* Merge the runs into proper connected regions */
  do
  {
    regionsCombined = false;
    numRegions = mConfidenceRegions.size();
    for (i = 0; i < numRegions; i++)
    {
      for (j = i + 1; j < numRegions; j++)
      {
        if ( mConfidenceRegions[i]->TouchesRegion(*mConfidenceRegions[j]) )
        {
          mConfidenceRegions[i]->MergeInRegion(*mConfidenceRegions[j]);
          delete mConfidenceRegions[j];
          mConfidenceRegions.erase(mConfidenceRegions.begin() + j);
          j--;
          numRegions--;
          regionsCombined = true;
        }
      }
    }
  } while ( regionsCombined );

  mConfidenceRegionsValid = true;
  return &mConfidenceRegions;
}

bool Image::CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth)
{
  int x, y;
  int lineWidth;
  unsigned char* destLine;
  int* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

  InvalidateBuffers();

  if ( !SetSize(width, height) )
    return false;

  lineWidth = 3 * width;

  srcLine = buffer;
  destLine = mBuffer;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      destLine[3 * x] = (srcLine[x] >> 24) & 0xFF;
      destLine[3 * x + 1] = (srcLine[x] >> 16) & 0xFF;
      destLine[3 * x + 2] = (srcLine[x] >> 8) & 0xFF;
    }
    srcLine += bufferWidth;
    destLine += lineWidth;
  }

  return true;
}

bool Image::CopyARGBBuffer(int width, int height, int* buffer, int bufferWidth)
{
  int x, y;
  int lineWidth;
  unsigned char* destLine;
  int* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

  InvalidateBuffers();

  if ( !SetSize(width, height) )
    return false;

  lineWidth = 3 * width;

  srcLine = buffer;
  destLine = mBuffer;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      destLine[3 * x] = (srcLine[x] >> 16) & 0xFF;
      destLine[3 * x + 1] = (srcLine[x] >> 8) & 0xFF;
      destLine[3 * x + 2] = srcLine[x] & 0xFF;
    }
    srcLine += bufferWidth;
    destLine += lineWidth;
  }

  return true;
}

bool Image::CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth)
{
  int y;
  int lineWidth;
  unsigned char* destLine;
  unsigned char* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

  InvalidateBuffers();

  if ( !SetSize(width, height) )
    return false;

  lineWidth = 3 * width;

  srcLine = buffer;
  destLine = mBuffer;
  for (y = 0; y < height; y++)
  {
    memcpy(destLine, srcLine, lineWidth);
    srcLine += bufferWidth;
    destLine += lineWidth;
  }

  return true;
}

bool Image::Save(const char* filename)
{
  char* extStart;

  if ( !filename )
    return false;

  extStart = strrchr(filename, '.');
  if ( !extStart )
    return false;

  if ( !strcmp(extStart, ".ppm") )
    return SavePPM(filename);

  return false;
}

bool Image::SavePPM(const char* filename)
{
  int numBytes, headerLen, fileDesc;
  char headerBuffer[64];

  fileDesc = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if ( fileDesc < 0 )
    return false;

  numBytes = mWidth * mHeight * 3;
  sprintf(headerBuffer, "P6\n%d %d\n255\n", mWidth, mHeight);
  headerLen = strlen(headerBuffer);
  write(fileDesc, headerBuffer, headerLen);
  write(fileDesc, mBuffer, numBytes);
  close(fileDesc);

  return true;
}

bool Image::SetSize(int width, int height)
{
  int sizeNeeded;
  unsigned char* tmp;

  sizeNeeded = width * height * 3;
  if ( sizeNeeded > mBufferSize )
  {
    tmp = (unsigned char *)realloc(mBuffer, sizeNeeded);
    if ( !tmp )
      return false;
    mBuffer = tmp;
    mBufferSize = sizeNeeded;
  }
  mWidth = width;
  mHeight = height;

  return true;
}

bool Image::ResizeBuffer(double** buffer, int* bufferAlloc, int numFeatures)
{
  int sizeNeeded;
  double* tmp;

  if ( !buffer || !bufferAlloc )
    return false;

  sizeNeeded = mWidth * mHeight * numFeatures;
  if ( sizeNeeded > *bufferAlloc )
  {
    tmp = (double *)realloc(*buffer, sizeNeeded * sizeof(double));
    if ( !tmp )
      return false;
    *buffer = tmp;
    *bufferAlloc = sizeNeeded;
  }

  return true;
}

void Image::InvalidateBuffers()
{
  mYIQValid = false;
  mScaledRGBValid = false;
  mCustomValid = false;
  mCustomIntegralValid = false;
}

void Image::ClearRegions()
{
  int i, numRegions;

  numRegions = mConfidenceRegions.size();
  for (i = 0; i < numRegions; i++)
    delete mConfidenceRegions[i];

  mConfidenceRegions.clear();
}
