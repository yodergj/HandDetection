#include <stdio.h>
#include <errno.h>
#if 0
#include <avifile.h>
#include <avm_fourcc.h>
#endif

#include "VideoEncoder.h"
#include "Image.h"

VideoEncoder::VideoEncoder()
{
  mWidth = 0;
  mHeight = 0;
  mFPS = 0;
  mNumFrames = 0;

  mAVIOutFile = 0;
  mAVIVidStr = 0;

  mVPXOutFile = 0;

  mAVIProcessing = false;
  mVPXProcessing = false;
}

VideoEncoder::~VideoEncoder()
{
}

bool VideoEncoder::Open(const char* filename, int width, int height, int fps)
{
  bool retVal = true;
  string extension;
  size_t dotPos;

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
  dotPos = mFilename.rfind('.');
  if ( dotPos != string::npos )
    dotPos++;
  extension = mFilename.substr( dotPos );
  mWidth = width;
  mHeight = height;
  mFPS = fps;
  mNumFrames = 0;

  if ( extension == "avi" )
    mAVIProcessing = true;
  else if ( extension == "webm" || extension == "vpx" )
    mVPXProcessing = true;

  if ( mAVIProcessing )
  {
#if 0
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
#endif
  }
  else if ( mVPXProcessing )
  {
    vpx_codec_err_t errorCode = vpx_codec_enc_config_default(vpx_codec_vp8_cx(), &mVPXConfig, 0);
    if ( errorCode )
    {
      fprintf(stderr, "VideoEncoder::Open - Error getting VPX config - %s\n",
              vpx_codec_err_to_string(errorCode));
      return false;
    }

    mVPXConfig.rc_target_bitrate = mWidth * mHeight * mVPXConfig.rc_target_bitrate / mVPXConfig.g_w / mVPXConfig.g_h;
    mVPXConfig.g_w = mWidth;
    mVPXConfig.g_h = mHeight;

#if 1
    if ( !vpx_img_alloc(&mVPXRawImage, VPX_IMG_FMT_I420, mWidth, mHeight, 1) )
#else
    if ( !vpx_img_alloc(&mVPXRawImage, VPX_IMG_FMT_RGB24, mWidth, mHeight, 1) )
#endif
    {
      fprintf(stderr, "VideoEncoder::Open - Failed allocating VPX raw frame %d x %d \n",
              mWidth, mHeight);
    }

    if ( vpx_codec_enc_init(&mVPXCodec, vpx_codec_vp8_cx(), &mVPXConfig, 0) )
    {
      fprintf(stderr, "VideoEncoder::Open - Error initializing VPX encoder - %s\n",
              vpx_codec_error(&mVPXCodec));
      const char* errorDetails = vpx_codec_error_detail(&mVPXCodec);
      if ( errorDetails )
        fprintf(stderr, "VideoEncoder::Open - Error details - %s\n", errorDetails);
      return false;
    }

    mVPXOutFile = fopen(filename, "wb");
    if ( !mVPXOutFile )
    {
      fprintf(stderr, "VideoEncoder::Open - Failed opening %s - %s\n",
              filename, strerror(errno));
      return false;
    }

    if ( !WriteVPXFileHeader() )
    {
      retVal = false;
      fclose(mVPXOutFile);
      mVPXOutFile = 0;
    }
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
#if 0
    mAVIVidStr->Stop();
    mAVIVidStr = 0;
    delete mAVIOutFile;
    mAVIOutFile = 0;
    mAVIProcessing = false;
#endif
  }
  else if ( mVPXProcessing )
  {
    retVal = WriteVPXFrameData(true);
    if ( vpx_codec_destroy(&mVPXCodec) )
    {
      fprintf(stderr, "VideoEncoder::Close - Error destroying VPX codec - %s\n",
              vpx_codec_error(&mVPXCodec));
      const char* errorDetails = vpx_codec_error_detail(&mVPXCodec);
      if ( errorDetails )
        fprintf(stderr, "VideoEncoder::Close - Error details - %s\n", errorDetails);
      retVal = false;
    }
    // Update file header now that number of frames is known
    if ( !fseek(mVPXOutFile, 0, SEEK_SET) )
      WriteVPXFileHeader();
    else
      fprintf(stderr, "VideoEncoder::Close - Error seeking to beginning of file - %s\n",
              strerror(errno));
    fclose(mVPXOutFile);
    mVPXOutFile = 0;
    mVPXProcessing = false;
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
#if 0
    unsigned char* data = image->GetBGRBuffer();
    if ( !data )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - Failed getting image buffer\n");
      return false;
    }
    avm::CImage frame(data, mWidth, mHeight);
    mAVIVidStr->AddFrame(&frame);
#endif
  }
  else if ( mVPXProcessing )
  {
#if 0
    unsigned char* data = image->GetRGBBuffer();
#else
    unsigned char* data = image->GetI420Buffer();
#endif
    if ( !data )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - Failed getting image buffer\n");
      return false;
    }
#if 0
    memcpy(mVPXRawImage.planes[0], data, mWidth * mHeight * 3);
#else
    memcpy(mVPXRawImage.planes[0], data, mWidth * mHeight * 3 / 2);
#endif
    retVal = WriteVPXFrameData(false);
  }
  else
    retVal = false;

  if ( retVal )
    mNumFrames++;

  return retVal;
}

bool VideoEncoder::WriteVPXFileHeader()
{
  unsigned char data[32];

  if ( (mVPXConfig.g_pass != VPX_RC_ONE_PASS) && (mVPXConfig.g_pass != VPX_RC_LAST_PASS)  )
  {
    fprintf(stderr, "VideoEncoder::WriteVPXFileHeader - Invalid g_pass in VPX configuration");
    return false;
  }

  data[0] = 'D';
  data[1] = 'K';
  data[2] = 'I';
  data[3] = 'F';
  Put16(data + 4, 0);
  Put16(data + 6, 32);
  Put32(data + 8, 0x30385056);
  Put16(data + 12, mVPXConfig.g_w);
  Put16(data + 14, mVPXConfig.g_h);
  Put32(data + 16, mVPXConfig.g_timebase.den);
  Put32(data + 20, mVPXConfig.g_timebase.num);
  Put32(data + 24, mNumFrames);
  Put32(data + 28, 0);
  if ( fwrite(data, 1, 32, mVPXOutFile) != 32 )
  {
    fprintf(stderr, "VideoEncoder::WriteVPXFileHeader - Error writing header - %s\n",
            strerror(errno) );
    return false;
  }
  return true;
}

bool VideoEncoder::WriteVPXFrameData(bool flush)
{
#if 0
  if ( vpx_codec_encode(&mVPXCodec, flush ? 0 : &mVPXRawImage, mNumFrames, 1, 0, VPX_DL_REALTIME) )
#else
  if ( vpx_codec_encode(&mVPXCodec, flush ? 0 : &mVPXRawImage, mNumFrames, 1, 0, VPX_DL_BEST_QUALITY) )
#endif
  {
    fprintf(stderr, "VideoEncoder::AddFrame - Error encoding frame for VPX - %s\n",
            vpx_codec_error(&mVPXCodec));
    const char* errorDetails = vpx_codec_error_detail(&mVPXCodec);
    if ( errorDetails )
      fprintf(stderr, "VideoEncoder::AddFrame - Error details - %s\n", errorDetails);
    return false;
  }
  vpx_codec_iter_t iter = 0;
  const vpx_codec_cx_pkt_t* pkt;
  unsigned char pktHdr[12];
  vpx_codec_pts_t pts;
  pkt = vpx_codec_get_cx_data(&mVPXCodec, &iter);
  while ( pkt )
  {
    switch ( pkt->kind )
    {
      case VPX_CODEC_CX_FRAME_PKT:
        pts = pkt->data.frame.pts;
        Put32(pktHdr, pkt->data.frame.sz);
        Put32(pktHdr + 4, pts & 0xFFFFFFFF);
        Put32(pktHdr + 8, pts >> 32);
        fwrite(pktHdr, 1, 12, mVPXOutFile);
        fwrite(pkt->data.frame.buf, 1, pkt->data.frame.sz, mVPXOutFile);
        break;
      default:
        fprintf(stderr, "VideoEncoder::AddFrame - Packet type %d\n", pkt->kind);
        break;
    }
    pkt = vpx_codec_get_cx_data(&mVPXCodec, &iter);
  }
  return true;
}

void VideoEncoder::Put16(unsigned char* buffer, int data)
{
  buffer[0] = (unsigned char)data;
  buffer[1] = (unsigned char)(data >> 8);
}

void VideoEncoder::Put32(unsigned char* buffer, int data)
{
  buffer[0] = (unsigned char)data;
  buffer[1] = (unsigned char)(data >> 8);
  buffer[2] = (unsigned char)(data >> 16);
  buffer[3] = (unsigned char)(data >> 24);
}
