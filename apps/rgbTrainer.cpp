#include "BayesianClassifier.h"
#include <qapplication.h>
#include <qimage.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QColor color;
  int j;
  int u, v;
  int width, height;
  int r, g, b;
  double maxVal;
  double rFlat, gFlat, bFlat;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex = 0;
  int classComponents[] = {2, 5};

  if ( argc < 4 )
    return 0;

  input.SetSize(3, 1);
  classifier.Create(3, 2, classComponents);

  for (j=1; j<argc; j++)
  {
    if ( !strcmp(argv[j], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.load(argv[j]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand image %s\n", argv[j]);
      else
        printf("Processing other image %s\n", argv[j]);
      width = inputImage.width();
      height = inputImage.height();
      for (v = 0; v < height; v++)
      {
        for (u = 0; u < width; u++)
        {
          color = inputImage.pixel(u,v);
          color.getRgb(&r, &g, &b);

          if ( (r >= g) && (r >= b) )
            maxVal = r;
          else if ( (g >= r) && (g >= b) )
            maxVal = g;
          else
            maxVal = b;

          if ( maxVal == 0 )
          {
#if 0
            rFlat = 0;
            gFlat = 0;
            bFlat = 0;
#else
            rFlat = 1;
            gFlat = 1;
            bFlat = 1;
#endif
          }
          else
          {
            rFlat = r / maxVal;
            gFlat = g / maxVal;
            bFlat = b / maxVal;
          }

          if ( (classIndex == 0) && (maxVal <= 55) )
            continue;

          input.SetValue(0, 0, rFlat);
          input.SetValue(1, 0, gFlat);
          input.SetValue(2, 0, bFlat);
          classifier.AddTrainingData(input, classIndex);
        }
      }
    }
  }

  printf("Starting training\n");
  classifier.Train();
  classifier.Save("hand.cfg");

  return 0;
}
