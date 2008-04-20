#include "BayesianClassifier.h"
#include "Image.h"
#include <qapplication.h>
#include <qimage.h>

#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QRgb* srcPixel;
  int i, j;
  int x, y;
  int width, height;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int classComponents[] = {2, 5};
  int revNumber = 0;
  Image image;
  double* featureBuffer;
  double* pixel;
  std::string featureList;
  int numFeatures;

  if ( argc < 5 )
    return 0;

  featureList = argv[1];
  numFeatures = featureList.size();
  input.SetSize(numFeatures, 1);
  classifier.Create(numFeatures, 2, classComponents);

  for (i = 1; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing flesh image %s\n", argv[i]);
      else
        printf("Processing other image %s\n", argv[i]);
      width = inputImage.width();
      height = inputImage.height();
      srcPixel = (QRgb*)inputImage.bits();
      image.CopyARGBBuffer(width, height, (int*)srcPixel, width);
      featureBuffer = image.GetCustomBuffer(featureList);
      pixel = featureBuffer;
      for (y = 0; y < height; y++)
      {
        for (x = 0; x < width; x++, srcPixel++, pixel += numFeatures)
        {
          if ( (classIndex == 0) &&
               (MAX(qRed(*srcPixel), MAX(qGreen(*srcPixel), qBlue(*srcPixel))) <= 55) )
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
    sprintf(filename, "flesh-%d-%d.rev%d.cfg", classComponents[0], classComponents[1], revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  file = fopen(filename, "w");
  fprintf(file, "%s\n", featureList.c_str());
  classifier.Save(file);
  fclose(file);

  return 0;
}
