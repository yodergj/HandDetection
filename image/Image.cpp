#include "Image.h"
#include <stdlib.h>
#include <string.h>

Image::Image()
{
  mWidth = 0;
  mHeight = 0;
  mBuffer = NULL;
  mBufferSize = 0;
}

Image::~Image()
{
  if ( mBuffer )
    free(mBuffer);
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
