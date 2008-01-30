#include "GaussianMixtureModel.h"
#include <math.h>

int main(int argc, char* argv[])
{
  GaussianMixtureModel model;
  Gaussian gaussian;
  Matrix mean, variance, input;

  mean.SetSize(2, 1);
  variance.SetSize(2, 2);
  variance.SetValue(0, 0, 1);
  variance.SetValue(1, 1, 1);
  input.SetSize(2, 1);

  gaussian.SetNumDimensions(2);
  gaussian.SetMean(mean);
  gaussian.SetVariance(variance);

  //model.Create(2, 1);
  model.Create(2, 2);

  model.AddTrainingData(input);

  input.SetValue(0, 0, .5);
  model.AddTrainingData(input);

  input.SetValue(1, 0, .5);
  model.AddTrainingData(input);

  input.SetValue(0, 0, 0);
  model.AddTrainingData(input);

#if 0
  input.SetValue(0, 0, -1);
  model.AddTrainingData(input);

  input.SetValue(0, 0, 0);
  model.AddTrainingData(input);
  input.SetValue(1, 0, 2);
  model.AddTrainingData(input);
#endif

  model.Train();

  printf("Multivariate Result %f\n", gaussian.Probability(input));
  printf("Mixture Result %f\n", model.Probability(input));

  return 0;
}
