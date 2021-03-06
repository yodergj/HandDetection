#include <string>
#include "VideoDecoder.h"

using std::string;

int main(int argc, char* argv[])
{
  VideoDecoder decoder;
  Image* inputImage;
  int frameNumber = 0;
  string vidFilename;
  char outputFilename[1024];

  if ( argc < 3 )
  {
    printf("Usage: %s <video file> <output directory>\n", argv[0]);
    return 1;
  }

  vidFilename = argv[1];
  decoder.SetFilename(vidFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", argv[1]);
    return 1;
  }

  while ( decoder.UpdateFrame() )
  {
    inputImage = decoder.GetFrame();
    sprintf(outputFilename, "%s/frame%05d.png", argv[2], frameNumber);
    inputImage->Save(outputFilename);

    frameNumber++;
  }

  return 0;
}
