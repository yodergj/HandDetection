#include "BayesianClassifier.h"
#include "Image.h"
#include <qapplication.h>
#include <qimage.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QImage fleshImage, nonFleshImage;
  QString basename;
  QRgb* srcPixel;
  int i;
  int x, y;
  int width, height;
  int imageIndex;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex;
  double confidence;

  Image image;
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
    if ( !inputImage.load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(1);
    }

    basename = argv[imageIndex];
    basename.truncate( basename.findRev('.') );

    width = inputImage.width();
    height = inputImage.height();

    srcPixel = (QRgb*)inputImage.bits();
    image.CopyARGBBuffer(width, height, (int*)srcPixel, width);
    featureBuffer = image.GetCustomBuffer(featureList);
    featurePixel = featureBuffer;

    fleshImage.create(width, height, 32);
    nonFleshImage.create(width, height, 32);
    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++, srcPixel++, featurePixel += numFeatures)
      {
        for (i = 0; i < numFeatures; i++)
          input.SetValue(i, 0, featurePixel[i]);
        classifier.Classify(input, classIndex, confidence);

        if ( classIndex == 0 )
        {
          // Pixel is flesh colored
          fleshImage.setPixel(x, y, *srcPixel);
          nonFleshImage.setPixel(x, y, qRgb(255, 255, 255));
        }
        else
        {
          // Pixel is not flesh colored
          fleshImage.setPixel(x, y, qRgb(255, 255, 255));
          nonFleshImage.setPixel(x, y, *srcPixel);
        }
      }
    }

    fleshImage.save(basename + "_flesh.png", "PNG");
    nonFleshImage.save(basename + "_nonflesh.png", "PNG");
  }

  return 0;
}
