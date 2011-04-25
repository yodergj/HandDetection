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

class VideoEncoder
{
  public:
    VideoEncoder();
    ~VideoEncoder();
    bool Open(const char* filename, int width, int height, int fps);
    bool Close();
    bool AddFrame(Image* image);
  private:
    string mFilename;
    int mWidth;
    int mHeight;
    int mFPS;
    bool mAVIProcessing;

    avm::IWriteFile* mAVIOutFile;
    avm::IVideoWriteStream* mAVIVidStr;
};

#endif