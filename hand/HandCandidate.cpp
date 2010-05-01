#include <math.h>
#include <ConnectedRegion.h>
#include <Line.h>
#include "HandCandidate.h"

HandCandidate::HandCandidate(ConnectedRegion* region)
{
  mRegion = region;
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
  mEdgePoints = ref.mEdgePoints;

  return *this;
}

HandCandidate::~HandCandidate()
{
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
  DoublePoint nearEdge, farEdge;
  Line shortLine, longLine;
  double angle;

  if ( !mRegion )
    return false;

  if ( !mRegion->GetCentroid(mCentroid.x, mCentroid.y) )
    return false;

  mRegion->GetEdgePoints(mEdgePoints);

  nearEdge = GetClosestEdge(mCentroid.x, mCentroid.y);
  farEdge = GetFarthestEdge(mCentroid.x, mCentroid.y);
  shortLine = Line(mCentroid, nearEdge);
  longLine = Line(mCentroid, farEdge);
  angle = longLine.GetInnerAngle(shortLine);

  return true;
}
