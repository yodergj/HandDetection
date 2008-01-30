#include "GaussianMixtureModel.h"
#include "math.h"

GaussianMixtureModel::GaussianMixtureModel()
{
  mNumDimensions = 0;
  m2dDataHistogram = NULL;
  m2dDataHistogramSize = 0;
}

GaussianMixtureModel::~GaussianMixtureModel()
{
  Clear();
}

void GaussianMixtureModel::Clear()
{
  int i, len;

  len = mComponents.size();
  for (i = 0; i < len; i++)
    delete mComponents[i];
  len = mTrainingData.size();
  for (i = 0; i < len; i++)
    delete mTrainingData[i];
  mComponents.clear();
  mComponentWeights.clear();
  mTrainingData.clear();
  mTrainingDataFreq.clear();
  if ( m2dDataHistogram )
    free(m2dDataHistogram);
}

bool GaussianMixtureModel::Create(int numDimensions, int numComponents)
{
  if ( (numDimensions <= 0) || (numComponents <= 0) )
    return false;

  Clear();

  mNumDimensions = numDimensions;
  mNumComponents = numComponents;

  return true;
}

bool GaussianMixtureModel::AddTrainingData(Matrix& data)
{
  Matrix* duplicate;
  int len;
  int minPos, curPos, maxPos;

  if ( (data.GetRows() != mNumDimensions) || (data.GetColumns() != 1) )
    return false;

  if ( mTrainingData.empty() )
  {
    duplicate = new Matrix;
    *duplicate = data;
    mTrainingData.push_back(duplicate);
    mTrainingDataFreq.push_back(1);
    return true;
  }

  /* Find the correct position in the list */
  len = mTrainingData.size();
  minPos = 0;
  maxPos = len - 1;
  curPos = len / 2;
  while ( (maxPos >= minPos) && (data != *(mTrainingData[curPos]) ) )
  {
    if ( data < *(mTrainingData[curPos]) )
      maxPos = curPos - 1;
    else
      minPos = curPos + 1;
    curPos = (minPos + maxPos) / 2;
  }

  if ( maxPos >= minPos )
  {
    /* We found the entry, so just update the frequency */
    mTrainingDataFreq[curPos]++;
    return true;
  }

  /* Insert a new entry at the correct spot */
  duplicate = new Matrix;
  *duplicate = data;
  mTrainingData.insert(mTrainingData.begin() + maxPos + 1, duplicate);
  mTrainingDataFreq.insert(mTrainingDataFreq.begin() + maxPos + 1, 1);

  return true;
}

int* GaussianMixtureModel::Get2dDataHistogram(int binsPerSide, double scaleFactor)
{
  int i;
  int sizeNeeded, numSamples, row, column;
  int* tmpPtr;

  if ( binsPerSide <= 0 )
    return NULL;

  sizeNeeded = binsPerSide * binsPerSide;
  if ( sizeNeeded > m2dDataHistogramSize )
  {
    tmpPtr = (int *)realloc(m2dDataHistogram, sizeNeeded * sizeof(int));
    if ( !tmpPtr )
      return NULL;
    m2dDataHistogram = tmpPtr;
    m2dDataHistogramSize = sizeNeeded;
  }
  memset(m2dDataHistogram, 0, sizeNeeded * sizeof(int));
  numSamples = mTrainingData.size();
  for (i = 0; i < numSamples; i++)
  {
    row = (int)(mTrainingData[i]->GetValue(0, 0) * scaleFactor + .5);
    column = (int)(mTrainingData[i]->GetValue(1, 0) * scaleFactor + .5);
    if ( row < 0 )
      row = 0;
    if ( row >= binsPerSide )
      row = binsPerSide - 1;
    if ( column < 0 )
      column = 0;
    if ( column >= binsPerSide )
      column = binsPerSide - 1;
    m2dDataHistogram[row * binsPerSide + column] += mTrainingDataFreq[i];
  }
  return m2dDataHistogram;
}

double GaussianMixtureModel::Probability(Matrix& data)
{
  if ( (data.GetRows() != mNumDimensions) || (data.GetColumns() != 1) )
    return -1;

  return Probability(data, mComponents, mComponentWeights);
}

double GaussianMixtureModel::Probability(Matrix& data,
                                         std::vector<Gaussian *> components,
                                         std::vector<double> weights)
{
  int i;
  double probability = 0;

  for (i = 0; i < mNumComponents; i++)
    probability += components[i]->Probability(data) * weights[i];

  return probability;
}

bool GaussianMixtureModel::Train()
{
  if ( mTrainingData.empty() )
    return false;

  return TrainEM();
}

#if 0
#define MEAN_THRESH     .1
#define VARIANCE_THRESH .1
#else
#if 0
#define MEAN_THRESH     .02
#define VARIANCE_THRESH .02
#else
#define MEAN_THRESH     .005
#define VARIANCE_THRESH .005
#endif
#endif
#define WEIGHT_THRESH   .01
bool GaussianMixtureModel::TrainEM()
{
  int i;
  int dimension, component, sample;
  bool done = false;
  int numSamples, totalSamples;
  Gaussian* currentGaussian;
  std::vector<Gaussian *> components;
  std::vector<double> componentWeights;
  Matrix mean, variance;

  Matrix sampleWeights;
  double sampleProb;

  std::vector<double> updatedWeights;
  Matrix *updatedMeans = new Matrix[mNumComponents];
  Matrix *updatedVariances = new Matrix[mNumComponents];
  double sum;
  bool thresholdExceeded;
  Matrix diffMatrix, productMatrix;

  numSamples = mTrainingData.size();
  srand(time(NULL));
  for (component = 0; component < mNumComponents; component++)
  {
    updatedWeights.push_back(0);
    updatedMeans[component].SetSize(mNumDimensions, 1);
    updatedVariances[component].SetSize(mNumDimensions, mNumDimensions);
  }
  sampleWeights.SetSize(numSamples, mNumComponents);

  /* Initialize the model */
  mean.SetSize(mNumDimensions, 1, false);
  variance.SetSize(mNumDimensions, mNumDimensions);
  for (component = 0; component < mNumComponents; component++)
  {
    currentGaussian = new Gaussian;
    currentGaussian->SetNumDimensions(mNumDimensions);
    for (dimension = 0; dimension < mNumDimensions; dimension++)
    {
      mean.SetValue(dimension, 0, rand() / (RAND_MAX + 1.0));
      variance.SetValue(dimension, dimension, rand() / (RAND_MAX + 1.0));
    }
    currentGaussian->SetMean(mean);
    currentGaussian->SetVariance(variance);
    components.push_back(currentGaussian);
    componentWeights.push_back(1.0 / mNumComponents);
  }

  totalSamples = 0;
  for (sample = 0; sample < numSamples; sample++)
    totalSamples += mTrainingDataFreq[sample];

  while ( !done )
  {
    /* Get sample probabilities */
    for (sample = 0; sample < numSamples; sample++)
    {
      sampleProb = Probability(*(mTrainingData[sample]), components, componentWeights);
      for (component = 0; component < mNumComponents; component++)
#if 0
        sampleWeights.SetValue(sample, component, 
            components[component]->Probability(*(mTrainingData[sample])) / sampleProb);
#else
        sampleWeights.SetValue(sample, component, 
            componentWeights[component] * components[component]->Probability(*(mTrainingData[sample])) / sampleProb);
#endif
    }

    /* Update component weights */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
#if 0
        sum += sampleWeights.GetValue(sample, component);
      updatedWeights[component] = sum / numSamples;
#else
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
      updatedWeights[component] = sum / totalSamples;
#endif
    }

    /* Update component means */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
#if 0
        sum += sampleWeights.GetValue(sample, component);
#else
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
#endif

      updatedMeans[component].Clear();
      for (sample = 0; sample < numSamples; sample++)
#if 0
        updatedMeans[component] += *(mTrainingData[sample]) * sampleWeights.GetValue(sample, component);
#else
        updatedMeans[component] += *(mTrainingData[sample]) * sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
#endif
      updatedMeans[component] *= 1 / sum;
    }

    /* Update component variances */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
#if 0
        sum += sampleWeights.GetValue(sample, component);
#else
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
#endif

      updatedVariances[component].Clear();

      for (sample = 0; sample < numSamples; sample++)
      {
        diffMatrix = *(mTrainingData[sample]) - updatedMeans[component];
        productMatrix = diffMatrix * diffMatrix.Transpose();
        productMatrix *= sampleWeights.GetValue(sample, component);
#if 0
        updatedVariances[component] += productMatrix;
#else
        for (i = 0; i < mTrainingDataFreq[sample]; i++)
          updatedVariances[component] += productMatrix;
#endif
      }

      updatedVariances[component] *= 1 / sum;
    }

    /* Check if the system has converged sufficiently */
    thresholdExceeded = false;
    for (component = 0; component < mNumComponents; component++)
    {
      if ( fabs(updatedWeights[component] - componentWeights[component]) > WEIGHT_THRESH )
        thresholdExceeded = true;
      if ( components[component]->UpdateMean(updatedMeans[component]) > MEAN_THRESH )
        thresholdExceeded = true;
      if ( components[component]->UpdateVariance(updatedVariances[component]) > VARIANCE_THRESH )
        thresholdExceeded = true;
    }
    componentWeights = updatedWeights;
    if ( !thresholdExceeded )
      done = true;
  }

  mComponents = components;
  mComponentWeights = componentWeights;
  delete[] updatedMeans;
  delete[] updatedVariances;

  return true;
}

#define LABEL "GaussianMixtureModel"
#define LABEL_LEN 20
bool GaussianMixtureModel::Save(FILE* file)
{
  int i;

  if ( !file || mComponents.empty() || mComponentWeights.empty() )
    return false;

  fprintf(file, "%s %d %d\n", LABEL, mNumDimensions, mNumComponents);
  for (i = 0; i < mNumComponents; i++)
  {
    fprintf(file, "%f\n", mComponentWeights[i]);
    mComponents[i]->Save(file);
  }

  return true;
}

#define MAX_STR_LEN 32
bool GaussianMixtureModel::Load(FILE* file)
{
  int i;
  int dimensions, numComponents;
  double weight;
  Gaussian* gaussian;
  char buf[MAX_STR_LEN];

  if ( !file )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - NULL file\n");
    return false;
  }

  Clear();

  fgets(buf, LABEL_LEN + 1, file);
  if ( strncmp(LABEL, buf, LABEL_LEN) )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Header string didn't match\n");
    return false;
  }

  if ( !fscanf(file, "%d %d", &dimensions, &numComponents) )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Failed getting dimensions and components\n");
    return false;
  }

  if ( !Create(dimensions, numComponents) )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Failed Create(%d, %d)\n", dimensions, numComponents);
    return false;
  }

  for (i = 0; i < mNumComponents; i++)
  {
    if ( !fscanf(file, "%lf\n", &weight) )
    {
      fprintf(stderr, "GaussianMixtureModel::Load - Failed getting component weight %d\n", i);
      return false;
    }
    gaussian = new Gaussian;
    if ( !gaussian->Load(file) )
    {
      delete gaussian;
      fprintf(stderr, "GaussianMixtureModel::Load - Failed loading gaussian %d\n", i);
      return false;
    }
    mComponents.push_back(gaussian);
    mComponentWeights.push_back(weight);
  }

  return true;
}
