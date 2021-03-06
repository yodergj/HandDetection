#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <vector>
#include "ConnectedRegion.h"
#include <opencv/cv.h>
using std::string;
using std::vector;

class Point;
class LineSegment;
class Rect;

class Image
{
  public:
    Image();
    Image(const Image& ref);
    virtual ~Image();
    bool Create(int width, int height);
    int GetWidth();
    int GetHeight();
    unsigned char* GetRGBBuffer();
    unsigned char* GetBGRBuffer();
    unsigned char* GetI420Buffer();
    unsigned char* GetYUVBuffer();
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
    bool CopyBGRABuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyARGBBuffer(int width, int height, int* buffer, int bufferWidth);
    bool CopyRGBBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
    bool CopyBGRBuffer(int width, int height, unsigned char* buffer, int bufferWidth);
    bool CopyIplImage(IplImage* image);
    bool DrawBox(const unsigned char* color, int lineWidth, int left, int top, int right, int bottom);
    bool DrawBox(const unsigned char* color1, const unsigned char* color2, const unsigned char* color3, const unsigned char* color4, int lineWidth, int left, int top, int right, int bottom);
    bool DrawRect(const unsigned char* color, int lineWidth, const Rect& rect);
    bool DrawLine(const unsigned char* color, int lineWidth, int x1, int y1, int x2, int y2);
    bool DrawLine(const unsigned char* color, int lineWidth, const Point& p1, const Point& p2);
    bool DrawLine(const unsigned char* color, int lineWidth, const LineSegment& line);
    bool Save(const char* filename);
    bool Save(const string& filename);
    bool Load(const char* filename);
    bool Load(const string& filename);
    IplImage* GetIplImage();
    Image& operator=(const Image& ref);
  protected:
    bool SavePPM(const char* filename);
    bool SetSize(int width, int height);
    bool ResizeBuffer(double** buffer, int* bufferAlloc, int numFeatures);
    void InvalidateBuffers();
    void ClearRegions();

    int mWidth;
    int mHeight;
    unsigned char* mBuffer;
    int mBufferSize;
    unsigned char* mBGRBuffer;
    int mBGRBufferSize;
    bool mBGRValid;
    unsigned char* mI420Buffer;
    int mI420BufferSize;
    bool mI420Valid;

    unsigned char* mYUVBuffer;
    int mYUVBufferSize;
    bool mYUVValid;

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

    IplImage mIplImage;
};

#endif
