#ifndef IMAGE_H
#define IMAGE_H

class Image
{
  public:
    Image();
    ~Image();
    bool CopyRGBABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
  private:
    bool SetSize(int width, int height);
    int mWidth;
    int mHeight;
    unsigned char* mBuffer;
    int mBufferSize;
};

#endif
