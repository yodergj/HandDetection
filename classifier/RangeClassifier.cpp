#include <stdio.h>
#include <string.h>
#include <map>
using std::map;
#include "RangeClassifier.h"

#define LOWER_THRESHOLD_STR "LowerThreshold"
#define UPPER_THRESHOLD_STR "UpperThreshold"
#define INNER_CLASS_STR "InnerClass"
#define OUTER_CLASS_STR "OuterClass"

RangeClassifier::RangeClassifier()
{
  mLowerThreshold = 0x7FFFFFFF;
  mUpperThreshold = 0;
  mInnerClass = 0;
  mOuterClass = 1;
}

RangeClassifier::RangeClassifier(const RangeClassifier& ref)
{
  operator=(ref);
}

RangeClassifier::~RangeClassifier()
{
}

RangeClassifier& RangeClassifier::operator=(const RangeClassifier& ref)
{
  WeakClassifier::operator=(ref);
  mLowerThreshold = ref.mLowerThreshold;
  mUpperThreshold = ref.mUpperThreshold;
  mInnerClass = ref.mInnerClass;
  mOuterClass = ref.mOuterClass;
  return *this;
}

int RangeClassifier::Classify(double value)
{
  return (value <= mUpperThreshold && value >= mLowerThreshold) ? mInnerClass : mOuterClass;
}

bool RangeClassifier::Train(const vector<double>& samples, const vector<double>& weights,
                            const vector<int>& classes, double* trainingError)
{
  int i, j;
  int numSamples, innerClass, outerClass;
  int bestInnerClass, bestOuterClass;
  double bestLowerThreshold, bestUpperThreshold, bestError;
  double lowerThreshold, upperThreshold, error;

  if ( samples.empty() || (samples.size() != classes.size()) || (weights.size() != samples.size()) )
  {
    fprintf(stderr, "RangeClassifier::Train - Invalid parameter\n");
    return false;
  }

  bestError = 0x7FFFFFFF;
  numSamples = samples.size();

  map<double, int> thresholds;
  for (i = 0; i < numSamples; i++)
    thresholds[ samples[i] ] |= (1 << classes[i]);

  map<double, int>::iterator threshItr, nextItr, prevItr;
  map<double, int>::iterator firstInner, lastInner, upperItr;
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

  innerClass = 0;
  outerClass = 1;
  for (i = 0; i < 2; i++)
  {
    firstInner = thresholds.end();
    lastInner = thresholds.end();
    for (threshItr = thresholds.begin(); threshItr != thresholds.end(); threshItr++)
    {
      if ( threshItr->second & (1 << innerClass) )
      {
        if ( firstInner == thresholds.end() )
          firstInner = threshItr;
        lastInner = threshItr;
      }
    }

    for (threshItr = firstInner; threshItr != lastInner; threshItr++)
    {
      if ( !(threshItr->second & (1 << innerClass)) )
        continue;

      lowerThreshold = threshItr->first;
      bool passDone = false;
      for (upperItr = threshItr; !passDone; upperItr++)
      {
        passDone = (upperItr == lastInner);
        if ( !(upperItr->second & (1 << innerClass)) )
          continue;

        upperThreshold = upperItr->first;
        error = 0;
        for (j = 0; j < numSamples; j++)
        {
          if ( (samples[j] >= lowerThreshold) && (samples[j] <= upperThreshold) )
          {
            if ( classes[j] != innerClass )
              error += weights[j];
          }
          else
          {
            if ( classes[j] != outerClass )
              error += weights[j];
          }
        }
        if ( error < bestError )
        {
          bestLowerThreshold = lowerThreshold;
          bestUpperThreshold = upperThreshold;
          bestInnerClass = innerClass;
          bestOuterClass = outerClass;
          bestError = error;
        }
      }
    }

    innerClass = 1;
    outerClass = 0;
  }

  mInnerClass = bestInnerClass;
  mOuterClass = bestOuterClass;
  mLowerThreshold = bestLowerThreshold;
  mUpperThreshold = bestUpperThreshold;
  if ( trainingError )
    *trainingError = bestError;

  return true;
}

bool RangeClassifier::Print(FILE* file)
{
  if ( !file )
  {
    fprintf(stderr, "RangeClassifier::Print - Invalid parameter\n");
    return false;
  }

  fprintf(file, "RangeClassifier\n");
  fprintf(file, "LowerThreshold %f\n", mLowerThreshold);
  fprintf(file, "UpperThreshold %f\n", mUpperThreshold);
  fprintf(file, "Classes %d %d\n", mInnerClass, mOuterClass);
  fprintf(file, "Feature %s\n", mFeatureString.c_str());

  return true;
}

xercesc::DOMElement* RangeClassifier::Save(xercesc::DOMDocument* document, bool toRootElem)
{
  xercesc::DOMElement* classifierNode = WeakClassifier::Save(document, toRootElem);
  if ( classifierNode )
  {
    SetStringValue(classifierNode, CLASSIFIER_TYPE_STR, RANGE_CLASSIFIER_STR);
    SetDoubleValue(classifierNode, LOWER_THRESHOLD_STR, mLowerThreshold);
    SetDoubleValue(classifierNode, UPPER_THRESHOLD_STR, mUpperThreshold);
    SetIntValue(classifierNode, INNER_CLASS_STR, mInnerClass);
    SetIntValue(classifierNode, OUTER_CLASS_STR, mOuterClass);
  }

  return classifierNode;
}

bool RangeClassifier::LoadClassifier(xercesc::DOMElement* classifierNode)
{
  if ( !classifierNode )
    return false;

  bool retCode = WeakClassifier::LoadClassifier(classifierNode);
  mLowerThreshold = GetDoubleValue(classifierNode, LOWER_THRESHOLD_STR, 0);
  mUpperThreshold = GetDoubleValue(classifierNode, UPPER_THRESHOLD_STR, 0);
  mInnerClass = GetIntValue(classifierNode, INNER_CLASS_STR, 0);
  mOuterClass = GetIntValue(classifierNode, OUTER_CLASS_STR, 1);

  return retCode;
}
