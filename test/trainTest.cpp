#include "GaussianMixtureModel.h"
#include <math.h>

#define DIMENSIONS 3

int main(int argc, char* argv[])
{
  GaussianMixtureModel model;
  Gaussian gaussian;
  Matrix mean, variance, input;

#if 0
  mean.SetSize(2, 1);
  variance.SetSize(2, 2);
  variance.SetValue(0, 0, 1);
  variance.SetValue(1, 1, 1);

  gaussian.SetNumDimensions(2);
  gaussian.SetMean(mean);
  gaussian.SetVariance(variance);
#endif
  input.SetSize(DIMENSIONS, 1);

  model.Create(DIMENSIONS, 5);

  model.AddTrainingData(input);

  input.SetValue(0, 0, .5);
  model.AddTrainingData(input);

  input.SetValue(1, 0, .5);
  model.AddTrainingData(input);

  input.SetValue(0, 0, 0);
  model.AddTrainingData(input);

  input.SetValue(2, 0, .8);
  model.AddTrainingData(input);

  input.SetValue(0, 0, .123);
  input.SetValue(1, 0, .456);
  input.SetValue(2, 0, .789);
  model.AddTrainingData(input);

  input.SetValue(0, 0, .9);
  input.SetValue(1, 0, .01);
  input.SetValue(2, 0, .5);
  model.AddTrainingData(input);

  input.SetValue(0, 0, .42);
  input.SetValue(1, 0, .87);
  input.SetValue(2, 0, .11);
  model.AddTrainingData(input);

  input.SetValue(0, 0, .32);
  input.SetValue(1, 0, .22);
  input.SetValue(2, 0, .44);
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
  model.Save(stdout);

#if 0
  printf("Multivariate Result %f\n", gaussian.Probability(input));
#endif
  printf("Mixture Result %f\n", model.Probability(input));

  return 0;
}
