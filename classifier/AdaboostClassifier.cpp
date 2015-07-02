#include <math.h>
#include <string.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include "AdaboostClassifier.h"
#include "ThresholdClassifier.h"
#include "RangeClassifier.h"
#include "XMLUtils2.h"

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
  vector<WeakClassifier*>::iterator itr;
  for (itr = mWeakClassifiers.begin(); itr != mWeakClassifiers.end(); itr++)
    delete *itr;
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
  mClassifierFeatures.resize( numWeakClassifiers );
  mTrainingData.resize( numClasses );
  mTrainingDataFreq.resize( numClasses );

  return true;
}

bool AdaboostClassifier::AddTrainingData(const Matrix& data, int classIndex)
{
  int len;
  int minPos, curPos, maxPos;

  if ( (classIndex < 0) || (classIndex >= mNumClasses) || (data.GetRows() != mNumDimensions) )
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
  mTrainingData[classIndex].insert(mTrainingData[classIndex].begin() + (maxPos + 1), data);
  mTrainingDataFreq[classIndex].insert(mTrainingDataFreq[classIndex].begin() + (maxPos + 1), 1);

  return true;
}

int AdaboostClassifier::Classify(const Matrix& data)
{
  int i, numClassifiers;
  double total;

  if ( mNumClasses < 2 )
  {
    fprintf(stderr, "AdaboostClassifier::Classify - Classifier is not initialized\n");
    return -1;
  }

  // TODO Figure out how to adapt this for more than 2 classes
  total = 0;
  numClassifiers = mWeakClassifiers.size();
  for (i = 0; i < numClassifiers; i++)
  {
    if ( mWeakClassifiers[i]->Classify( data.GetValue(mClassifierFeatures[i], 0) ) == 0 )
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
  int classNum, weightNum, featureNum;
  double baseWeight;
  vector<double> weights;

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
      totalSamples += mTrainingDataFreq[i][j];
  }

  numClassifiers = mWeakClassifiers.size();
  numSampleBins = 0;
  baseWeight = 1.0 / totalSamples;
  for (i = 0; i < mNumClasses; i++)
  {
    len = mTrainingData[i].size();
    numSampleBins += len;
    for (j = 0; j < len; j++)
      weights.push_back(baseWeight * mTrainingDataFreq[i][j]);
  }

  for (i = 0; i < numClassifiers; i++)
  {
    TrainLevel(weights, i, featureNum);

    // Update weights for next classifier
    weightNum = 0;
    for (classNum = 0; classNum < mNumClasses; classNum++)
    {
      len = mTrainingData[classNum].size();
      for (j = 0; j < len; j++, weightNum++)
        if ( mWeakClassifiers[i]->Classify( mTrainingData[classNum][j].GetValue(featureNum, 0) ) != classNum )
          weights[weightNum] *= exp( mClassifierWeights[i] );
    }
  }

  // Update the feature string to what was actually selected
  mFeatureString = "";
  for (i = 0; i < numClassifiers; i++)
    if ( mFeatureString.find( mWeakClassifiers[i]->GetFeatureString()[0] ) == string::npos )
      mFeatureString += mWeakClassifiers[i]->GetFeatureString();

  return true;
}

bool AdaboostClassifier::TrainLevel(const vector<double>& weights, int levelNum, int& chosenFeature)
{
  int i, len;
  double weightTotal, trainingError, errorRatio;
  vector<double> data;
  vector<int> truth;

  int numFeatures, bestFeature;
  double bestError;
  WeakClassifier* bestClassifier = NULL;
  ThresholdClassifier thresholdClassifier;
  RangeClassifier rangeClassifier;
fprintf(stderr, "AdaboostClassifier::TrainLevel - Starting level %d\n", levelNum);

  weightTotal = 0;
  len = weights.size();
  for (i = 0; i < len; i++)
    weightTotal += weights[i];

  bestFeature = -1;
  numFeatures = mFeatureString.size();
  for (i = 0; i < numFeatures; i++)
  {
fprintf(stderr, "AdaboostClassifier::TrainLevel - Starting feature %d\n", i);
    if ( !FillData(i, data, truth) )
    {
      fprintf(stderr, "AdaboostClassifier::TrainLevel - Error getting data for weak classifier %d\n", levelNum);
      return false;
    }
fprintf(stderr, "AdaboostClassifier::TrainLevel - Data size %d\n", data.size());

    // ThresholdClassifier and RangeClassifier calculates error the way we need, so just use its result.
    // Will need to calculate error for ourself if we use a different kind of weak classifier
    if ( !thresholdClassifier.Train(data, weights, truth, &trainingError) )
    {
      fprintf(stderr, "AdaboostClassifier::TrainLevel - Error training threshold classifier %d\n", levelNum);
      return false;
    }
    if ( !bestClassifier || (trainingError < bestError) )
    {
      bestFeature = i;
      bestError = trainingError;
      delete bestClassifier;
      bestClassifier = new ThresholdClassifier(thresholdClassifier);
      if ( trainingError == 0 )
        break;
    }

    if ( !rangeClassifier.Train(data, weights, truth, &trainingError) )
    {
      fprintf(stderr, "AdaboostClassifier::TrainLevel - Error training range classifier %d\n", levelNum);
      return false;
    }
    if ( trainingError < bestError )
    {
      bestFeature = i;
      bestError = trainingError;
      delete bestClassifier;
      bestClassifier = new RangeClassifier(rangeClassifier);
      if ( trainingError == 0 )
        break;
    }
  }

  mWeakClassifiers[levelNum] = bestClassifier;
  mWeakClassifiers[levelNum]->SetFeatureString( mFeatureString[bestFeature] );
  mClassifierFeatures[levelNum] = bestFeature;
  chosenFeature = bestFeature;

  if ( bestError == 0 )
  {
    fprintf(stderr, "AdaboostClassifier::TrainLevel - Perfect classifier in slot %d\n", levelNum);
    mClassifierWeights[levelNum] = 100;
  }
  else
  {
    errorRatio = bestError / weightTotal;
    mClassifierWeights[levelNum] = log( (1 - errorRatio) / errorRatio );
    fprintf(stderr, "AdaboostClassifier::TrainLevel - Classifier in slot %d Error Ratio %f\n", levelNum, errorRatio);
  }

  return true;
}

bool AdaboostClassifier::FillData(int featureNum, vector<double>& data, vector<int>& truth)
{
  int i, j, len;

  if ( (featureNum < 0) || (featureNum >= (int)mFeatureString.size()) )
  {
    fprintf(stderr, "AdaboostClassifier::FillData - Invalid feature number %d\n", featureNum);
    return false;
  }

  data.clear();
  truth.clear();
  for (i = 0; i < mNumClasses; i++)
  {
    len = mTrainingData[i].size();
    for (j = 0; j < len; j++)
    {
      data.push_back( mTrainingData[i][j].GetValue(featureNum, 0) );
      truth.push_back(i);
    }
  }
  return true;
}

string AdaboostClassifier::GetFeatureString() const
{
  return mFeatureString;
}

bool AdaboostClassifier::SetFeatureString(const string& featureString)
{
  if ( (int)featureString.size() < mNumDimensions )
  {
    fprintf(stderr, "AdaboostClassifier::SetFeatureString - Feature string \"%s\" doesn't have length >= %d\n", featureString.c_str(), mNumDimensions);
    return false;
  }

  mFeatureString = featureString;

  return true;
}

bool AdaboostClassifier::Print(FILE* file)
{
  bool retVal = true;
  int i, numClassifiers;
  if ( !file )
  {
    fprintf(stderr, "AdaboostClassifier::Print - Invalid parameter\n");
    return false;
  }

  numClassifiers = mWeakClassifiers.size();
  fprintf(file, "AdaboostClassifier\n");
  fprintf(file, "Feature %s\n", mFeatureString.c_str());
  fprintf(file, "Num Weak Classifiers %d\n", numClassifiers);
  for (i = 0; i < numClassifiers; i++)
  {
    fprintf(file, "Classifier Weight %f\n", mClassifierWeights[i]);
    if ( mWeakClassifiers[i] )
      retVal &= mWeakClassifiers[i]->Print(file);
    else
    {
      retVal = false;
      fprintf(file, "Missing Classifier %d\n", i);
    }
  }

  return retVal;
}

bool AdaboostClassifier::Save(const char* filename)
{
  FILE* file;
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

  InitXerces();
  xercesc::DOMImplementation* impl = xercesc::DOMImplementationRegistry::getDOMImplementation(XMLSTR("Core"));
  xercesc::DOMLSOutput* output = impl->createLSOutput();
  xercesc::DOMLSSerializer* serializer = impl->createLSSerializer();
  xercesc::MemBufFormatTarget* formatTarget = new xercesc::MemBufFormatTarget();
  output->setByteStream(formatTarget);
  xercesc::DOMDocument* doc = impl->createDocument(0, XMLSTR(ADABOOST_CLASSIFIER_STR), 0);
  xercesc::DOMElement* rootElem = doc->getDocumentElement();

  Save(doc, rootElem);

  serializer->write(doc, output);
  const unsigned char* data = formatTarget->getRawBuffer();
  unsigned int len = formatTarget->getLen();
  fwrite(data, 1, len, file);

  doc->release();
  delete formatTarget;
  fclose(file);

  return retCode;
}

bool AdaboostClassifier::Save(xercesc::DOMDocument* doc, xercesc::DOMElement* classifierNode)
{
  if ( !doc || !classifierNode )
    return false;

  int i, numClassifiers;
  bool retCode = true;
  xercesc::DOMElement* weakNode;

  numClassifiers = mWeakClassifiers.size();
  // If not all of the input features are selected for the weak features, then
  // mNumDimensions will be larger than we actually want.
  SetIntValue(classifierNode, DIMENSION_STR, mFeatureString.size());
  SetIntValue(classifierNode, CLASSES_STR, mNumClasses);
  SetIntValue(classifierNode, CLASSIFIERS_STR, numClassifiers);
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString.c_str());

  for (i = 0; (i < numClassifiers) && retCode; i++)
  {
    weakNode = mWeakClassifiers[i]->Save(doc, false);
    if ( weakNode )
    {
      classifierNode->appendChild(weakNode);
      SetDoubleValue(weakNode, WEIGHT_STR, mClassifierWeights[i]);
    }
    else
    {
      fprintf(stderr, "AdaboostClassifier::Save - Failed saviing weak classifier %d\n", i);
      retCode = false;
    }
  }

  return retCode;
}

bool AdaboostClassifier::Load(const char* filename)
{
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "AdaboostClassifier::Load - Bad filename\n");
    return false;
  }

  InitXerces();

  xercesc::XercesDOMParser* domParser = new xercesc::XercesDOMParser();
  xercesc::DOMDocument* doc = 0;

  domParser->parse(XMLSTR(filename));
  doc = domParser->getDocument();
  xercesc::DOMElement* rootElem = doc->getDocumentElement();
  
  retCode = Load(rootElem);

  delete domParser;

  return retCode;
}

bool AdaboostClassifier::Load(xercesc::DOMElement* classifierNode)
{
  if ( !classifierNode )
    return false;

  int i, numClassifiersFound, dimensions, numClasses, numClassifiers;
  size_t pos;
  char feature;
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
  xercesc::DOMNodeList* nodeList = classifierNode->getChildNodes();
  for (XMLSize_t j = 0; retCode && (j < nodeList->getLength()); j++)
  {
    xercesc::DOMNode* node = nodeList->item(j);
    xercesc::DOMElement* elem = dynamic_cast<xercesc::DOMElement*>(node);
    if ( !elem )
      continue;

    std::string nodeName = CHAR(elem->getNodeName());
    if ( !strcmp(nodeName.c_str(), WEAK_CLASSIFIER_STR) )
    {
      if ( numClassifiersFound == numClassifiers )
      {
        fprintf(stderr, "AdaboostClassifier::Load - Too many weak classifiers found\n");
        retCode = false;
      }
      else
      {
        mClassifierWeights[numClassifiersFound] = GetDoubleValue(elem, WEIGHT_STR, -1);
        if ( mClassifierWeights[numClassifiersFound] < 0 )
        {
          retCode = false;
          fprintf(stderr, "AdaboostClassifier::Load - Invalid classifier weight %f loading %d\n",
                  mClassifierWeights[numClassifiersFound], numClassifiersFound);
        }
        else
        {
          mWeakClassifiers[numClassifiersFound] = WeakClassifier::Load(elem);
          if ( !mWeakClassifiers[numClassifiersFound] )
          {
            retCode = false;
            fprintf(stderr, "AdaboostClassifier::Load - Failed loading %d\n", numClassifiersFound);
          }
        }
        numClassifiersFound++;
      }
    }
    else if ( strcmp(nodeName.c_str(), "text") )
    {
      fprintf(stderr, "AdaboostClassifier::Load - Found unknown node %s\n", nodeName.c_str());
      retCode = false;
    }
  }

  for (i = 0; retCode && i < numClassifiers; i++)
  {
    if ( mWeakClassifiers[i]->GetFeatureString().empty() )
    {
      fprintf(stderr, "AdaboostClassifier::Load - Classifier %d has no feature\n", i);
      retCode = false;
    }
    else
    {
      feature = mWeakClassifiers[i]->GetFeatureString()[0];
      pos = mFeatureString.find( feature );
      if ( pos == string::npos )
      {
        fprintf(stderr, "AdaboostClassifier::Load - Classifier %d has mismatched feature %c\n", i, feature);
        retCode = false;
      }
      else
        mClassifierFeatures[i] = pos;
    }
  }

  return retCode;
}
