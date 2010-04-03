#include <string.h>
#include "AdaboostClassifier.h"
#include "Image.h"

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

int main(int argc, char* argv[])
{
  Image inputImage;
  unsigned char* srcPixel;
  int i, j;
  int x, y;
  int width, height;
  AdaboostClassifier classifier;
  Matrix input;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int revNumber = 0;
  double* featureBuffer;
  double* pixel;
  string featureList;
  int numFeatures, numWeakClassifiers;

  if ( argc < 6 )
  {
    printf("Usage: %s <num weak classifiers> <feature string> <flesh Image> [...] -x <non-flesh image> [...]\n", argv[0]);
    return 0;
  }

  numWeakClassifiers = atoi(argv[1]);
  if ( numWeakClassifiers < 1 )
  {
    printf("Invalid number of weak classifiers %d\n", numWeakClassifiers );
    return 1;
  }
  featureList = argv[2];
  numFeatures = featureList.size();
  input.SetSize(numFeatures, 1);
  classifier.Create(numFeatures, 2, numWeakClassifiers);
  classifier.SetFeatureString(featureList);

  for (i = 3; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.Load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing flesh image %s\n", argv[i]);
      else
        printf("Processing other image %s\n", argv[i]);
      width = inputImage.GetWidth();
      height = inputImage.GetHeight();
      srcPixel = inputImage.GetRGBBuffer();
      featureBuffer = inputImage.GetCustomBuffer(featureList);
      pixel = featureBuffer;
      for (y = 0; y < height; y++)
      {
        for (x = 0; x < width; x++, srcPixel += 3, pixel += numFeatures)
        {
          if ( (classIndex == 0) &&
               (MAX(srcPixel[0],
                    MAX(srcPixel[1], srcPixel[2]) ) <= 55) )
            continue;

          for (j = 0; j < numFeatures; j++)
            input.SetValue(j, 0, pixel[j]);
          classifier.AddTrainingData(input, classIndex);
        }
      }
    }
  }

  printf("Starting training\n");
  classifier.Train();

  do
  {
    sprintf(filename, "adaflesh-%d.rev%d.cfg", numWeakClassifiers, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  classifier.Save(filename);

  return 0;
}
