#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "GaussianMixtureModel.h"

//#define TRAIN_DEBUG

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

#define NUM_DIM_STR "NumDimensions"
#define NUM_GAUSSIAN_STR "NumGaussians"
#define WEIGHT_STR "Weight"

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
  {
    fprintf(stderr, "GaussianMixtureModel::Create - Invalid parameter\n");
    return false;
  }

  Clear();

  mNumDimensions = numDimensions;
  mNumComponents = numComponents;
  mTrainingDataMin.SetSize(mNumDimensions, 1);
  mTrainingDataMax.SetSize(mNumDimensions, 1);
  mScalingFactors.SetSize(mNumDimensions, 1);
  mScaledInput.SetSize(mNumDimensions, 1);

  return true;
}

bool GaussianMixtureModel::AddTrainingData(Matrix& data)
{
  Matrix* duplicate;
  int i, len;
  int minPos, curPos, maxPos;
  double val;

  if ( (data.GetRows() != mNumDimensions) || (data.GetColumns() != 1) )
  {
    fprintf(stderr, "GaussianMixtureModel::AddTrainingData - Size mismatch got %dx%d expected %dx%d\n", data.GetRows(), data.GetColumns(), mNumDimensions, 1);
    return false;
  }

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
  {
    fprintf(stderr, "GaussianMixtureModel::Get2dDataHistogram - Invalid parameter\n");
    return NULL;
  }

  sizeNeeded = binsPerSide * binsPerSide;
  if ( sizeNeeded > m2dDataHistogramSize )
  {
    tmpPtr = (int *)realloc(m2dDataHistogram, sizeNeeded * sizeof(int));
    if ( !tmpPtr )
    {
      fprintf(stderr, "GaussianMixtureModel::Get2dDataHistogram - Failed allocating memory.\n");
      return NULL;
    }
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
  {
    fprintf(stderr, "GaussianMixtureModel::Probability - Size mismatch got %dx%d expected %dx%d\n", data.GetRows(), data.GetColumns(), mNumDimensions, 1);
    return -1;
  }

  return Probability(data, mComponents, mComponentWeights);
}

double GaussianMixtureModel::Probability(Matrix& data,
                                         vector<Gaussian *>& components,
                                         vector<double>& weights)
{
  int i;
  double probability = 0;

  mScaledInput.SetFromCellProducts(data, mScalingFactors);

  for (i = 0; i < mNumComponents; i++)
    probability += components[i]->Probability(mScaledInput) * weights[i];

  if ( probability < MIN_PROB )
    probability = MIN_PROB;

  return probability;
}

bool GaussianMixtureModel::Train()
{
  if ( mTrainingData.empty() )
  {
    fprintf(stderr, "GaussianMixtureModel::Train - No training data\n");
    return false;
  }

#ifdef TRAIN_DEBUG
  unsigned int i;
  for (i = 0; i < mTrainingData.size(); i++)
  {
    fprintf(stderr, "Freq %d\n", mTrainingDataFreq[i]);
    mTrainingData[i]->Print(stderr);
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
  vector<Gaussian *> components;
  vector<double> componentWeights;
  Matrix mean, variance;
  double val, updateThresh, minSpan, scale;

  Matrix sampleWeights;
  double sampleProb;

  vector<double> updatedWeights;
  Matrix *updatedMeans = new Matrix[mNumComponents];
  Matrix *updatedVariances = new Matrix[mNumComponents];
  double sum, varianceDiff, meanDiff;
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

  /* Figure out the scaling factors to use for each dimension.  We want to keep the variances from becoming too small, or else there will be round-off problems with the determinant becoming too small (especially for higher numbers of dimensions). */
  for (dimension = 0; dimension < mNumDimensions; dimension++)
  {
    val = diffMatrix.GetValue(dimension, 0);
    if ( val >= 1 )
      scale = 1;
    else if ( val >= .1 )
      scale = 100.0;
    else if ( val >= .01 )
      scale = 10000.0;
    else if ( val >= .001 )
      scale = 1000000.0;
    else if ( val >= .0001 )
      scale = 100000000.0;
    else
      scale = 10000000000.0;
    mScalingFactors.SetValue(dimension, 0, scale);
  }
  diffMatrix.Scale(mScalingFactors);

#ifdef TRAIN_DEBUG
  printf("Training Data Span\n");
  diffMatrix.Print(stdout);
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
            mTrainingDataMin.GetValue(dimension, 0) * mScalingFactors.GetValue(dimension, 0);
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
    components[foo]->Print(stdout);
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
      // Probability() sets mScaledInput, and we'll take advantage of that below
      sampleProb = Probability(*(mTrainingData[sample]), components, componentWeights);
#ifdef TRAIN_DEBUG
      printf("Sample %d Mix Prob %f\n", sample, sampleProb);
#endif
      for (component = 0; component < mNumComponents; component++)
      {
#ifdef TRAIN_DEBUG
        printf("Sample %d Component %d Prob %f\n", sample, component,
            components[component]->Probability(mScaledInput) );
#endif
        sampleWeights.SetValue(sample, component,
            MAX(MIN_PROB,
                componentWeights[component] * components[component]->Probability(mScaledInput) / sampleProb) );
      }
    }

#ifdef TRAIN_DEBUG
    printf("Sample Weights\n");
    sampleWeights.Print(stdout);
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
      {
        mScaledInput.SetFromCellProducts(*(mTrainingData[sample]), mScalingFactors);
        mScaledInput *= sampleWeights.GetValue(sample, component) * mTrainingDataFreq[sample];
        updatedMeans[component] += mScaledInput;
      }
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
        mScaledInput.SetFromCellProducts(*(mTrainingData[sample]), mScalingFactors);
        diffMatrix = mScaledInput - updatedMeans[component];
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
      if ( !components[component]->UpdateMean(updatedMeans[component], meanDiff) ||
           !components[component]->UpdateVariance(updatedVariances[component], varianceDiff) )
      {
        fprintf(stderr, "TrainEM: Failed updating mean or variance\n");
        delete[] updatedMeans;
        delete[] updatedVariances;
        return false;
      }
      if ( (meanDiff > updateThresh) || (varianceDiff > updateThresh) )
        thresholdExceeded = true;
    }

#ifdef TRAIN_DEBUG
  printf("Update %d-------------------------------------------------\n", bar);
  bar++;
  for (foo = 0; foo < mNumComponents; foo++)
  {
    printf("Weight %f\n", componentWeights[foo]);
    components[foo]->Print(stdout);
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

bool GaussianMixtureModel::Print(FILE* file)
{
  int i;

  if ( !file || mComponents.empty() || mComponentWeights.empty() )
  {
    fprintf(stderr, "GaussianMixtureModel::Print - NULL file or empty model\n");
    return false;
  }

  fprintf(file, "%s %d %d\n", GMM_STR, mNumDimensions, mNumComponents);
  for (i = 0; i < mNumComponents; i++)
  {
    fprintf(file, "%f\n", mComponentWeights[i]);
    mComponents[i]->Print(file);
  }

  return true;
}

bool GaussianMixtureModel::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "GaussianMixtureModel::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "GaussianMixtureModel::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)GMM_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool GaussianMixtureModel::Save(xmlNodePtr modelNode)
{
  int i;
  bool retCode = true;
  xmlNodePtr gaussianNode;
  xmlNodePtr scaleNode;

  SetIntValue(modelNode, NUM_DIM_STR, mNumDimensions);
  SetIntValue(modelNode, NUM_GAUSSIAN_STR, mNumComponents);

  scaleNode = xmlNewNode(NULL, (const xmlChar*)MATRIX_STR);
  xmlAddChild(modelNode, scaleNode);
  retCode = mScalingFactors.Save(scaleNode);

  for (i = 0; (i < mNumComponents) && retCode; i++)
  {
    gaussianNode = xmlNewNode(NULL, (const xmlChar*)GAUSSIAN_STR);
    xmlAddChild(modelNode, gaussianNode);
    SetDoubleValue(gaussianNode, WEIGHT_STR, mComponentWeights[i]);
    retCode = mComponents[i]->Save(gaussianNode);
  }

  return retCode;
}

bool GaussianMixtureModel::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "GaussianMixtureModel:Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "GaussianMixtureModel::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool GaussianMixtureModel::Load(xmlNodePtr modelNode)
{
  int numGaussiansFound, dimensions, numGaussians;
  xmlNodePtr node;
  double weight;
  Gaussian* gaussian;
  bool retCode = true;
  bool scalingFound = false;

  dimensions = GetIntValue(modelNode, NUM_DIM_STR, 0);
  numGaussians = GetIntValue(modelNode, NUM_GAUSSIAN_STR, 0);
  if ( (dimensions < 1) || (numGaussians < 1) )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Invalid property\n");
    return false;
  }

  if ( !Create(dimensions, numGaussians) )
  {
    fprintf(stderr, "GaussianMixtureModel::Load - Failed Create(%d, %d)\n", dimensions, numGaussians);
    return false;
  }

  numGaussiansFound = 0;
  node = modelNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, GAUSSIAN_STR) )
    {
      if ( numGaussiansFound == mNumComponents )
      {
        fprintf(stderr, "GaussianMixtureModel::Load - Too many gaussians found\n");
        retCode = false;
      }
      else
      {
        weight = GetDoubleValue(node, WEIGHT_STR, 1.0 / mNumComponents);
        gaussian = new Gaussian;
        if ( gaussian->Load(node) )
        {
          mComponents.push_back(gaussian);
          mComponentWeights.push_back(weight);
          numGaussiansFound++;
        }
        else
        {
          delete gaussian;
          fprintf(stderr, "GaussianMixtureModel::Load - Failed loading gaussian %d\n", numGaussiansFound);
          retCode = false;
        }
      }
    }
    else if ( !strcmp((char *)node->name, MATRIX_STR) )
    {
      if ( scalingFound )
      {
        fprintf(stderr, "GaussianMixtureModel::Load - Multiple sets of scaling factors found\n");
        retCode = false;
      }
      else if ( mScalingFactors.Load(node) )
        scalingFound = true;
      else
      {
        fprintf(stderr, "GaussianMixtureModel::Load - Failed loading scaling factors\n");
        retCode = false;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "GaussianMixtureModel::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  if ( retCode && !scalingFound )
    mScalingFactors.Fill(1);

  return retCode;
}