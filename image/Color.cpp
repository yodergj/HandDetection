#include <math.h>
#include "Color.h"

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

double GetHue(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;
  double hue;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  if ( maxVal == minVal )
    hue = -1;
  else if ( r == maxVal )
  {
    if ( g >= b )
      hue = (g - b) / (6 * (maxVal - minVal));
    else
      hue = (g - b) / (6 * (maxVal - minVal)) + 1;
  }
  else if ( g == maxVal )
    hue = (b - r) / (6 * (maxVal - minVal)) + (1 / 3.0);
  else
    hue = (r - g) / (6 * (maxVal - minVal)) + (2 / 3.0);

  return hue;
}

double GetHue(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;
  double hue;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  if ( maxVal == minVal )
    hue = -1;
  else if ( r == maxVal )
  {
    if ( g >= b )
      hue = (g - b) / (6 * (maxVal - minVal));
    else
      hue = (g - b) / (6 * (maxVal - minVal)) + 1;
  }
  else if ( g == maxVal )
    hue = (b - r) / (6 * (maxVal - minVal)) + (1 / 3.0);
  else
    hue = (r - g) / (6 * (maxVal - minVal)) + (2 / 3.0);

  return hue;
}

bool HueInRange(double refHue, double tolerance, double testHue)
{
  // We set hue to -1 for shades of gray
  if ( (testHue < 0) && (refHue < 0) )
    return true;

  if ( (testHue < 0) || (refHue < 0) )
    return false;

  if ( refHue < tolerance )
    return ( (testHue < refHue + tolerance) || (testHue > 1 + refHue - tolerance) );

  if ( refHue + tolerance > 1.0 )
    return ( (testHue > refHue - tolerance) || (testHue < refHue + tolerance - 1) );

  return ( (testHue > refHue - tolerance) && (testHue < refHue + tolerance) );
}

double GetHueDistance(double hueA, double hueB)
{
  double dist = fabs(hueA - hueB);
  if ( dist > .5 )
    dist -= .5;
  return dist;
}

double GetHSLSaturation(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation;
  if ( maxVal == minVal )
    saturation = 0;
  else if ( maxVal + minVal <= 1 )
    saturation = (maxVal - minVal) / (maxVal + minVal);
  else
    saturation = (maxVal - minVal) / (2 - maxVal + minVal);

  return saturation;
}

double GetHSLSaturation(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation;
  if ( maxVal == minVal )
    saturation = 0;
  else if ( maxVal + minVal <= 1 )
    saturation = (maxVal - minVal) / (maxVal + minVal);
  else
    saturation = (maxVal - minVal) / (2 - maxVal + minVal);

  return saturation;
}

double GetLightness(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  return (maxVal + minVal) / 2;
}

double GetLightness(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  return (maxVal + minVal) / 2;
}

double GetHSISaturation(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation;
  if ( maxVal == minVal )
    saturation = 0;
  else
    saturation = 1 - minVal / ( (r + g + b) / 3.0 );

  return saturation;
}

double GetHSISaturation(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation;
  if ( maxVal == minVal )
    saturation = 0;
  else
    saturation = 1 - minVal / ( (r + g + b) / 3.0 );

  return saturation;
}

double GetIntensity(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  return (r + g + b) / 3.0;
}

double GetIntensity(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  return (r + g + b) / 3.0;
}

double GetHSVSaturation(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation = (maxVal - minVal) / maxVal;

  return saturation;
}

double GetHSVSaturation(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  maxVal = MAX(r, MAX(g, b));
  minVal = MIN(r, MIN(g, b));

  double saturation = (maxVal - minVal) / maxVal;

  return saturation;
}

double GetHSVValue(int R, int G, int B)
{
  double maxVal, minVal;
  double r, g, b;

  r = R / 255.0;
  g = G / 255.0;
  b = B / 255.0;

  return MAX(r, MAX(g, b));
}

double GetHSVValue(unsigned char* rgbVals)
{
  double maxVal, minVal;
  double r, g, b;

  r = rgbVals[0] / 255.0;
  g = rgbVals[1] / 255.0;
  b = rgbVals[2] / 255.0;

  return MAX(r, MAX(g, b));
}