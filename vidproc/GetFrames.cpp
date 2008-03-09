#include "VideoDecoder.h"
#include <qapplication.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QColor color;
  VideoDecoder decoder;
  QImage* inputImage;
  int frameNumber = 0;
  QString vidFilename;
  char outputFilename[1024];

  if ( argc < 3 )
  {
    printf("Usage: getframes <video file> <output directory>\n");
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
    inputImage->save(outputFilename, "PNG");

    frameNumber++;
  }

  return 0;
}
