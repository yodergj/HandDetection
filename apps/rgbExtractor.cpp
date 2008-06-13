#include "BayesianClassifier.h"
#include <qapplication.h>
#include <qimage.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QImage fleshImage, nonFleshImage;
  QString basename;
  QColor color;
  int u, v;
  int width, height;
  int r, g, b;
  double maxVal;
  double rFlat, gFlat, bFlat;
  int imageIndex;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex;
  double confidence;
  FILE *file;

  if ( argc < 3 )
    return 0;

  file = fopen(argv[1], "r");
  if ( !classifier.Load(file) )
  {
    fclose(file);
    fprintf(stderr, "Error loading %s\n", argv[1]);
    exit(1);
  }
  fclose(file);

  input.SetSize(3, 1);

  for (imageIndex = 2; imageIndex < argc; imageIndex++)
  {
    if ( !inputImage.load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(1);
    }

    basename = argv[imageIndex];
    basename.truncate( basename.findRev('.') );

    width = inputImage.width();
    height = inputImage.height();
    fleshImage.create(width, height, 32);
    nonFleshImage.create(width, height, 32);
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

        if ( maxVal > 0 )
        {
          rFlat = r / maxVal;
          gFlat = g / maxVal;
          bFlat = b / maxVal;
        }
        else
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

        input.SetValue(0, 0, rFlat);
        input.SetValue(1, 0, gFlat);
        input.SetValue(2, 0, bFlat);
#if 1
        classifier.Classify(input, classIndex, confidence);
#else
        if ( maxVal <= 55 )
          classIndex = 1;
        else
          classIndex = 0;
#endif

        if ( classIndex == 0 )
        {
          // Pixel is flesh colored
          fleshImage.setPixel(u, v, qRgb(r, g, b));
          nonFleshImage.setPixel(u, v, qRgb(255, 255, 255));
        }
        else
        {
          // Pixel is not flesh colored
          fleshImage.setPixel(u, v, qRgb(255, 255, 255));
          nonFleshImage.setPixel(u, v, qRgb(r, g, b));
        }
      }
    }

    fleshImage.save(basename + "_flesh.png", "PNG");
    nonFleshImage.save(basename + "_nonflesh.png", "PNG");
  }

  return 0;
}
