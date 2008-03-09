/* Gabe Yoder
   CS660
   Course Project
*/

#include "ImageRoutines.h"

#define LOWER_DIFF_THRESH 120
#define UPPER_DIFF_THRESH 134
#define PROXIMITY_THRESH  30

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

bool GetBlockStatistics(QImage *image, int width, int height, BlockStat *blocks, ColorComponent color)
{
  int row, col;
  int x, y;
  int xmin, xmax, ymin, ymax;
  int imageWidth, imageHeight;
  int blockWidth, blockHeight;
  QColor pixel;
  double sum;
  double val;
  int h, s, v;
  int r, g, b;

  if ( !image || !blocks || (width <= 0) || (height <= 0) )
    return false;

  imageWidth = image->width();
  imageHeight = image->height();
  blockWidth = imageWidth / width;
  blockHeight = imageHeight / height;

  /* Get the mean values */
  for (row=0; row < height; row++)
  {
    ymin = row * blockHeight;
    ymax = (row + 1) * blockHeight - 1;
    for (col=0; col < width; col++)
    {
      xmin = col * blockWidth;
      xmax = (col + 1) * blockWidth - 1;
      sum = 0;
      for (y=ymin; y <= ymax; y++)
      {
        for (x=xmin; x <= xmax; x++)
        {
          pixel = image->pixel(x,y);
          if ( (color == COLOR_HUE) ||
               (color == COLOR_SATURATION) ||
               (color == COLOR_VALUE) )
          {
              pixel.getHsv(&h, &s, &v);
              if ( color == COLOR_HUE )
                sum += h;
              else if ( color == COLOR_SATURATION )
                sum += s;
              else
                sum += v;
          }
          else
          {
            pixel.getRgb(&r, &g, &b);
            if ( color == COLOR_RED )
              sum += r;
            else if ( color == COLOR_GREEN )
              sum += g;
            else if ( color == COLOR_BLUE )
              sum += b;
            else if ( color == COLOR_Y )
              sum += r * .299 + g *  .587 + b *  .114;
            else if ( color == COLOR_I )
              sum += ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
            else
              sum += ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;
          }
        }
      }
      blocks[row * width + col].mean = sum / (blockWidth * blockHeight);

      sum = 0;
      for (y=ymin; y <= ymax; y++)
      {
        for (x=xmin; x <= xmax; x++)
        {
          pixel = image->pixel(x,y);
          if ( (color == COLOR_HUE) ||
               (color == COLOR_SATURATION) ||
               (color == COLOR_VALUE) )
          {
              pixel.getHsv(&h, &s, &v);
              if ( color == COLOR_HUE )
                val = h;
              else if ( color == COLOR_SATURATION )
                val = s;
              else
                val = v;
          }
          else
          {
            pixel.getRgb(&r, &g, &b);
            if ( color == COLOR_RED )
              val = r;
            else if ( color == COLOR_GREEN )
              val = g;
            else if ( color == COLOR_BLUE )
              val = b;
            else if ( color == COLOR_Y )
              val = r * .299 + g *  .587 + b *  .114;
            else if ( color == COLOR_I )
              val = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
            else
              val = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;
          }
          sum += (val - blocks[row * width + col].mean) *
                 (val - blocks[row * width + col].mean);
        }
      }
      blocks[row * width + col].variance = sum / (blockWidth * blockHeight);
    }
  }
  return true;
}

bool SubtractImage(QImage *imageA, QImage *imageB, QImage *output)
{
  int x, y;
  int width, height;
  QColor pixelA;
  QColor pixelB;
  int rA, gA, bA;
  int rB, gB, bB;
  int rDiff, gDiff, bDiff;

  if ( !imageA || !imageB || !output )
    return false;

  width = imageA->width();
  height = imageA->height();

  if ( (width != imageB->width()) || (height != imageB->height()) ||
       (width != output->width()) || (height != output->height()) )
    return false;

  for (y=0; y<height; y++)
  {
    for (x=0; x<width; x++)
    {
      pixelA = imageA->pixel(x,y);
      pixelB = imageB->pixel(x,y);

      pixelA.getRgb(&rA, &gA, &bA);
      pixelB.getRgb(&rB, &gB, &bB);

      rDiff = rA - rB;
      gDiff = gA - gB;
      bDiff = bA - bB;

      output->setPixel(x, y, qRgb((rDiff+255)/2,(gDiff+255)/2,(bDiff+255)/2));
    }
  }

  return true;
}

QImage GetIQImage(QImage *inputImage)
{
  QImage output;
  QColor pixel;
  int width, height;
  double ic, qc;
  int x, y;
  int r, g, b;

  width = inputImage->width();
  height = inputImage->height();
  output.create(width,height,inputImage->depth());

  for (y=0; y < height; y++)
  {
    for (x=0; x < width; x++)
    {
      pixel = inputImage->pixel(x,y);
      pixel.getRgb(&r, &g, &b);
      ic = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
      qc = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;
      output.setPixel(x, y, qRgb((int)ic,(int)qc,0));
    }
  }

  return output;
}

int GetDifferenceBlocks(QImage *image, QPtrList<QRect> *blocklist)
{
  int i, j;
  int x, y;
  int r, g, b;
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
      pixel.getRgb(&r, &g, &b);

      if ( (r <= LOWER_DIFF_THRESH) || (r >= UPPER_DIFF_THRESH) ||
           (g <= LOWER_DIFF_THRESH) || (g >= UPPER_DIFF_THRESH) ||
           (b <= LOWER_DIFF_THRESH) || (b >= UPPER_DIFF_THRESH) )
      {
        printf("Hit (%d,%d)\n",x,y);
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
          printf("New block\n");
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
      printf("Block (%d %d %d %d) vs (%d %d %d %d)\n",
          blocklist->at(i)->left(),
          blocklist->at(i)->right(),
          blocklist->at(i)->top(),
          blocklist->at(i)->bottom(),
          blocklist->at(j)->left(),
          blocklist->at(j)->right(),
          blocklist->at(j)->top(),
          blocklist->at(j)->bottom());
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
        printf("Merge\n");
      }
      else
      {
        printf("Skip\n");
        j++;
      }
    }
  }
  printf("%d hits %d clusters\n",numHits,numClusters);

  return numClusters;
}
