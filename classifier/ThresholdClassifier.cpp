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

ThresholdClassifier::~ThresholdClassifier()
{
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
  numSamples = samples.size();
  lowerClass = 0;
  upperClass = 1;

  map<double, int> thresholds;
  for (i = 0; i < numSamples; i++)
    thresholds[ samples[i] ] |= (1 << classes[i]);
  fprintf(stderr, "ThresholdClassifier::Train - Data size %d, Threshold size %d\n", samples.size(), thresholds.size());

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
  fprintf(stderr, "ThresholdClassifier::Train - Filtered Threshold size %d\n", thresholds.size());

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

string ThresholdClassifier::GetFeatureString() const
{
  return mFeatureString;
}

bool ThresholdClassifier::SetFeatureString(const string& featureString)
{
  if ( (int)featureString.size() != 1 )
  {
    fprintf(stderr, "ThresholdClassifier::SetFeatureString - Feature string \"%s\" doesn't have length 1\n", featureString.c_str());
    return false;
  }

  mFeatureString = featureString;

  return true;
}

bool ThresholdClassifier::SetFeatureString(const char featureLetter)
{
  mFeatureString = featureLetter;
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

bool ThresholdClassifier::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "ThresholdClassifier::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "ThresholdClassifier::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)THRESHOLD_CLASSIFIER_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool ThresholdClassifier::Save(xmlNodePtr classifierNode)
{
  SetDoubleValue(classifierNode, THRESHOLD_STR, mThreshold);
  SetIntValue(classifierNode, LOWER_CLASS_STR, mLowerClass);
  SetIntValue(classifierNode, UPPER_CLASS_STR, mUpperClass);
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString);

  return true;
}

bool ThresholdClassifier::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "ThresholdClassifier::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "ThresholdClassifier::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "ThresholdClassifier::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool ThresholdClassifier::Load(xmlNodePtr classifierNode)
{
  mThreshold = GetDoubleValue(classifierNode, THRESHOLD_STR, 0);
  mLowerClass = GetIntValue(classifierNode, LOWER_CLASS_STR, 0);
  mUpperClass = GetIntValue(classifierNode, UPPER_CLASS_STR, 1);
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);

  return true;
}
