#include "ConnectedRegion.h"
#include <stdio.h>

#ifndef MIN
#define MIN(a, b) ( (a) < (b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#endif

ConnectedRegion::ConnectedRegion()
{
  mXMin = -1;
  mYMin = -1;
  mXMax = -1;
  mYMax = -1;
  mNumPixels = 0;
}

ConnectedRegion::~ConnectedRegion()
{
}

void ConnectedRegion::AddRun(int xStart, int y, int length)
{
  PixelRun run;
  int rightEdge;

  run.xStart = xStart;
  run.y = y;
  run.length = length;
  mRuns.push_back(run);
  mNumPixels += length;

  rightEdge = xStart + length - 1;

  if ( (mXMin == -1) || (xStart < mXMin) )
    mXMin = xStart;
  if ( rightEdge > mXMax )
    mXMax = rightEdge;

  if ( (mYMin == -1) || (y < mYMin) )
    mYMin = y;
  if ( y > mYMax )
    mYMax = y;
}

bool ConnectedRegion::MergeInRegion(ConnectedRegion& refRegion)
{
  int i, numRuns;

  if ( &refRegion == this )
    return false;

  mNumPixels += refRegion.mNumPixels;
  numRuns = refRegion.mRuns.size();

  if ( numRuns == 0 )
    return true;

  for (i = 0; i < numRuns; i++)
    mRuns.push_back(refRegion.mRuns[i]);

  if ( refRegion.mXMin < mXMin )
    mXMin = refRegion.mXMin;
  if ( refRegion.mXMax > mXMax )
    mXMax = refRegion.mXMax;

  if ( refRegion.mYMin < mYMin )
    mYMin = refRegion.mYMin;
  if ( refRegion.mYMax > mYMax )
    mYMax = refRegion.mYMax;

  return true;
}

bool ConnectedRegion::TouchesRegion(ConnectedRegion& refRegion)
{
  int i, j, numRuns, numRefRuns, leftEdge, rightEdge, y;
  int xMin, yMin, xMax, yMax, refLeft, refRight;

  if ( &refRegion == this )
    return true;

  xMin = refRegion.mXMin - 1;
  xMax = refRegion.mXMax + 1;
  yMin = refRegion.mYMin - 1;
  yMax = refRegion.mYMax + 1;

  if ( (mXMin > xMax) || (mXMax < xMin) || (mYMin > yMax) || (mYMax < yMin) )
    return false;

  numRuns = mRuns.size();
  numRefRuns = refRegion.mRuns.size();

  if ( (numRuns == 0) || (numRefRuns == 0) )
    return false;

  for (i = 0; i < numRuns; i++)
  {
    y = mRuns[i].y;

    if ( (y < yMin) || (y > yMax) )
      continue;

    leftEdge = mRuns[i].xStart;
    rightEdge = leftEdge + mRuns[i].length - 1;
    if ( (leftEdge > xMax) || (rightEdge < xMin) )
      continue;

    for (j = 0; j < numRefRuns; j++)
    {
      if ( (refRegion.mRuns[j].y > y + 1) || (refRegion.mRuns[j].y < y - 1) )
        continue;

      refLeft = refRegion.mRuns[j].xStart;
      refRight = refLeft + refRegion.mRuns[j].length - 1;
      if ( (refLeft > rightEdge + 1) || (refRight < leftEdge - 1) )
        continue;

      return true;
    }
  }

  return false;
}

bool ConnectedRegion::GetBounds(int& left, int& right, int& top, int& bottom)
{
  if ( mXMax == -1 )
    return false;

  left = mXMin;
  right = mXMax;
  top = mYMin;
  bottom = mYMax;
  
  return true;
}

double ConnectedRegion::GetDensity()
{
  int area;

  area = (mXMax - mXMin + 1) * (mYMax - mYMin + 1);
  if ( area == 0 )
    return 0;

  return (double)mNumPixels / area;
}

double ConnectedRegion::GetAverageRunsPerRow()
{
  int height;

  height = mYMax - mYMin + 1;
  if ( height == 0 )
    return 0;

  return (double)mRuns.size() / height;
}
