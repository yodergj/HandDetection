#include "VideoDecoder.h"
#include <stdlib.h>

static bool AVRegistered = false;

VideoDecoder::VideoDecoder()
{
  mFormatContext = NULL;
  mCodecContext = NULL;
  mCodec = NULL;
  mFrame = NULL;
  mFrameRGB = NULL;
  mPacket.data = NULL;
  mVideoStream = -1;
  mBuffer = NULL;
  mStartFrame = 0;
  mFilename = "";

  if ( !AVRegistered )
  {
    av_register_all();
    AVRegistered = true;
  }
  mDoneReading = true;
}

VideoDecoder::~VideoDecoder()
{
  if ( mFrame )
    av_free(mFrame);
  if ( mFrameRGB )
    av_free(mFrameRGB);
  if ( mCodecContext )
    avcodec_close(mCodecContext);
  if ( mFormatContext )
    av_close_input_file(mFormatContext);
  if ( mBuffer )
    free(mBuffer);
}

QImage* VideoDecoder::GetFrame()
{
  if ( mDoneReading )
    return NULL;
  return &mQImage;
}

Image* VideoDecoder::GetMyFrame()
{
  if ( mDoneReading )
    return NULL;
  return &mImage;
}

bool VideoDecoder::SetStartFrame(int frame)
{
  if ( frame < 0 )
    return false;

  mStartFrame = frame;
  return true;
}

void VideoDecoder::SetFilename(QString filename)
{
  mFilename = filename;
}

void VideoDecoder::Reset()
{
  int i;

  if ( mFilename != "" )
  {
    Load();
    for (i=0; i <= mStartFrame; i++)
      UpdateFrame();
  }
}

bool VideoDecoder::GetNextFrame()
{
  int frameFinished;

  while ( av_read_frame(mFormatContext, &mPacket) >= 0 )
  {
    if ( mPacket.stream_index == mVideoStream )
    {
      avcodec_decode_video(mCodecContext, mFrame, &frameFinished, mPacket.data, mPacket.size);

      if ( frameFinished )
      {
        img_convert((AVPicture *)mFrameRGB, PIX_FMT_RGB24, (AVPicture *)mFrame, mCodecContext->pix_fmt, mCodecContext->width, mCodecContext->height);

        return true;
      }
    }
    av_free_packet(&mPacket);
  }
  mDoneReading = true;

  return false;
}

bool VideoDecoder::UpdateFrame()
{
  int *pixel;
  int srcWidth, srcHeight;
  int x,y;
  uint8_t *srcData = NULL;

  if ( !GetNextFrame() )
    return false;

  if ( !mQImage.isNull() )
  {
    srcWidth = mCodecContext->width;
    srcHeight = mCodecContext->height;
    pixel = (int *)mQImage.bits();

    for (y=0; y<srcHeight; y++)
    {
      srcData = mFrameRGB->data[0] + y * mFrameRGB->linesize[0];
      for (x=0; x<srcWidth; x++)
      {
        *pixel = srcData[2] | (srcData[1] << 8) | (srcData[0] << 16);
        pixel++;
        srcData += 3;
      }
    }
  }

  return mImage.CopyRGBBuffer(mCodecContext->width, mCodecContext->height, mFrameRGB->data[0], mFrameRGB->linesize[0]);
}

bool VideoDecoder::Load()
{
  unsigned int i;
  int numBytes;
  uint8_t *tmp;

  if ( av_open_input_file(&mFormatContext, mFilename.ascii(), NULL, 0, NULL) != 0 )
    return false;

  if ( av_find_stream_info(mFormatContext) < 0 )
    return false;

  /* Some debug info */
  dump_format(mFormatContext, 0, mFilename.ascii(), false);

  for (i=0; i < mFormatContext->nb_streams; i++)
  {
    if ( mFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO )
    {
      mVideoStream = i;
      break;
    }
  }

  if ( mVideoStream == -1 )
    return false;

  mCodecContext = mFormatContext->streams[mVideoStream]->codec;
  mCodec = avcodec_find_decoder(mCodecContext->codec_id);
  if ( !mCodec )
    return false;

  if ( avcodec_open(mCodecContext, mCodec) < 0 )
    return false;

  mFrame = avcodec_alloc_frame();
  if ( !mFrame )
    return false;

  mFrameRGB = avcodec_alloc_frame();
  if ( !mFrameRGB )
    return false;

  /* Determine required buffer size and allocate buffer */
  numBytes = avpicture_get_size(PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);
  tmp = (uint8_t *)realloc(mBuffer,numBytes*sizeof(uint8_t));
  if ( !tmp )
    return false;
  mBuffer = tmp;

  avpicture_fill((AVPicture *)mFrameRGB, mBuffer, PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);

  /* Have to use 32-bit depth since 24-bit isn't supported anymore */
  if ( !mQImage.create(mCodecContext->width, mCodecContext->height, 32) )
    return false;

  mDoneReading = false;

  return true;
}
