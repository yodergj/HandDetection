#include "GaussianMixtureModel.h"
#include "math.h"

//#define TRAIN_DEBUG

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

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
  mTrainingDataMin.SetSize(mNumDimensions, 1);
  mTrainingDataMax.SetSize(mNumDimensions, 1);

  return true;
}

bool GaussianMixtureModel::AddTrainingData(Matrix& data)
{
  Matrix* duplicate;
  int i, len;
  int minPos, curPos, maxPos;
  double val;

  if ( (data.GetRows() != mNumDimensions) || (data.GetColumns() != 1) )
    return false;

  if ( mTrainingData.empty() )
  {
    duplicate = new Matrix;
    *duplicate = data;
    mTrainingData.push_back(duplicate);
    mTrainingDataFreq.push_back(1);

    mTrainingDataMin = data;
    mTrainingDataMax = data;

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

  for (i = 0; i < mNumDimensions; i++)
  {
    val = data.GetValue(i, 0);
    if ( val < mTrainingDataMin.GetValue(i, 0) )
      mTrainingDataMin.SetValue(i, 0, val);
    if ( val > mTrainingDataMax.GetValue(i, 0) )
      mTrainingDataMax.SetValue(i, 0, val);
  }

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

  if ( probability < MIN_PROB )
    probability = MIN_PROB;

  return probability;
}

bool GaussianMixtureModel::Train()
{
  if ( mTrainingData.empty() )
    return false;

#ifdef TRAIN_DEBUG
  int i;
  for (i = 0; i < mTrainingData.size(); i++)
  {
    fprintf(stderr, "Freq %d\n", mTrainingDataFreq[i]);
    mTrainingData[i]->Save(stderr);
  }
#endif

  return TrainEM();
}

#define THRESH_PERCENT  .005
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
  double val, updateThresh, minSpan;

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
  diffMatrix = mTrainingDataMax - mTrainingDataMin;

#ifdef TRAIN_DEBUG
  printf("Training Data Span\n");
  diffMatrix.Save(stdout);
#endif

  minSpan = -1;
  for (dimension = 0; dimension < mNumDimensions; dimension++)
  {
    val = diffMatrix.GetValue(dimension, 0);
    if ( (val > 0) && ( (minSpan == -1) || (val < minSpan) ) )
      minSpan = val;
  }
  if ( minSpan == -1 )
    updateThresh = THRESH_PERCENT;
  else
    updateThresh = minSpan * THRESH_PERCENT;

  for (component = 0; component < mNumComponents; component++)
  {
    currentGaussian = new Gaussian;
    currentGaussian->SetNumDimensions(mNumDimensions);
    for (dimension = 0; dimension < mNumDimensions; dimension++)
    {
      val = (rand() / (double)RAND_MAX) * diffMatrix.GetValue(dimension, 0) +
            mTrainingDataMin.GetValue(dimension, 0);
      mean.SetValue(dimension, 0, val);
      val = diffMatrix.GetValue(dimension, 0) * (rand() + 1.0) / (RAND_MAX + 1.0);
      if ( val < updateThresh )
        val = updateThresh;
      variance.SetValue(dimension, dimension, val);
    }
    currentGaussian->SetMean(mean);
    currentGaussian->SetVariance(variance);
    components.push_back(currentGaussian);
    componentWeights.push_back(1.0 / mNumComponents);
  }

#ifdef TRAIN_DEBUG
  int foo, bar;
  bar = 0;
  printf("Initial components\n");
  for (foo = 0; foo < mNumComponents; foo++)
  {
    printf("Weight %f\n", componentWeights[foo]);
    components[foo]->Save(stdout);
  }
#endif

  totalSamples = 0;
  for (sample = 0; sample < numSamples; sample++)
    totalSamples += mTrainingDataFreq[sample];

  while ( !done )
  {
    /* Get sample probabilities */
    for (sample = 0; sample < numSamples; sample++)
    {
      sampleProb = Probability(*(mTrainingData[sample]), components, componentWeights);
#ifdef TRAIN_DEBUG
      printf("Sample %d Mix Prob %f\n", sample, sampleProb);
#endif
      for (component = 0; component < mNumComponents; component++)
      {
#ifdef TRAIN_DEBUG
        printf("Sample %d Component %d Prob %f\n", sample, component,
            components[component]->Probability(*(mTrainingData[sample])) );
#endif
        sampleWeights.SetValue(sample, component, 
            MAX(MIN_PROB, 
                componentWeights[component] * components[component]->Probability(*(mTrainingData[sample])) / sampleProb) );
      }
    }

#ifdef TRAIN_DEBUG
    printf("Sample Weights\n");
    sampleWeights.Save(stdout);
#endif

    /* Update component weights */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
      updatedWeights[component] = sum / totalSamples;
    }

    /* Update component means */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];

      updatedMeans[component].Clear();
      for (sample = 0; sample < numSamples; sample++)
        updatedMeans[component] += *(mTrainingData[sample]) * sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
      updatedMeans[component] *= 1 / sum;
    }

    /* Update component variances */
    for (component = 0; component < mNumComponents; component++)
    {
      sum = 0;
      for (sample = 0; sample < numSamples; sample++)
        sum += sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];

      updatedVariances[component].Clear();

      for (sample = 0; sample < numSamples; sample++)
      {
        diffMatrix = *(mTrainingData[sample]) - updatedMeans[component];
        productMatrix = diffMatrix * diffMatrix.Transpose();
        productMatrix *= sampleWeights.GetValue(sample, component);
        for (i = 0; i < mTrainingDataFreq[sample]; i++)
          updatedVariances[component] += productMatrix;
      }

      updatedVariances[component] *= 1 / sum;
    }

    /* Check if the system has converged sufficiently */
    thresholdExceeded = false;
    for (component = 0; component < mNumComponents; component++)
    {
      if ( fabs(updatedWeights[component] - componentWeights[component]) > WEIGHT_THRESH )
        thresholdExceeded = true;
      if ( components[component]->UpdateMean(updatedMeans[component]) > updateThresh )
        thresholdExceeded = true;
      if ( components[component]->UpdateVariance(updatedVariances[component]) > updateThresh )
        thresholdExceeded = true;
    }

#ifdef TRAIN_DEBUG
  printf("Update %d-------------------------------------------------\n", bar);
  bar++;
  for (foo = 0; foo < mNumComponents; foo++)
  {
    printf("Weight %f\n", componentWeights[foo]);
    components[foo]->Save(stdout);
  }
#endif

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
