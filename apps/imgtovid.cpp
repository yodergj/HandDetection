#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Image.h"
#include "VideoEncoder.h"

int main(int argc, char* argv[])
{
  int i;
  int width = 0;
  int height = 0;
  VideoEncoder encoder;
  string videoFilename;
  Image inputImage;
  bool needInit = true;

  if ( argc < 3 )
  {
    printf("Usage: %s <output filename> <image> [<image> ...]\n", argv[0]);
    return 0;
  }

  videoFilename = argv[1];
  for (i = 2; i < argc; i++)
  {
    if ( !inputImage.Load(argv[i]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[i]);
      return 1;
    }

    if ( needInit )
    {
      needInit = false;
      width = inputImage.GetWidth();
      height = inputImage.GetHeight();

      if ( !encoder.Open(videoFilename.c_str(), width, height, 10) )
      {
        fprintf(stderr, "Failed opening %s\n", videoFilename.c_str());
        return 1;
      }
    }

    encoder.AddFrame(&inputImage);
  }
  encoder.Close();

  return 0;
}