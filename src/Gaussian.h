#ifndef GAUSSIAN_H
#define GAUUSIAN_H

#include "Matrix.h"

class Gaussian
{
  public:
    Gaussian();
    ~Gaussian();
    int GetNumDimensions();
    bool SetNumDimensions(int dimensions);
    bool SetMean(Matrix& mean);
    double UpdateMean(Matrix& mean);
    bool SetVariance(Matrix& variance);
    double UpdateVariance(Matrix& variance);
    double Probability(Matrix& input);
    bool Save(FILE* file);
    bool Load(FILE* file);
  private:
    int mDimensions;
    Matrix mMean;
    Matrix mVariance;
    Matrix mVarianceInverse;
    double mProbabilityScaleFactor;
};

#endif
