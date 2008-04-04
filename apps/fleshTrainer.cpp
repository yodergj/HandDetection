#include "BayesianClassifier.h"
#include <qapplication.h>
#include <qimage.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QRgb* scanline;
  int i;
  int x, y;
  int width, height;
  int r, g, b;
  double maxVal;
  double rFlat, gFlat, bFlat;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int classComponents[] = {2, 5};
  int revNumber = 0;

  if ( argc < 4 )
    return 0;

  input.SetSize(3, 1);
  classifier.Create(3, 2, classComponents);

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
      for (y = 0; y < height; y++)
      {
        scanline = (QRgb*)inputImage.scanLine(y);
        for (x = 0; x < width; x++)
        {
          r = qRed(scanline[x]);
          g = qGreen(scanline[x]);
          b = qBlue(scanline[x]);

          if ( (r >= g) && (r >= b) )
            maxVal = r;
          else if ( (g >= r) && (g >= b) )
            maxVal = g;
          else
            maxVal = b;

          if ( (classIndex == 0) && (maxVal <= 55) )
            continue;

          if ( maxVal == 0 )
          {
            rFlat = 1;
            gFlat = 1;
            bFlat = 1;
          }
          else
          {
            rFlat = r / maxVal;
            gFlat = g / maxVal;
            bFlat = b / maxVal;
          }

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

  do 
  {
    sprintf(filename, "hand-%d-%d.rev%d.cfg", classComponents[0], classComponents[1], revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  file = fopen(filename, "w");
  classifier.Save(file);
  fclose(file);

  return 0;
}
