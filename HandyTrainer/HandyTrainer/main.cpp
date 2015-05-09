#include <string.h>
#include <string>
#include <QtCore/QCoreApplication>
#include "AdaboostClassifier.h"
#include "Image.h"
#include "ColorRegion.h"

int main(int argc, char *argv[])
{
#if 0
  QCoreApplication a(argc, argv);
  return a.exec();
#endif

  int i, j, k;
  int width, height;
  int numWeakClassifiers;
  int classIndex = 0;
  string className;
  Image image;
  AdaboostClassifier handClassifier;
  char filename[256];
  FILE *file;
  int revNumber = 0;

  if ( argc < 6 )
  {
    printf("Usage: %s <class name> <num weak classifiers> <hand class Image> [...] -x <hand non-class image> [...]\n", argv[0]);
    return 0;
  }

  className = argv[1];

  numWeakClassifiers = atoi(argv[2]);
  if ( numWeakClassifiers < 1 )
  {
    printf("Invalid number of weak classifiers %d\n", numWeakClassifiers );
    return 1;
  }

  for (i = 3; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( image.Load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand class image %s\n", argv[i]);
      else
        printf("Processing other hand image %s\n", argv[i]);
      width = image.GetWidth();
      height = image.GetHeight();
      Point startPt(width / 2, height / 2);
      ColorRegion region;
      if ( region.Grow(image, startPt) )
      {
        int* integralBuffer = region.GetIntegralBuffer();
        if ( integralBuffer )
        {
          int regWidth = region.GetWidth();
          int regHeight = region.GetHeight();
          Matrix featureData;
          featureData.SetSize(17, 1);

          int xVals[4], yVals[4];
          for (j = 0; j < 4; j++)
          {
            xVals[j] = (j * regWidth) / 4 - 1;
            yVals[j] = (j * regHeight) / 4 - 1;
          }

          int x, y;
          double blockArea = regWidth * regHeight / 16.0;
          for (j = 0; j < 4; j++)
          {
            y = yVals[j];
            for (k = 0; k < 4; k++)
            {
              x = xVals[k];
              int pixelCount = integralBuffer[y * regWidth + x];
              if ( j > 0 )
                pixelCount -= integralBuffer[yVals[j - 1] * width + x];
              if ( k > 0 )
                pixelCount -= integralBuffer[y * width + xVals[k - 1]];
              if ( (j > 0) && (k > 0) )
                pixelCount += integralBuffer[yVals[j - 1] * width + xVals[k - 1]];
              featureData.SetValue(j * 4 + k, 0, pixelCount / blockArea);
            }
          }

          double aspectRatio = width / (double)height;
          featureData.SetValue(16, 0, aspectRatio);
          handClassifier.AddTrainingData(featureData, classIndex);
        }
        else
          fprintf(stderr, "Failed getting integral buffer while processing %s\n", argv[i]);
      }
      else
        fprintf(stderr, "Failed getting color region while processing %s\n", argv[i]);
    }
  }

  printf("Starting training\n");
  if ( !handClassifier.Train() )
  {
    printf("Failed training the hand classifier\n");
    return 1;
  }

  do
  {
    sprintf(filename, "adahand-%s-%d.rev%d.cfg", className.c_str(), numWeakClassifiers, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  handClassifier.Save(filename);

  return 0;
}
