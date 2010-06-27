#ifndef HAND_CANDIDATE_H
#define HAND_CANDIDATE_H

#include <DoublePoint.h>
#include <LineSegment.h>
class ConnectedRegion;

class HandCandidate
{
  public:
    HandCandidate(ConnectedRegion* region);
    HandCandidate(const HandCandidate& ref);
    HandCandidate& operator=(const HandCandidate& ref);
    ~HandCandidate();
    bool GetScaledFeatures(int xScale, int yScale, DoublePoint& centroid,
                           DoublePoint& nearEdge, DoublePoint& farEdge,
                           LineSegment& shortLine, LineSegment& longLine, double& angle);
  private:
    Point GetClosestEdge(double x, double y);
    Point GetFarthestEdge(double x, double y);
    bool GetBaseFeatures();

    ConnectedRegion* mRegion;
    DoublePoint mCentroid;
    DoublePoint mNearEdge;
    DoublePoint mFarEdge;
    LineSegment mShortLine;
    LineSegment mLongLine;
    double mAngle;
    vector<Point> mEdgePoints;
    bool mFeaturesGenerated;
};

#endif
