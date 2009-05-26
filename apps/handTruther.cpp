#include <stdio.h>
#include <string.h>
#include <string>
#include "Image.h"

using std::string;

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

int main(int argc, char* argv[])
{
  int i;
  int x, y;
  int width, height;
  int left, right, top, bottom;
  char filename[256];
  FILE *file;
  int revNumber = 0;
  Image image;
  unsigned char* srcPixel;

  if ( argc < 2 )
  {
    printf("Usage: %s <hand Image> [...]\n", argv[0]);
    return 0;
  }

  do
  {
    sprintf(filename, "handTruth.rev%d.idx", revNumber);
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

  for (i = 1; i < argc; i++)
  {
    if ( !image.Load(argv[i]) )
      continue;

    printf("Processing hand image %s\n", argv[i]);
    width = image.GetWidth();
    height = image.GetHeight();
    srcPixel = image.GetRGBBuffer();
    left = width - 1;
    right = 0;
    top = height - 1;
    bottom = 0;

    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++, srcPixel++)
      {
        if ( (MAX(srcPixel[0], MAX(srcPixel[1], srcPixel[2])) > 55) )
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
    fprintf(file, "%s 1 %d %d %d %d\n", argv[i], left, top, right - left + 1, bottom - top + 1);
  }

  fclose(file);

  return 0;
}