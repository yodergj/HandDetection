#ifndef IMAGE_H
#define IMAGE_H

#include <string>

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
    bool CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyARGBBuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
    double* GetCustomBuffer(std::string &featureList);    
  private:
    bool SetSize(int width, int height);
    bool ResizeBuffer(double** buffer, int* bufferAlloc, int numFeatures);

    int mWidth;
    int mHeight;
    unsigned char* mBuffer;
    int mBufferSize;
    double* mYIQBuffer;
    int mYIQAlloc;
    double* mScaledRGBBuffer;
    int mScaledRGBAlloc;
    double* mCustomBuffer;
    int mCustomAlloc;
};

#endif
