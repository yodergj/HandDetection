#include "BayesianClassifier.h"

int main(int argc, char* argv[])
{
  BayesianClassifier classifier;
  Matrix input;
  int classIndex;
  double confidence;
  FILE *file;

  input.SetSize(2, 1);

  classifier.Create(2, 2);

  classifier.AddTrainingData(input, 0);
  input.SetValue(0, 0, .5);
  classifier.AddTrainingData(input, 0);
  input.SetValue(1, 0, .5);
  classifier.AddTrainingData(input, 0);
  input.SetValue(0, 0, 0);
  classifier.AddTrainingData(input, 0);

  input.SetValue(0, 0, 1.5);
  input.SetValue(1, 0, 1.5);
  classifier.AddTrainingData(input, 1);
  input.SetValue(0, 0, 2.0);
  classifier.AddTrainingData(input, 1);
  input.SetValue(1, 0, 2.0);
  classifier.AddTrainingData(input, 1);
  input.SetValue(0, 0, 1.5);
  classifier.AddTrainingData(input, 1);

  classifier.Train();

  file = fopen("test.cfg", "w");
  classifier.Save(file);
  fclose(file);

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
