#ifndef GAUSSIAN_MIXTURE_MODEL_H
#define GAUSSIAN_MIXTURE_MODEL_H

#include <vector>
#include "Gaussian.h"
#include "XMLUtils.h"
using std::vector;

#define GMM_STR "GaussianMixtureModel"

class GaussianMixtureModel
{
  public:
    GaussianMixtureModel();
    ~GaussianMixtureModel();
    bool Create(int numDimensions, int numComponents);
    bool AddTrainingData(Matrix& data);
    int* Get2dDataHistogram(int binsPerSide, double scaleFactor);
    double Probability(Matrix& data);
    bool Train();
    bool Print(FILE* file);
    bool Save(const char* filename);
    bool Save(xmlNodePtr modelNode);
    bool Load(const char* filename);
    bool Load(xmlNodePtr modelNode);
  private:
    void Clear();
    bool TrainEM();
    double Probability(Matrix& data,
                       vector<Gaussian *>& components,
                       vector<double>& weights);

    vector<Gaussian *> mComponents;
    vector<double> mComponentWeights;
    vector<Matrix *> mTrainingData;
    vector<int> mTrainingDataFreq;
    Matrix mTrainingDataMin;
    Matrix mTrainingDataMax;
    Matrix mScalingFactors;
    Matrix mScaledInput;
    int mNumDimensions;
    int mNumComponents;
    int* m2dDataHistogram;
    int m2dDataHistogramSize;
};

#endif
