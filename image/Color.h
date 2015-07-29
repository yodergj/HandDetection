#ifndef _COLOR_H
#define _COLOR_H

double GetHue(int R, int G, int B);
double GetHue(unsigned char* rgbVals);
bool HueInRange(double refHue, double tolerance, double testHue);
double GetHueDistance(double hueA, double hueB);

double GetHSLSaturation(int R, int G, int B);
double GetHSLSaturation(unsigned char* rgbVals);

double GetLightness(int R, int G, int B);
double GetLightness(unsigned char* rgbVals);

double GetHSISaturation(int R, int G, int B);
double GetHSISaturation(unsigned char* rgbVals);

double GetIntensity(int R, int G, int B);
double GetIntensity(unsigned char* rgbVals);

double GetHSVSaturation(int R, int G, int B);
double GetHSVSaturation(unsigned char* rgbVals);

double GetHSVValue(int R, int G, int B);
double GetHSVValue(unsigned char* rgbVals);

#endif