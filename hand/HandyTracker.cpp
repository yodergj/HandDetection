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
}

bool HandyTracker::SetOpenClassifier(AdaboostClassifier* classifier)
{
  if ( !classifier )
    return false;

  mOpenClassifier = classifier;
  return true;
}

bool HandyTracker::SetClosedClassifier(AdaboostClassifier* classifier)
{
  if ( !classifier )
    return false;

  mClosedClassifier = classifier;
  return true;
}

HandyTracker::HandState HandyTracker::GetLastState()
{
  return mStateHistory.back();
}

bool HandyTracker::AnalyzeRegion(ColorRegion* region)
{
  if ( !region || !mOpenClassifier || !mClosedClassifier )
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
    xVals[i] = (i * width) / 4 - 1;
    yVals[i] = (i * height) / 4 - 1;
  }

  int x, y;
  double blockArea = width * height / 16.0;
  for (i = 0; i < 4; i++)
  {
    y = yVals[i];
    for (j = 0; j < 4; j++)
    {
      x = xVals[j];
      int pixelCount = integralBuffer[y * width + x];
      if ( i > 0 )
        pixelCount -= integralBuffer[yVals[i - 1] * width + x];
      if ( j > 0 )
        pixelCount -= integralBuffer[y * width + xVals[j - 1]];
      if ( (i > 0) && (j > 0) )
        pixelCount += integralBuffer[yVals[i - 1] * width + xVals[j - 1]];
      featureData.SetValue(i * 4 + j, 0, pixelCount / blockArea);
    }
  }

  double aspectRatio = width / (double)height;
  featureData.SetValue(16, 0, aspectRatio);

  int openResult = mOpenClassifier->Classify(featureData);
  int closedResult = mClosedClassifier->Classify(featureData);

  mRegionHistory.push_back(region);
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