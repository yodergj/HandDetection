#include "VideoDecoder.h"
#include "BayesianClassifier.h"
#include <qapplication.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QColor color;
  VideoDecoder decoder;
  BayesianClassifier classifier;
  FILE* file;
  Matrix input;
  QImage* inputImage;
  QImage fleshImage;
  int frameNumber = 0;
  QString vidFilename;
  char outputFilename[1024];
  int width, height;
  int u, v;
  int r, g, b;
  double y, i, q;
  int classIndex;
  double confidence;

  if ( argc < 4 )
  {
    printf("Usage: procframes <classifier file> <video file> <output directory>\n");
    return 1;
  }

  file = fopen(argv[1], "r");
  if ( !classifier.Load(file) )
  {
    fprintf(stderr, "Error loading classifier %s\n", argv[1]);
    return 1;
  }
  fclose(file);

  vidFilename = argv[2];
  decoder.SetFilename(vidFilename);
  if ( !decoder.Load() )
  {
    fprintf(stderr, "Error loading video %s\n", argv[2]);
    return 1;
  }

  input.SetSize(2, 1);
  while ( decoder.UpdateFrame() )
  {
    inputImage = decoder.GetFrame();

    width = inputImage->width();
    height = inputImage->height();
    fleshImage.create(width, height, 32);
    for (v = 0; v < height; v++)
    {
      for (u = 0; u < width; u++)
      {
        color = inputImage->pixel(u,v);
        color.getRgb(&r, &g, &b);
        y =   r * .299 + g *  .587 + b *  .114;
        i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
        q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

        input.SetValue(0, 0, i / 255);
        input.SetValue(1, 0, q / 255);
        classifier.Classify(input, classIndex, confidence);

        if ( classIndex == 0 )
        {
          // Pixel is flesh colored
          fleshImage.setPixel(u, v, qRgb(r, g, b));
        }
        else
        {
          // Pixel is not flesh colored
          fleshImage.setPixel(u, v, qRgb(255, 255, 255));
        }
      }
    }

    sprintf(outputFilename, "%s/frame%05d.png", argv[3], frameNumber);
    fleshImage.save(outputFilename, "PNG");

    frameNumber++;
  }

  return 0;
}
