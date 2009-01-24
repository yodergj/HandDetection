#include "FleshDetector.h"
#include "HandDetector.h"
#include "Image.h"
#include <qapplication.h>
#include <qimage.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QString basename;

  int i, j, imageIndex;
  int width, height;
  int numFleshRegions, numHands, xScale, yScale;
  int left, right, top, bottom;
  Image image, outlineImage;
  Image* fleshImage;
  Image* confidenceImage;
  FleshDetector fleshDetector;
  char outputFilename[1024];
  vector<ConnectedRegion*>* fleshRegionVector;
  HandDetector handDetector;
  vector<Hand*> hands;
  unsigned char boxColor[] = {255, 255, 255};
  int numLargeRegions;

  if ( argc < 4 )
  {
    printf("Usage: %s <flesh classifier file> <hand classifier file> <image file> [ <image file> ... ]", argv[0]);
    return 1;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  if ( !handDetector.Load(argv[2]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[2]);
    return 1;
  }

  for (imageIndex = 3; imageIndex < argc; imageIndex++)
  {
    if ( !inputImage.load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[imageIndex]);
      return 1;
    }

    basename = argv[imageIndex];
    basename.truncate( basename.findRev('.') );

    width = inputImage.width();
    height = inputImage.height();

    hands.clear();
    image.CopyARGBBuffer(width, height, (int*)inputImage.bits(), width);
    outlineImage = image;
    if ( fleshDetector.Process(&image, NULL, &fleshImage, &confidenceImage) )
    {
      fleshRegionVector = fleshDetector.GetFleshRegions(&image, xScale, yScale);
      if ( fleshRegionVector )
      {
        numFleshRegions = fleshRegionVector->size();
        numLargeRegions = 0;
        for (i = 0; i < numFleshRegions; i++)
        {
          if ( !(*fleshRegionVector)[i]->GetBounds(left, right, top, bottom) )
          {
            fprintf(stderr, "Error getting flesh block %d bounds\n", i);
            return 1;
          }
          left *= xScale;
          right = (right + 1) * xScale - 1;
          top *= yScale;
          bottom = (bottom + 1) * yScale - 1;
          if ( (right - left + 1 < 20) || (bottom - top + 1 < 20) )
            continue;
          numLargeRegions++;

          if ( !handDetector.Process(&image, left, right, top, bottom, hands) )
          {
            fprintf(stderr, "Error detecting hand in flesh block %d\n", i);
            return 1;
          }
        }
        numHands = hands.size();
        printf("Num Flesh Regions %d of %d\nNum Hands %d\n", numLargeRegions, numFleshRegions, numHands);
        for (j = 0; j < numHands; j++)
        {
          hands[j]->GetBounds(left, right, top, bottom);
          outlineImage.DrawBox(boxColor, 3, left, top, right, bottom);
        }
      }

      sprintf(outputFilename, "%s_flesh.ppm", basename.latin1());
      fleshImage->Save(outputFilename);
      sprintf(outputFilename, "%s_confidence.ppm", basename.latin1());
      confidenceImage->Save(outputFilename);

      sprintf(outputFilename, "%s_frame.ppm", basename.latin1());
      outlineImage.Save(outputFilename);
    }
  }

  return 0;
}
