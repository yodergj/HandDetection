#include "HandDetector.h"
#include "Image.h"
#include <qapplication.h>
#include <qimage.h>

#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QRgb* srcPixel;
  int i;
  int x, y;
  int width, height;
  int left, right, top, bottom, xCenter, yCenter;
  HandDetector detector;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int xResolution = 8;
  int yResolution = 8;
  int handGaussians = 2;
  int nonHandGaussians = 2;
  int revNumber = 0;
  Image image;
  std::string featureList;

  if ( argc < 5 )
  {
    printf("Usage: handTrainer <feature string> <hand Image> [...] -x <non-hand image> [...]\n");
    return 0;
  }

  featureList = argv[1];
  if ( !detector.Create(featureList, xResolution, yResolution, handGaussians, nonHandGaussians) )
  {
    printf("Failed creating detector\n");
    return 1;
  }

  for (i = 2; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand image %s\n", argv[i]);
      else
        printf("Processing other image %s\n", argv[i]);
      width = inputImage.width();
      height = inputImage.height();
      srcPixel = (QRgb*)inputImage.bits();
      image.CopyARGBBuffer(width, height, (int*)srcPixel, width);
      if ( classIndex == 0 )
      {
        left = width - 1;
        right = 0;
        top = height - 1;
        bottom = 0;

        for (y = 0; y < height; y++)
        {
          for (x = 0; x < width; x++, srcPixel++)
          {
            if ( (MAX(qRed(*srcPixel), MAX(qGreen(*srcPixel), qBlue(*srcPixel))) > 55) )
            {
              if ( x < left )
                left = x;
              if ( x > right )
                right = x;
              if ( y < top )
                top = y;
              if ( y > bottom )
                bottom = y;
            }
          }
        }
        if ( !detector.AddTrainingSample(&image, left, right, top, bottom, true) )
        {
          printf("Failed adding sample\n");
          return 1;
        }
      }
      else
      {
        left = 0;
        right = width - 1;
        top = 0;
        bottom = height - 1;
        xCenter = width / 2;
        yCenter = height / 2;
        /* Load the full image as a sample */
        detector.AddTrainingSample(&image, left, right, top, bottom, false);

        /* Load horizontal and vertical halves as samples */
        detector.AddTrainingSample(&image, left, xCenter - 1, top, bottom, false);
        detector.AddTrainingSample(&image, xCenter, right, top, bottom, false);
        detector.AddTrainingSample(&image, left, right, top, yCenter - 1, false);
        detector.AddTrainingSample(&image, left, right, yCenter, bottom, false);

        /* Load quarters as samples */
        detector.AddTrainingSample(&image, left, xCenter - 1, top, yCenter - 1, false);
        detector.AddTrainingSample(&image, left, xCenter - 1, yCenter, bottom, false);
        detector.AddTrainingSample(&image, xCenter, right, top, yCenter - 1, false);
        detector.AddTrainingSample(&image, xCenter, right, yCenter, bottom, false);
      }
    }
  }

  printf("Starting training\n");
  if ( !detector.Train() )
  {
    printf("Failed training the hand detector\n");
    return 1;
  }

  do 
  {
    sprintf(filename, "hand-%d-%d.rev%d.cfg", handGaussians, nonHandGaussians, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  detector.Save(filename);

  return 0;
}
