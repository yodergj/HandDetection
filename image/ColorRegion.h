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
  bool ContainsPixel(int x, int y) const;

  bool Grow(Image& image, const Point& startPt);
  bool TrackFromOldRegion(Image& image, const ColorRegion& oldRegion);

  int GetMinX() { return mMinX; };
  int GetMinY() { return mMinY; };
  int GetMaxX() { return mMaxX; };
  int GetMaxY() { return mMaxY; };
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
  void FreeIntegralBuffer();

  int GetReferenceHeight() { return mRefHeight; };
  bool SetReferenceHeight(int refHeight);
private:
  void GenerateIntegralBuffer();
  bool ColorMatches(int R, int G, int B) const;
  bool ColorMatches(unsigned char* rgbVals) const;
  std::set<Point> mPoints;

  int mMinX, mMaxX, mMinY, mMaxY;
  Point mCentroid;

  double mMeanR, mMeanG, mMeanB;
  int mMinR, mMaxR;
  int mMinG, mMaxG;
  int mMinB, mMaxB;

  double mMinH, mMaxH, mMeanH;
  double mMinS, mMaxS, mMeanS;
  int mPixelsInMean;

  int mStaleResultCount;

  int* mIntegralBuffer;

  int mRefHeight;

  static const int mColorTolerance;
  static const double mChannelTolerance;
  static const double mHueTolerance;
  static const double mSaturationTolerance;
  static const double mWeakSaturationThreshold;
  static const double mLowValueThreshold;
};

#endif