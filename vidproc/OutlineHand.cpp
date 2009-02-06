#include "VideoDecoder.h"
#include "BayesianClassifier.h"
#include <qapplication.h>

#define PROXIMITY_THRESH  30

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

#define DEBUG_BLOCKS 0

int GetFleshBlocks(QColor ignoreColor, QImage *image, QPtrList<QRect> *blocklist)
{
  int i, j;
  int x, y;
  int width, height;
  int numClusters = 0;
  bool blockFound;
  QColor pixel;  
  QRect *rect;
  int numHits = 0;

  width = image->width();
  height = image->height();

  for (y=0; y<height; y++)
  {
    for (x=0; x<width; x++)
    {
      pixel = image->pixel(x,y);
      if ( pixel != ignoreColor )
      {
#if DEBUG_BLOCKS
        printf("Hit (%d,%d)\n",x,y);
#endif
        numHits++;
        blockFound = false;
        for (i=0; (i<numClusters) && !blockFound; i++)
        {
          if ( (x >= blocklist->at(i)->left() - PROXIMITY_THRESH) &&
               (x <= blocklist->at(i)->right() + PROXIMITY_THRESH) &&
               (y >= blocklist->at(i)->top() - PROXIMITY_THRESH) &&
               (y <= blocklist->at(i)->bottom() + PROXIMITY_THRESH) )
          {
            if ( x < blocklist->at(i)->left() )
              blocklist->at(i)->setLeft(x);
            if ( x > blocklist->at(i)->right() )
              blocklist->at(i)->setRight(x);
            if ( y < blocklist->at(i)->top() )
              blocklist->at(i)->setTop(y);
            if ( y > blocklist->at(i)->bottom() )
              blocklist->at(i)->setBottom(y);
            blockFound = true;
          }
        }
        if ( !blockFound )
        {
#if DEBUG_BLOCKS
          printf("New block\n");
#endif
          rect = new QRect(x,y,1,1);
          blocklist->append(rect);
          numClusters++;
        }
      }
    }
  }

  for (i=0; i < (int)blocklist->count(); i++)
  {
    if ( (blocklist->at(i)->width() < 5) ||
         (blocklist->at(i)->height() < 5) )
      continue;
    j = i + 1;
    while (j < (int)blocklist->count())
    {
      if ( (blocklist->at(j)->width() < 5) ||
           (blocklist->at(j)->height() < 5) )
      {
        j++;
        continue;
      }
#if DEBUG_BLOCKS
      printf("Block (%d %d %d %d) vs (%d %d %d %d)\n",
          blocklist->at(i)->left(),
          blocklist->at(i)->right(),
          blocklist->at(i)->top(),
          blocklist->at(i)->bottom(),
          blocklist->at(j)->left(),
          blocklist->at(j)->right(),
          blocklist->at(j)->top(),
          blocklist->at(j)->bottom());
#endif
      if ( ( ( (blocklist->at(i)->left() >= blocklist->at(j)->left() - PROXIMITY_THRESH) &&
               (blocklist->at(i)->left() <= blocklist->at(j)->right() + PROXIMITY_THRESH) ) ||
             ( (blocklist->at(i)->right() >= blocklist->at(j)->left() - PROXIMITY_THRESH) &&
               (blocklist->at(i)->right() <= blocklist->at(j)->right() + PROXIMITY_THRESH) ) ) &&
           ( ( (blocklist->at(i)->top() >= blocklist->at(j)->top() - PROXIMITY_THRESH) &&
               (blocklist->at(i)->top() <= blocklist->at(j)->bottom() + PROXIMITY_THRESH) ) ||
             ( (blocklist->at(i)->bottom() >= blocklist->at(j)->top() - PROXIMITY_THRESH) &&
               (blocklist->at(i)->bottom() <= blocklist->at(j)->bottom() + PROXIMITY_THRESH) ) ) )
      {
        blocklist->at(i)->setLeft( MIN(blocklist->at(i)->left(), blocklist->at(j)->left()) );
        blocklist->at(i)->setRight( MAX(blocklist->at(i)->right(), blocklist->at(j)->right()) );
        blocklist->at(i)->setTop( MIN(blocklist->at(i)->top(), blocklist->at(j)->top()) );
        blocklist->at(i)->setBottom( MAX(blocklist->at(i)->bottom(), blocklist->at(j)->bottom()) );
        blocklist->remove(j);
        numClusters--;
#if DEBUG_BLOCKS
        printf("Merge\n");
#endif
      }
      else
      {
#if DEBUG_BLOCKS
        printf("Skip\n");
#endif
        j++;
      }
    }
  }
#if DEBUG_BLOCKS
  printf("%d hits %d clusters\n",numHits,numClusters);
#endif

  return numClusters;
}

void GetFleshImage(BayesianClassifier* classifier, QColor background, QImage* inputImage, QImage* fleshImage)
{
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
  Matrix input;
  int width, height;
  QRgb* scanline;

  width = inputImage->width();
  height = inputImage->height();
#if 0
  input.SetSize(2, 1);
#else
  input.SetSize(3, 1);
#endif

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
      classifier->Classify(input, classIndex, confidence);

      if ( classIndex == 0 )
      {
        // Pixel is flesh colored
        fleshImage->setPixel(u, v, qRgb(r, g, b));
      }
      else
      {
        // Pixel is not flesh colored
        fleshImage->setPixel(u, v, background.rgb());
      }
    }
  }
}

void GetOutlineImage(QColor background, QColor outlineColor, QImage* inputImage, QImage* fleshImage, QImage* outlineImage)
{
  int i;
  int x, y;
  int numBlocks;
  int left, right, top, bottom;
  QPtrList<QRect> blocklist;

  *outlineImage = *inputImage; 
  numBlocks = GetFleshBlocks(background, fleshImage, &blocklist);
  for (i = 0; i < numBlocks; i++)
  {
    if ( (blocklist.at(i)->width() < 20) ||
         (blocklist.at(i)->height() < 20) )
      continue;

    left = blocklist.at(i)->left();
    right = blocklist.at(i)->right();
    top = blocklist.at(i)->top();
    bottom = blocklist.at(i)->bottom();

    for (x = left; x <= right; x++)
    {
      outlineImage->setPixel(x, top, outlineColor.rgb());
      outlineImage->setPixel(x, bottom, outlineColor.rgb());
    }
    for (y = top + 1; y < bottom; y++)
    {
      outlineImage->setPixel(left, y, outlineColor.rgb());
      outlineImage->setPixel(right, y, outlineColor.rgb());
    }
  }
}

int main(int argc, char* argv[])
{
  QApplication app(argc,argv);
  VideoDecoder decoder;
  BayesianClassifier classifier;
  QImage* inputImage;
  QImage fleshImage;
  QImage outputImage;
  int frameNumber = 0;
  QString vidFilename;
  char outputFilename[1024];
  int width = 0;
  int height;
  QColor background(255, 255, 255);
  QColor outline(0, 255, 0);

  if ( argc < 4 )
  {
    printf("Usage: outlinehand <classifier file> <video file> <output directory>\n");
    return 1;
  }

  if ( !classifier.Load(argv[1]) )
  {
    fprintf(stderr, "Error loading classifier %s\n", argv[1]);
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
    inputImage = decoder.GetFrame();

    if ( width == 0 )
    {
      width = inputImage->width();
      height = inputImage->height();
      fleshImage.create(width, height, 32);
      outputImage.create(width, height, 32);
    }

    GetFleshImage(&classifier, background, inputImage, &fleshImage);
    GetOutlineImage(background, outline, inputImage, &fleshImage, &outputImage);

    sprintf(outputFilename, "%s/flesh%05d.png", argv[3], frameNumber);
    fleshImage.save(outputFilename, "PNG");
    sprintf(outputFilename, "%s/frame%05d.png", argv[3], frameNumber);
    outputImage.save(outputFilename, "PNG");

    frameNumber++;
  }

  return 0;
}
