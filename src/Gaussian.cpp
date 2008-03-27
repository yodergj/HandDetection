#include "Gaussian.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

Gaussian::Gaussian()
{
  mDimensions = 0;
}

Gaussian::~Gaussian()
{
}

int Gaussian::GetNumDimensions()
{
  return mDimensions;
}

bool Gaussian::SetNumDimensions(int dimensions)
{
  if ( dimensions <= 0 )
    return false;

  mDimensions = dimensions;
  return true;
}

bool Gaussian::SetMean(Matrix& mean)
{
  if ( (mDimensions == 0) || (mean.GetRows() != mDimensions) || (mean.GetColumns() != 1) )
    return false;

  mMean = mean;

  return true;
}

double Gaussian::UpdateMean(Matrix& mean)
{
  int i;
  double difference;
  double maxDiff = 0;
  Matrix diffMatrix;

  if ( (mDimensions == 0) || (mean.GetRows() != mDimensions) || (mean.GetColumns() != 1) )
    return -1;

  diffMatrix = mMean - mean;
  for (i = 0; i < mDimensions; i++)
  {
    difference = fabs(diffMatrix.GetValue(i, 0));
    if ( difference > maxDiff )
      maxDiff = difference;
  }
  mMean = mean;

  return maxDiff;
}

bool Gaussian::SetVariance(Matrix& variance)
{
  int i;
  double determinant, piComponent;

  if ( (mDimensions == 0) || (variance.GetRows() != mDimensions) || (variance.GetColumns() != mDimensions) )
    return false;

  mVariance = variance;

  /* Precompute the scaling factor to save time on the probability function */
  if ( !mVariance.GetDeterminant(determinant) )
    fprintf(stderr, "SetVariance: GetDerminant failed\n");

  piComponent = 2 * M_PI;
  for (i = 1; i < mDimensions; i++)
    piComponent *= 2 * M_PI;

  mProbabilityScaleFactor = 1 / sqrt(piComponent * determinant);

  /* Compute the variance inverse to save time on the probability function */
  mVarianceInverse = mVariance.Inverse();

  return true;
}

double Gaussian::UpdateVariance(Matrix& variance)
{
  int i, j;
  double difference;
  double maxDiff = 0;
  Matrix diffMatrix;

  if ( (mDimensions == 0) || (variance.GetRows() != mDimensions) || (variance.GetColumns() != mDimensions) )
    return -1;

  diffMatrix = mVariance - variance;
  for (i = 0; i < mDimensions; i++)
  {
    for (j = 0; j < mDimensions; j++)
    {
      difference = fabs(diffMatrix.GetValue(i, j));
      if ( difference > maxDiff )
        maxDiff = difference;
    }
  }
  if ( !SetVariance(variance) )
    return -1;

  return maxDiff;
}

/* Calculate the value of the multivariate dormal distribution */
double Gaussian::Probability(Matrix& input)
{
  double result;
  Matrix diffMatrix;
  Matrix halfProduct;
  Matrix fullProduct;

  if ( (mDimensions == 0) || (input.GetRows() != mDimensions) || (input.GetColumns() != 1) )
    return false;

  diffMatrix = input - mMean;
  halfProduct = diffMatrix.Transpose() * mVarianceInverse;
  fullProduct = halfProduct * diffMatrix;

  result = mProbabilityScaleFactor * exp(-.5 * fullProduct.GetValue(0, 0));
  if ( result < MIN_PROB )
    result = MIN_PROB;
  return result;
}

#define GAUSSIAN_LABEL "Gaussian"
#define GAUSSIAN_LABEL_LEN 8
#define MEAN_LABEL "Mean"
#define MEAN_LABEL_LEN 4
#define VARIANCE_LABEL "Variance"
#define VARIANCE_LABEL_LEN 8
bool Gaussian::Save(FILE* file)
{
  if ( !file )
    return false;

  fprintf(file, "%s %d\n", GAUSSIAN_LABEL, mDimensions);
  fprintf(file, "%s ", MEAN_LABEL);
  mMean.Save(file);
  fprintf(file, "%s ", VARIANCE_LABEL);
  mVariance.Save(file);

  return true;
}

#define MAX_STR_LEN 16
bool Gaussian::Load(FILE* file)
{
  char buf[MAX_STR_LEN];
  int dimensions;
  Matrix matrix;

  if ( !file )
  {
    fprintf(stderr, "Gaussian::Load - NULL file\n");
    return false;
  }

  fgets(buf, GAUSSIAN_LABEL_LEN + 1, file);
  if ( strncmp(GAUSSIAN_LABEL, buf, GAUSSIAN_LABEL_LEN) )
  {
    fprintf(stderr, "Gaussian::Load - Header string didn't match\n");
    return false;
  }

  if ( !fscanf(file, "%d\n", &dimensions) )
  {
    fprintf(stderr, "Gaussian::Load - Faild getting dimensions\n");
    return false;
  }

  if ( !SetNumDimensions(dimensions) )
  {
    fprintf(stderr, "Gaussian::Load - Faild setting dimensions %d\n", dimensions);
    return false;
  }

  fgets(buf, MEAN_LABEL_LEN + 1, file);
  if ( strncmp(MEAN_LABEL, buf, MEAN_LABEL_LEN) )
  {
    fprintf(stderr, "Gaussian::Load - Mean header string didn't match\n");
    return false;
  }
  fgetc(file); // Skip the space between Mean and Matrix

  if ( !matrix.Load(file) )
  {
    fprintf(stderr, "Gaussian::Load - Failed loading mean\n");
    return false;
  }

  if ( !SetMean(matrix) )
  {
    fprintf(stderr, "Gaussian::Load - Failed setting mean\n");
    return false;
  }

  fgets(buf, VARIANCE_LABEL_LEN + 1, file);
  if ( strncmp(VARIANCE_LABEL, buf, VARIANCE_LABEL_LEN) )
  {
    fprintf(stderr, "Gaussian::Load - Variance header string didn't match\n");
    return false;
  }
  fgetc(file); // Skip the space between Variance and Matrix

  if ( !matrix.Load(file) )
  {
    fprintf(stderr, "Gaussian::Load - Failed loading variance\n");
    return false;
  }

  if ( !SetVariance(matrix) )
  {
    fprintf(stderr, "Gaussian::Load - Failed setting variance\n");
    return false;
  }

  return true;
}
