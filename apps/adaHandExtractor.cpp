#include <stdlib.h>
#include <string.h>
#include <string>
#include "FleshDetector.h"
#include "HandCandidate.h"
#include "Hand.h"
#include "Image.h"

using std::string;

int main(int argc, char* argv[])
{
  int i, j, imageIndex;
  int width, height;
  int numFleshRegions, numHands, xScale, yScale, dotPos;
  int left, right, top, bottom;
  Image image, outlineImage;
  Image* fleshImage;
  Image* confidenceImage;
  FleshDetector fleshDetector;
  vector<ConnectedRegion*>* fleshRegionVector;
  vector<Hand*> hands;
  vector<HandCandidate*> handCandidates;
  HandCandidate* candidate;
  unsigned char boxColor[] = {255, 255, 255};
  unsigned char longColor[] = {0, 255, 0};
  unsigned char shortColor[] = {0, 0, 255};
  unsigned char pointColor[] = {255, 0, 0};
  int numLargeRegions;
  string basename;
  DoublePoint centroid, nearEdge, farEdge;
  LineSegment shortLine, longLine;
  double angle;

  if ( argc < 4 )
  {
    printf("Usage: %s <flesh classifier file> <hand classifier file> <image file> [ <image file> ... ]\n", argv[0]);
    return 1;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

#if 0
  if ( !handDetector.Load(argv[2]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[2]);
    return 1;
  }
#endif

  for (imageIndex = 3; imageIndex < argc; imageIndex++)
  {
    if ( !image.Load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[imageIndex]);
      return 1;
    }
    printf("Processing %s\n", argv[imageIndex]);

    basename = argv[imageIndex];
    dotPos = basename.rfind('.');
    if ( dotPos != (int)string::npos )
      basename = basename.substr(0, dotPos);

    width = image.GetWidth();
    height = image.GetHeight();

    hands.clear();
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

          candidate = new HandCandidate( (*fleshRegionVector)[i] );
          
          if ( !candidate->GetScaledFeatures(xScale, yScale, centroid, nearEdge, farEdge, shortLine, longLine, angle) )
          {
            fprintf(stderr, "Error getting hand candidate features for flesh block %d\n", i);
            return 1;
          }

          outlineImage.DrawLine(longColor, 1, longLine);
          outlineImage.DrawLine(shortColor, 1, shortLine);
          outlineImage.DrawLine(pointColor, 1, centroid, centroid);
          outlineImage.DrawLine(pointColor, 1, nearEdge, nearEdge);
          outlineImage.DrawLine(pointColor, 1, farEdge, farEdge);

          fleshImage->DrawLine(longColor, 1, longLine);
          fleshImage->DrawLine(shortColor, 1, shortLine);
          fleshImage->DrawLine(pointColor, 1, centroid, centroid);
          fleshImage->DrawLine(pointColor, 1, nearEdge, nearEdge);
          fleshImage->DrawLine(pointColor, 1, farEdge, farEdge);

          printf("Angle %f (degrees)\n", angle);

#if 0
          if ( !handDetector.Process(&image, left, right, top, bottom, hands) )
          {
            fprintf(stderr, "Error detecting hand in flesh block %d\n", i);
            return 1;
          }
#endif
        }
        numHands = hands.size();
        printf("Num Flesh Regions %d of %d\nNum Hands %d\n", numLargeRegions, numFleshRegions, numHands);
        for (j = 0; j < numHands; j++)
        {
          hands[j]->GetBounds(left, right, top, bottom);
          outlineImage.DrawBox(boxColor, 3, left, top, right, bottom);
        }
      }

      fleshImage->Save(basename + "_flesh.png");
      confidenceImage->Save(basename + "_confidence.png");
      outlineImage.Save(basename + "_frame.png");
    }
  }

  return 0;
}
