#ifndef HAND_CANDIDATE_H
#define HAND_CANDIDATE_H

#include <string>
using std::string;
#include <DoublePoint.h>
#include <LineSegment.h>
#include <Rect.h>
class ConnectedRegion;
class Matrix;

class HandCandidate
{
  public:
    HandCandidate(ConnectedRegion* region);
    HandCandidate(const HandCandidate& ref);
    HandCandidate& operator=(const HandCandidate& ref);
    ~HandCandidate();
    bool GetFeatureVector(const string& featureStr, Matrix& features);
    bool GetScaledFeatures(int xScale, int yScale, DoublePoint& centroid, DoublePoint& center,
                           DoublePoint& nearEdge, DoublePoint& farEdge,
                           LineSegment& shortLine, LineSegment& longLine, LineSegment& offsetLine,
                           double& edgeAngle, double& offsetAngle);
    Rect GetAngledBoundingBox(const LineSegment& line);
  private:
    Point GetClosestEdge(double x, double y);
    Point GetFarthestEdge(double x, double y);
    bool GetBaseFeatures();

    ConnectedRegion* mRegion;
    DoublePoint mCentroid;
    DoublePoint mCenter;
    DoublePoint mNearEdge;
    DoublePoint mFarEdge;
    LineSegment mShortLine;
    LineSegment mLongLine;
    LineSegment mOffsetLine;
    double mEdgeAngle;
    double mOffsetAngle;
    vector<Point> mEdgePoints;
    bool mFeaturesGenerated;
};

#endif
