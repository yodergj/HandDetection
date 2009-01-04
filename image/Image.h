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
    void MarkBufferAsUpdated();
    int GetBufferUpdateIndex();
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
    bool DrawBox(const unsigned char* color, int lineWidth, int left, int top, int right, int bottom);
    bool DrawLine(const unsigned char* color, int lineWidth, int x1, int y1, int x2, int y2);
    bool Save(const char* filename);
    Image& operator=(const Image& ref);
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

    int mBufferUpdateIndex;
};

#endif
