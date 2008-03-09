#include <stdio.h>
#include <qapplication.h>
#include <qimage.h>
#include "ImageRoutines.h"

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  QImage inputImage1;
  QImage inputImage2;
  QImage outputImage;
  QString basename;
  int i;
  int width, height;

  for (i=2; i<argc; i++)
  {
    basename = argv[i];
    basename.truncate( basename.findRev('.') );
    if ( inputImage1.load(argv[i-1]) && inputImage2.load(argv[i]) )
    {
      width = inputImage1.width();
      height = inputImage1.height();
      if ( (width != inputImage2.width()) || (height != inputImage2.height()) )
        continue;
      outputImage.create(width,height,32);
      if ( SubtractImage(&inputImage2, &inputImage1, &outputImage) )
        outputImage.save(basename + "_diff.png", "PNG");
    }
  }

  return 0;
}
