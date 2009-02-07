#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <string>
#include "Image.h"
extern "C" {
#include <ffmpeg/avformat.h>
}

using std::string;

class VideoDecoder
{
  public:
    VideoDecoder();
    ~VideoDecoder();
    bool SetStartFrame(int frame);
    void SetFilename(const string& filename);
    void SetFilename(const char* filename);
    bool Load();
    void Reset();
    Image* GetFrame();
    bool UpdateFrame();
  private:
    bool GetNextFrame();
    AVFormatContext *mFormatContext;
    AVCodecContext *mCodecContext;
    AVCodec *mCodec;
    AVFrame *mFrame;
    AVFrame *mFrameRGB;
    AVPacket mPacket;
    int mVideoStream;
    uint8_t *mBuffer;
    Image mImage;
    int mStartFrame;
    string mFilename;
    bool mDoneReading;
};

#endif
