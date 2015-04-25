#ifndef _COLOR_REGION_H
#define _COLOR_REGION_H

#include <Point.h>
#include <Image.h>
#include <set>

class ColorRegion
{
public:
  ColorRegion();
  ColorRegion(const ColorRegion& ref);
  ~ColorRegion();

  ColorRegion& operator=(const ColorRegion& ref);

  bool Empty() const;
  bool ColorMatches(int R, int G, int B) const;
  bool ColorMatches(unsigned char* rgbVals) const;

  bool Grow(Image& image, const Point& startPt);
  bool TrackFromOldRegion(Image& image, const ColorRegion& oldRegion);

  int GetWidth() { return mMaxX - mMinX + 1; };
  int GetHeight() { return mMaxY - mMinY + 1; };
  Point GetCentroid() { return mCentroid; };
  int GetMinR() { return mMinR; };
  int GetMaxR() { return mMaxR; };
  int GetMinG() { return mMinG; };
  int GetMaxG() { return mMaxG; };
  int GetMinB() { return mMinB; };
  int GetMaxB() { return mMaxB; };
  double GetMeanR() { return mMeanR; };
  double GetMeanG() { return mMeanG; };
  double GetMeanB() { return mMeanB; };
  int* GetIntegralBuffer();
private:
  void GenerateIntegralBuffer();
  std::set<Point> mPoints;

  int mMinX, mMaxX, mMinY, mMaxY;
  Point mCentroid;

  double mMeanR, mMeanG, mMeanB;
  int mMinR, mMaxR;
  int mMinG, mMaxG;
  int mMinB, mMaxB;

  int mStaleResultCount;

  int* mIntegralBuffer;
};

#endif