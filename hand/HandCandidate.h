#ifndef HAND_CANDIDATE_H
#define HAND_CANDIDATE_H

#include <DoublePoint.h>
class ConnectedRegion;

class HandCandidate
{
  public:
    HandCandidate(ConnectedRegion* region);
    HandCandidate(const HandCandidate& ref);
    HandCandidate& operator=(const HandCandidate& ref);
    ~HandCandidate();
    Point GetClosestEdge(double x, double y);
    Point GetFarthestEdge(double x, double y);
  private:
    bool GetBaseFeatures();

    ConnectedRegion* mRegion;
    DoublePoint mCentroid;
    vector<Point> mEdgePoints;
};

#endif
