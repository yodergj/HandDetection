#ifndef _CONNECTED_REGION_H
#define _CONNECTED_REGION_H

#include <vector>
using std::vector;

typedef struct
{
  int xStart;
  int y;
  int length;
} PixelRun;

class ConnectedRegion
{
  public:
    ConnectedRegion();
    ~ConnectedRegion();
    void AddRun(int xstart, int y, int length);
    bool MergeInRegion(ConnectedRegion& refRegion);
    bool TouchesRegion(ConnectedRegion& refRegion);
  private:
    vector<PixelRun> mRuns;
    int mXMin;
    int mYMin;
    int mXMax;
    int mYMax;
};

#endif
