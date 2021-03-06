#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include "CompositeClassifier.h"
#include "FleshDetector.h"
#include "DummyFleshDetector.h"
#include "HandCandidate.h"
#include "Hand.h"
#include "SubImage.h"
#include "DirectoryUtils.h"

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
  time_t currentTime;
  char outputFilename[64];
  FILE* outputFile;
  FILE* listFile;
  vector<string> filenames;
  int numFiles;
  string outputDir, fullOutputFilename, subDir;

  if ( argc < 5 )
  {
    printf("Usage: %s <output dir> <flesh classifier file> <hand classifier file> <image file> [ <image file> ... ]\n", argv[0]);
    printf("       %s <output dir> <flesh classifier file> <hand classifier file> -l <list file>\n", argv[0]);
    return 1;
  }

  outputDir = argv[1];
  if ( !MakeDirectory(outputDir) )
  {
    fprintf(stderr, "Error creating output directory %s\n", outputDir.c_str());
    return 1;
  }

  // Either loads a real detector or gets a dummy detector if arg is "DUMMY"
  fleshDetector = FleshDetector::Get(argv[2]);
  if ( !fleshDetector )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[2]);
    return 1;
  }

  if ( !postureDetector.Load(argv[3]) )
  {
    fprintf(stderr, "Error loading hand detector %s\n", argv[3]);
    return 1;
  }
  features = postureDetector.GetFeatureString();

  if ( !strcmp(argv[4], "-l") )
  {
    if ( argc < 6 )
    {
      fprintf(stderr, "List option given, but no list file specified.\n");
      return 1;
    }
    listFile = fopen(argv[5], "r");
    if ( !listFile )
    {
      fprintf(stderr, "Failed opening list file %s\n", argv[5]);
      return 1;
    }
    char filename[4096];
    while ( fscanf(listFile, "%s.4095", filename) != EOF )
      filenames.push_back(filename);
    fclose(listFile);
  }
  else
  {
    for (i = 4; i < argc; i++)
      filenames.push_back(argv[i]);
  }
  numFiles = filenames.size();

  time(&currentTime);
  strftime(outputFilename, 64, "postureResults-%Y%m%d-%H:%M:%S", localtime(&currentTime));
  fullOutputFilename = outputDir + "/" + outputFilename;
  outputFile = fopen(fullOutputFilename.c_str(), "w");
  if ( !outputFile )
  {
    fprintf(stderr, "Error opening output file %s - %s\n", fullOutputFilename.c_str(), strerror(errno));
    return 1;
  }
  fprintf(outputFile, "Flesh Detection: %s\tPosture Detection: %s\n", argv[1], argv[2]);

  for (imageIndex = 0; imageIndex < numFiles; imageIndex++)
  {
    if ( !image.Load(filenames[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", filenames[imageIndex].c_str());
      fclose(outputFile);
      return 1;
    }
    printf("Processing %s\n", filenames[imageIndex].c_str());

    basename = filenames[imageIndex];
    dotPos = basename.rfind('.');
    if ( dotPos != (int)string::npos )
      basename = basename.substr(0, dotPos);

    subDir = GetDirectoryPart(basename);
    if ( !MakeDirectory(outputDir + "/" + subDir) )
    {
      fprintf(stderr, "Error creating output directory %s\n", (outputDir + "/" + subDir).c_str());
      return 1;
    }

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
            fclose(outputFile);
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
            fprintf(stderr, "Failed getting full resolution hand candidate %d on %s\n", i, filenames[imageIndex].c_str());
            fclose(outputFile);
            return 1;
          }
          int regionIndex = 0;
          if ( numFullResRegions > 1 )
          {
            for (k = 1; k < numFullResRegions; k++)
              if ( (*fullResRegions)[k]->HasMorePixels( *((*fullResRegions)[regionIndex]) ) )
                regionIndex = k;
            fprintf(stderr, "Flesh block %d on %s yielded %d regions - only processing the largest (%d)\n", i, filenames[imageIndex].c_str(), numFullResRegions, regionIndex);
          }

          candidate = new HandCandidate( (*fullResRegions)[regionIndex] );
          if ( !candidate->GetScaledFeatures(1, 1, centroid, center, nearEdge, farEdge,
                                             shortLine, longLine, offsetLine, edgeAngle, offsetAngle) )
          {
            fprintf(stderr, "Error getting hand candidate features for flesh block %d\n", i);
            fclose(outputFile);
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
            fclose(outputFile);
            return 1;
          }

          classIndex = postureDetector.Classify(input);
          if ( classIndex != -1 )
          {
            hand = new Hand;
            hand->SetBounds(left, right, top, bottom);
            hand->SetPostureString(postureDetector.GetClassName(classIndex));
            hands.push_back(hand);
            printf("Detected Class %d %s\n", classIndex, hand->GetPostureString().c_str());
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
        fprintf(outputFile, "%s", filenames[imageIndex].c_str());
        for (j = 0; j < numHands; j++)
        {
          fprintf(outputFile, " %s", hand->GetPostureString().c_str());
          hands[j]->GetBounds(left, right, top, bottom);
          outlineImage.DrawBox(hands[j]->GetPostureColor(0),
                               hands[j]->GetPostureColor(1),
                               hands[j]->GetPostureColor(2),
                               hands[j]->GetPostureColor(3),
                               3, left, top, right, bottom);
          delete hands[j];
        }
        if ( numHands == 0 )
          fprintf(outputFile, " None");
        fprintf(outputFile, "\n");
        hands.clear();
      }

      fleshImage->Save(outputDir + "/" + basename + "_flesh.png");
      confidenceImage->Save(outputDir + "/" + basename + "_confidence.png");
      outlineImage.Save(outputDir + "/" + basename + "_frame.png");
    }
  }
  fclose(outputFile);

  return 0;
}