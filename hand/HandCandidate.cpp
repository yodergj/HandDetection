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
  double boxAspect = 0;
  double boxFillRatio = 0;

  numFeatures = featureStr.size();
  features.SetSize(numFeatures, 1);

  if ( !mFeaturesGenerated && !GetBaseFeatures() )
  {
    fprintf(stderr, "HandCandidate::GetFeatureVector - Failed getting features\n");
    retVal = false;
  }

  if ( retVal && featureStr.find_first_of("bB") != string::npos )
  {
    Rect boundingRect = GetAngledBoundingBox(mLongLine);
    boxAspect = boundingRect.GetAspectRatio();
    boxFillRatio = mRegion->GetNumPixels() / boundingRect.GetArea();
  }

  for (i = 0; retVal && i < numFeatures; i++)
  {
    switch (featureStr[i])
    {
      case 'e':
        features.SetValue(i, 0, mEdgeAngle);
        break;
      case 'E':
        features.SetValue(i, 0, mLongLine.GetLength() / mShortLine.GetLength());
        break;
      case 'c':
        features.SetValue(i, 0, mOffsetAngle);
        break;
      case 'C':
        features.SetValue(i, 0, mOffsetLine.GetLength() / mLongLine.GetLength());
        break;
      case 'b':
        features.SetValue(i, 0, boxAspect);
        break;
      case 'B':
        features.SetValue(i, 0, boxFillRatio);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
        features.SetValue(i, 0, mFarPointLines[featureStr[i] - '0'].GetLength() / mLongLine.GetLength());
        break;
      case 'z':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[1]));
        break;
      case 'a':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[2]));
        break;
      case 'q':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[3]));
        break;
      case 'Z':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[4]));
        break;
      case 'A':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[5]));
        break;
      case 'Q':
        features.SetValue(i, 0, mLongLine.GetAngleToLineRad(mFarPointLines[6]));
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

bool HandCandidate::GetFarPoints(vector<Point>& farPoints)
{
  int i;

  if ( !mFeaturesGenerated && !GetBaseFeatures() )
  {
    fprintf(stderr, "HandCandidate::GetFarPoints - Failed getting features\n");
    return false;
  }

  // First and the last aren't actually calculated
  for (i = 1; i < NUM_HAND_SECTIONS - 1; i++)
    farPoints.push_back(mFarPoints[i]);

  return true;
}

Point HandCandidate::GetClosestPoint(double x, double y, const vector<Point> points) const
{
  int i, numPoints, bestPoint;
  double distance, bestDistance;

  if ( points.empty() )
  {
    fprintf(stderr, "HandCandidate::GetClosestEdge - No points given\n");
    return Point();
  }

  bestPoint = 0;
  bestDistance = sqrt( (points[0].x - x) * (points[0].x - x) +
                       (points[0].y - y) * (points[0].y - y) );
  numPoints = (int)points.size();
  for (i = 1; i < numPoints; i++)
  {
    distance = sqrt( (points[i].x - x) * (points[i].x - x) +
                     (points[i].y - y) * (points[i].y - y) );
    if ( distance < bestDistance )
    {
      bestPoint = i;
      bestDistance = distance;
    }
  }

  return points[bestPoint];
}

Point HandCandidate::GetClosestEdge(double x, double y)
{
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

  return GetClosestPoint(x, y, mEdgePoints);
}

Point HandCandidate::GetFarthestPoint(double x, double y, const vector<Point> points) const
{
  int i, numPoints, bestPoint;
  double distance, bestDistance;

  if ( points.empty() )
  {
    fprintf(stderr, "HandCandidate::GetFarthestEdge - No points given\n");
    return Point();
  }

  bestPoint = 0;
  bestDistance = sqrt( (points[0].x - x) * (points[0].x - x) +
                       (points[0].y - y) * (points[0].y - y) );
  numPoints = (int)points.size();
  for (i = 1; i < numPoints; i++)
  {
    distance = sqrt( (points[i].x - x) * (points[i].x - x) +
                     (points[i].y - y) * (points[i].y - y) );
    if ( distance > bestDistance )
    {
      bestPoint = i;
      bestDistance = distance;
    }
  }

  return points[bestPoint];
}

Point HandCandidate::GetFarthestEdge(double x, double y)
{
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

  return GetFarthestPoint(x, y, mEdgePoints);
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

  bool pointAssigned;
  int i, j, numPoints;
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

  LineSegment refLines[NUM_HAND_SECTIONS];
  vector<Point> wedgeEdges[NUM_HAND_SECTIONS];
  double angleFraction = 2.0 / NUM_HAND_SECTIONS;
  for (i = 0; i < NUM_HAND_SECTIONS; i++)
  {
    refLines[i] = mLongLine;
    refLines[i].RotateAroundEnd(i * angleFraction * M_PI);
  }
  numPoints = mEdgePoints.size();
  for (i = 0; i < numPoints; i++)
  {
    pointAssigned = false;
    for (j = 0; !pointAssigned && j < NUM_HAND_SECTIONS; j++)
      if ( PointBetweenLines(mEdgePoints[i], refLines[j], refLines[(j + 1) % NUM_HAND_SECTIONS]) )
      {
        wedgeEdges[j].push_back(mEdgePoints[i]);
        pointAssigned = true;
      }
  }
  // Points from the first and last section are useless since they're almost identical to the mLongLine
  DoublePoint tmpPt;
  for (i = 1; i < NUM_HAND_SECTIONS - 1; i++)
  {
    mFarPoints[i] = GetFarthestPoint(mCentroid.x, mCentroid.y, wedgeEdges[i]);
    tmpPt = mFarPoints[i];
    mFarPointLines[i] = LineSegment(mCentroid, tmpPt);
  }

  mFeaturesGenerated = true;

  return true;
}

bool HandCandidate::PointBetweenLines(const DoublePoint& pt, const LineSegment& line1, const LineSegment& line2)
{
  double cross12, cross1pt, cross2pt;

  if ( line1.GetFirstPoint() != line2.GetFirstPoint() )
  {
    fprintf(stderr, "HandCandidate::PointBetweenLines - Line starting points are not equal\n");
    return false;
  }

  LineSegment ptLine( line1.GetFirstPoint(), pt );
  DoublePoint vector1 = line1.GetVector();
  DoublePoint vector2 = line2.GetVector();
  DoublePoint ptVector = ptLine.GetVector();

  cross12 = vector1.x * vector2.y - vector1.y * vector2.x;
  cross1pt = vector1.x * ptVector.y - vector1.y * ptVector.x;
  cross2pt = vector2.x * ptVector.y - vector2.y * ptVector.x;

  if ( cross12 > 0 )
    return (cross1pt > 0) && (cross2pt < 0);
  return (cross1pt < 0) && (cross2pt > 0);
}

bool HandCandidate::PointBetweenLines(const Point& pt, const LineSegment& line1, const LineSegment& line2)
{
  DoublePoint dblPt(pt);
  return PointBetweenLines(dblPt, line1, line2);
}