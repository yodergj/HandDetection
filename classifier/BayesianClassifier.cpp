#include "BayesianClassifier.h"
#include <string.h>

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
    return false;

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
      return false;
  }

  return true;
}

bool BayesianClassifier::AddTrainingData(Matrix& data, int classIndex)
{
  if ( (classIndex < 0) || (classIndex >= mNumClasses) )
    return false;

  mClassCounts[classIndex]++;
  return mModels[classIndex].AddTrainingData(data);
}

int* BayesianClassifier::Get2dDataHistogram(int classIndex, int binsPerSide, double scaleFactor)
{
  if ( (classIndex < 0) || (classIndex >= mNumClasses) )
    return NULL;
  return mModels[classIndex].Get2dDataHistogram(binsPerSide, scaleFactor);
}

bool BayesianClassifier::Classify(Matrix& data, int& classIndex, double& confidence)
{
  int i;
  double probability;
  double max = 0;
  double sum = 0;

  if ( mNumClasses < 2 )
    return false;

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
    return false;

  for (i = 0; i < mNumClasses; i++)
  {
    totalData += mClassCounts[i];
    if ( !mModels[i].Train() )
      return false;
  }

  for (i = 0; i < mNumClasses; i++)
    mClassWeights[i] = mClassCounts[i] / totalData;

  return true;
}

#define LABEL "BayesianClassifier"
#define LABEL_LEN 18
bool BayesianClassifier::Save(FILE* file)
{
  int i;

  if ( !file )
    return false;

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
