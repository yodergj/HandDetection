#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avifile.h>
#include <avm_fourcc.h>
#include "Image.h"

#define FRAME_RATE 10

void InsertFrame(Image* image, avm::IVideoWriteStream* outVidStr)
{
  int x, y;
  int width, height;
  unsigned char* data;
  unsigned char* srcPixel;
  unsigned char* destPixel;

  width = image->GetWidth();
  height = image->GetHeight();
  printf("width %d height %d\n", width, height);
  data = (unsigned char*)malloc(3 * width * height);

  srcPixel = image->GetRGBBuffer();
  destPixel = data;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++, srcPixel += 3, destPixel += 3)
    {
      destPixel[0] = srcPixel[2];
      destPixel[1] = srcPixel[1];
      destPixel[2] = srcPixel[0];
    }
  }

  avm::CImage frame(data, width, height);
  outVidStr->AddFrame(&frame);

  free(data);
}

int main(int argc, char* argv[])
{
  Image inputImage;
  int i;
  int width = 0;
  int height = 0;
  avm::IWriteFile* outFile = avm::CreateWriteFile("out.avi");
  BITMAPINFOHEADER bi;
  //fourcc_t codec = fccMP42;
  fourcc_t codec = fccDIV3;
  //fourcc_t codec = fccIV32;
  //fourcc_t codec = fccCVID;
  avm::IVideoWriteStream* vidStr;

  if ( argc < 2 )
    return 0;

  for (i = 1; i < argc; i++)
  {
    if ( !inputImage.Load(argv[i]) )
    {
      fprintf(stderr, "Error loading %s\n", argv[i]);
      return 1;
    }

    if ( i == 1 )
    {
      width = inputImage.GetWidth();
      height = inputImage.GetHeight();

      memset(&bi, 0, sizeof(BITMAPINFOHEADER));
      bi.biSize = sizeof(BITMAPINFOHEADER);
      bi.biWidth = width;
      bi.biHeight = height;
      bi.biSizeImage = width * height * 3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;

      vidStr = outFile->AddVideoStream(codec, &bi, 1000000 / FRAME_RATE);
      vidStr->Start();
    }

    InsertFrame(&inputImage, vidStr);
  }
  vidStr->Stop();

  return 0;
}
