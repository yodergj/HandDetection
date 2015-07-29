#include "ColorRegion.h"
#include "Color.h"

const int ColorRegion::mColorTolerance = (int)(.20 * 255 + .5);
const double ColorRegion::mChannelTolerance = .075 * 255;
const double ColorRegion::mHueTolerance = .0175;
const double ColorRegion::mSaturationTolerance = .15;
const double ColorRegion::mWeakSaturationThreshold = .35;
const double ColorRegion::mLowValueThreshold = .075;

ColorRegion::ColorRegion()
{
  mMinX = -1;
  mMaxX = -1;
  mMinY = -1;
  mMaxY = -1;

  mMinR = 255;
  mMaxR = 0;
  mMeanR = -1;
  mMinG = 255;
  mMaxG = 0;
  mMeanG = -1;
  mMinB = 255;
  mMaxB = 0;
  mMeanB = -1;

  mMinH = FLT_MAX;
  mMaxH = 0;
  mMeanH = -1;
  mMinS = FLT_MAX;
  mMaxS = 0;
  mMeanS = -1;

  mPixelsInMean = 0;

  mStaleResultCount = 0;
  mIntegralBuffer = 0;
  mRefHeight = 0;
}

ColorRegion::ColorRegion(const ColorRegion& ref)
{
  mIntegralBuffer = 0;
  operator=(ref);
}

ColorRegion::~ColorRegion()
{
  delete[] mIntegralBuffer;
}

ColorRegion& ColorRegion::operator=(const ColorRegion& ref)
{
  if ( &ref != this )
  {
    mPoints = ref.mPoints;
    mMinX = ref.mMinX;
    mMaxX = ref.mMaxX;
    mMinY = ref.mMinY;
    mMaxY = ref.mMaxY;
    mCentroid = ref.mCentroid;

    mMinR = ref.mMinR;
    mMaxR = ref.mMaxR;
    mMeanR = ref.mMeanR;
    mMinG = ref.mMinG;
    mMaxG = ref.mMaxG;
    mMeanG = ref.mMeanG;
    mMinB = ref.mMinB;
    mMaxB = ref.mMaxB;
    mMeanB = ref.mMeanB;

    mMinH = ref.mMinH;
    mMaxH = ref.mMaxH;
    mMeanH = ref.mMeanH;
    mMinS = ref.mMinS;
    mMaxS = ref.mMaxS;
    mMeanS = ref.mMeanS;
    mPixelsInMean = ref.mPixelsInMean;

    mStaleResultCount = ref.mStaleResultCount;
    mRefHeight = ref.mRefHeight;
    if ( mIntegralBuffer )
    {
      delete[] mIntegralBuffer;
      mIntegralBuffer = 0;
    }
    if ( ref.mIntegralBuffer )
    {
      int bufSize = (mMaxX - mMinX + 1) * (mMaxY - mMinY + 1);
      mIntegralBuffer = new int[bufSize];
      memcpy(mIntegralBuffer, ref.mIntegralBuffer, bufSize * sizeof(int));
    }
  }

  return *this;
}

bool ColorRegion::SetReferenceHeight(int refHeight)
{
  if ( refHeight < 0 )
    return false;

  mRefHeight = refHeight;
  return true;
}

bool ColorRegion::Empty() const
{
  if ( (mMaxX < 0) || (mMaxY < 0) || (mMinX < 0) || (mMinY < 0) )
    return true;

  return false;
}

bool ColorRegion::ColorMatches(int R, int G, int B) const
{
#if 0
  return ( (R >= mMinR) && (R <= mMaxR) && (G >= mMinG) && (G <= mMaxG) && (B >= mMinB) && (B <= mMaxB) );
#else
  double hue = GetHue(R, G, B);
  double colorDist = sqrt( (R - mMeanR) * (R - mMeanR) +
                           (G - mMeanG) * (G - mMeanG) +
                           (B - mMeanB) * (B - mMeanB) );
  return ( (colorDist < mColorTolerance) || HueInRange(mMeanH, mHueTolerance, hue) );
#endif
}

bool ColorRegion::ColorMatches(unsigned char* rgbVals) const
{
#if 0
  return ( (rgbVals[0] >= mMinR) && (rgbVals[0] <= mMaxR) && (rgbVals[1] >= mMinG) && (rgbVals[1] <= mMaxG) && (rgbVals[2] >= mMinB) && (rgbVals[2] <= mMaxB) );
#else
  int R = rgbVals[0];
  int G = rgbVals[1];
  int B = rgbVals[2];
  double hue = GetHue(rgbVals);
  double colorDist = sqrt( (R - mMeanR) * (R - mMeanR) +
                           (G - mMeanG) * (G - mMeanG) +
                           (B - mMeanB) * (B - mMeanB) );
  return ( (colorDist < mColorTolerance) || HueInRange(mMeanH, mHueTolerance, hue) );
#endif
}

bool ColorRegion::ContainsPixel(int x, int y) const
{
  Point pt(x, y);
  return (mPoints.find(pt) != mPoints.end());
}

bool ColorRegion::Grow(Image& image, const Point& startPt)
{
  int width = image.GetWidth();
  int height = image.GetHeight();

  if ( (width == 0) || (height == 0) || (startPt.x < 0) || (startPt.x >= width) || (startPt.y < 0) || (startPt.y >= height) )
    return false;

  unsigned char* imgBuffer = image.GetRGBBuffer();
  if ( !imgBuffer )
    return false;

  std::set<Point> rejectedPoints;
  std::set<DistPointPair, DistPointPairCompare> checkPoints;
  mPoints.clear();
  mMinX = startPt.x;
  mMaxX = startPt.x;
  mMinY = startPt.y;
  mMaxY = startPt.y;

  int pixelIndex = 3 * (startPt.y * width + startPt.x);
  mMinR = mMaxR = mMeanR = imgBuffer[pixelIndex];
  mMinG = mMaxG = mMeanG = imgBuffer[pixelIndex + 1];
  mMinB = mMaxB = mMeanB = imgBuffer[pixelIndex + 2];
  mMinH = mMaxH = mMeanH = GetHue(imgBuffer + pixelIndex);
#if 0
  mMinS = mMaxS = mMeanS = GetHSLSaturation(imgBuffer + pixelIndex);
#else
  mMinS = mMaxS = mMeanS = GetHSVSaturation(imgBuffer + pixelIndex);
#endif

  mPoints.insert(startPt);
  if ( startPt.x > 0 )
    checkPoints.insert( DistPointPair(1, Point(startPt.x - 1, startPt.y)) );
  if ( startPt.x < width - 1 )
    checkPoints.insert( DistPointPair(1, Point(startPt.x + 1, startPt.y)) );
  if ( startPt.y > 0 )
    checkPoints.insert( DistPointPair(1, Point(startPt.x, startPt.y - 1)) );
  if ( startPt.y < height - 1 )
    checkPoints.insert( DistPointPair(1, Point(startPt.x, startPt.y + 1)) );

  double centroidX = startPt.x;
  double centroidY = startPt.y;

  while ( !checkPoints.empty() )
  {
    Point pt = checkPoints.begin()->second;
    checkPoints.erase( checkPoints.begin() );

    pixelIndex = 3 * (pt.y * width + pt.x);
    int R = imgBuffer[pixelIndex];
    int G = imgBuffer[pixelIndex + 1];
    int B = imgBuffer[pixelIndex + 2];
    double hue = GetHue(imgBuffer + pixelIndex);
#if 0
    double saturation = GetHSLSaturation(imgBuffer + pixelIndex);
#else
    double saturation = GetHSVSaturation(imgBuffer + pixelIndex);
#endif

#if 0
    double colorDist = sqrt( (R - mMeanR) * (R - mMeanR) +
                             (G - mMeanG) * (G - mMeanG) +
                             (B - mMeanB) * (B - mMeanB) );
    if ( (colorDist < mColorTolerance) || HueInRange(mMeanH, mHueTolerance, hue) )
#else
#if 0
    double rDist = fabs(R - mMeanR);
    double gDist = fabs(G - mMeanG);
    double bDist = fabs(B - mMeanB);
    double colorDist = sqrt( rDist * rDist + gDist * gDist + bDist * bDist );
    if ( ( (colorDist < mColorTolerance) &&
           (rDist < mChannelTolerance) &&
           (gDist < mChannelTolerance) &&
           (bDist < mChannelTolerance) ) ||
         HueInRange(mMeanH, mHueTolerance, hue) )
#else
#if 0
    double hueDist = GetHueDistance(hue, mMeanH);
    double satDist = fabs(saturation - mMeanS);
    if ( ( (hueDist < mHueTolerance) && (satDist < mSaturationTolerance) ) ||
         ( (hueDist < .5 * mHueTolerance) && (satDist < 2 * mSaturationTolerance) ) )
#else
    bool hueMatch = false;
    bool hueMatchGoodForAvg = false;
    bool distMatch = false;

    double meanValue = GetHSVValue((int)mMeanR, (int)mMeanG, (int)mMeanB);

    if ( (mMeanS < mWeakSaturationThreshold) ||
         (meanValue < mLowValueThreshold) )
    {
      double colorDist = sqrt( (R - mMeanR) * (R - mMeanR) +
                               (G - mMeanG) * (G - mMeanG) +
                               (B - mMeanB) * (B - mMeanB) );
      double satDist = fabs(saturation - mMeanS);
      distMatch = ( (colorDist < mColorTolerance) && (satDist < mSaturationTolerance) );
    }
    else
    {
      double hueDist = GetHueDistance(hue, mMeanH);
      double satDist = fabs(saturation - mMeanS);
      if ( (hueDist < mHueTolerance) && (satDist < mSaturationTolerance) )
      {
        hueMatch = true;
        hueMatchGoodForAvg = true;
      }
      else
        hueMatch = ( (hueDist < .5 * mHueTolerance) && (satDist < 2 * mSaturationTolerance) );
    }

    if ( hueMatch || distMatch )
#endif
#endif
#endif
    {
      if ( pt.x < mMinX )
        mMinX = pt.x;
      if ( pt.x > mMaxX )
        mMaxX = pt.x;
      if ( pt.y < mMinY )
        mMinY = pt.y;
      if ( pt.y > mMaxY )
        mMaxY = pt.y;

      centroidX = (pt.x + centroidX * mPoints.size()) / (mPoints.size() + 1);
      centroidY = (pt.y + centroidY * mPoints.size()) / (mPoints.size() + 1);

#if 0
      if ( colorDist < mColorTolerance )
#else
#if 0
      if ( (colorDist < mColorTolerance) &&
           (rDist < mChannelTolerance) &&
           (gDist < mChannelTolerance) &&
           (bDist < mChannelTolerance) )
#else
#if 0
      if ( (hueDist < mHueTolerance) && (satDist < mSaturationTolerance) )
#else
      if ( hueMatchGoodForAvg || distMatch )
#endif
#endif
#endif
      {
        if ( R < mMinR )
          mMinR = R;
        if ( R > mMaxR )
          mMaxR = R;
        mMeanR = (R + mMeanR * mPixelsInMean) / (mPixelsInMean + 1);

        if ( G < mMinG )
          mMinG = G;
        if ( G > mMaxG )
          mMaxG = G;
        mMeanG = (G + mMeanG * mPixelsInMean) / (mPixelsInMean + 1);

        if ( B < mMinB )
          mMinB = B;
        if ( B > mMaxB )
          mMaxB = B;
        mMeanB = (B + mMeanB * mPixelsInMean) / (mPixelsInMean + 1);

        if ( hue < mMinH )
          mMinH = hue;
        if ( hue > mMaxH )
          mMaxH = hue;
#if 1
        mMeanH = (hue + mMeanH * mPixelsInMean) / (mPixelsInMean + 1);
#else
        mMeanH = GetHue((int)(mMeanR + .5), (int)(mMeanG + .5), (int)(mMeanB + .5));
#endif
        mMeanS = (saturation + mMeanS * mPixelsInMean) / (mPixelsInMean + 1);

        mPixelsInMean++;
      }

      mPoints.insert(pt);

      if ( pt.x > 0 )
      {
        Point candPt(pt.x - 1, pt.y);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert( DistPointPair(candPt.GetTaxicabDistance(startPt), candPt) );
      }
      if ( pt.x < width - 1 )
      {
        Point candPt(pt.x + 1, pt.y);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert( DistPointPair(candPt.GetTaxicabDistance(startPt), candPt) );
      }
      if ( pt.y > 0 )
      {
        Point candPt(pt.x, pt.y - 1);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert( DistPointPair(candPt.GetTaxicabDistance(startPt), candPt) );
      }
      if ( pt.y < height - 1 )
      {
        Point candPt(pt.x, pt.y + 1);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert( DistPointPair(candPt.GetTaxicabDistance(startPt), candPt) );
      }
    }
    else
      rejectedPoints.insert(pt);
  }

  mCentroid.x = (int)(centroidX + .5);
  mCentroid.y = (int)(centroidY + .5);

  return true;
}

bool ColorRegion::TrackFromOldRegion(Image& image, const ColorRegion& oldRegion)
{
  int width = image.GetWidth();
  int height = image.GetHeight();

  if ( !Empty() || (width == 0) || (height == 0) || oldRegion.Empty() || (oldRegion.mCentroid.x >= width) || (oldRegion.mCentroid.y >= height) )
    return false;

  unsigned char* imgBuffer = image.GetRGBBuffer();
  if ( !imgBuffer )
    return false;

  mRefHeight = oldRegion.mRefHeight;

  int x,y;
  int centerX = oldRegion.mCentroid.x;
  int centerY = oldRegion.mCentroid.y;
  int pixelIndex = 3 * (centerY * width + centerX);

  if ( oldRegion.ColorMatches(imgBuffer + pixelIndex) )
    return Grow(image, oldRegion.mCentroid);

  int refWidth = oldRegion.mMaxX - oldRegion.mMinX + 1;
  int refHeight = oldRegion.mMaxY - oldRegion.mMinY + 1;
  int maxDist = (refWidth > refHeight) ? refWidth : refHeight;

  for (int dist = 1; dist <= maxDist; dist++)
  {
    int minX = (dist > centerX) ? 0 : centerX - dist;
    int maxX = (dist + centerX >= width) ? width - 1 : centerX + dist;
    int minY = (dist > centerY) ? 0 : centerY - dist;
    int maxY = (dist + centerY >= height) ? height - 1 : centerY + dist;
    bool checkTop = (centerY >= dist);
    bool checkBottom = (centerY + dist < height);
    bool checkLeft = (centerX >= dist);
    bool checkRight = (centerX + dist < width);

    if ( checkTop || checkBottom )
    {
      for (x = minX; x <= maxX; x++)
      {
        if ( checkTop )
        {
          y = minY;
          pixelIndex = 3 * (y * width + x);
          if ( oldRegion.ColorMatches(imgBuffer + pixelIndex) )
            return Grow(image, Point(x,y));
        }
        if ( checkBottom )
        {
          y = maxY;
          pixelIndex = 3 * (y * width + x);
          if ( oldRegion.ColorMatches(imgBuffer + pixelIndex) )
            return Grow(image, Point(x,y));
        }
      }
    }
    if ( checkLeft || checkRight )
    {
      for (y = minY + 1; y < maxY; y++)
      {
        if ( checkLeft )
        {
          x = minX;
          pixelIndex = 3 * (y * width + x);
          if ( oldRegion.ColorMatches(imgBuffer + pixelIndex) )
            return Grow(image, Point(x,y));
        }
        if ( checkRight )
        {
          x = maxX;
          pixelIndex = 3 * (y * width + x);
          if ( oldRegion.ColorMatches(imgBuffer + pixelIndex) )
            return Grow(image, Point(x,y));
        }
      }
    }
  }

  // TODO Do more heroic attempts to locate the hand

  if ( Empty() )
  {
    *this = oldRegion;
    mStaleResultCount++;
  }

  return true;
}

int* ColorRegion::GetIntegralBuffer()
{
  if ( !mIntegralBuffer )
    GenerateIntegralBuffer();
  return mIntegralBuffer;
}

void ColorRegion::FreeIntegralBuffer()
{
  if ( mIntegralBuffer )
    delete[] mIntegralBuffer;
  mIntegralBuffer = 0;
}

void ColorRegion::GenerateIntegralBuffer()
{
  if ( mIntegralBuffer )
    delete[] mIntegralBuffer;

  int i, j, x, y, bufX, bufY;
  int width = mMaxX - mMinX + 1;
  int height = mMaxY - mMinY + 1;
  int bufSize = width * height;
  int leftOffset = -1;
  int upOffset = width * leftOffset;
  int diagOffset = leftOffset + upOffset;
  mIntegralBuffer = new int[bufSize];
  memset(mIntegralBuffer, 0, bufSize * sizeof(int));
  x = mMinX;
  y = mMinY;

  // Points in the set are ordered top->bottom, left->right.  Since we're working with a connected region, we're guaranteed to have something in every row and column.
  for (std::set<Point>::iterator itr = mPoints.begin(); itr != mPoints.end(); itr++)
  {
    // Finish remainder on the line above
    for (; y < itr->y; y++)
    {
      bufY = y - mMinY;
      for (; x <= mMaxX; x++)
      {
        bufX = x - mMinX;
        if ( bufX > 0 )
          mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + leftOffset];
        if ( bufY > 0 )
        {
          mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + upOffset];
          if ( bufX > 0 )
            mIntegralBuffer[bufY * width + bufX] -= mIntegralBuffer[bufY * width + bufX + diagOffset];
        }
      }
      x = mMinX;
    }

    // Process pixels to the left on this row
    bufY = y - mMinY;
    for (; x < itr->x; x++)
    {
      bufX = x - mMinX;
      if ( bufX > 0 )
        mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + leftOffset];
      if ( bufY > 0 )
      {
        mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + upOffset];
        if ( bufX > 0 )
          mIntegralBuffer[bufY * width + bufX] -= mIntegralBuffer[bufY * width + bufX + diagOffset];
      }
    }

    // Do the current pixel
    bufX = x - mMinX;
    mIntegralBuffer[bufY * width + bufX] = 1;
    if ( bufX > 0 )
      mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + leftOffset];
    if ( bufY > 0 )
    {
      mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + upOffset];
      if ( bufX > 0 )
        mIntegralBuffer[bufY * width + bufX] -= mIntegralBuffer[bufY * width + bufX + diagOffset];
    }

    x++;
    if ( x > mMaxX )
    {
      x = mMinX;
      y++;
    }
  }

  // Finish the remainder
  for (; y <= mMaxY; y++)
  {
    bufY = y - mMinY;
    for (; x <= mMaxX; x++)
    {
      bufX = x - mMinX;
      if ( bufX > 0 )
        mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + leftOffset];
      if ( bufY > 0 )
      {
        mIntegralBuffer[bufY * width + bufX] += mIntegralBuffer[bufY * width + bufX + upOffset];
        if ( bufX > 0 )
          mIntegralBuffer[bufY * width + bufX] -= mIntegralBuffer[bufY * width + bufX + diagOffset];
      }
    }
    x = mMinX;
  }
}