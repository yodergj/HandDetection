#ifndef IMAGE_H
#define IMAGE_H

class Image
{
  public:
    Image();
    ~Image();
    int GetWidth();
    int GetHeight();
    unsigned char* GetRGBBuffer();
    double* GetYIQBuffer();
    double* GetScaledRGBBuffer();
    bool CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
  private:
    bool SetSize(int width, int height);
    bool ResizeBuffer(double** buffer, int* bufferAlloc);

    int mWidth;
    int mHeight;
    unsigned char* mBuffer;
    int mBufferSize;
    double* mYIQBuffer;
    int mYIQAlloc;
    double* mScaledRGBBuffer;
    int mScaledRGBAlloc;
};

#endif
