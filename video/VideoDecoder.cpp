#include <stdio.h>
#include <stdlib.h>
#include "VideoDecoder.h"

static bool AVRegistered = false;

VideoDecoder::VideoDecoder()
{
  mFormatContext = NULL;
  mCodecContext = NULL;
  mCodec = NULL;
  mFrame = NULL;
  mFrameRGB = NULL;
  mPacket.data = NULL;
  mSwsContext = NULL;
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

Image* VideoDecoder::GetFrame()
{
  if ( mDoneReading )
    return NULL;
  return &mImage;
}

bool VideoDecoder::SetStartFrame(int frame)
{
  if ( frame < 0 )
  {
    fprintf(stderr, "VideoDecoder::SetStartFrame - Invalid frame number %d\n", frame);
    return false;
  }

  mStartFrame = frame;
  return true;
}

void VideoDecoder::SetFilename(const string& filename)
{
  mFilename = filename;
}

void VideoDecoder::SetFilename(const char* filename)
{
  mFilename = filename;
}

void VideoDecoder::Reset()
{
  int i;

  if ( mFilename != "" )
  {
    Load();
    for (i = 0; i <= mStartFrame; i++)
      UpdateFrame();
  }
}

bool VideoDecoder::GetNextFrame()
{
  int frameFinished;
  int len;

  while ( av_read_frame(mFormatContext, &mPacket) >= 0 )
  {
    if ( mPacket.stream_index == mVideoStream )
    {
      len = avcodec_decode_video2(mCodecContext, mFrame, &frameFinished, &mPacket);

      if ( frameFinished )
      {
        sws_scale(mSwsContext, mFrame->data, mFrame->linesize, 0, mCodecContext->height, mFrameRGB->data, mFrameRGB->linesize);

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
  if ( !GetNextFrame() )
    return false;

  return mImage.CopyRGBBuffer(mCodecContext->width, mCodecContext->height, mFrameRGB->data[0], mFrameRGB->linesize[0]);
}

bool VideoDecoder::Load()
{
  unsigned int i;
  int numBytes;
  uint8_t *tmp;

  if ( avformat_open_input(&mFormatContext, mFilename.c_str(), NULL, NULL) != 0 )
  {
    fprintf(stderr, "VideoDecoder::Load - av_open_input_file failed\n");
    return false;
  }

  if ( av_find_stream_info(mFormatContext) < 0 )
  {
    fprintf(stderr, "VideoDecoder::Load - av_find_stream_info failed\n");
    return false;
  }

  /* Some debug info */
  av_dump_format(mFormatContext, 0, mFilename.c_str(), false);

  for (i = 0; i < mFormatContext->nb_streams; i++)
  {
    if ( mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
    {
      mVideoStream = i;
      break;
    }
  }

  if ( mVideoStream == -1 )
  {
    fprintf(stderr, "VideoDecoder::Load - No video stream found.\n");
    return false;
  }

  mCodecContext = mFormatContext->streams[mVideoStream]->codec;
  mCodec = avcodec_find_decoder(mCodecContext->codec_id);
  if ( !mCodec )
  {
    fprintf(stderr, "VideoDecoder::Load - avcodec_find_decoder failed\n");
    return false;
  }

  if ( avcodec_open(mCodecContext, mCodec) < 0 )
  {
    fprintf(stderr, "VideoDecoder::Load - avcodec_open failed\n");
    return false;
  }

  mFrame = avcodec_alloc_frame();
  if ( !mFrame )
  {
    fprintf(stderr, "VideoDecoder::Load - Failed allocating frame.\n");
    return false;
  }

  mFrameRGB = avcodec_alloc_frame();
  if ( !mFrameRGB )
  {
    fprintf(stderr, "VideoDecoder::Load - Failed allocating RGB frame.\n");
    return false;
  }

  /* Determine required buffer size and allocate buffer */
  numBytes = avpicture_get_size(PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);
  tmp = (uint8_t *)realloc(mBuffer, numBytes * sizeof(uint8_t));
  if ( !tmp )
  {
    fprintf(stderr, "VideoDecoder::Load - Failed allocating buffer.\n");
    return false;
  }
  mBuffer = tmp;

  avpicture_fill((AVPicture *)mFrameRGB, mBuffer, PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);

  mSwsContext = sws_getContext(mCodecContext->width, mCodecContext->height, mCodecContext->pix_fmt,
                               mCodecContext->width, mCodecContext->height, PIX_FMT_RGB24,
                               SWS_BICUBIC, NULL, NULL, NULL);
  if ( !mSwsContext )
  {
    fprintf(stderr, "VideoDecoder::Load - sws_getContext failed.\n");
    return false;
  }

  mDoneReading = false;

  return true;
}
