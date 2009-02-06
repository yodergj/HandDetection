#include "BayesianClassifier.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
  BayesianClassifier classifier;
  Matrix input;
  int classIndex;
  double confidence;

  if ( !classifier.Load("test.cfg") )
  {
    fprintf(stderr, "Error loading test.cfg\n");
    exit(1);
  }

  input.SetSize(2, 1);

  input.SetValue(0, 0, 1.5);
  input.SetValue(1, 0, 2.0);
  classifier.Classify(input, classIndex, confidence);
  printf("Point (%f, %f) is class %d, confidence %f\n", input.GetValue(0, 0), input.GetValue(1, 0), classIndex, confidence);

  input.SetValue(0, 0, 1.0);
  input.SetValue(1, 0, 1.0);
  classifier.Classify(input, classIndex, confidence);
  printf("Point (%f, %f) is class %d, confidence %f\n", input.GetValue(0, 0), input.GetValue(1, 0), classIndex, confidence);

  input.SetValue(0, 0, 0.5);
  input.SetValue(1, 0, 0.5);
  classifier.Classify(input, classIndex, confidence);
  printf("Point (%f, %f) is class %d, confidence %f\n", input.GetValue(0, 0), input.GetValue(1, 0), classIndex, confidence);

  return 0;
}
