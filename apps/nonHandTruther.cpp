#include <string.h>
#include <string>
#include "FleshDetector.h"
#include "Image.h"

using std::string;

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

int main(int argc, char* argv[])
{
  int i, j;
  int width, height;
  int left, right, top, bottom;
  int blockWidth, blockHeight, numTooSmall;
  char filename[256];
  FILE *file;
  int revNumber = 0;
  Image image;
  unsigned char* srcPixel;
  FleshDetector fleshDetector;
  vector<ConnectedRegion*>* fleshRegionVector;
  int xScale, yScale, numFleshRegions;

  if ( argc < 3 )
  {
    printf("Usage: %s <flesh classifier file> <non-hand image> [...]\n", argv[0]);
    return 0;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Failed loading flesh detector %s\n", argv[1]);
    return 1;
  }

  do
  {
    sprintf(filename, "nonHandTruth.rev%d.idx", revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );
  file = fopen(filename, "w");

  if ( !file )
  {
    fprintf(stderr, "Failed writing output file %s\n", filename);
    return 1;
  }

  for (i = 2; i < argc; i++)
  {
    if ( !image.Load(argv[i]) )
      continue;

    printf("Processing other image %s\n", argv[i]);
    width = image.GetWidth();
    height = image.GetHeight();
    srcPixel = image.GetRGBBuffer();
    /* Load samples from flesh false-positives */
    fleshRegionVector = fleshDetector.GetFleshRegions(&image, xScale, yScale);
    if ( fleshRegionVector )
    {
      numFleshRegions = fleshRegionVector->size();
      numTooSmall = 0;
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
        blockWidth = right - left + 1;
        blockHeight = bottom - top + 1;

        if ( (blockWidth < 30) || (blockHeight < 40) )
          numTooSmall++;
      }
      fprintf(file, "%s %d", argv[i], numFleshRegions - numTooSmall);
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
        blockWidth = right - left + 1;
        blockHeight = bottom - top + 1;

        if ( (blockWidth < 30) || (blockHeight < 40) )
          continue;

        fprintf(file, " %d %d %d %d", left, top, blockWidth, blockHeight);
      }
      fprintf(file, "\n");
    }
  }

  fclose(file);

  return 0;
}