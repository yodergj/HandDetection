#include <stdio.h>
#include <string.h>
#include <map>
using std::map;
#include "ThresholdClassifier.h"

#define THRESHOLD_STR "Threshold"
#define LOWER_CLASS_STR "LowerClass"
#define UPPER_CLASS_STR "UpperClass"
#define FEATURE_STR "Features"

ThresholdClassifier::ThresholdClassifier()
{
  mThreshold = 0;
  mLowerClass = 0;
  mUpperClass = 1;
}

ThresholdClassifier::ThresholdClassifier(const ThresholdClassifier& ref)
{
  operator=(ref);
}

ThresholdClassifier::~ThresholdClassifier()
{
}

ThresholdClassifier& ThresholdClassifier::operator=(const ThresholdClassifier& ref)
{
  WeakClassifier::operator=(ref);
  mThreshold = ref.mThreshold;
  mLowerClass = ref.mLowerClass;
  mUpperClass = ref.mUpperClass;
  return *this;
}

int ThresholdClassifier::Classify(double value)
{
  return (value < mThreshold) ? mLowerClass : mUpperClass;
}

bool ThresholdClassifier::Train(const vector<double>& samples, const vector<double>& weights,
                                const vector<int>& classes, double* trainingError)
{
  int i, j;
  int numSamples, lowerClass, upperClass;
  double bestThreshold, bestError;
  double worstThreshold, worstError;
  double threshold, error;

  if ( samples.empty() || (samples.size() != classes.size()) || (weights.size() != samples.size()) )
  {
    fprintf(stderr, "ThresholdClassifier::Train - Invalid parameter\n");
    return false;
  }

  bestError = 0x7FFFFFFF;
  worstError = 0;
  numSamples = samples.size();
  lowerClass = 0;
  upperClass = 1;

  map<double, int> thresholds;
  for (i = 0; i < numSamples; i++)
    thresholds[ samples[i] ] |= (1 << classes[i]);

  map<double, int>::iterator threshItr, nextItr, prevItr;
  bool keep;
  int bothClasses = 3;
  if ( thresholds.size() > 2 )
  {
    prevItr = thresholds.begin();
    threshItr = prevItr;
    threshItr++;
    while ( (threshItr != thresholds.end()) &&
            (prevItr->second == threshItr->second) &&
            (prevItr->second != bothClasses) )
    {
      thresholds.erase(prevItr);
      prevItr = threshItr;
      threshItr++;
    }

    while ( threshItr != thresholds.end() )
    {
      keep = false;
      nextItr = threshItr;
      nextItr++;

      if ( (threshItr->second == bothClasses) ||
           (threshItr->second != prevItr->second) ||
           ( (nextItr != thresholds.end()) && (threshItr->second != nextItr->second) ) )
        keep = true;

      if ( keep )
        prevItr = threshItr;
      else
        thresholds.erase( threshItr );
      threshItr = nextItr;
    }
  }

  for (threshItr = thresholds.begin(); threshItr != thresholds.end(); threshItr++)
  {
    threshold = threshItr->first;
    error = 0;
    for (j = 0; j < numSamples; j++)
    {
      if ( samples[j] < threshold )
      {
        if ( classes[j] != lowerClass )
          error += weights[j];
      }
      else
      {
        if ( classes[j] != upperClass )
          error += weights[j];
      }
    }
    if ( error < bestError )
    {
      bestThreshold = threshold;
      bestError = error;
    }
    if ( error >= worstError )
    {
      worstThreshold = threshold;
      worstError = error;
    }
  }

  lowerClass = 1;
  upperClass = 0;
  error = 0;
  for (j = 0; j < numSamples; j++)
  {
    if ( samples[j] < worstThreshold )
    {
      if ( classes[j] != lowerClass )
        error += weights[j];
    }
    else
    {
      if ( classes[j] != upperClass )
        error += weights[j];
    }
  }

  if ( error < bestError )
  {
    mLowerClass = 1;
    mUpperClass = 0;
    mThreshold = worstThreshold;

    if ( trainingError )
      *trainingError = error;
  }
  else
  {
    mLowerClass = 0;
    mUpperClass = 1;
    mThreshold = bestThreshold;

    if ( trainingError )
      *trainingError = bestError;
  }

  return true;
}

bool ThresholdClassifier::Print(FILE* file)
{
  if ( !file )
  {
    fprintf(stderr, "ThresholdClassifier::Print - Invalid parameter\n");
    return false;
  }

  fprintf(file, "ThresholdClassifier\n");
  fprintf(file, "Threshold %f\n", mThreshold);
  fprintf(file, "Classes %d %d\n", mLowerClass, mUpperClass);
  fprintf(file, "Feature %s\n", mFeatureString.c_str());

  return true;
}

xercesc::DOMElement* ThresholdClassifier::Save(xercesc::DOMDocument* document, bool toRootElem)
{
  xercesc::DOMElement* classifierNode = WeakClassifier::Save(document, toRootElem);
  if ( classifierNode )
  {
    SetStringValue(classifierNode, CLASSIFIER_TYPE_STR, THRESHOLD_CLASSIFIER_STR);
    SetDoubleValue(classifierNode, THRESHOLD_STR, mThreshold);
    SetIntValue(classifierNode, LOWER_CLASS_STR, mLowerClass);
    SetIntValue(classifierNode, UPPER_CLASS_STR, mUpperClass);
  }

  return classifierNode;
}

bool ThresholdClassifier::LoadClassifier(xercesc::DOMElement* classifierNode)
{
  if ( !classifierNode )
    return false;

  bool retCode = WeakClassifier::LoadClassifier(classifierNode);
  mThreshold = GetDoubleValue(classifierNode, THRESHOLD_STR, 0);
  mLowerClass = GetIntValue(classifierNode, LOWER_CLASS_STR, 0);
  mUpperClass = GetIntValue(classifierNode, UPPER_CLASS_STR, 1);

  return retCode;
}
