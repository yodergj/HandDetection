#include "Image.h"
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

Image::Image()
{
  mWidth = 0;
  mHeight = 0;
  mBuffer = NULL;
  mBufferSize = 0;
  mYIQBuffer = NULL;
  mYIQAlloc = 0;
}

Image::~Image()
{
  if ( mBuffer )
    free(mBuffer);
  if ( mYIQBuffer )
    free(mYIQBuffer);
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

  if ( !ResizeBuffer(&mYIQBuffer, &mYIQAlloc) )
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

  return mYIQBuffer;
}

double* Image::GetScaledRGBBuffer()
{
  int x, y;
  unsigned char* srcPixel;
  double* destPixel;
  double maxVal;

  if ( !ResizeBuffer(&mScaledRGBBuffer, &mScaledRGBAlloc) )
    return NULL;

  srcPixel = mBuffer;
  destPixel = mYIQBuffer;
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
        destPixel[1] = srcPixel[0] / maxVal;
        destPixel[2] = srcPixel[0] / maxVal;
      }
      srcPixel += 3;
      destPixel += 3;
    }
  }

  return mYIQBuffer;
}

bool Image::CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth)
{
  int x, y;
  int lineWidth;
  unsigned char* destLine;
  int* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

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

bool Image::CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth)
{
  int y;
  int lineWidth;
  unsigned char* destLine;
  unsigned char* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

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

bool Image::ResizeBuffer(double** buffer, int* bufferAlloc)
{
  int sizeNeeded;
  double* tmp;

  if ( !buffer || !bufferAlloc )
    return false;

  sizeNeeded = mWidth * mHeight * 3;
  if ( sizeNeeded > *bufferAlloc )
  {
    tmp = (double *)realloc(*buffer, sizeNeeded);
    if ( !tmp )
      return false;
    *buffer = tmp;
    *bufferAlloc = sizeNeeded;
  }

  return true;
}
