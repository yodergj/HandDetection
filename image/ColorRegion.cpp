#include "ColorRegion.h"

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
  return ( (R >= mMinR) && (R <= mMaxR) && (G >= mMinG) && (G <= mMaxG) && (B >= mMinB) && (B <= mMaxB) );
}

bool ColorRegion::ColorMatches(unsigned char* rgbVals) const
{
  return ( (rgbVals[0] >= mMinR) && (rgbVals[0] <= mMaxR) && (rgbVals[1] >= mMinG) && (rgbVals[1] <= mMaxG) && (rgbVals[2] >= mMinB) && (rgbVals[2] <= mMaxB) );
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

  std::set<Point> rejectedPoints, checkPoints;
  mPoints.clear();
  mMinX = startPt.x;
  mMaxX = startPt.x;
  mMinY = startPt.y;
  mMaxY = startPt.y;

  int pixelIndex = 3 * (startPt.y * width + startPt.x);
  mMinR = mMaxR = mMeanR = imgBuffer[pixelIndex];
  mMinG = mMaxG = mMeanG = imgBuffer[pixelIndex + 1];
  mMinB = mMaxB = mMeanB = imgBuffer[pixelIndex + 2];

  mPoints.insert(startPt);
  if ( startPt.x > 0 )
    checkPoints.insert( Point(startPt.x - 1, startPt.y) );
  if ( startPt.x < width - 1 )
    checkPoints.insert( Point(startPt.x + 1, startPt.y) );
  if ( startPt.y > 0 )
    checkPoints.insert( Point(startPt.x, startPt.y - 1) );
  if ( startPt.y < height - 1 )
    checkPoints.insert( Point(startPt.x + 1, startPt.y) );

  double centroidX = startPt.x;
  double centroidY = startPt.y;

  int colorTol = (int)(.20 * 255 + .5);

  while ( !checkPoints.empty() )
  {
    Point pt = *checkPoints.begin();
    checkPoints.erase( checkPoints.begin() );

    pixelIndex = 3 * (pt.y * width + pt.x);
    int R = imgBuffer[pixelIndex];
    int G = imgBuffer[pixelIndex + 1];
    int B = imgBuffer[pixelIndex + 2];

#if 0
#if 0
    bool rInRange = ( ( (mMinR <= colorTol) || (R >= mMinR - colorTol) ) &&
                      ( (mMaxR >= 255 - colorTol) || (R <= mMaxR + colorTol) ) );
    bool gInRange = ( ( (mMinG <= colorTol) || (G >= mMinG - colorTol) ) &&
                      ( (mMaxG >= 255 - colorTol) || (G <= mMaxG + colorTol) ) );
    bool bInRange = ( ( (mMinB <= colorTol) || (B >= mMinB - colorTol) ) &&
                      ( (mMaxB >= 255 - colorTol) || (B <= mMaxB + colorTol) ) );
#else
    bool rInRange = ( (R >= mMinR - colorTol) && (R <= mMaxR + colorTol) );
    bool gInRange = ( (G >= mMinG - colorTol) && (G <= mMaxG + colorTol) );
    bool bInRange = ( (B >= mMinB - colorTol) && (B <= mMaxB + colorTol) );
#endif

    if ( rInRange && gInRange && bInRange )
#else
    double colorDist = sqrt( (R - mMeanR) * (R - mMeanR) +
                             (G - mMeanG) * (G - mMeanG) +
                             (B - mMeanB) * (B - mMeanB) );
    if ( colorDist < colorTol )
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

      if ( R < mMinR )
        mMinR = R;
      if ( R > mMaxR )
        mMaxR = R;
      mMeanR = (R + mMeanR * mPoints.size()) / (mPoints.size() + 1);

      if ( G < mMinG )
        mMinG = G;
      if ( G > mMaxG )
        mMaxG = G;
      mMeanG = (G + mMeanG * mPoints.size()) / (mPoints.size() + 1);

      if ( B < mMinB )
        mMinB = B;
      if ( B > mMaxB )
        mMaxB = B;
      mMeanB = (B + mMeanB * mPoints.size()) / (mPoints.size() + 1);

      mPoints.insert(pt);

      if ( pt.x > 0 )
      {
        Point candPt(pt.x - 1, pt.y);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert(candPt);
      }
      if ( pt.x < width - 1 )
      {
        Point candPt(pt.x + 1, pt.y);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert(candPt);
      }
      if ( pt.y > 0 )
      {
        Point candPt(pt.x, pt.y - 1);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert(candPt);
      }
      if ( pt.y < height - 1 )
      {
        Point candPt(pt.x, pt.y + 1);
        if ( (rejectedPoints.find(candPt) == rejectedPoints.end()) && (mPoints.find(candPt) == mPoints.end()) )
          checkPoints.insert(candPt);
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