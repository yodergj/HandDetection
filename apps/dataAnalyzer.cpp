#include <stdlib.h>
#include <string.h>
#include <string>
#include "Image.h"
#include "BayesianClassifier.h"

using std::string;

int main(int argc, char* argv[])
{
  Image inputImage;
  string basename;
  Image histogramImage;
  Image fleshImage, nonFleshImage;
  int j;
  int u, v;
  int width, height;
  int r, g, b;
  double y, i, q;
  BayesianClassifier classifier;
  Matrix input;
  int classIndex = 0;
  int *histogram;
  int numBins = 256;
  double scaleFactor = 255;
  int intensity;
  int maxFreq = 0;
  int equalizationLevels = 20;
  int *frequencyLevels;
//  int *freqHistogram;
  int totalPixels;
//  int totalBins, currentLevel, currentBinLimit, currentFreqBin;
  int red, green, blue;
  int dotPos;
  double ratio;
  int classComponents[] = {2, 5};
  unsigned char* pixel;
  unsigned char* fleshPixel;
  unsigned char* nonFleshPixel;
  unsigned char white[] = {255, 255, 255};

  if ( argc < 4 )
    return 0;

  frequencyLevels = (int *)malloc(equalizationLevels * sizeof(int));

  input.SetSize(2, 1);
  classifier.Create(2, 2, classComponents);
  histogramImage.Create(256, 256);

  for (j = 1; j < argc; j++)
  {
    if ( !strcmp(argv[j], "-x") )
    {
      classIndex = 1;
      continue;
    }
    if ( inputImage.Load(argv[j]) )
    {
      if ( classIndex == 0 )
        printf("Processing hand image %s\n", argv[j]);
      else
        printf("Processing other image %s\n", argv[j]);
      basename = argv[j];
      dotPos = basename.rfind('.');
      if ( dotPos != (int)string::npos )
        basename = basename.substr(0, dotPos);
      width = inputImage.GetWidth();
      height = inputImage.GetHeight();
      if ( classIndex == 0 )
      {
        fleshImage.Create(width, height);
        nonFleshImage.Create(width, height);
      }
      pixel = inputImage.GetRGBBuffer();
      fleshPixel = fleshImage.GetRGBBuffer();
      nonFleshPixel = nonFleshImage.GetRGBBuffer();
      for (v = 0; v < height; v++)
      {
        for (u = 0; u < width; u++, pixel += 3, fleshPixel += 3, nonFleshPixel += 3)
        {
          r = pixel[0];
          g = pixel[1];
          b = pixel[2];
          y =   r * .299 + g *  .587 + b *  .114;
          i = ((r * .596 + g * -.275 + b * -.321)/.596 + 255)/2;
          q = ((r * .212 + g * -.523 + b *  .311)/.523 + 255)/2;

          if ( classIndex == 0 )
          {
            if (y <= 50)
            {
              // Pixel is not flesh colored
              memcpy(fleshPixel, white, 3);
              memcpy(nonFleshPixel, pixel, 3);
            }
            else
            {
              // Pixel is flesh colored
              memcpy(fleshPixel, pixel, 3);
              memcpy(nonFleshPixel, white, 3);
            }
          }

          if ( (classIndex == 0) && (y <= 50) )
            continue;

          input.SetValue(0, 0, i / 255);
          input.SetValue(1, 0, q / 255);
          classifier.AddTrainingData(input, classIndex);
        }
      }
      if ( classIndex == 0 )
      {
        fleshImage.Save(basename + "_flesh.png");
        nonFleshImage.Save(basename + "_nonflesh.png");
      }
    }
  }

  histogram = classifier.Get2dDataHistogram(0, numBins, scaleFactor);
  totalPixels = 0;
  for (v = 0; v < 256; v++)
    for (u = 0; u < 256; u++)
    {
      totalPixels += histogram[v * numBins + u];
      if ( histogram[v * numBins + u] > maxFreq )
        maxFreq = histogram[v * numBins + u];
    }
#if 0
  freqHistogram = (int *)malloc(maxFreq * sizeof(int));
  memset(freqHistogram, 0, maxFreq * sizeof(int));
  for (v = 0; v < 256; v++)
    for (u = 0; u < 256; u++)
      freqHistogram[histogram[v * numBins + u]]++;

  totalBins = 0;
  currentFreqBin = 0;
  for (currentLevel = 0; currentLevel < equalizationLevels; currentLevel++)
  {
    currentBinLimit = (int)(65536.0 / (equalizationLevels - 1) * (currentLevel + 1) + .5);
    while ( (currentFreqBin < maxFreq) && (totalBins < currentBinLimit) )
    {
      totalBins += freqHistogram[currentFreqBin];
      currentFreqBin++;
    }
    printf("Total bins %d\n", totalBins);
    frequencyLevels[currentLevel] = currentFreqBin;
    printf("Level %d - %d\n", currentLevel, frequencyLevels[currentLevel]);
  }
#endif

  printf("Flesh Max Freq %d\n", maxFreq);

  pixel = histogramImage.GetRGBBuffer();
  for (v = 0; v < 256; v++)
  {
    for (u = 0; u < 256; u++, pixel += 3)
    {
#if 1
#if 0
      if ( histogram[v * numBins + u] > 0 )
        printf("(%d, %d) %d\n", u, v, histogram[v * numBins + u]);
#endif
#if 0
      intensity = (int)(histogram[v * numBins + u] / (double)maxFreq * 255 + .5);
      pixel[0] = (unsigned char)intensity;
      pixel[1] = (unsigned char)intensity;
      pixel[2] = (unsigned char)intensity;
#else
      ratio = histogram[v * numBins + u] / (double)maxFreq * 0xffffff;
      red = (int)(ratio / 0x10000 + .5);
      if ( red > 0 )
        green = 255;
      else
        green = (int)(ratio / 0x100 + .5);
      if ( green > 0 )
        blue = 255;
      else
        blue = (int)(ratio + .5);
      pixel[0] = (unsigned char)red;
      pixel[1] = (unsigned char)green;
      pixel[2] = (unsigned char)blue;
#endif
#else
      for (currentLevel = 0; currentLevel < equalizationLevels; currentLevel++)
      {
        if ( histogram[v * numBins + u] < frequencyLevels[currentLevel] )
          intensity = (int)(currentLevel * 256.0 / (equalizationLevels - 1) + .5);
        else
          break;
      }
      histogramImage.setPixel(u, v, qRgb(intensity, intensity, intensity));
#endif
    }
  }
  histogramImage.Save("FleshCluster.png");

  maxFreq = 0;
  totalPixels = 0;
  histogram = classifier.Get2dDataHistogram(1, numBins, scaleFactor);
  for (v = 0; v < 256; v++)
    for (u = 0; u < 256; u++)
    {
      totalPixels += histogram[v * numBins + u];
      if ( histogram[v * numBins + u] > maxFreq )
        maxFreq = histogram[v * numBins + u];
    }

  printf("Non Flesh Max Freq %d\n", maxFreq);

  pixel = histogramImage.GetRGBBuffer();
  for (v = 0; v < 256; v++)
  {
    for (u = 0; u < 256; u++, pixel += 3)
    {
#if 0
      intensity = (int)(histogram[v * numBins + u] / (double)maxFreq * 255 + .5);
      pixel[0] = (unsigned char)intensity;
      pixel[1] = (unsigned char)intensity;
      pixel[2] = (unsigned char)intensity;
#else
      ratio = histogram[v * numBins + u] / (double)maxFreq * 0xffffff;
      red = (int)(ratio / 0x10000 + .5);
      if ( red > 0 )
        green = 255;
      else
        green = (int)(ratio / 0x100 + .5);
      if ( green > 0 )
        blue = 255;
      else
        blue = (int)(ratio + .5);
      pixel[0] = (unsigned char)red;
      pixel[1] = (unsigned char)green;
      pixel[2] = (unsigned char)blue;
#endif
    }
  }
  histogramImage.Save("NonFleshClusterl.png");

  free(frequencyLevels);
  return 0;
}
