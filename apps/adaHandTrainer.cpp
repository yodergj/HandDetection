#include <string.h>
#include <string>
#include "FleshDetector.h"
#include "DummyFleshDetector.h"
#include "AdaboostClassifier.h"
#include "HandCandidate.h"
#include "SubImage.h"

using std::string;

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

int main(int argc, char* argv[])
{
  int i, j;
  int width, height;
  int left, right, top, bottom;
  int classIndex = 0;
  char filename[256];
  FILE *file;
  int revNumber = 0;
  Image image;
  unsigned char* srcPixel;
  string featureList;
#if 0
  FleshDetector fleshDetector;
#else
  DummyFleshDetector fleshDetector;
#endif
  AdaboostClassifier handClassifier;
  vector<ConnectedRegion*>* fleshRegionVector;
  int xScale, yScale, numFleshRegions;
  Matrix input;
  HandCandidate* candidate;
  int numFeatures, numWeakClassifiers;
  SubImage handImage;

  if ( argc < 7 )
  {
    printf("Usage: %s <num weak classifiers> <feature string> <flesh classifier file> <hand Image> [...] -x <non-hand image> [...]\n", argv[0]);
    return 0;
  }

  numWeakClassifiers = atoi(argv[1]);
  if ( numWeakClassifiers < 1 )
  {
    printf("Invalid number of weak classifiers %d\n", numWeakClassifiers );
    return 1;
  }

  featureList = argv[2];
  numFeatures = featureList.size();
  input.SetSize(numFeatures, 1);
  if ( !handClassifier.Create(numFeatures, 2, numWeakClassifiers) )
  {
    printf("Failed creating classifier\n");
    return 1;
  }
  handClassifier.SetFeatureString(featureList);

  if ( !fleshDetector.Load(argv[3]) )
  {
    printf("Failed loading flesh detector %s\n", argv[2]);
    return 1;
  }

  for (i = 4; i < argc; i++)
  {
    if ( !strcmp(argv[i], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( image.Load(argv[i]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand image %s\n", argv[i]);
      else
        printf("Processing other image %s\n", argv[i]);
      width = image.GetWidth();
      height = image.GetHeight();
      srcPixel = image.GetRGBBuffer();

      fleshRegionVector = fleshDetector.GetFleshRegions(&image, xScale, yScale);
      if ( fleshRegionVector )
      {
        numFleshRegions = fleshRegionVector->size();
        for (j = 0; j < numFleshRegions; j++)
        {
          if ( !(*fleshRegionVector)[j]->GetBounds(left, right, top, bottom) )
          {
            fprintf(stderr, "Error getting flesh block %d bounds\n", j);
            return 1;
          }
          left *= xScale;
          right = (right + 1) * xScale - 1;
          top *= yScale;
          bottom = (bottom + 1) * yScale - 1;
          if ( (right - left + 1 < 40) || (bottom - top + 1 < 40) )
            continue;

          handImage.CreateFromParent(&image, left, right, top, bottom);
          vector<ConnectedRegion*>* fullResRegions;
          fullResRegions = fleshDetector.GetFleshRegions(&handImage);
          int numFullResRegions = 0;
          if ( fullResRegions )
            numFullResRegions = fullResRegions->size();
          if ( !numFullResRegions )
            fprintf(stderr, "Failed getting full resolution hand candidate %d on %s\n", j, argv[i]);
          else
          {
            int regionIndex = 0;
            if ( numFullResRegions > 1 )
            {
              for (int k = 1; k < numFullResRegions; k++)
                if ( (*fullResRegions)[k]->HasMorePixels( *((*fullResRegions)[regionIndex]) ) )
                  regionIndex = k;
              fprintf(stderr, "Flesh block %d on %s yielded %d regions - only processing the largest (%d)\n", j, argv[i], numFullResRegions, regionIndex);
            }

            candidate = new HandCandidate( (*fullResRegions)[regionIndex] );
            if ( !candidate->GetFeatureVector(featureList, input) )
            {
              fprintf(stderr, "Error getting hand candidate features for flesh block %d\n", j);
              return 1;
            }
            delete candidate;
          }

          handClassifier.AddTrainingData(input, classIndex);
        }
      }
      else
        fprintf(stderr, "NULL flesh region vector processing %s\n", argv[i]);
    }
  }

  printf("Starting training\n");
  if ( !handClassifier.Train() )
  {
    printf("Failed training the hand detector\n");
    return 1;
  }

  do
  {
    sprintf(filename, "adahand-%d.rev%d.cfg", numWeakClassifiers, revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  handClassifier.Save(filename);

  return 0;
}