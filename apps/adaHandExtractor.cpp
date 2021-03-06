#include <stdlib.h>
#include <string.h>
#include <string>
#include "AdaboostClassifier.h"
#include "FleshDetector.h"
#include "DummyFleshDetector.h"
#include "HandCandidate.h"
#include "Hand.h"
#include "SubImage.h"

using std::string;

int main(int argc, char* argv[])
{
  int i, j, k, imageIndex;
  int width, height;
  int numFleshRegions, numHands, xScale, yScale, dotPos;
  int left, right, top, bottom;
  Image image, outlineImage;
  Image* fleshImage;
  Image* confidenceImage;
  FleshDetector* fleshDetector;
  vector<ConnectedRegion*>* fleshRegionVector;
  vector<Hand*> hands;
  Hand* hand;
  vector<HandCandidate*> handCandidates;
  HandCandidate* candidate;
  unsigned char boxColor[] = {255, 255, 255};
  unsigned char angledBoxColor[] = {255, 255, 0};
  unsigned char longColor[] = {0, 255, 0};
  unsigned char shortColor[] = {0, 0, 255};
  unsigned char offsetColor[] = {0, 255, 255};
  unsigned char pointColor[] = {255, 0, 0};
  unsigned char farPointColor[] = {255, 0, 255};
  int numLargeRegions;
  string basename;
  DoublePoint centroid, center, nearEdge, farEdge;
  LineSegment shortLine, longLine, offsetLine;
  Rect angledBox;
  double edgeAngle, offsetAngle;
  AdaboostClassifier handDetector;
  string features;
  Matrix input;
  int classIndex;
  SubImage handImage;
  vector<Point> farPoints;
  int numFarPoints;

  if ( argc < 4 )
  {
    printf("Usage: %s <flesh classifier file> <hand classifier file> <image file> [ <image file> ... ]\n", argv[0]);
    return 1;
  }

  // Either loads a real detector or gets a dummy detector if arg is "DUMMY"
  fleshDetector = FleshDetector::Get(argv[1]);
  if ( !fleshDetector )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  if ( !handDetector.Load(argv[2]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[2]);
    return 1;
  }
  features = handDetector.GetFeatureString();

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
    if ( fleshDetector->Process(&image, NULL, &fleshImage, &confidenceImage) )
    {
      fleshRegionVector = fleshDetector->GetFleshRegions(&image, xScale, yScale);
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
          if ( (right - left + 1 < FLESH_REGION_MIN_DIMENSION) || (bottom - top + 1 < FLESH_REGION_MIN_DIMENSION) )
            continue;
          numLargeRegions++;

          handImage.CreateFromParent(&image, left, right, top, bottom);
          vector<ConnectedRegion*>* fullResRegions;
          fullResRegions = fleshDetector->GetFleshRegions(&handImage);
          int numFullResRegions = 0;
          if ( fullResRegions )
            numFullResRegions = fullResRegions->size();
          if ( !numFullResRegions )
          {
            fprintf(stderr, "Failed getting full resolution hand candidate %d on %s\n", i, argv[imageIndex]);
            return 1;
          }
          int regionIndex = 0;
          if ( numFullResRegions > 1 )
          {
            for (k = 1; k < numFullResRegions; k++)
              if ( (*fullResRegions)[k]->HasMorePixels( *((*fullResRegions)[regionIndex]) ) )
                regionIndex = k;
            fprintf(stderr, "Flesh block %d on %s yielded %d regions - only processing the largest (%d)\n", i, argv[imageIndex], numFullResRegions, regionIndex);
          }

          candidate = new HandCandidate( (*fullResRegions)[regionIndex] );
          if ( !candidate->GetScaledFeatures(1, 1, centroid, center, nearEdge, farEdge,
                                             shortLine, longLine, offsetLine, edgeAngle, offsetAngle) )
          {
            fprintf(stderr, "Error getting hand candidate features for flesh block %d\n", i);
            return 1;
          }
          angledBox = candidate->GetAngledBoundingBox(longLine);
          farPoints.clear();
          if ( !candidate->GetFarPoints(farPoints) )
            fprintf(stderr, "Error getting far points for flesh block %d\n", i);
          numFarPoints = farPoints.size();

          centroid = handImage.GetTopLevelCoords(centroid);
          center = handImage.GetTopLevelCoords(center);
          nearEdge = handImage.GetTopLevelCoords(nearEdge);
          farEdge = handImage.GetTopLevelCoords(farEdge);
          shortLine.Translate(left, top);
          longLine.Translate(left, top);
          offsetLine.Translate(left, top);
          angledBox.Translate(left, top);
          for (k = 0; k < numFarPoints; k++)
            farPoints[k] = handImage.GetTopLevelCoords(farPoints[k]);

          if ( !candidate->GetFeatureVector(features, input) )
          {
            fprintf(stderr, "Error getting hand candidate features for flesh block %d\n", i);
            return 1;
          }

          classIndex = handDetector.Classify(input);
          if ( classIndex == 0 )
          {
            hand = new Hand;
            hand->SetBounds(left, right, top, bottom);
            hands.push_back(hand);
          }

          delete candidate;

          outlineImage.DrawLine(longColor, 1, longLine);
          outlineImage.DrawLine(shortColor, 1, shortLine);
          outlineImage.DrawLine(offsetColor, 1, offsetLine);
          outlineImage.DrawLine(pointColor, 1, centroid, centroid);
          outlineImage.DrawLine(pointColor, 1, center, center);
          outlineImage.DrawLine(pointColor, 1, nearEdge, nearEdge);
          outlineImage.DrawLine(pointColor, 1, farEdge, farEdge);
          outlineImage.DrawRect(angledBoxColor, 1, angledBox);
          for (k = 0; k < numFarPoints; k++)
            outlineImage.DrawLine(farPointColor, 1, centroid, farPoints[k]);

          fleshImage->DrawLine(longColor, 1, longLine);
          fleshImage->DrawLine(shortColor, 1, shortLine);
          fleshImage->DrawLine(offsetColor, 1, offsetLine);
          fleshImage->DrawLine(pointColor, 1, centroid, centroid);
          fleshImage->DrawLine(pointColor, 1, center, center);
          fleshImage->DrawLine(pointColor, 1, nearEdge, nearEdge);
          fleshImage->DrawLine(pointColor, 1, farEdge, farEdge);
          fleshImage->DrawRect(angledBoxColor, 1, angledBox);
          for (k = 0; k < numFarPoints; k++)
            fleshImage->DrawLine(farPointColor, 1, centroid, farPoints[k]);
        }
        numHands = hands.size();
        printf("Num Flesh Regions %d of %d\nNum Hands %d\n", numLargeRegions, numFleshRegions, numHands);
        for (j = 0; j < numHands; j++)
        {
          hands[j]->GetBounds(left, right, top, bottom);
          outlineImage.DrawBox(boxColor, 3, left, top, right, bottom);
          delete hands[j];
        }
        hands.clear();
      }

      fleshImage->Save(basename + "_flesh.png");
      confidenceImage->Save(basename + "_confidence.png");
      outlineImage.Save(basename + "_frame.png");
    }
  }

  return 0;
}
