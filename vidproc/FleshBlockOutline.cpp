#include "VideoDecoder.h"
#include "FleshDetector.h"
#include <qapplication.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  VideoDecoder decoder;
  FleshDetector fleshDetector;
  Image* inputImage;
  Image* fleshImage;
  Image* outlineImage;
  int frameNumber = 0;
  QString vidFilename;
  char outputFilename[1024];
  int width = 0;
  int height;
  QImage fleshQImage;
  QImage outputQImage;
  int x, y;
  unsigned char* fleshSrc;
  unsigned char* outlineSrc;

  if ( argc < 4 )
  {
    printf("Usage: fleshblockoutline <classifier file> <video file> <output directory>\n");
    return 1;
  }

  if ( !fleshDetector.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading flesh detector %s\n", argv[1]);
    return 1;
  }

  vidFilename = argv[2];
  decoder.SetFilename(vidFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", argv[2]);
    return 1;
  }

  while ( decoder.UpdateFrame() )
  {
    inputImage = decoder.GetMyFrame();

    if ( width == 0 )
    {
      width = inputImage->GetWidth();
      height = inputImage->GetHeight();
      fleshQImage.create(width, height, 32);
      outputQImage.create(width, height, 32);
    }

    if ( fleshDetector.Process(inputImage, &outlineImage, &fleshImage) )
    {
      fleshSrc = fleshImage->GetRGBBuffer();
      outlineSrc = outlineImage->GetRGBBuffer();
      for (y = 0; y < height; y++)
      {
        for (x = 0; x < width; x++, fleshSrc += 3, outlineSrc += 3)
        {
          fleshQImage.setPixel(x, y, qRgb(fleshSrc[0], fleshSrc[1], fleshSrc[2]));
          outputQImage.setPixel(x, y, qRgb(outlineSrc[0], outlineSrc[1], outlineSrc[2]));
        }
      }
      sprintf(outputFilename, "%s/flesh%05d.png", argv[3], frameNumber);
      fleshQImage.save(outputFilename, "PNG");
      sprintf(outputFilename, "%s/frame%05d.png", argv[3], frameNumber);
      outputQImage.save(outputFilename, "PNG");
    }

    frameNumber++;
  }

  return 0;
}
