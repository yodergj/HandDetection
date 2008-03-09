/* Gabe Yoder
   CS660
   Course Project
*/

#ifndef IMAGE_ROUTINES_H
#define IMAGE_ROUTINES_H

#include <qimage.h>

typedef struct
{
  double mean, variance;
} BlockStat;

typedef enum
{
  COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_HUE, COLOR_SATURATION, COLOR_VALUE, COLOR_Y, COLOR_I, COLOR_Q
} ColorComponent;

bool GetBlockStatistics(QImage *image, int width, int height, BlockStat *blocks, ColorComponent color);
bool SubtractImage(QImage *imageA, QImage *imageB, QImage *output);
QImage GetIQImage(QImage *inputImage);
int GetDifferenceBlocks(QImage *image, QPtrList<QRect> *blocklist);


#endif
