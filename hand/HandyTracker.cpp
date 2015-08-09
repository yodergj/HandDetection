#include "HandyTracker.h"
#include "ColorRegion.h"
#include "AdaboostClassifier.h"

int HandyTracker::mNumFeatures = GRID_DIM_SIZE * GRID_DIM_SIZE + 3;

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

void HandyTracker::PurgeRegion(int frameNumber)
{
  if ( (frameNumber < 0) || (frameNumber >= (int)mRegionHistory.size()) )
    return;
  delete mRegionHistory[frameNumber];
  mRegionHistory[frameNumber] = 0;
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
  int restrictedWidth = width;
  if ( (heightRestriction > 0) && (height > heightRestriction) )
  {
    height = heightRestriction;
    int* lineBuffer = &integralBuffer[height * width];
    while ( (restrictedWidth > 1) && ( lineBuffer[restrictedWidth - 2] == lineBuffer[restrictedWidth - 1] ) )
      restrictedWidth--;
  }
  region->SetFeatureWidth(restrictedWidth);

  featureData.SetSize(mNumFeatures, 1);

  int i, j;
  int xVals[GRID_DIM_SIZE], yVals[GRID_DIM_SIZE];
  for (i = 0; i < GRID_DIM_SIZE; i++)
  {
    xVals[i] = ( (i + 1) * restrictedWidth) / GRID_DIM_SIZE - 1;
    yVals[i] = ( (i + 1) * height) / GRID_DIM_SIZE - 1;
  }

  int x, y, blockWidth, blockHeight;
  for (i = 0; i < GRID_DIM_SIZE; i++)
  {
    y = yVals[i];
    if ( i == 0 )
      blockHeight = y;
    else
      blockHeight = y - yVals[i - 1];
    for (j = 0; j < GRID_DIM_SIZE; j++)
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
      featureData.SetValue(i * GRID_DIM_SIZE + j, 0, pixelCount / blockArea);
    }
  }

  double aspectRatio = restrictedWidth / (double)height;
  featureData.SetValue(ASPECT_RATIO_INDEX, 0, aspectRatio);

  x = xVals[0];
  int thumbY = -1;
  for (y = 0; (thumbY < 0) && (y < height); y++)
  {
    if ( integralBuffer[y * width + x] )
      thumbY = y;
  }
  featureData.SetValue(ASPECT_RATIO_INDEX + 1, 0, thumbY / (double)height);

  double fistCenterArea = (xVals[GRID_DIM_SIZE - 2] - xVals[0]) * (yVals[2] - yVals[0]);
  int centerPixels = integralBuffer[yVals[2] * width + xVals[GRID_DIM_SIZE - 2]]
                   - integralBuffer[yVals[0] * width + xVals[GRID_DIM_SIZE - 2]]
                   - integralBuffer[yVals[2] * width + xVals[0]]
                   + integralBuffer[yVals[0] * width + xVals[0]];
  featureData.SetValue(ASPECT_RATIO_INDEX + 2, 0, centerPixels / fistCenterArea);

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
#if 0
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
#else
  int stripWidth = 20;
  int* integralBuffer = region->GetIntegralBuffer();
  int width = region->GetWidth();
  int height = region->GetHeight();
  if ( integralBuffer[width - 1] - integralBuffer[width - stripWidth] == 0 )
  {
    int x, y;
    int stripY = 0;
    for (y = 1; y < height; y++)
    {
      if ( integralBuffer[(y + 1) * width - 1] - integralBuffer[(y + 1) * width - stripWidth] == 0 )
        stripY = y;
      else
        break;
    }
    if ( stripY > height / 4 )
    {
      x = width - stripWidth - 1;
      bool done = false;
      while ( !done )
      {
        int prevStripY = stripY;
        while ( (stripY >= 0) && (integralBuffer[stripY * width + x + stripWidth - 1] - integralBuffer[stripY * width + x] != 0) )
          stripY--;
        if ( prevStripY - stripY > stripWidth )
        {
          referenceHeight = prevStripY;
          region->SetReferenceHeight(referenceHeight);
          done = true;
        }
        else if ( stripY <= 0 )
          done = true;
        else
          x--;
      }
    }
  }

  if ( !GenerateFeatureData(region, openFeatureData) )
    return false;

  closedFeatureData = openFeatureData;
#endif

  Matrix openInput;
  if ( mOpenClassifier )
  {
    std::string openFeatureStr = mOpenClassifier->GetFeatureString();
    int openSize = (int)openFeatureStr.size();
    openInput.SetSize(openSize, 1);
    for (int i = 0; i < openSize; i++)
      openInput.SetValue(i, 0, openFeatureData.GetValue(openFeatureStr[i] - 'A', 0) );
  }

  Matrix closedInput;
  if ( mClosedClassifier )
  {
    std::string closedFeatureStr = mClosedClassifier->GetFeatureString();
    int closedSize = (int)closedFeatureStr.size();
    closedInput.SetSize(closedSize, 1);
    for (int i = 0; i < closedSize; i++)
      closedInput.SetValue(i, 0, closedFeatureData.GetValue(closedFeatureStr[i] - 'A', 0) );
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

  Matrix featureData;
  if ( !GenerateFeatureData(region, featureData) )
    return false;

  double aspectRatio = featureData.GetValue(ASPECT_RATIO_INDEX, 0);
  if ( aspectRatio > 1 )
  {
    mRegionHistory.push_back(region);
    mOpenFeatureHistory.push_back(featureData);
    mClosedFeatureHistory.push_back(featureData);
    mStateHistory.push_back(ST_REJECT);
    return true;
  }

  Matrix openInput;
  std::string openFeatureStr = mOpenClassifier->GetFeatureString();
  int openSize = (int)openFeatureStr.size();
  openInput.SetSize(openSize, 1);
  for (int i = 0; i < openSize; i++)
    openInput.SetValue(i, 0, featureData.GetValue(openFeatureStr[i] - 'A', 0) );
  int openResult = mOpenClassifier->Classify(openInput);

  Matrix closedInput;
  std::string closedFeatureStr = mClosedClassifier->GetFeatureString();
  int closedSize = (int)closedFeatureStr.size();
  closedInput.SetSize(closedSize, 1);
  for (int i = 0; i < closedSize; i++)
    closedInput.SetValue(i, 0, featureData.GetValue(closedFeatureStr[i] - 'A', 0) );
  int closedResult = mClosedClassifier->Classify(closedInput);

  HandState initialResult = CalculateStateResult(openResult, closedResult);
  const double avgOpenAspectRatio = .85;

#if 0
  if ( (initialResult == ST_OPEN) || (aspectRatio >= avgOpenAspectRatio) )
  {
    mRegionHistory.push_back(region);
    mOpenFeatureHistory.push_back(featureData);
    mClosedFeatureHistory.push_back(featureData);
    mStateHistory.push_back(initialResult);
    return true;
  }
#endif

#if 1
  int openHeight = 0;
  int stripWidth = 20;
  int* integralBuffer = region->GetIntegralBuffer();
  int width = region->GetWidth();
  int height = region->GetHeight();
  if ( integralBuffer[width - 1] - integralBuffer[width - stripWidth] == 0 )
  {
    int x, y;
    int stripY = 0;
    for (y = 1; y < height; y++)
    {
      if ( integralBuffer[(y + 1) * width - 1] - integralBuffer[(y + 1) * width - stripWidth] == 0 )
        stripY = y;
      else
        break;
    }
    if ( stripY > height / 4 )
    {
      x = width - stripWidth - 1;
      bool done = false;
      while ( !done )
      {
        int prevStripY = stripY;
        while ( (stripY >= 0) && (integralBuffer[stripY * width + x + stripWidth - 1] - integralBuffer[stripY * width + x] != 0) )
          stripY--;
        if ( prevStripY - stripY > stripWidth )
        {
          openHeight = prevStripY;
          done = true;
        }
        else if ( stripY <= 0 )
          done = true;
        else
          x--;
      }
    }
  }

  if ( openHeight == 0 )
  {
    mRegionHistory.push_back(region);
    mOpenFeatureHistory.push_back(featureData);
    mClosedFeatureHistory.push_back(featureData);
    mStateHistory.push_back(initialResult);
    return true;
  }
#endif

#if 0
  int openHeight = (int)(region->GetWidth() / avgOpenAspectRatio);
#endif
#if 0
  int closedHeight = openHeight / 2;
#else
  int closedHeight = openHeight;
#endif

  Matrix openFeatureData;
  GenerateFeatureData(region, openFeatureData, openHeight);
  for (int i = 0; i < openSize; i++)
    openInput.SetValue(i, 0, openFeatureData.GetValue(openFeatureStr[i] - 'A', 0) );
  openResult = mOpenClassifier->Classify(openInput);

  Matrix closedFeatureData;
  GenerateFeatureData(region, closedFeatureData, closedHeight);
  for (int i = 0; i < closedSize; i++)
    closedInput.SetValue(i, 0, closedFeatureData.GetValue(closedFeatureStr[i] - 'A', 0) );
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