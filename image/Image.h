#ifndef IMAGE_H
#define IMAGE_H

#include "ConnectedRegion.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

class Image
{
  public:
    Image();
    ~Image();
    bool Create(int width, int height);
    int GetWidth();
    int GetHeight();
    unsigned char* GetRGBBuffer();
    double* GetYIQBuffer();
    double* GetScaledRGBBuffer();
    double* GetCustomBuffer(string &featureList);    
    double* GetCustomIntegralBuffer(string &featureList);    
    bool GetConfidenceBuffer(double* &buffer, int &bufferWidth, int &bufferHeight, int &bufferAlloc);
    bool SetConfidenceBuffer(double* buffer, int bufferWidth, int bufferHeight, int bufferAlloc);
    vector<ConnectedRegion*>* GetRegionsFromConfidenceBuffer();
    bool CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyARGBBuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
    bool Save(const char* filename);
  private:
    bool SavePPM(const char* filename);
    bool SetSize(int width, int height);
    bool ResizeBuffer(double** buffer, int* bufferAlloc, int numFeatures);
    void InvalidateBuffers();
    void ClearRegions();

    int mWidth;
    int mHeight;
    unsigned char* mBuffer;
    int mBufferSize;
    double* mYIQBuffer;
    int mYIQAlloc;
    bool mYIQValid;
    double* mScaledRGBBuffer;
    int mScaledRGBAlloc;
    bool mScaledRGBValid;
    double* mCustomBuffer;
    int mCustomAlloc;
    bool mCustomValid;
    string mCustomString;

    double* mConfidenceBuffer;
    int mConfidenceBufferAlloc;
    int mConfidenceBufferWidth;
    int mConfidenceBufferHeight;
    vector<ConnectedRegion*> mConfidenceRegions;
    bool mConfidenceRegionsValid;

    double* mCustomIntegralBuffer;
    int mCustomIntegralAlloc;
    bool mCustomIntegralValid;
};

#endif
