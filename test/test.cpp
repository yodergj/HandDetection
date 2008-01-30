#include "Gaussian.h"
#include <math.h>

/* One dimensionsal normal probability distribution.  Currently unused */
double GaussianProbability(double value, double mean, double variance)
{
  double offset, result;

  offset = value - mean;
  result = exp( -offset * offset / (2 * variance) );
  result /= sqrt(2 * M_PI * variance);
  return result;
}

int main(int argc, char* argv[])
{
  Gaussian gaussian;
  Matrix mean, variance, input;

  mean.SetSize(2, 1);

#if 0
  mean.SetValue(0, 0, 0.255630);
  mean.SetValue(1, 0, 0.253541);
#endif

  variance.SetSize(2, 2);
#if 0
  variance.SetValue(0, 0, 1);
  variance.SetValue(1, 1, 1);
#endif
#if 0
  variance.SetValue(0, 0, 1.56);
  variance.SetValue(0, 1, -.97);
  variance.SetValue(1, 0, -.97);
  variance.SetValue(1, 1, 2.68);
#endif
#if 0
  variance.SetValue(0, 0,  0.062468);
  variance.SetValue(0, 1, -0.000012);
  variance.SetValue(1, 0, -0.000012);
  variance.SetValue(1, 1, 0.062487);
#endif
#if 1
  variance.SetValue(0, 0, 2.00);
  variance.SetValue(0, 1, 0.00);
  variance.SetValue(1, 0, 0.00);
  variance.SetValue(1, 1, 2.00);
#endif

  input.SetSize(2, 1);

#if 0
  input.SetValue(0, 0, 0.000000);
  input.SetValue(1, 0, 0.500000);
#else
  input.SetValue(0, 0, 2.000000);
  input.SetValue(1, 0, 0.000000);
#endif

  gaussian.SetNumDimensions(2);
  gaussian.SetMean(mean);
  gaussian.SetVariance(variance);

  printf("1D Result %f\n", GaussianProbability(0, 0, 1));
  printf("Multivariate Result %f\n", gaussian.Probability(input));

  return 0;
}
