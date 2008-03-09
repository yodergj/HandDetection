#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <qimage.h>
extern "C" {
#include <ffmpeg/avformat.h>
}

class VideoDecoder
{
  public:
    VideoDecoder();
    ~VideoDecoder();
    bool SetStartFrame(int frame);
    void SetFilename(QString filename);
    bool Load();
    void Reset();
    QImage* GetFrame();
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
    QImage mImage;
    int mStartFrame;
    QString mFilename;
    bool mDoneReading;
};

#endif
