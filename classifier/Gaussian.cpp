#include "Gaussian.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MIN_DIAG_VARIANCE .000000001
#define MIN_DETERMINANT   .000000000001
#define MIN_VARIANCE_ADJUST .000001

#define NUM_DIM_STR "NumDimensions"
#define MEAN_STR "Mean"
#define VARIANCE_STR "Variance"
#define ID_STR "Identifier"

Gaussian::Gaussian()
{
  mDimensions = 0;
  mProbabilityScaleFactor = 0;
  mMinProb = MIN_PROB;
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
  {
    fprintf(stderr, "Gaussian::SetNumDimensions - Invalid parameter\n");
    return false;
  }

  mDimensions = dimensions;

  /* Make sure that the Matrix memory gets allocated while we're still just
     in the setup portion of processing. */
  mMean.SetSize(dimensions, 1, false);
  mVariance.SetSize(dimensions, dimensions, false);
  mVarianceInverse.SetSize(dimensions, dimensions, false);

  mDiffMatrix.SetSize(dimensions, dimensions, false);
  mHalfProduct.SetSize(dimensions, dimensions, false);
  mTranspose.SetSize(1, dimensions, false);
  mProductMatrix.SetSize(1, 1, false);

  return true;
}

bool Gaussian::SetMean(Matrix& mean)
{
  if ( (mDimensions == 0) || (mean.GetRows() != mDimensions) || (mean.GetColumns() != 1) )
  {
    fprintf(stderr, "Gaussian::SetMean - Invalid parameter\n");
    return false;
  }

  mMean = mean;

  return true;
}

bool Gaussian::UpdateMean(Matrix& mean, double& maxDifference)
{
  int i;
  double difference;

  if ( (mDimensions == 0) || (mean.GetRows() != mDimensions) || (mean.GetColumns() != 1) )
  {
    fprintf(stderr, "Gaussian::UpdateMean - Invalid parameter\n");
    return false;
  }

  maxDifference = 0;
  mDiffMatrix = mMean - mean;
  for (i = 0; i < mDimensions; i++)
  {
    difference = fabs(mDiffMatrix.GetValue(i, 0));
    if ( difference > maxDifference )
      maxDifference = difference;
  }
  mMean = mean;

  return true;
}

bool Gaussian::SetVariance(Matrix& variance)
{
  int i;
  double determinant, piComponent;

  if ( (mDimensions == 0) || (variance.GetRows() != mDimensions) || (variance.GetColumns() != mDimensions) )
  {
    fprintf(stderr, "Gaussian::SetVariance - Invalid parameter\n");
    return false;
  }

  mVariance = variance;

  for (i = 0; i < mDimensions; i++)
    if ( mVariance.GetValue(i, i) < MIN_DIAG_VARIANCE )
      mVariance.SetValue(i, i, MIN_DIAG_VARIANCE);

  /* Precompute the scaling factor to save time on the probability function */
  if ( !mVariance.GetDeterminant(determinant) )
  {
    fprintf(stderr, "Gaussian::SetVariance - GetDeterrminant failed\n");
    return false;
  }

  if ( determinant < MIN_DETERMINANT )
  {
    fprintf(stderr, "Gaussian::SetVariance - Determinant is not positive.\n");
    return false;
  }

  piComponent = 2 * M_PI;
  for (i = 1; i < mDimensions; i++)
    piComponent *= 2 * M_PI;

  mProbabilityScaleFactor = 1 / sqrt(piComponent * determinant);

  /* Compute the variance inverse to save time on the probability function */
  mVarianceInverse.SetAsInverse(mVariance);

  mMinProb = MIN_PROB * Probability(mMean, false);

  return true;
}

bool Gaussian::UpdateVariance(Matrix& variance, double& maxDifference)
{
  int i, j;
  double difference, determinant, adjustment, value;

  if ( (mDimensions == 0) || (variance.GetRows() != mDimensions) || (variance.GetColumns() != mDimensions) )
  {
    fprintf(stderr, "Gaussian::UpdateVariance - Invalid parameter\n");
    return false;
  }

  determinant = -1;

  while ( determinant < MIN_DETERMINANT )
  {
    if ( !variance.GetDeterminant(determinant) )
    {
      fprintf(stderr, "Gaussian::UpdateVariance - GetDeterminant failed\n");
      return false;
    }

    if ( determinant < MIN_DETERMINANT )
    {
      adjustment = fabs(determinant);
      if ( adjustment < MIN_VARIANCE_ADJUST )
        adjustment = MIN_VARIANCE_ADJUST;
      fprintf(stderr, "Gaussian::UpdateVariance - Determinant is not positive - adjusting diagonal by %f\n", adjustment);
      for (i = 0; i < mDimensions; i++)
      {
        value = variance.GetValue(i, i);
        value += adjustment;
        variance.SetValue(i, i, value);
      }
    }
  }

  maxDifference = 0;
  mDiffMatrix = mVariance - variance;
  for (i = 0; i < mDimensions; i++)
  {
    for (j = 0; j < mDimensions; j++)
    {
      difference = fabs(mDiffMatrix.GetValue(i, j));
      if ( difference > maxDifference )
        maxDifference = difference;
    }
  }
  if ( !SetVariance(variance) )
  {
    fprintf(stderr, "Gaussian::UpdateVariance - SetVariance failed.\n");
    return false;
  }

  return true;
}

/* Calculate the value of the multivariate dormal distribution */
double Gaussian::Probability(Matrix& input, bool useMinimumAsNeeded)
{
  double result, fullProduct;

  if ( (mDimensions == 0) || (input.GetRows() != mDimensions) || (input.GetColumns() != 1) )
  {
    fprintf(stderr, "Gaussian::Probability - Invalid parameter\n");
    return -1;
  }

  mDiffMatrix.SetFromDifference(input, mMean);
  mTranspose.SetAsTranspose(mDiffMatrix);
  mHalfProduct.SetFromProduct(mTranspose, mVarianceInverse);
  mProductMatrix.SetFromProduct(mHalfProduct, mDiffMatrix);
  fullProduct = mProductMatrix.GetValue(0, 0);

  result = mProbabilityScaleFactor * exp(-.5 * fullProduct);
  if ( useMinimumAsNeeded && (result < mMinProb) )
    result = mMinProb;
  return result;
}

bool Gaussian::Print(FILE* file)
{
  if ( !file )
  {
    fprintf(stderr, "Gaussian::Print - Invalid parameter\n");
    return false;
  }

  fprintf(file, "Gaussian %d\n", mDimensions);
  fprintf(file, "Mean ");
  mMean.Print(file);
  fprintf(file, "Variance ");
  mVariance.Print(file);

  return true;
}

bool Gaussian::Save(const char* filename)
{
  FILE* file;
  xmlDocPtr document;
  xmlNodePtr rootNode;
  bool retCode = true;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "Gaussian::Save - Invalid parameter\n");
    return false;
  }

  file = fopen(filename, "w");
  if ( !file )
  {
    fprintf(stderr, "Gaussian::Save - Failed opening %s\n", filename);
    return false;
  }

  document = xmlNewDoc(NULL);
  rootNode = xmlNewDocNode(document, NULL, (const xmlChar *)GAUSSIAN_STR, NULL);
  xmlDocSetRootElement(document, rootNode);

  retCode = Save(rootNode);

  xmlDocFormatDump(file, document, 1);
  fclose(file);
  xmlFreeDoc(document);

  return retCode;
}

bool Gaussian::Save(xmlNodePtr gaussianNode)
{
  xmlNodePtr meanNode;
  xmlNodePtr varianceNode;

  SetIntValue(gaussianNode, NUM_DIM_STR, mDimensions);

  meanNode = xmlNewNode(NULL, (const xmlChar*)MATRIX_STR);
  xmlAddChild(gaussianNode, meanNode);
  SetStringValue(meanNode, ID_STR, MEAN_STR);
  if ( !mMean.Save(meanNode) )
  {
    fprintf(stderr, "Gaussian::Save - Failed saving mean\n");
    return false;
  }

  varianceNode = xmlNewNode(NULL, (const xmlChar*)MATRIX_STR);
  xmlAddChild(gaussianNode, varianceNode);
  SetStringValue(varianceNode, ID_STR, VARIANCE_STR);
  if ( !mVariance.Save(varianceNode) )
  {
    fprintf(stderr, "Gaussian::Save - Failed saving covariance\n");
    return false;
  }

  return true;
}

bool Gaussian::Load(const char* filename)
{
  bool retCode = true;
  xmlDocPtr document;
  xmlNodePtr node;

  if ( !filename || !*filename )
  {
    fprintf(stderr, "Gaussian::Load - Bad filename\n");
    return false;
  }

  document = xmlParseFile(filename);

  if ( !document )
  {
    fprintf(stderr, "Gaussian::Load - Failed parsing %s\n", filename);
    return false;
  }

  node = xmlDocGetRootElement(document);
  if ( !node )
  {
    xmlFreeDoc(document);
    fprintf(stderr, "Gaussian::Load - No root node in %s\n", filename);
    return false;
  }

  retCode = Load(node);

  xmlFreeDoc(document);

  return retCode;
}

bool Gaussian::Load(xmlNodePtr gaussianNode)
{
  int dimensions;
  xmlNodePtr node;
  Matrix matrix;
  string matrixID;
  bool meanFound = false;
  bool varianceFound = false;
  bool retCode = true;

  dimensions = GetIntValue(gaussianNode, NUM_DIM_STR, 0);
  if ( dimensions < 1 )
  {
    fprintf(stderr, "Gaussian::Load - Invalid property\n");
    return false;
  }

  if ( !SetNumDimensions(dimensions) )
  {
    fprintf(stderr, "Gaussian::Load - Failed setting dimensions %d\n", dimensions);

    return false;
  }

  node = gaussianNode->children;
  while ( node && retCode )
  {
    if ( !strcmp((char *)node->name, MATRIX_STR) )
    {
      matrixID = GetStringValue(node, ID_STR);
      if ( matrixID == MEAN_STR )
      {
        if ( meanFound )
        {
          fprintf(stderr, "Gaussian::Load - Found extra mean matrix\n");
          retCode = false;
        }
        else
        {
          if ( !matrix.Load(node) )
          {
            fprintf(stderr, "Gaussian::Load - Failed loading mean\n");
            retCode = false;
          }
          else if ( !SetMean(matrix) )
          {
            fprintf(stderr, "Gaussian::Load - Failed setting mean\n");
            retCode = false;
          }
        }
        meanFound = true;
      }
      else if ( matrixID == VARIANCE_STR )
      {
        if ( varianceFound )
        {
          fprintf(stderr, "Gaussian::Load - Found extra variance matrix\n");
          retCode = false;
        }
        else
        {
          if ( !matrix.Load(node) )
          {
            fprintf(stderr, "Gaussian::Load - Failed loading variance\n");
            retCode = false;
          }
          else if ( !SetVariance(matrix) )
          {
            fprintf(stderr, "Gaussian::Load - Failed setting variance\n");
            retCode = false;
          }
        }
        varianceFound = true;
      }
      else
      {
        fprintf(stderr, "Gaussian::Load - Found unknown matrix with ID %s\n", matrixID.c_str());
        retCode = false;
      }
    }
    else if ( strcmp((char *)node->name, "text") )
    {
      fprintf(stderr, "Gaussian::Load - Found unknown node %s\n", (char*)node->name);
      retCode = false;
    }
    node = node->next;
  }

  return retCode;
}
