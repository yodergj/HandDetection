#include "FleshDetector.h"
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
  int i, j;
  int x, y;
  int width, height;
  int left, right, top, bottom, xCenter, yCenter;
  HandDetector detector;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int xResolution = 2;
  int yResolution = 2;
  int handGaussians = 2;
  int nonHandGaussians = 2;
  int revNumber = 0;
  Image image;
  std::string featureList;
  FleshDetector fleshDetector;
  vector<ConnectedRegion*>* fleshRegionVector;
  int xScale, yScale, numFleshRegions;  

  if ( argc < 6 )
  {
    printf("Usage: %s <feature string> <flesh classifier file> <hand Image> [...] -x <non-hand image> [...]\n", argv[0]);
    return 0;
  }

  featureList = argv[1];
  if ( !detector.Create(featureList, xResolution, yResolution, handGaussians, nonHandGaussians) )
  {
    printf("Failed creating detector\n");
    return 1;
  }

  if ( !fleshDetector.Load(argv[2]) )
  {
    printf("Failed loading flesh detector %s\n", argv[2]);
    return 1;
  }

  for (i = 3; i < argc; i++)
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
#if 1
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
#else
        left = 0;
        right = width - 1;
        top = 0;
        bottom = height - 1;
#endif
        if ( !detector.AddTrainingSample(&image, left, right, top, bottom, true) )
        {
          printf("Failed adding sample\n");
          return 1;
        }
      }
      else
      {
#if 0
        left = 0;
        right = width - 1;
        top = 0;
        bottom = height - 1;
        xCenter = width / 2;
        yCenter = height / 2;
        /* Load the full image as a sample */
        detector.AddTrainingSample(&image, left, right, top, bottom, false);

#if 0
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
#endif
#else
        /* Load samples from flesh false-positives */
        fleshRegionVector = fleshDetector.GetFleshRegions(&image, xScale, yScale);
        if ( fleshRegionVector )
        {
          numFleshRegions = fleshRegionVector->size();
          for (j = 0; j < numFleshRegions; j++)
          {
            if ( !(*fleshRegionVector)[j]->GetBounds(left, right, top, bottom) )
            {
              fprintf(stderr, "Error getting flesh block %d bounds\n", i);
              return 1;
            }
            left *= xScale;
            right = (right + 1) * xScale - 1;
            top *= yScale;
            bottom = (bottom + 1) * yScale - 1;

            detector.AddTrainingSample(&image, left, right, top, bottom, false);
          }
        }
#endif
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
