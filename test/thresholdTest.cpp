#include <stdio.h>
#include "ThresholdClassifier.h"

int main(int argc, char* argv[])
{
  int i;
  int numSamples;
  ThresholdClassifier classifier;
  vector<double> samples;
  vector<double> weights;
  vector<int> classes;
  double error;

  samples.push_back(1);
  samples.push_back(2);
  samples.push_back(3);
  samples.push_back(4);
  samples.push_back(5);

  classes.push_back(1);
  classes.push_back(1);
  classes.push_back(0);
  classes.push_back(0);
  classes.push_back(1);

  numSamples = samples.size();
  for (i = 0; i < numSamples; i++)
  {
    weights.push_back( 1.0 / numSamples );
  }
#if 1
  weights[0] *= .5;
  weights[1] *= .5;
  weights[4] *= 2;
#endif

  if ( classifier.Train(samples, weights, classes, &error) )
  {
    classifier.Print(stdout);
    printf("Training Error %f\n", error);
  }
  else
  {
    printf("Error in training\n");
  }

  return 0;
}
