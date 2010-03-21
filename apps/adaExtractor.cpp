#include <stdlib.h>
#include <string.h>
#include <string>
#include "AdaboostClassifier.h"
#include "Image.h"

using std::string;

int main(int argc, char* argv[])
{
  Image image;
  Image fleshImage, nonFleshImage;
  string basename;
  unsigned char* srcPixel;
  unsigned char* fleshPixel;
  unsigned char* nonFleshPixel;
  int i;
  int x, y;
  int width, height;
  int imageIndex, dotPos;
  AdaboostClassifier classifier;
  Matrix input;
  int classIndex;
  unsigned char white[] = {255, 255, 255};

  double* featureBuffer;
  double* featurePixel;
  string featureList;
  int numFeatures;

  if ( argc < 3 )
  {
    printf("Usage: %s <classifier> <image> [<image> ...]\n", argv[0]);
    return 0;
  }

  if ( !classifier.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading %s\n", argv[1]);
    exit(1);
  }

  featureList = classifier.GetFeatureString();
  numFeatures = featureList.size();

  input.SetSize(numFeatures, 1);

  for (imageIndex = 2; imageIndex < argc; imageIndex++)
  {
    if ( !image.Load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(1);
    }

    basename = argv[imageIndex];
    dotPos = basename.rfind('.');
    if ( dotPos != (int)string::npos )
      basename = basename.substr(0, dotPos);

    width = image.GetWidth();
    height = image.GetHeight();

    featureBuffer = image.GetCustomBuffer(featureList);
    featurePixel = featureBuffer;

    fleshImage.Create(width, height);
    nonFleshImage.Create(width, height);

    srcPixel = image.GetRGBBuffer();
    fleshPixel = fleshImage.GetRGBBuffer();
    nonFleshPixel = nonFleshImage.GetRGBBuffer();
    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++, srcPixel += 3, fleshPixel += 3, nonFleshPixel += 3, featurePixel += numFeatures)
      {
        for (i = 0; i < numFeatures; i++)
          input.SetValue(i, 0, featurePixel[i]);
        classIndex = classifier.Classify(input);

        if ( classIndex == 0 )
        {
          // Pixel is flesh colored
          memcpy(fleshPixel, srcPixel, 3);
          memcpy(nonFleshPixel, white, 3);
        }
        else
        {
          // Pixel is not flesh colored
          memcpy(fleshPixel, white, 3);
          memcpy(nonFleshPixel, srcPixel, 3);
        }
      }
    }

    fleshImage.Save(basename + "_flesh.png");
    nonFleshImage.Save(basename + "_nonflesh.png");
  }

  return 0;
}
