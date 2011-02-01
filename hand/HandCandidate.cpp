#include <stdio.h>
#include <math.h>
#include <ConnectedRegion.h>
#include <LineSegment.h>
#include <Matrix.h>
#include "HandCandidate.h"

HandCandidate::HandCandidate(ConnectedRegion* region)
{
  mRegion = region;
  mEdgeAngle = 0;
  mOffsetAngle = 0;
  mFeaturesGenerated = false;
}

HandCandidate::HandCandidate(const HandCandidate& ref)
{
  operator=(ref);
}

HandCandidate& HandCandidate::operator=(const HandCandidate& ref)
{
  if ( ref.mRegion )
    mRegion = new ConnectedRegion(*ref.mRegion);
  else
    mRegion = 0;
  mCentroid = ref.mCentroid;
  mCenter = ref.mCenter;
  mNearEdge = ref.mNearEdge;
  mFarEdge = ref.mFarEdge;
  mShortLine = ref.mShortLine;
  mLongLine = ref.mLongLine;
  mOffsetLine = ref.mOffsetLine;
  mEdgeAngle = ref.mEdgeAngle;
  mOffsetAngle = ref.mOffsetAngle;
  mEdgePoints = ref.mEdgePoints;
  mFeaturesGenerated = ref.mFeaturesGenerated;

  return *this;
}

HandCandidate::~HandCandidate()
{
}

bool HandCandidate::GetFeatureVector(const string& featureStr, Matrix& features)
{
  int i, numFeatures;
  bool retVal = true;
  DoublePoint centroid, center, nearEdge, farEdge;
  LineSegment shortLine, longLine, offsetLine;
  double edgeAngle, offsetAngle;
  double boxAspect = 0;
  double boxFillRatio = 0;

  numFeatures = featureStr.size();
  features.SetSize(numFeatures, 1);

  retVal = GetScaledFeatures(1, 1, centroid, center, nearEdge, farEdge,
                             shortLine, longLine, offsetLine, edgeAngle, offsetAngle);
  if ( retVal && featureStr.find_first_of("bB") != string::npos )
  {
    Rect boundingRect = GetAngledBoundingBox(longLine);
    boxAspect = boundingRect.GetAspectRatio();
    boxFillRatio = mRegion->GetNumPixels() / boundingRect.GetArea();
  }
  for (i = 0; retVal && i < numFeatures; i++)
  {
    switch (featureStr[i])
    {
      case 'e':
        features.SetValue(i, 0, edgeAngle);
        break;
      case 'E':
        features.SetValue(i, 0, longLine.GetLength() / shortLine.GetLength());
        break;
      case 'c':
        features.SetValue(i, 0, offsetAngle);
        break;
      case 'C':
        features.SetValue(i, 0, offsetLine.GetLength() / longLine.GetLength());
        break;
      case 'b':
        features.SetValue(i, 0, boxAspect);
        break;
      case 'B':
        features.SetValue(i, 0, boxFillRatio);
        break;
    }
  }

  return retVal;
}

bool HandCandidate::GetScaledFeatures(int xScale, int yScale,
                                      DoublePoint& centroid, DoublePoint& center,
                                      DoublePoint& nearEdge, DoublePoint& farEdge,
                                      LineSegment& shortLine, LineSegment& longLine,
                                      LineSegment& offsetLine,
                                      double& edgeAngle, double& offsetAngle)
{
  if ( (xScale < 0) || (yScale < 0) )
  {
    fprintf(stderr, "HandCandidate::GetScaledFeatures - Invalid parameter\n");
    return false;
  }

  if ( !mFeaturesGenerated && !GetBaseFeatures() )
  {
    fprintf(stderr, "HandCandidate::GetScaledFeatures - Failed getting features\n");
    return false;
  }

  centroid.x = xScale * mCentroid.x;
  centroid.y = yScale * mCentroid.y;
  center.x = xScale * mCenter.x;
  center.y = yScale * mCenter.y;
  nearEdge.x = xScale * mNearEdge.x;
  nearEdge.y = yScale * mNearEdge.y;
  farEdge.x = xScale * mFarEdge.x;
  farEdge.y = yScale * mFarEdge.y;
  shortLine = mShortLine;
  shortLine.Scale(xScale, yScale);
  longLine = mLongLine;
  longLine.Scale(xScale, yScale);
  offsetLine = mOffsetLine;
  offsetLine.Scale(xScale, yScale);
  if ( xScale == yScale )
  {
    edgeAngle = mEdgeAngle;
    offsetAngle = mOffsetAngle;
  }
  else
  {
    edgeAngle = longLine.GetInnerAngleDeg(shortLine);
    offsetAngle = longLine.GetInnerAngleDeg(offsetLine);
  }

  return true;
}

Point HandCandidate::GetClosestEdge(double x, double y)
{
  int i, numPoints, bestPoint;
  double distance, bestDistance;

  if ( mEdgePoints.empty() )
  {
    if ( !mRegion )
      return Point();
    mRegion->GetEdgePoints(mEdgePoints);
    if ( mEdgePoints.empty() )
    {
      fprintf(stderr, "HandCandidate::GetClosestEdge - Failed getting edge points\n");
      return Point();
    }
  }

  bestPoint = 0;
  bestDistance = sqrt( (mEdgePoints[0].x - x) * (mEdgePoints[0].x - x) +
                       (mEdgePoints[0].y - y) * (mEdgePoints[0].y - y) );
  numPoints = (int)mEdgePoints.size();
  for (i = 1; i < numPoints; i++)
  {
    distance = sqrt( (mEdgePoints[i].x - x) * (mEdgePoints[i].x - x) +
                     (mEdgePoints[i].y - y) * (mEdgePoints[i].y - y) );
    if ( distance < bestDistance )
    {
      bestPoint = i;
      bestDistance = distance;
    }
  }

  return mEdgePoints[bestPoint];
}

Point HandCandidate::GetFarthestEdge(double x, double y)
{
  int i, numPoints, bestPoint;
  double distance, bestDistance;

  if ( mEdgePoints.empty() )
  {
    if ( !mRegion )
      return Point();
    mRegion->GetEdgePoints(mEdgePoints);
    if ( mEdgePoints.empty() )
    {
      fprintf(stderr, "HandCandidate::GetFarthestEdge - Failed getting edge points\n");
      return Point();
    }
  }

  bestPoint = 0;
  bestDistance = sqrt( (mEdgePoints[0].x - x) * (mEdgePoints[0].x - x) +
                       (mEdgePoints[0].y - y) * (mEdgePoints[0].y - y) );
  numPoints = (int)mEdgePoints.size();
  for (i = 1; i < numPoints; i++)
  {
    distance = sqrt( (mEdgePoints[i].x - x) * (mEdgePoints[i].x - x) +
                     (mEdgePoints[i].y - y) * (mEdgePoints[i].y - y) );
    if ( distance > bestDistance )
    {
      bestPoint = i;
      bestDistance = distance;
    }
  }

  return mEdgePoints[bestPoint];
}

Rect HandCandidate::GetAngledBoundingBox(const LineSegment& line)
{
  int i, numPoints;
  double angle = line.GetAngleRad();
  double cosVal = cos(-angle);
  double sinVal = sin(-angle);
  double xMin, xMax, yMin, yMax;
  double x, y;
  DoublePoint a, b, c, d;

  if ( mEdgePoints.empty() )
  {
    if ( !mRegion )
      return Rect();
    mRegion->GetEdgePoints(mEdgePoints);
    if ( mEdgePoints.empty() )
    {
      fprintf(stderr, "HandCandidate::GetAngledBoundingBox - Failed getting edge points\n");
      return Rect();
    }
  }

  xMin = mEdgePoints[0].x * cosVal - mEdgePoints[0].y * sinVal;
  xMax = xMin;
  yMin = mEdgePoints[0].y * cosVal + mEdgePoints[0].x * sinVal;
  yMax = yMin;
  numPoints = (int)mEdgePoints.size();
  for (i = 1; i < numPoints; i++)
  {
    x = mEdgePoints[i].x * cosVal - mEdgePoints[i].y * sinVal;
    y = mEdgePoints[i].y * cosVal + mEdgePoints[i].x * sinVal;
    if ( x < xMin )
      xMin = x;
    if ( x > xMax )
      xMax = x;
    if ( y < yMin )
      yMin = y;
    if ( y > yMax )
      yMax = y;
  }

  a.x = xMin * cosVal - yMin * -sinVal;
  a.y = yMin * cosVal + xMin * -sinVal;

  b.x = xMin * cosVal - yMax * -sinVal;
  b.y = yMax * cosVal + xMin * -sinVal;

  c.x = xMax * cosVal - yMax * -sinVal;
  c.y = yMax * cosVal + xMax * -sinVal;

  d.x = xMax * cosVal - yMin * -sinVal;
  d.y = yMin * cosVal + xMax * -sinVal;

  return Rect(a, b, c, d);
}

bool HandCandidate::GetBaseFeatures()
{
  if ( !mRegion )
    return false;

  if ( !mRegion->GetCentroid(mCentroid.x, mCentroid.y) )
    return false;

  int left, right, top, bottom;
  if ( !mRegion->GetBounds(left, right, top, bottom) )
    return false;
  mCenter.x = (left + right) / 2.0;
  mCenter.y = (top + bottom) / 2.0;

  mRegion->GetEdgePoints(mEdgePoints);

  mNearEdge = GetClosestEdge(mCentroid.x, mCentroid.y);
  mFarEdge = GetFarthestEdge(mCentroid.x, mCentroid.y);
  mShortLine = LineSegment(mCentroid, mNearEdge);
  mLongLine = LineSegment(mCentroid, mFarEdge);
  mOffsetLine = LineSegment(mCentroid, mCenter);
  mEdgeAngle = mLongLine.GetInnerAngleDeg(mShortLine);
  mOffsetAngle = mLongLine.GetInnerAngleDeg(mOffsetLine);
  mFeaturesGenerated = true;

  return true;
}
