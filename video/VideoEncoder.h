#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <string>
using std::string;
class Image;

//#define USE_VPX
#define USE_FFMPEG

#ifdef USE_VPX
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"
#endif

#ifdef USE_FFMPEG
extern "C" {
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}
#endif

class VideoEncoder
{
  public:
    VideoEncoder();
    ~VideoEncoder();
    bool Open(const char* filename, int width, int height, int fps);
    bool Close();
    bool AddFrame(Image* image);
  private:
#ifdef USE_VPX
    bool WriteVPXFileHeader();
    bool WriteVPXFrameData(bool flush);
    void Put16(unsigned char* buffer, int data);
    void Put32(unsigned char* buffer, int data);
#endif
    string mFilename;
    int mWidth;
    int mHeight;
    int mFPS;
    int mNumFrames;
    bool mVPXProcessing;

#ifdef USE_VPX
    vpx_codec_ctx_t mVPXCodec;
    vpx_codec_enc_cfg_t mVPXConfig;
    vpx_image_t mVPXRawImage;
    FILE* mVPXOutFile;
#endif

#ifdef USE_FFMPEG
    AVFormatContext *mFormatContext;
    AVCodecContext *mCodecContext;
    AVCodec *mCodec;
    AVFrame *mFrame;
    AVFrame *mFrameRGB;
    AVPacket mPacket;
    struct SwsContext* mSwsContext;
    int mVideoStream;
    uint8_t *mBuffer;

    enum AVCodecID mCodecID;
    FILE* mOutFile;
#endif
};

#endif