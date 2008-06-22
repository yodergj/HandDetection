#ifndef GAUSSIAN_H
#define GAUUSIAN_H

#include "Matrix.h"

#define MIN_PROB .000001

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
    double Probability(Matrix& input);
    bool Save(FILE* file);
    bool Load(FILE* file);
  private:
    int mDimensions;
    Matrix mMean;
    Matrix mVariance;
    Matrix mVarianceInverse;
    double mProbabilityScaleFactor;

    // These would be local, but this reduces memory churn
    Matrix mDiffMatrix;
    Matrix mHalfProduct;
};

#endif
