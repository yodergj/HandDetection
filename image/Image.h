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
    double* GetCustomBuffer(std::string &featureList);    
    double* GetCustomIntegralBuffer(std::string &featureList);    
    bool CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyARGBBuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
    bool Save(const char* filename);
  private:
    bool SavePPM(const char* filename);
    bool SetSize(int width, int height);
    bool ResizeBuffer(double** buffer, int* bufferAlloc, int numFeatures);
    void InvalidateBuffers();

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
    std::string mCustomString;

    double* mCustomIntegralBuffer;
    int mCustomIntegralAlloc;
    bool mCustomIntegralValid;
};

#endif
