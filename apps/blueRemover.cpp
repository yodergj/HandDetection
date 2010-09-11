#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "Image.h"

using std::string;

int main(int argc, char* argv[])
{
  Image image, outputImage;
  string basename;
  unsigned char* srcPixel;
  unsigned char* outPixel;
  int x, y;
  int width, height;
  int imageIndex, dotPos;
  unsigned char black[] = {0, 0, 0};

  if ( argc < 2 )
  {
    printf("Usage: %s <image> [<image> ...]\n", argv[0]);
    return 0;
  }

  for (imageIndex = 1; imageIndex < argc; imageIndex++)
  {
    if ( !image.Load(argv[imageIndex]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(1);
    }

    basename = argv[imageIndex];
    dotPos = basename.rfind('.');
    if ( dotPos != (int)string::npos )
      basename = basename.substr(0, dotPos);

    width = image.GetWidth();
    height = image.GetHeight();

    outputImage.Create(width, height);

    srcPixel = image.GetRGBBuffer();
    outPixel = outputImage.GetRGBBuffer();
    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++, srcPixel += 3, outPixel += 3)
      {

        if ( (srcPixel[2] > srcPixel[0]) && (srcPixel[2] > srcPixel[1]) )
          memcpy(outPixel, black, 3);
        else
          memcpy(outPixel, srcPixel, 3);
      }
    }

    outputImage.Save(basename + "_out.png");
  }

  return 0;
}
