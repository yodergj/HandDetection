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
    bool TouchesRegion(ConnectedRegion& refRegion) const;
    bool GetBounds(int& left, int& right, int& top, int& bottom) const;
    int GetNumPixels() const;
    double GetDensity() const;
    double GetAverageRunsPerRow() const;
    bool GetCentroid(double& x, double& y) const;
    void GetEdgePoints(vector<Point>& points);
    bool HasMorePixels(const ConnectedRegion& ref) const;
    bool HasLargerBoundingBox(const ConnectedRegion& ref) const;
  private:
    vector<PixelRun> mRuns;
    int mXMin;
    int mYMin;
    int mXMax;
    int mYMax;
    int mNumPixels;
};

#endif
