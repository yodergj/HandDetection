#ifndef GAUSSIAN_H
#define GAUUSIAN_H

#include "Matrix.h"
#include "XMLUtils.h"

#define MIN_PROB .000001

#define GAUSSIAN_STR "Gaussian"

class Gaussian
{
  public:
    Gaussian();
    ~Gaussian();
    int GetNumDimensions();
    bool SetNumDimensions(int dimensions);
    bool SetMean(Matrix& mean);
    bool UpdateMean(Matrix& mean, double& maxDifference);
    bool SetVariance(Matrix& variance);
    bool UpdateVariance(Matrix& variance, double& maxDifference);
    double Probability(Matrix& input, bool useMinimumAsNeeded=true);
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr gaussianNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr gaussianNode);
  private:
    int mDimensions;
    Matrix mMean;
    Matrix mVariance;
    Matrix mVarianceInverse;
    double mProbabilityScaleFactor;
    double mMinProb;

    // These would be local, but this reduces memory churn
    Matrix mDiffMatrix;
    Matrix mHalfProduct;
    Matrix mTranspose;
    Matrix mProductMatrix;
};

#endif
