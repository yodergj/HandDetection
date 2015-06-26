#include "HandyTracker.h"
#include "ColorRegion.h"
#include "AdaboostClassifier.h"

HandyTracker::HandyTracker()
{
  mOpenClassifier = 0;
  mClosedClassifier = 0;
}

HandyTracker::~HandyTracker()
{
  for (size_t i = 0; i < mRegionHistory.size(); i++)
    delete mRegionHistory[i];
  delete mOpenClassifier;
  delete mClosedClassifier;
}

bool HandyTracker::SetOpenClassifier(AdaboostClassifier* classifier)
{
  if ( !classifier )
    return false;

  if ( mOpenClassifier )
    delete mOpenClassifier;

  mOpenClassifier = classifier;
  return true;
}

bool HandyTracker::SetClosedClassifier(AdaboostClassifier* classifier)
{
  if ( !classifier )
    return false;

  if ( mClosedClassifier )
    delete mClosedClassifier;

  mClosedClassifier = classifier;
  return true;
}

HandyTracker::HandState HandyTracker::GetLastState()
{
  return mStateHistory.back();
}

HandyTracker::HandState HandyTracker::GetState(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mStateHistory.size()) )
    return ST_UNKNOWN;
  return mStateHistory[frameNumber];
}

ColorRegion* HandyTracker::GetRegion(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mRegionHistory.size()) )
    return 0;
  return mRegionHistory[frameNumber];
}

Matrix* HandyTracker::GetFeatureData(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mFeatureHistory.size()) )
    return 0;
  return &mFeatureHistory[frameNumber];
}

bool HandyTracker::AnalyzeRegion(ColorRegion* region)
{
  if ( !region )
    return false;

  int* integralBuffer = region->GetIntegralBuffer();
  int width = region->GetWidth();
  int height = region->GetHeight();

  Matrix featureData;
  featureData.SetSize(17, 1);

  int i, j;
  int xVals[4], yVals[4];
  for (i = 0; i < 4; i++)
  {
    xVals[i] = ( (i + 1) * width) / 4 - 1;
    yVals[i] = ( (i + 1) * height) / 4 - 1;
  }

  int x, y, blockWidth, blockHeight;
  for (i = 0; i < 4; i++)
  {
    y = yVals[i];
    if ( i == 0 )
      blockHeight = y;
    else
      blockHeight = y - yVals[i - 1];
    for (j = 0; j < 4; j++)
    {
      x = xVals[j];
      if ( j == 0 )
        blockWidth = x;
      else
        blockWidth = x - xVals[j - 1];
      int pixelCount = integralBuffer[y * width + x];
      if ( i > 0 )
        pixelCount -= integralBuffer[yVals[i - 1] * width + x];
      if ( j > 0 )
        pixelCount -= integralBuffer[y * width + xVals[j - 1]];
      if ( (i > 0) && (j > 0) )
        pixelCount += integralBuffer[yVals[i - 1] * width + xVals[j - 1]];
      double blockArea = blockWidth * blockHeight;
      featureData.SetValue(i * 4 + j, 0, pixelCount / blockArea);
    }
  }

  double aspectRatio = width / (double)height;
  featureData.SetValue(16, 0, aspectRatio);

  int openResult = ST_UNKNOWN;
  if ( mOpenClassifier )
    openResult = mOpenClassifier->Classify(featureData);
  int closedResult = ST_UNKNOWN;
  if ( mClosedClassifier )
    closedResult = mClosedClassifier->Classify(featureData);

  mRegionHistory.push_back(region);
  mFeatureHistory.push_back(featureData);
  if ( openResult )
  {
    if ( closedResult )
      mStateHistory.push_back(ST_CONFLICT);
    else
      mStateHistory.push_back(ST_OPEN);
  }
  else if ( closedResult )
    mStateHistory.push_back(ST_CLOSED);
  else
    mStateHistory.push_back(ST_UNKNOWN);

  return true;
}