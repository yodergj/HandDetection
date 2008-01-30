#include "BayesianClassifier.h"
#include <qapplication.h>
#include <qimage.h>

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
  double y, i, q;
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

  input.SetSize(2, 1);

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
        y =   r * .299 + g *  .587 + b *  .114;
        i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
        q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

        input.SetValue(0, 0, i / 255);
        input.SetValue(1, 0, q / 255);
        classifier.Classify(input, classIndex, confidence);

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
