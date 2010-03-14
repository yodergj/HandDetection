#include "ThresholdClassifier.h"
#include <stdio.h>
#include <string.h>

#define THRESHOLD_STR "Threshold"
#define LOWER_CLASS_STR "LowerClass"
#define UPPER_CLASS_STR "UpperClass"

ThresholdClassifier::ThresholdClassifier()
{
  mThreshold = 0;
  mLowerClass = 0;
  mUpperClass = 1;
}

ThresholdClassifier::~ThresholdClassifier()
{
}

int ThresholdClassifier::Classifiy(double value)
{
  return (value < mThreshold) ? mLowerClass : mUpperClass;
}

bool ThresholdClassifier::Train(vector<double>& samples, vector<double>& weights, vector<int>& classes, double* trainingError)
{
  int i, j;
  int numSamples, lowerClass, upperClass;
  double bestThreshold, bestError, pass1BestError;
  double threshold, error;

  if ( samples.empty() || (samples.size() != classes.size()) || (weights.size() != samples.size()) )
  {
    fprintf(stderr, "ThresholdClassifier::Train - Invalid parameter\n");
    return false;
  }

  // Worst case error should be 1, so pick something we can definitely beat
  bestError = 2;
  numSamples = samples.size();
  lowerClass = 0;
  upperClass = 1;
  for (i = 0; i < numSamples; i++)
  {
    threshold = samples[i];
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
  }
  pass1BestError = bestError;

  lowerClass = 1;
  upperClass = 0;
  for (i = 0; i < numSamples; i++)
  {
    threshold = samples[i];
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
  }

  if ( bestError == pass1BestError )
  {
    mLowerClass = 0;
    mUpperClass = 1;
  }
  else
  {
    mLowerClass = 1;
    mUpperClass = 0;
  }
  mThreshold = bestThreshold;

  if ( trainingError )
    *trainingError = bestError;

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

  return true;
}
