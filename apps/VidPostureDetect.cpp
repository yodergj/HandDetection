#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "CompositeClassifier.h"
#include "FleshDetector.h"
#include "DummyFleshDetector.h"
#include "HandCandidate.h"
#include "Hand.h"
#include "SubImage.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"

using std::string;

int main(int argc, char* argv[])
{
  int i, j, k;
  int width, height;
  int numFleshRegions, numHands, xScale, yScale;
  int left, right, top, bottom;
  Image* image;
  Image outlineImage;
  FleshDetector* fleshDetector;
  vector<ConnectedRegion*>* fleshRegionVector;
  vector<Hand*> hands;
  Hand* hand;
  vector<HandCandidate*> handCandidates;
  HandCandidate* candidate;
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
  CompositeClassifier postureDetector;
  string features;
  Matrix input;
  int classIndex;
  SubImage handImage;
  vector<Point> farPoints;
  int numFarPoints;
  string inputFilename, outputFilename;
  VideoDecoder decoder;
  VideoEncoder encoder;
  bool needInit = true;

  if ( argc < 5 )
  {
    printf("Usage: %s <flesh classifier file> <hand classifier file> <input file> <output file>\n", argv[0]);
    return 1;
  }

  // Either loads a real detector or gets a dummy detector if arg is "DUMMY"
  fleshDetector = FleshDetector::Get(argv[1]);
  if ( !fleshDetector )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  if ( !postureDetector.Load(argv[2]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[2]);
    return 1;
  }
  features = postureDetector.GetFeatureString();

  inputFilename = argv[3];
  outputFilename = argv[4];

  decoder.SetFilename(inputFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", inputFilename.c_str());
    return 1;
  }

  while ( decoder.UpdateFrame() )
  {
    image = decoder.GetFrame();

    if ( needInit )
    {
      needInit = false;
      width = image->GetWidth();
      height = image->GetHeight();

      if ( !encoder.Open(outputFilename.c_str(), width, height, 10) )
      {
        fprintf(stderr, "Failed opening %s\n", outputFilename.c_str());
        return 1;
      }
    }

    hands.clear();
    outlineImage = *image;
    fleshRegionVector = fleshDetector->GetFleshRegions(image, xScale, yScale);
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

        handImage.CreateFromParent(image, left, right, top, bottom);
        vector<ConnectedRegion*>* fullResRegions;
        fullResRegions = fleshDetector->GetFleshRegions(&handImage);
        int numFullResRegions = 0;
        if ( fullResRegions )
          numFullResRegions = fullResRegions->size();
        if ( !numFullResRegions )
        {
          fprintf(stderr, "Failed getting full resolution hand candidate\n");
          return 1;
        }
        int regionIndex = 0;
        if ( numFullResRegions > 1 )
        {
          for (k = 1; k < numFullResRegions; k++)
            if ( (*fullResRegions)[k]->HasMorePixels( *((*fullResRegions)[regionIndex]) ) )
              regionIndex = k;
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

        classIndex = postureDetector.Classify(input);
        if ( classIndex != -1 )
        {
          hand = new Hand;
          hand->SetBounds(left, right, top, bottom);
          hand->SetPostureString(postureDetector.GetClassName(classIndex));
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
      }
      numHands = hands.size();
      for (j = 0; j < numHands; j++)
      {
        hands[j]->GetBounds(left, right, top, bottom);
        outlineImage.DrawBox(hands[j]->GetPostureColor(0),
                             hands[j]->GetPostureColor(1),
                             hands[j]->GetPostureColor(2),
                             hands[j]->GetPostureColor(3),
                             3, left, top, right, bottom);
        delete hands[j];
      }
      hands.clear();
    }

    if ( !encoder.AddFrame(&outlineImage) )
    {
      fprintf(stderr, "Error inserting video frame\n");
      return 1;
    }
  }
  encoder.Close();

  return 0;
}