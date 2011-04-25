#include <stdio.h>
#include <avifile.h>
#include <avm_fourcc.h>
#include "VideoEncoder.h"
#include "Image.h"

VideoEncoder::VideoEncoder()
{
  mWidth = 0;
  mHeight = 0;
  mFPS = 0;
  mAVIProcessing = false;
}

VideoEncoder::~VideoEncoder()
{
}

bool VideoEncoder::Open(const char* filename, int width, int height, int fps)
{
  bool retVal = true;
  string extension;

  if ( !filename || (strlen(filename) < 5) || (width <= 0) || (height <= 0) || (fps <= 0) )
  {
    fprintf(stderr, "VideoEncoder::Open - Invalid parameter\n");
    return false;
  }

  if ( !mFilename.empty() )
  {
    fprintf(stderr, "VideoEncoder::Open - Can't open %s until %s is closed.\n", filename, mFilename.c_str());
    return false;
  }

  mFilename = filename;
  extension = mFilename.substr( mFilename.size() - 3 );
  mWidth = width;
  mHeight = height;
  mFPS = fps;

  if ( extension == "avi" )
    mAVIProcessing = true;

  if ( mAVIProcessing )
  {
    mAVIOutFile = avm::CreateWriteFile(mFilename.c_str());
    if ( !mAVIOutFile )
    {
      fprintf(stderr, "VideoEncoder::Open - Error opening AVI %s\n", filename);
      mAVIProcessing = false;
      return false;
    }

    BITMAPINFOHEADER bi;
    //fourcc_t codec = fccMP42;
    fourcc_t codec = fccDIV3;
    //fourcc_t codec = fccIV32;
    //fourcc_t codec = fccCVID;

    memset(&bi, 0, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = mWidth;
    bi.biHeight = mHeight;
    bi.biSizeImage = mWidth * mHeight * 3;
    bi.biPlanes = 1;
    bi.biBitCount = 24;

    mAVIVidStr = mAVIOutFile->AddVideoStream(codec, &bi, 1000000 / mFPS);
    mAVIVidStr->Start();
  }
  else
    retVal = false;

  return retVal;
}

bool VideoEncoder::Close()
{
  bool retVal = true;

  if ( mAVIProcessing )
  {
    mAVIVidStr->Stop();
    mAVIVidStr = 0;
    delete mAVIOutFile;
    mAVIOutFile = 0;
    mAVIProcessing = false;
  }
  else
    retVal = false;

  mFilename = "";

  return retVal;
}

bool VideoEncoder::AddFrame(Image* image)
{
  bool retVal = true;

  if ( !image || (image->GetWidth() != mWidth) || (image->GetHeight() != mHeight) )
  {
    fprintf(stderr, "VideoEncoder::AddFrame - Invalid parameter\n");
    return false;
  }

  if ( mAVIProcessing )
  {
    unsigned char* data = image->GetBGRBuffer();
    if ( !data )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - Failed getting image buffer\n");
      return false;
    }
    avm::CImage frame(data, mWidth, mHeight);
    mAVIVidStr->AddFrame(&frame);
  }
  else
    retVal = false;

  return retVal;
}