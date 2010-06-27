#include <stdio.h>
#include <math.h>
#include <ConnectedRegion.h>
#include <LineSegment.h>
#include "HandCandidate.h"

HandCandidate::HandCandidate(ConnectedRegion* region)
{
  mRegion = region;
  mAngle = 0;
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
  mNearEdge = ref.mNearEdge;
  mFarEdge = ref.mFarEdge;
  mShortLine = ref.mShortLine;
  mLongLine = ref.mLongLine;
  mEdgePoints = ref.mEdgePoints;
  mFeaturesGenerated = ref.mFeaturesGenerated;

  return *this;
}

HandCandidate::~HandCandidate()
{
}

bool HandCandidate::GetScaledFeatures(int xScale, int yScale, DoublePoint& centroid,
                                      DoublePoint& nearEdge, DoublePoint& farEdge,
                                      LineSegment& shortLine, LineSegment& longLine, double& angle)
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
  nearEdge.x = xScale * mNearEdge.x;
  nearEdge.y = yScale * mNearEdge.y;
  farEdge.x = xScale * mFarEdge.x;
  farEdge.y = yScale * mFarEdge.y;
  shortLine = mShortLine;
  shortLine.Scale(xScale, yScale);
  longLine = mLongLine;
  longLine.Scale(xScale, yScale);
  if ( xScale == yScale )
    angle = mAngle;
  else
    angle = longLine.GetInnerAngleDeg(shortLine);

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

bool HandCandidate::GetBaseFeatures()
{
  if ( !mRegion )
    return false;

  if ( !mRegion->GetCentroid(mCentroid.x, mCentroid.y) )
    return false;

  mRegion->GetEdgePoints(mEdgePoints);

  mNearEdge = GetClosestEdge(mCentroid.x, mCentroid.y);
  mFarEdge = GetFarthestEdge(mCentroid.x, mCentroid.y);
  mShortLine = LineSegment(mCentroid, mNearEdge);
  mLongLine = LineSegment(mCentroid, mFarEdge);
  mLongLine.GetInnerAngleRad(mShortLine);
  mAngle = mLongLine.GetInnerAngleDeg(mShortLine);
  mFeaturesGenerated = true;

  return true;
}
