#include <stdio.h>
#include <qapplication.h>
#include <qimage.h>
#include "ImageRoutines.h"

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  QImage inputImage;
  QImage outputMeanImage;
  QImage outputVarianceImage;
  QString basename;
  int i;
  int x, y;
  int outputWidth, outputHeight;
  ColorComponent color;
  BlockStat *blocks;
  BlockStat *currentBlock;
  double minMean, maxMean;
  double minVar, maxVar;
  int val;

  outputWidth = 32;
  outputHeight = 32;
  blocks = (BlockStat *)malloc(outputWidth * outputHeight * sizeof(BlockStat));

  if ( !blocks )
  {
    fprintf(stderr,"Memory allocation error\n");
    return -1;
  }

  outputMeanImage.create(outputWidth,outputHeight,32);
  outputVarianceImage.create(outputWidth,outputHeight,32);

  for (i=1; i<argc; i++)
  {
    basename = argv[i];
    basename.truncate( basename.findRev('.') );
    if ( inputImage.load(argv[i]) )
    {
      for (color = COLOR_HUE; color <= COLOR_Q; color = (ColorComponent)(color + 1))
      {
        if ( GetBlockStatistics(&inputImage, outputWidth, outputHeight, blocks, color) )
        {
          minMean = 999999;
          minVar = 999999;
          maxMean = -999999;
          maxVar = -999999;
          for (y=0; y<outputHeight; y++)
          {
            for (x=0; x<outputWidth; x++)
            {
              currentBlock = &(blocks[y*outputWidth + x]);
              if ( currentBlock->mean < minMean )
                minMean = currentBlock->mean;
              if ( currentBlock->mean > maxMean )
                maxMean = currentBlock->mean;

              if ( currentBlock->variance < minVar )
                minVar = currentBlock->variance;
              if ( currentBlock->variance > maxVar )
                maxVar = currentBlock->variance;
            }
          }
          for (y=0; y<outputHeight; y++)
          {
            for (x=0; x<outputWidth; x++)
            {
              currentBlock = &(blocks[y*outputWidth + x]);
              val = (int)( (currentBlock->mean - minMean) / (maxMean - minMean) * 255 );
              outputMeanImage.setPixel(x, y, qRgb(val,val,val));

              val = (int)( (currentBlock->variance - minVar) / (maxVar - minVar) * 255 );
              outputVarianceImage.setPixel(x, y, qRgb(val,val,val));
            }
          }
          switch ( color )
          {
            case COLOR_HUE:
              outputMeanImage.save(basename + "_bm_h.png", "PNG");
              outputVarianceImage.save(basename + "_bv_h.png", "PNG");
              break;
            case COLOR_SATURATION:
              outputMeanImage.save(basename + "_bm_s.png", "PNG");
              outputVarianceImage.save(basename + "_bv_s.png", "PNG");
              break;
            case COLOR_VALUE:
              outputMeanImage.save(basename + "_bm_v.png", "PNG");
              outputVarianceImage.save(basename + "_bv_v.png", "PNG");
              break;
            case COLOR_Y:
              outputMeanImage.save(basename + "_bm_y.png", "PNG");
              outputVarianceImage.save(basename + "_bv_y.png", "PNG");
              break;
            case COLOR_I:
              outputMeanImage.save(basename + "_bm_i.png", "PNG");
              outputVarianceImage.save(basename + "_bv_i.png", "PNG");
              break;
            case COLOR_Q:
              outputMeanImage.save(basename + "_bm_q.png", "PNG");
              outputVarianceImage.save(basename + "_bv_q.png", "PNG");
              break;
            default:
              break;
          }
        }
      }
    }
  }

  return 0;
}
