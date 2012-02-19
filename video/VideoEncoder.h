#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <string>
using std::string;
class Image;

namespace avm
{
  class IWriteFile;
  class IVideoWriteStream;
}

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

class VideoEncoder
{
  public:
    VideoEncoder();
    ~VideoEncoder();
    bool Open(const char* filename, int width, int height, int fps);
    bool Close();
    bool AddFrame(Image* image);
  private:
    bool WriteVPXFileHeader();
    bool WriteVPXFrameData(bool flush);
    void Put16(unsigned char* buffer, int data);
    void Put32(unsigned char* buffer, int data);
    string mFilename;
    int mWidth;
    int mHeight;
    int mFPS;
    int mNumFrames;
    bool mAVIProcessing;
    bool mVPXProcessing;

    avm::IWriteFile* mAVIOutFile;
    avm::IVideoWriteStream* mAVIVidStr;

    vpx_codec_ctx_t mVPXCodec;
    vpx_codec_enc_cfg_t mVPXConfig;
    vpx_image_t mVPXRawImage;
    FILE* mVPXOutFile;
};

#endif