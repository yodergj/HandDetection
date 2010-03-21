#include <math.h>
#include <string.h>
#include "AdaboostClassifier.h"

#define DIMENSION_STR "NumDimensions"
#define CLASSES_STR "NumClasses"
#define CLASSIFIERS_STR "NumClassifiers"
#define FEATURE_STR "Features"
#define WEIGHT_STR "Weight"

AdaboostClassifier::AdaboostClassifier()
{
  mNumDimensions = 0;
  mNumClasses = 0;
}

AdaboostClassifier::~AdaboostClassifier()
{
  Clear();
}

void AdaboostClassifier::Clear()
{
  mTrainingData.clear();
  mTrainingDataFreq.clear();
  mWeakClassifiers.clear();
  mClassifierWeights.clear();
}

bool AdaboostClassifier::Create(int numDimensions, int numClasses, int numWeakClassifiers)
{
  if ( (numDimensions < 1) || (numClasses < 2) || (numWeakClassifiers < 1) )
  {
    fprintf(stderr, "AdaboostClassifier::Create - Invalid parameter\n");
    return false;
  }

  Clear();

  mNumDimensions = numDimensions;
  mNumClasses = numClasses;
  mWeakClassifiers.resize( numWeakClassifiers );
  mClassifierWeights.resize( numWeakClassifiers );
  mTrainingData.resize( numClasses );
  mTrainingDataFreq.resize( numClasses );

  return true;
}

bool AdaboostClassifier::AddTrainingData(const Matrix& data, int classIndex)
{
  int len;
  int minPos, curPos, maxPos;

  if ( (classIndex < 0) || (classIndex >= mNumClasses) )
  {
    fprintf(stderr, "AdaboostClassifier::AddTrainingData - Invalid parameter\n");
    return false;
  }

  if ( mTrainingData[classIndex].empty() )
  {
    mTrainingData[classIndex].push_back(data);
    mTrainingDataFreq[classIndex].push_back(1);
    return true;
  }

  /* Find the correct position in the list */
  len = mTrainingData[classIndex].size();
  minPos = 0;
  maxPos = len - 1;
  curPos = len / 2;
  while ( (maxPos >= minPos) && (data != mTrainingData[classIndex][curPos]) )
  {
    if ( data < mTrainingData[classIndex][curPos] )
      maxPos = curPos - 1;
    else
      minPos = curPos + 1;
    curPos = (minPos + maxPos) / 2;
  }

  if ( maxPos >= minPos )
  {
    /* We found the entry, so just update the frequency */
    mTrainingDataFreq[classIndex][curPos]++;
    return true;
  }

  /* Insert a new entry at the correct spot */
  mTrainingData[classIndex].insert(mTrainingData[classIndex].begin() + maxPos + 1, data);
  mTrainingDataFreq[classIndex].insert(mTrainingDataFreq[classIndex].begin() + maxPos + 1, 1);

  return true;
}

int AdaboostClassifier::Classify(const Matrix& data)
{
  int i, numClassifiers;
  double input, total;

  if ( mNumClasses < 2 )
  {
    fprintf(stderr, "AdaboostClassifier::Classify - Classifier is not initialized\n");
    return false;
  }

  // TODO Figure out how to adapt this for more than 2 classes
  input = data.GetValue(0, 0);
  total = 0;
  numClassifiers = mWeakClassifiers.size();
  for (i = 0; i < numClassifiers; i++)
  {
    if ( mWeakClassifiers[i].Classify(input) == 0 )
      total -= mClassifierWeights[i];
    else
      total += mClassifierWeights[i];
  }

  if ( total < 0 )
    return 0;
  return 1;
}

bool AdaboostClassifier::Train()
{
  int i, j, len, numClassifiers, numSampleBins, totalSamples;
  double baseWeight, weightTotal, trainingError, errorRatio;
  vector<double> weights;
  vector<double> classifierWeights;
  vector<double> data;
  vector<int> dataFreq;
  vector<int> truth;

  if ( mNumClasses < 2 )
  {
    fprintf(stderr, "AdaboostClassifier::Train - Classifier is not initialized\n");
    return false;
  }

  totalSamples = 0;
  for (i = 0; i < mNumClasses; i++)
  {
    len = mTrainingData[i].size();
    for (j = 0; j < len; j++)
    {
      totalSamples += mTrainingDataFreq[i][j];
      data.push_back( mTrainingData[i][j].GetValue(0, 0) );
      dataFreq.push_back( mTrainingDataFreq[i][j] );
      truth.push_back(i);
    }
  }

  numClassifiers = mWeakClassifiers.size();
  numSampleBins = data.size();
  weights.resize(numSampleBins);
  baseWeight = 1.0 / totalSamples;
  for (i = 0; i < numSampleBins; i++)
    weights[i] = baseWeight * dataFreq[i];

  for (i = 0; i < numClassifiers; i++)
  {
    // ThresholdClassifier calculates error the way we need, to just use its result.
    // Will need to calculate error for ourself if we use a different kind of weak classifier
    if ( !mWeakClassifiers[i].Train(data, weights, truth, &trainingError) )
    {
      fprintf(stderr, "AdaboostClassifier::Train - Error training weak classifier %d\n", i);
      return false;
    }

    if ( trainingError == 0 )
    {
      fprintf(stderr, "AdaboostClassifier::Train - Perfect classifier in slot %d\n", i);
      mClassifierWeights[i] = 100;
    }
    else
    {
      weightTotal = 0;
      for (j = 0; j < numSampleBins; j++)
        weightTotal += weights[j];
      errorRatio = trainingError / weightTotal;
      mClassifierWeights[i] = log( (1 - errorRatio) / errorRatio );
    }

    // Update weights for next classifier
    for (j = 0; j < numSampleBins; j++)
      if ( mWeakClassifiers[i].Classify( data[j] ) != truth[j] )
        weights[j] *= exp( mClassifierWeights[i] );
  }

  return true;
}

string AdaboostClassifier::GetFeatureString()
{
  return mFeatureString;
}

bool AdaboostClassifier::SetFeatureString(const string& featureString)
{
  if ( (int)featureString.size() != mNumDimensions )
  {
    fprintf(stderr, "AdaboostClassifier::SetFeatureString - Feature string \"%s\" doesn't have length %d\n", featureString.c_str(), mNumDimensions);
    return false;
  }

  mFeatureString = featureString;

  return true;
}

bool AdaboostClassifier::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "AdaboostClassifier::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "AdaboostClassifier::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)ADABOOST_CLASSIFIER_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool AdaboostClassifier::Save(xmlNodePtr classifierNode)
{
  int i, numClassifiers;
  bool retCode = true;
  xmlNodePtr weakNode;

  numClassifiers = mWeakClassifiers.size();
  SetIntValue(classifierNode, DIMENSION_STR, mNumDimensions);
  SetIntValue(classifierNode, CLASSES_STR, mNumClasses);
  SetIntValue(classifierNode, CLASSIFIERS_STR, numClassifiers);
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString);

  for (i = 0; (i < numClassifiers) && retCode; i++)
  {
    weakNode = xmlNewNode(NULL, (const xmlChar*)THRESHOLD_CLASSIFIER_STR);
    xmlAddChild(classifierNode, weakNode);
    SetDoubleValue(weakNode, WEIGHT_STR, mClassifierWeights[i]);
    retCode = mWeakClassifiers[i].Save(weakNode);
    if ( !retCode )
      fprintf(stderr, "AdaboostClassifier::Save - Failed saviing weak classifier %d\n", i);
  }

  return retCode;
}

bool AdaboostClassifier::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "AdaboostClassifier::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "AdaboostClassifier::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "AdaboostClassifier::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool AdaboostClassifier::Load(xmlNodePtr classifierNode)
{
  int numClassifiersFound, dimensions, numClasses, numClassifiers;
  xmlNodePtr node;
  bool retCode = true;

  dimensions = GetIntValue(classifierNode, DIMENSION_STR, 0);
  numClasses = GetIntValue(classifierNode, CLASSES_STR, 0);
  numClassifiers = GetIntValue(classifierNode, CLASSIFIERS_STR, 0);
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);
  if ( (dimensions < 1) || (numClasses < 2) || (numClassifiers < 1) )
  {
    fprintf(stderr, "AdaboostClassifier::Load - Invalid property\n");
    return false;
  }

  if ( !Create(dimensions, numClasses, numClassifiers) )
  {
    fprintf(stderr, "AdaboostClassifier::Load - Failed Create(%d, %d, %d)\n",
            dimensions, numClasses, numClassifiers);
    return false;
  }

  numClassifiersFound = 0;
  node = classifierNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, THRESHOLD_CLASSIFIER_STR) )
    {
      if ( numClassifiersFound == numClassifiers )
      {
        fprintf(stderr, "AdaboostClassifier::Load - Too many weak classifiers found\n");
        retCode = false;
      }
      else
      {
        mClassifierWeights[numClassifiersFound] = GetDoubleValue(node, WEIGHT_STR, -1);
        if ( mClassifierWeights[numClassifiersFound] < 0 )
        {
          retCode = false;
          fprintf(stderr, "AdaboostClassifier::Load - Invalid classifier weight %f loading %d\n",
                  mClassifierWeights[numClassifiersFound], numClassifiersFound);
        }
        else
        {
          retCode = mWeakClassifiers[numClassifiersFound].Load(node);
          if ( !retCode )
            fprintf(stderr, "AdaboostClassifier::Load - Failed loading  %d\n", numClassifiersFound);
        }
        numClassifiersFound++;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "AdaboostClassifier::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  return retCode;
}
