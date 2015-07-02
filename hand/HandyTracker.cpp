#include "HandyTracker.h"
#include "ColorRegion.h"
#include "AdaboostClassifier.h"

int HandyTracker::mNumFeatures = 17;

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

void HandyTracker::ResetHistory()
{
  for (size_t i = 0; i < mRegionHistory.size(); i++)
    delete mRegionHistory[i];
  mRegionHistory.clear();
  mFeatureHistory.clear();
  mStateHistory.clear();
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

bool HandyTracker::GenerateFeatureData(ColorRegion* region, Matrix& featureData)
{
  if ( !region )
    return false;

  int* integralBuffer = region->GetIntegralBuffer();
  int width = region->GetWidth();
  int height = region->GetHeight();

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

  return true;
}

bool HandyTracker::AnalyzeRegion(ColorRegion* region)
{
  Matrix featureData;
  if ( !GenerateFeatureData(region, featureData) )
    return false;

  int i;
  std::string openFeatureStr = mOpenClassifier->GetFeatureString();
  int openSize = (int)openFeatureStr.size();
  Matrix openInput;
  openInput.SetSize(openSize, 1);
  for (i = 0; i < openSize; i++)
    openInput.SetValue(i, 0, featureData.GetValue(openFeatureStr[i] - 'a', 0) );

  std::string closedFeatureStr = mClosedClassifier->GetFeatureString();
  int closedSize = (int)closedFeatureStr.size();
  Matrix closedInput;
  closedInput.SetSize(closedSize, 1);
  for (i = 0; i < closedSize; i++)
    closedInput.SetValue(i, 0, featureData.GetValue(closedFeatureStr[i] - 'a', 0) );

  // Classifiers return 0 for the first class and 1 for the second class (a leftover from when I intended to extend the classifiers for more than 2 classes each)
  int openResult = -1;
  if ( mOpenClassifier )
    openResult = mOpenClassifier->Classify(openInput);
  int closedResult = -1;
  if ( mClosedClassifier )
    closedResult = mClosedClassifier->Classify(closedInput);

  mRegionHistory.push_back(region);
  mFeatureHistory.push_back(featureData);

  if ( openResult < 0 )
  {
    if ( closedResult == 0 )
      mStateHistory.push_back(ST_CLOSED);
    else
      mStateHistory.push_back(ST_UNKNOWN);
  }
  else if ( closedResult < 0 )
  {
    if ( openResult == 0 )
      mStateHistory.push_back(ST_OPEN);
    else
      mStateHistory.push_back(ST_UNKNOWN);
  }
  else if ( openResult == 0 )
  {
    if ( closedResult == 0 )
      mStateHistory.push_back(ST_CONFLICT);
    else
      mStateHistory.push_back(ST_OPEN);
  }
  else if ( closedResult == 0 )
    mStateHistory.push_back(ST_CLOSED);
  else
    mStateHistory.push_back(ST_UNKNOWN);

  return true;
}