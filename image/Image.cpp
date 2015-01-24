#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <qimage.h>
#include "Image.h"
#include "ConnectedRegion.h"
#include "LineSegment.h"
#include "Rect.h"

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

QImage* gQImage = NULL;

bool SaveImageViaQt(const char* filename, const char* format, unsigned char* buffer, int width, int height)
{
  QImage image(buffer, width, height, width * 3,  QImage::Format_RGB888);

  return image.save(filename, format);
}

Image::Image()
{
  mWidth = 0;
  mHeight = 0;
  mBuffer = NULL;
  mBufferSize = 0;
  mBGRBuffer = NULL;
  mBGRBufferSize = 0;
  mBGRValid = false;
  mI420Buffer = NULL;
  mI420BufferSize = 0;
  mI420Valid = false;
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
  mBufferUpdateIndex = 0;
  memset(&mIplImage, 0, sizeof(IplImage));
  mIplImage.nSize = sizeof(IplImage);
}

Image::Image(const Image& ref)
{
  *this = ref;
}

Image::~Image()
{
  if ( mBuffer )
    free(mBuffer);
  if ( mBGRBuffer )
    free(mBGRBuffer);
  if ( mI420Buffer )
    free(mI420Buffer);
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

void Image::MarkBufferAsUpdated()
{
  mBufferUpdateIndex++;
  InvalidateBuffers();
}

int Image::GetBufferUpdateIndex()
{
  return mBufferUpdateIndex;
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
          /* YUV Colorspace - same Y as for YIQ */
        case 'U':
          destPixel[i] = r * -.169 + g * -.331 + b * .500 + .5;
          break;
        case 'V':
          destPixel[i] = r * .500 + g * -.419 + b * -.081 + .5;
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

  MarkBufferAsUpdated();

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

bool Image::CopyBGRABuffer(int width, int height, int* buffer, int bufferWidth)
{
  int x, y;
  int lineWidth;
  unsigned char* destLine;
  int* srcLine;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

  MarkBufferAsUpdated();

  if ( !SetSize(width, height) )
    return false;

  lineWidth = 3 * width;

  srcLine = buffer;
  destLine = mBuffer;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      destLine[3 * x] = (srcLine[x] >> 8) & 0xFF;
      destLine[3 * x + 1] = (srcLine[x] >> 16) & 0xFF;
      destLine[3 * x + 2] = (srcLine[x] >> 24) & 0xFF;
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

  MarkBufferAsUpdated();

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

  MarkBufferAsUpdated();

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

bool Image::CopyBGRBuffer(int width, int height, unsigned char* buffer, int bufferWidth)
{
  int x, y;
  int lineWidth;
  unsigned char* destLine;
  unsigned char* srcLine;
  unsigned char* destPixel;
  unsigned char* srcPixel;

  if ( (width <= 0) || (height <= 0) || !buffer || (bufferWidth <= 0) )
    return false;

  MarkBufferAsUpdated();

  if ( !SetSize(width, height) )
    return false;

  lineWidth = 3 * width;

  srcLine = buffer;
  destLine = mBuffer;
  for (y = 0; y < height; y++)
  {
    srcPixel = srcLine;
    destPixel = destLine;
    for (x = 0; x < width; x++)
    {
      destPixel[0] = srcPixel[2];
      destPixel[1] = srcPixel[1];
      destPixel[2] = srcPixel[0];
      srcPixel += 3;
      destPixel += 3;
    }

    srcLine += bufferWidth;
    destLine += lineWidth;
  }

  return true;
}

bool Image::CopyIplImage(IplImage* image)
{
  if ( image->depth != IPL_DEPTH_8U )
  {
    fprintf(stderr, "Image::CopyIplImage - Bad depth %d\n", image->depth);
    return false;
  }

  if ( image->nChannels == 3 )
    return CopyBGRBuffer(image->width, image->height, (unsigned char*)image->imageData, image->widthStep);

  if ( image->nChannels == 4 )
  {
    if ( image->widthStep % 4 != 0 )
    {
      fprintf(stderr, "Image::CopyIplImage - Width step %d is not divisible by 4\n", image->widthStep);
      return false;
    }
    return CopyBGRABuffer(image->width, image->height, (int*)image->imageData, image->widthStep / 4);
  }


  fprintf(stderr, "Image::CopyIplImage - Unhandled number of channels %d\n", image->nChannels);
  return false;
}

bool Image::DrawBox(const unsigned char* color, int lineWidth,
                    int left, int top, int right, int bottom)
{
  bool retCode = true;

  if ( !color || (lineWidth < 1) )
  {
    fprintf(stderr, "Image::DrawBox - Invalid parameter\n");
    return false;
  }

  retCode = DrawLine(color, lineWidth, left, top, right, top);
  if ( retCode )
    retCode = DrawLine(color, lineWidth, left, top, left, bottom);
  if ( retCode )
    retCode = DrawLine(color, lineWidth, right, top, right, bottom);
  if ( retCode )
    retCode = DrawLine(color, lineWidth, left, bottom, right, bottom);

  return retCode;
}

bool Image::DrawBox(const unsigned char* color1, const unsigned char* color2,
                    const unsigned char* color3, const unsigned char* color4,
                    int lineWidth, int left, int top, int right, int bottom)
{
  bool retCode = true;

  if ( lineWidth < 1 )
  {
    fprintf(stderr, "Image::DrawBox - Invalid parameter\n");
    return false;
  }

  retCode = DrawLine(color1, lineWidth, left, top, right, top);
  retCode &= DrawLine(color2, lineWidth, left, top, left, bottom);
  retCode &= DrawLine(color3, lineWidth, left, bottom, right, bottom);
  retCode &= DrawLine(color4, lineWidth, right, top, right, bottom);

  return retCode;
}

bool Image::DrawRect(const unsigned char* color, int lineWidth, const Rect& rect)
{
  bool retCode = true;

  if ( !color || (lineWidth < 1) )
  {
    fprintf(stderr, "Image::DrawBox - Invalid parameter\n");
    return false;
  }
  Point points[4];
  for (int i = 0; i < 4; i++)
    points[i] = rect.GetPoint(i);

  retCode &= DrawLine(color, lineWidth, points[0], points[1]);
  retCode &= DrawLine(color, lineWidth, points[1], points[2]);
  retCode &= DrawLine(color, lineWidth, points[2], points[3]);
  retCode &= DrawLine(color, lineWidth, points[3], points[0]);

  return retCode;
}

bool Image::DrawLine(const unsigned char* color, int lineWidth, int x1, int y1, int x2, int y2)
{
  int x, y;
  int left, right, top, bottom;
  int bufferWidth;
  unsigned char* destRow;
  unsigned char* destPixel;

  if ( !color || (lineWidth < 1) )
  {
    fprintf(stderr, "Image::DrawLine - Invalid parameter\n");
    return false;
  }

  bufferWidth = mWidth * 3;
  if ( (x1 != x2) && (y1 != y2) )
  {
    // Bresenham's line drawing algorithm
    int tmp, deltaX, deltaY, error, yStep;
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if ( steep )
    {
      tmp = y1;
      y1 = x1;
      x1 = tmp;

      tmp = y2;
      y2 = x2;
      x2 = tmp;
    }
    if ( x1 > x2 )
    {
      tmp = x1;
      x1 = x2;
      x2 = tmp;

      tmp = y1;
      y1 = y2;
      y2 = tmp;
    }

    deltaX = x2 - x1;
    deltaY = abs(y2 - y1);
    error = deltaX / 2;
    y = y1;
    yStep = (y1 < y2) ? 1 : -1;
    for (x = x1; x <= x2; x++)
    {
      if ( steep )
      {
        if ( (x >= 0) && (x < mHeight) && (y >= 0) && (y < mWidth) )
        {
          destPixel = mBuffer + x * bufferWidth + y * 3;
          destPixel[0] = color[0];
          destPixel[1] = color[1];
          destPixel[2] = color[2];
        }
      }
      else
      {
        if ( (x >= 0) && (x < mWidth) && (y >= 0) && (y < mHeight) )
        {
          destPixel = mBuffer + y * bufferWidth + x * 3;
          destPixel[0] = color[0];
          destPixel[1] = color[1];
          destPixel[2] = color[2];
        }
      }

      error -= deltaY;
      if ( error < 0 )
      {
        y += yStep;
        error += deltaX;
      }
    }
    return true;
  }

  if ( x1 < x2 )
  {
    left = x1 - lineWidth / 2;
    right = x2 + lineWidth / 2;
  }
  else
  {
    left = x2 - lineWidth / 2;
    right = x1 + lineWidth / 2;
  }
  if ( y1 < y2 )
  {
    top = y1 - lineWidth / 2;
    bottom = y2 + lineWidth / 2;
  }
  else
  {
    top = y2 - lineWidth / 2;
    bottom = y1 + lineWidth / 2;
  }

  if ( (left >= mWidth) || (right < 0) || (top >= mHeight) || (bottom < 0) )
    return true;

  if ( left < 0 )
    left = 0;
  if ( right >= mWidth )
    right = mWidth - 1;
  if ( top < 0 )
    top = 0;
  if ( bottom >= mHeight )
    bottom = mHeight - 1;

  destRow = mBuffer + top * bufferWidth + left * 3;
  for (y = top; y <= bottom; y++, destRow += bufferWidth)
  {
    destPixel = destRow;
    for (x = left; x <= right; x++, destPixel += 3)
    {
      destPixel[0] = color[0];
      destPixel[1] = color[1];
      destPixel[2] = color[2];
    }
  }

  return true;
}

bool Image::DrawLine(const unsigned char* color, int lineWidth, const Point& p1, const Point& p2)
{
  return DrawLine(color, lineWidth, p1.x, p1.y, p2.x, p2.y);
}

bool Image::DrawLine(const unsigned char* color, int lineWidth, const LineSegment& line)
{
  Point p1 = line.GetLeftPoint();
  Point p2 = line.GetRightPoint();
  return DrawLine(color, lineWidth, p1.x, p1.y, p2.x, p2.y);
}

bool Image::Save(const char* filename)
{
  const char* extStart;

  if ( !filename )
    return false;

  extStart = strrchr(filename, '.');
  if ( !extStart )
    return false;

  if ( !strcmp(extStart, ".ppm") )
    return SavePPM(filename);

  if ( !strcmp(extStart, ".bmp") )
    return SaveImageViaQt(filename, "BMP", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".jpg") )
    return SaveImageViaQt(filename, "JPG", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".jpeg") )
    return SaveImageViaQt(filename, "JPEG", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".png") )
    return SaveImageViaQt(filename, "PNG", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".tif") || !strcmp(extStart, ".tiff") )
    return SaveImageViaQt(filename, "TIFF", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".xbm") )
    return SaveImageViaQt(filename, "XBM", mBuffer, mWidth, mHeight);

  if ( !strcmp(extStart, ".xpm") )
    return SaveImageViaQt(filename, "XPM", mBuffer, mWidth, mHeight);

  return false;
}

bool Image::Save(const string& filename)
{
  return Save(filename.c_str());
}

bool Image::Load(const char* filename)
{
  if ( !filename )
    return false;

  if ( !gQImage )
    gQImage = new QImage;

  if ( !gQImage->load(filename) )
  {
    fprintf(stderr, "Image::Load - Qt failed to load the image %s\n", filename);
    return false;
  }

  *gQImage = gQImage->convertToFormat(QImage::Format_ARGB32);

  return CopyARGBBuffer(gQImage->width(), gQImage->height(), (int*)gQImage->bits(), gQImage->width());
}

bool Image::Load(const string& filename)
{
  return Load(filename.c_str());
}

unsigned char* Image::GetBGRBuffer()
{
  int i, numPixels;
  unsigned char* tmp;
  unsigned char* src;
  unsigned char* dest;

  if ( mBGRValid )
    return mBGRBuffer;

  if ( mBGRBufferSize < mBufferSize )
  {
    tmp = (unsigned char*)realloc(mBGRBuffer, mBufferSize);
    if ( !tmp )
      return NULL;
    mBGRBuffer = tmp;
    mBGRBufferSize = mBufferSize;
  }
  numPixels = mWidth * mHeight;
  src = mBuffer;
  dest = mBGRBuffer;
  for (i = 0; i < numPixels; i++, src += 3, dest += 3)
  {
    dest[0] = src[2];
    dest[1] = src[1];
    dest[2] = src[0];
  }
  mBGRValid = true;

  return mBGRBuffer;
}

unsigned char* Image::GetI420Buffer()
{
  int x, y, numPixels, sum;
  unsigned char rAvg, gAvg, bAvg;
  unsigned char* tmp;
  unsigned char* src;
  unsigned char* yDest;
  unsigned char* uDest;
  unsigned char* vDest;
  int sizeRequired = mBufferSize / 2;
  int lineWidth = mWidth * 3;

  if ( mI420Valid )
    return mI420Buffer;

  if ( mI420BufferSize < sizeRequired )
  {
    tmp = (unsigned char*)realloc(mI420Buffer, sizeRequired);
    if ( !tmp )
      return NULL;
    mI420Buffer = tmp;
    mI420BufferSize = sizeRequired;
  }
  // Need full res Y plane, followed by U plane and V plane at quarter resolution
  numPixels = mWidth * mHeight;
  src = mBuffer;
  yDest = mI420Buffer;
  uDest = yDest + numPixels;
  vDest = uDest + mWidth * mHeight / 4;
  for (y = 0; y < mHeight; y++)
    for (x = 0; x < mWidth; x++, src += 3, yDest++)
    {
      *yDest = (unsigned char)(src[0] * .299 + src[1] *  .587 + src[2] *  .114);
      if ( (x % 2) && (y % 2) )
      {
        sum = src[0] + (src - 3)[0] + (src - lineWidth)[0] + (src - lineWidth - 3)[0];
        rAvg = (unsigned char)(sum / 4);
        sum = src[1] + (src - 3)[1] + (src - lineWidth)[1] + (src - lineWidth - 3)[1];
        gAvg = (unsigned char)(sum / 4);
        sum = src[2] + (src - 3)[2] + (src - lineWidth)[2] + (src - lineWidth - 3)[2];
        bAvg = (unsigned char)(sum / 4);
        *vDest = (unsigned char)(rAvg * .500 + gAvg * -.419 + bAvg * -.081 + 128.5);
        *uDest = (unsigned char)(rAvg * -.169 + gAvg * -.331 + bAvg * .500 + 128.5);
        uDest++;
        vDest++;
      }
    }
  mI420Valid = true;

  return mI420Buffer;
}

IplImage* Image::GetIplImage()
{
  mIplImage.nChannels = 3;
  mIplImage.depth = IPL_DEPTH_8U;
  mIplImage.width = mWidth;
  mIplImage.height = mHeight;
  mIplImage.imageSize = mHeight * mWidth * 3;
  mIplImage.imageData = (char*)GetBGRBuffer();
  mIplImage.widthStep = mWidth * 3;

  return &mIplImage;
}

Image& Image::operator=(const Image& ref)
{
  if ( &ref != this )
    CopyRGBBuffer(ref.mWidth, ref.mHeight, ref.mBuffer, ref.mWidth * 3);

  return *this;
}

bool Image::SavePPM(const char* filename)
{
#ifdef WIN32
  std::ofstream outfile(filename, std::ios::out | std::ios::binary);

  int numBytes, headerLen;
  char headerBuffer[64];

  numBytes = mWidth * mHeight * 3;
  sprintf(headerBuffer, "P6\n%d %d\n255\n", mWidth, mHeight);
  headerLen = strlen(headerBuffer);
  outfile.write(headerBuffer, headerLen);
  outfile.write((const char*)mBuffer, numBytes);
  outfile.close();
#else
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
#endif

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
  mBGRValid = false;
  mI420Valid = false;
  mYIQValid = false;
  mScaledRGBValid = false;
  mCustomValid = false;
  mCustomIntegralValid = false;

  mConfidenceRegionsValid = false;
}

void Image::ClearRegions()
{
  int i, numRegions;

  numRegions = mConfidenceRegions.size();
  for (i = 0; i < numRegions; i++)
    delete mConfidenceRegions[i];

  mConfidenceRegions.clear();
}
