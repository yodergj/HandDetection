#ifndef _CONNECTED_REGION_H
#define _CONNECTED_REGION_H

#include <vector>
using std::vector;
#include <Point.h>

class PixelRun
{
  public:
    int xStart;
    int y;
    int length;

    bool operator<(const PixelRun& ref) const
    {
      return (y == ref.y) ? (xStart < ref.xStart) : (y < ref.y);
    }
};

class ConnectedRegion
{
  public:
    ConnectedRegion();
    ConnectedRegion(const ConnectedRegion& ref);
    ConnectedRegion& operator=(const ConnectedRegion& ref);
    ~ConnectedRegion();
    void AddRun(int xstart, int y, int length);
    bool MergeInRegion(ConnectedRegion& refRegion);
    bool TouchesRegion(ConnectedRegion& refRegion);
    bool GetBounds(int& left, int& right, int& top, int& bottom);
    double GetDensity();
    double GetAverageRunsPerRow();
    bool GetCentroid(double& x, double& y);
    void GetEdgePoints(vector<Point>& points);
    bool HasMorePixels(const ConnectedRegion& ref);
    bool HasLargerBoundingBox(const ConnectedRegion& ref);
  private:
    vector<PixelRun> mRuns;
    int mXMin;
    int mYMin;
    int mXMax;
    int mYMax;
    int mNumPixels;
};

#endif
