#include "BayesianClassifier.h"
#include "Image.h"
#include <qapplication.h>
#include <qimage.h>

#define MAX_FEATURES 512

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
  FILE *file;

  Image image;
  double* featureBuffer;
  double* featurePixel;
  char buf[MAX_FEATURES];
  std::string featureList;
  int numFeatures;

  if ( argc < 3 )
    return 0;

  file = fopen(argv[1], "r");
  fgets(buf, MAX_FEATURES, file);
  numFeatures = strlen(buf);
  if ( buf[numFeatures - 1] == '\n' )
  {
    buf[numFeatures - 1] = 0;
    numFeatures--;
  }
  if ( !classifier.Load(file) )
  {
    fclose(file);
    fprintf(stderr, "Error loading %s\n", argv[1]);
    exit(1);
  }
  fclose(file);

  featureList = buf;
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
