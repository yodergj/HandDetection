#ifndef HAND_CANDIDATE_H
#define HAND_CANDIDATE_H

#include <string>
using std::string;
#include <DoublePoint.h>
#include <LineSegment.h>
#include <Rect.h>
class ConnectedRegion;
class Matrix;

#define NUM_HAND_SECTIONS 8

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
    bool GetFarPoints(vector<Point>& farPoints);
  private:
    Point GetClosestPoint(double x, double y, const vector<Point> points) const;
    Point GetClosestEdge(double x, double y);
    Point GetFarthestPoint(double x, double y, const vector<Point> points) const;
    Point GetFarthestEdge(double x, double y);
    bool GetBaseFeatures();
    static bool PointBetweenLines(const DoublePoint& pt, const LineSegment& line1, const LineSegment& line2);
    static bool PointBetweenLines(const Point& pt, const LineSegment& line1, const LineSegment& line2);

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
    Point mFarPoints[NUM_HAND_SECTIONS];
    LineSegment mFarPointLines[NUM_HAND_SECTIONS];
    bool mFeaturesGenerated;
};

#endif
