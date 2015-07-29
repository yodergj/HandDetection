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
  mOpenFeatureHistory.clear();
  mClosedFeatureHistory.clear();
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

Matrix* HandyTracker::GetOpenFeatureData(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mOpenFeatureHistory.size()) )
    return 0;
  return &mOpenFeatureHistory[frameNumber];
}

Matrix* HandyTracker::GetClosedFeatureData(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mClosedFeatureHistory.size()) )
    return 0;
  return &mClosedFeatureHistory[frameNumber];
}

bool HandyTracker::GenerateFeatureData(ColorRegion* region, Matrix& featureData, int heightRestriction)
{
  if ( !region )
    return false;

  int* integralBuffer = region->GetIntegralBuffer();
  int width = region->GetWidth();
  int height = region->GetHeight();
  if ( (heightRestriction > 0) && (height > heightRestriction) )
    height = heightRestriction;

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

HandyTracker::HandState HandyTracker::CalculateStateResult(int openResult, int closedResult)
{
  if ( (openResult == 0) && (closedResult == 0) )
    return ST_CONFLICT;

  if ( openResult == 0 )
    return ST_OPEN;

  if ( closedResult == 0 )
    return ST_CLOSED;

  return ST_UNKNOWN;
}

bool HandyTracker::AnalyzeRegion(ColorRegion* region)
{
  if ( !region )
    return false;

  Matrix openFeatureData, closedFeatureData;
  int referenceHeight = region->GetReferenceHeight();
  if ( referenceHeight == 0 )
  {
    // No wrist compensation in play
    if ( !GenerateFeatureData(region, openFeatureData) )
      return false;

    closedFeatureData = openFeatureData;
  }
  else
  {
    if ( !GenerateFeatureData(region, openFeatureData, referenceHeight) )
      return false;

    if ( !GenerateFeatureData(region, closedFeatureData, referenceHeight / 2) )
      return false;
  }

  Matrix openInput;
  if ( mOpenClassifier )
  {
    std::string openFeatureStr = mOpenClassifier->GetFeatureString();
    int openSize = (int)openFeatureStr.size();
    openInput.SetSize(openSize, 1);
    for (int i = 0; i < openSize; i++)
      openInput.SetValue(i, 0, openFeatureData.GetValue(openFeatureStr[i] - 'a', 0) );
  }

  Matrix closedInput;
  if ( mClosedClassifier )
  {
    std::string closedFeatureStr = mClosedClassifier->GetFeatureString();
    int closedSize = (int)closedFeatureStr.size();
    closedInput.SetSize(closedSize, 1);
    for (int i = 0; i < closedSize; i++)
      closedInput.SetValue(i, 0, closedFeatureData.GetValue(closedFeatureStr[i] - 'a', 0) );
  }

  // Classifiers return 0 for the first class and 1 for the second class (a leftover from when I intended to extend the classifiers for more than 2 classes each)
  int openResult = -1;
  if ( mOpenClassifier )
    openResult = mOpenClassifier->Classify(openInput);
  int closedResult = -1;
  if ( mClosedClassifier )
    closedResult = mClosedClassifier->Classify(closedInput);

  mRegionHistory.push_back(region);
  mOpenFeatureHistory.push_back(openFeatureData);
  mClosedFeatureHistory.push_back(closedFeatureData);
  mStateHistory.push_back( CalculateStateResult(openResult, closedResult) );

  return true;
}

bool HandyTracker::AnalyzeRegionForInitialization(ColorRegion* region)
{
  if ( !region )
    return false;

  // If we're missing a classifier, just do the regular analysis
  if ( !mOpenClassifier || !mClosedClassifier )
    return AnalyzeRegion(region);

  // TODO Add reject capability as needed

  Matrix featureData;
  if ( !GenerateFeatureData(region, featureData) )
    return false;

  Matrix openInput;
  std::string openFeatureStr = mOpenClassifier->GetFeatureString();
  int openSize = (int)openFeatureStr.size();
  openInput.SetSize(openSize, 1);
  for (int i = 0; i < openSize; i++)
    openInput.SetValue(i, 0, featureData.GetValue(openFeatureStr[i] - 'a', 0) );
  int openResult = mOpenClassifier->Classify(openInput);

  Matrix closedInput;
  std::string closedFeatureStr = mClosedClassifier->GetFeatureString();
  int closedSize = (int)closedFeatureStr.size();
  closedInput.SetSize(closedSize, 1);
  for (int i = 0; i < closedSize; i++)
    closedInput.SetValue(i, 0, featureData.GetValue(closedFeatureStr[i] - 'a', 0) );
  int closedResult = mClosedClassifier->Classify(closedInput);

  HandState initialResult = CalculateStateResult(openResult, closedResult);
  double aspectRatio = featureData.GetValue(ASPECT_RATIO_INDEX, 0);
  const double avgOpenAspectRatio = .85;

  if ( (initialResult == ST_OPEN) || (aspectRatio >= avgOpenAspectRatio) )
  {
    mRegionHistory.push_back(region);
    mOpenFeatureHistory.push_back(featureData);
    mClosedFeatureHistory.push_back(featureData);
    mStateHistory.push_back(initialResult);
    return true;
  }

  int openHeight = (int)(region->GetWidth() * avgOpenAspectRatio);
  int closedHeight = openHeight / 2;

  Matrix openFeatureData;
  GenerateFeatureData(region, openFeatureData, openHeight);
  for (int i = 0; i < openSize; i++)
    openInput.SetValue(i, 0, openFeatureData.GetValue(openFeatureStr[i] - 'a', 0) );
  openResult = mOpenClassifier->Classify(openInput);

  Matrix closedFeatureData;
  GenerateFeatureData(region, closedFeatureData, closedHeight);
  for (int i = 0; i < closedSize; i++)
    closedInput.SetValue(i, 0, closedFeatureData.GetValue(closedFeatureStr[i] - 'a', 0) );
  closedResult = mClosedClassifier->Classify(closedInput);

  HandState retryResult = CalculateStateResult(openResult, closedResult);
  if ( retryResult == ST_OPEN )
    region->SetReferenceHeight(openHeight);

  mRegionHistory.push_back(region);
  mOpenFeatureHistory.push_back(openFeatureData);
  mClosedFeatureHistory.push_back(closedFeatureData);
  mStateHistory.push_back(retryResult);

  return true;
}