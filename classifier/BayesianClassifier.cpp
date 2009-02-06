#include "BayesianClassifier.h"
#include <string.h>

#define DIMENSION_STR "NumDimensions"
#define CLASSES_STR "NumClasses"
#define FEATURE_STR "Features"
#define WEIGHT_STR "Weight"

BayesianClassifier::BayesianClassifier()
{
  mNumDimensions = 0;
  mNumClasses = 0;
  mModels = NULL;
  mClassCounts = NULL;
  mClassWeights = NULL;
}

BayesianClassifier::~BayesianClassifier()
{
  delete[] mModels;
  delete[] mClassCounts;
  delete[] mClassWeights;
}

bool BayesianClassifier::Create(int numDimensions, int numClasses, int* classComponents)
{
  int i;
  int numComponents;

  if ( (numDimensions < 1) || (numClasses < 2) )
  {
    fprintf(stderr, "BayesianClassifier::Create - Invalid parameter\n");
    return false;
  }

  mNumDimensions = numDimensions;
  mNumClasses = numClasses;

  delete[] mModels;
  delete[] mClassCounts;
  delete[] mClassWeights;

  mModels = new GaussianMixtureModel[numClasses];
  mClassCounts = new int[numClasses];
  mClassWeights = new double[numClasses];

  memset(mClassCounts, 0, numClasses * sizeof(int));
  memset(mClassWeights, 0, numClasses * sizeof(double));

  for (i = 0; i < numClasses; i++)
  {
    if ( classComponents )
      numComponents = classComponents[i];
    else
      numComponents = 1;

    if ( !mModels[i].Create(numDimensions, numComponents) )
    {
      fprintf(stderr, "BayesianClassifier::Create - Failed creating model %d\n", i);
      return false;
    }
  }

  return true;
}

bool BayesianClassifier::AddTrainingData(Matrix& data, int classIndex)
{
  if ( (classIndex < 0) || (classIndex >= mNumClasses) )
  {
    fprintf(stderr, "BayesianClassifier::AddTrainingData - Invalid parameter\n");
    return false;
  }

  mClassCounts[classIndex]++;
  return mModels[classIndex].AddTrainingData(data);
}

int* BayesianClassifier::Get2dDataHistogram(int classIndex, int binsPerSide, double scaleFactor)
{
  if ( (classIndex < 0) || (classIndex >= mNumClasses) )
  {
    fprintf(stderr, "BayesianClassifier::Get2dDataHistogram - Invalid parameter\n");
    return NULL;
  }
  return mModels[classIndex].Get2dDataHistogram(binsPerSide, scaleFactor);
}

bool BayesianClassifier::Classify(Matrix& data, int& classIndex, double& confidence)
{
  int i;
  double probability;
  double max = 0;
  double sum = 0;

  if ( mNumClasses < 2 )
  {
    fprintf(stderr, "BayesianClassifier::Classify - Classifier is not initialized\n");
    return false;
  }

  for (i = 0; i < mNumClasses; i++)
  {
    probability = mModels[i].Probability(data);
    sum += probability;
    if ( probability > max )
    {
      max = probability;
      classIndex = i;
    }
  }
  confidence = max / sum;

  return true;
}

bool BayesianClassifier::Train()
{
  int i;
  double totalData = 0;

  if ( mNumClasses < 2 )
  {
    fprintf(stderr, "BayesianClassifier::Train - Classifier is not initialized\n");
    return false;
  }

  for (i = 0; i < mNumClasses; i++)
  {
    totalData += mClassCounts[i];
    if ( !mModels[i].Train() )
    {
      fprintf(stderr, "BayesianClassifier::Train - Failed training model %d\n", i);
      return false;
    }
  }

  for (i = 0; i < mNumClasses; i++)
    mClassWeights[i] = mClassCounts[i] / totalData;

  return true;
}

#if 0
#define LABEL "BayesianClassifier"
#define LABEL_LEN 18
bool BayesianClassifier::Save(FILE* file)
{
  int i;

  if ( !file )
  {
    fprintf(stderr, "BayesianClassifier::Save - Invalid parameter\n");
    return false;
  }

  fprintf(file, "%s %d %d\n", LABEL, mNumDimensions, mNumClasses);
  for (i = 0; i < mNumClasses; i++)
  {
    fprintf(file, "%f\n", mClassWeights[i]);
    mModels[i].Save(file);
  }

  return true;
}

#define MAX_STR_LEN 32
bool BayesianClassifier::Load(FILE* file)
{
  int i;
  int dimensions, numClasses;
  double weight;
  char buf[MAX_STR_LEN];

  if ( !file )
  {
    fprintf(stderr, "BayesianClassifier::Load - NULL file\n");
    return false;
  }

  fgets(buf, LABEL_LEN + 1, file);
  if ( strncmp(LABEL, buf, LABEL_LEN) )
  {
    fprintf(stderr, "BayesianClassifier::Load - Header string didn't match\n");
    return false;
  }

  if ( fscanf(file, "%d %d", &dimensions, &numClasses) != 2 )
  {
    fprintf(stderr, "BayesianClassifier::Load - Failed getting dimensions and classes\n");
    return false;
  }

  if ( !Create(dimensions, numClasses) )
  {
    fprintf(stderr, "BayesianClassifier::Load - Failed Create(%d, %d)\n", dimensions, numClasses);
    return false;
  }

  for (i = 0; i < mNumClasses; i++)
  {
    if ( !fscanf(file, "%lf\n", &weight) )
    {
      fprintf(stderr, "BayesianClassifier::Load - Failed getting class weight %d\n", i);
      return false;
    }
    if ( !mModels[i].Load(file) )
    {
      fprintf(stderr, "BayesianClassifier::Load - Failed loading mixture model %d\n", i);
      return false;
    }
    mClassWeights[i] = weight;
  }

  return true;
}
#else
string BayesianClassifier::GetFeatureString()
{
  return mFeatureString;
}

bool BayesianClassifier::SetFeatureString(const string& featureString)
{
  if ( (int)featureString.size() != mNumDimensions )
  {
    fprintf(stderr, "BayesianClassifier::SetFeatureString - Feature string \"%s\" doesn't have length %d\n", featureString.c_str(), mNumDimensions);
    return false;
  }

  mFeatureString = featureString;

  return true;
}

bool BayesianClassifier::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "BayesianClassifier::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "BayesianClassifier::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)BAYESIAN_CLASSIFIER_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool BayesianClassifier::Save(xmlNodePtr classifierNode)
{
  int i;
  bool retCode = true;
  xmlNodePtr modelNode;

  SetIntValue(classifierNode, DIMENSION_STR, mNumDimensions);
  SetIntValue(classifierNode, CLASSES_STR, mNumClasses);
  SetStringValue(classifierNode, FEATURE_STR, mFeatureString);

  for (i = 0; (i < mNumClasses) && retCode; i++)
  {
    modelNode = xmlNewNode(NULL, (const xmlChar*)GMM_STR);
    xmlAddChild(classifierNode, modelNode);
    SetDoubleValue(modelNode, WEIGHT_STR, mClassWeights[i]);
    retCode = mModels[i].Save(modelNode);
    if ( !retCode )
      fprintf(stderr, "BayesianClassifier::Save - Failed saviing GMM %d\n", i);
  }

  return retCode;
}

bool BayesianClassifier::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "BayesianClassifier::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "BayesianClassifier::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "BayesianClassifier::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool BayesianClassifier::Load(xmlNodePtr classifierNode)
{
  int numModelsFound, dimensions, numClasses;
  xmlNodePtr node;
  bool retCode = true;

  dimensions = GetIntValue(classifierNode, DIMENSION_STR, 0);
  numClasses = GetIntValue(classifierNode, CLASSES_STR, 0);
  mFeatureString = GetStringValue(classifierNode, FEATURE_STR);
  if ( (dimensions < 1) || (numClasses < 2) )
  {
    fprintf(stderr, "BayesianClassifier::Load - Invalid property\n");
    return false;
  }

  if ( !Create(dimensions, numClasses) )
  {
    fprintf(stderr, "BayesianClassifier::Load - Failed Create(%d, %d)\n", dimensions, numClasses);
    return false;
  }

  numModelsFound = 0;
  node = classifierNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, GMM_STR) )
    {
      if ( numModelsFound == mNumClasses )
      {
        fprintf(stderr, "BayesianClassifier::Load - Too many models found\n");
        retCode = false;
      }
      else
      {
        mClassWeights[numModelsFound] = GetDoubleValue(node, WEIGHT_STR, 1.0 / mNumClasses);
        retCode = mModels[numModelsFound].Load(node);
        if ( !retCode )
          fprintf(stderr, "BayesianClassifier::Load - Failed loading GMM %d\n", numModelsFound);
        numModelsFound++;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "BayesianClassifier::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  return retCode;
}
#endif