#include "VideoDecoder.h"
#include "BayesianClassifier.h"
#include <qapplication.h>

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  QRgb* scanline;
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
#if 0
  double y, i, q;
#else
  double maxVal;
  double rFlat, gFlat, bFlat;
#endif
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

#if 0
  input.SetSize(2, 1);
#else
  input.SetSize(3, 1);
#endif
  while ( decoder.UpdateFrame() )
  {
    inputImage = decoder.GetFrame();

    width = inputImage->width();
    height = inputImage->height();
    fleshImage.create(width, height, 32);
    for (v = 0; v < height; v++)
    {
      scanline = (QRgb*)inputImage->scanLine(v);
      for (u = 0; u < width; u++)
      {
        r = qRed(scanline[u]);
        g = qGreen(scanline[u]);
        b = qBlue(scanline[u]);

#if 0
        y =   r * .299 + g *  .587 + b *  .114;
        i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
        q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

        input.SetValue(0, 0, i / 255);
        input.SetValue(1, 0, q / 255);
#else
        if ( (r >= g) && (r >= b) )
          maxVal = r;
        else if ( (g >= r) && (g >= b) )
          maxVal = g;
        else
          maxVal = b;

        if ( (classIndex == 0) && (maxVal <= 55) )
          continue;

        if ( maxVal == 0 )
        {
          rFlat = 1;
          gFlat = 1;
          bFlat = 1;
        }
        else
        {
          rFlat = r / maxVal;
          gFlat = g / maxVal;
          bFlat = b / maxVal;
        }

        input.SetValue(0, 0, rFlat);
        input.SetValue(1, 0, gFlat);
        input.SetValue(2, 0, bFlat);
#endif
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
