#ifndef GAUSSIAN_MIXTURE_MODEL_H
#define GAUSSIAN_MIXTURE_MODEL_H

#include "Gaussian.h"
#include "XMLUtils.h"
#include <vector>

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
                       std::vector<Gaussian *>& components,
                       std::vector<double>& weights);

    std::vector<Gaussian *> mComponents;
    std::vector<double> mComponentWeights;
    std::vector<Matrix *> mTrainingData;
    std::vector<int> mTrainingDataFreq;
    Matrix mTrainingDataMin;
    Matrix mTrainingDataMax;
    int mNumDimensions;
    int mNumComponents;
    int* m2dDataHistogram;
    int m2dDataHistogramSize;
};

#endif
