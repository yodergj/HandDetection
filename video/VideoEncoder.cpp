#include <stdio.h>
#include <errno.h>

#include "VideoEncoder.h"
#include "Image.h"

VideoEncoder::VideoEncoder()
{
  mWidth = 0;
  mHeight = 0;
  mFPS = 0;
  mNumFrames = 0;

#ifdef USE_VPX
  mVPXOutFile = 0;
#endif

#ifdef USE_FFMPEG
  mFormatContext = NULL;
  mCodecContext = NULL;
  mCodec = NULL;
  mFrame = NULL;
  mFrameRGB = NULL;
  mPacket.data = NULL;
  mSwsContext = NULL;
  mVideoStream = -1;
  mBuffer = NULL;
  mFilename = "";

  mCodecID = AV_CODEC_ID_H264;
  mOutFile = 0;
#endif

  mVPXProcessing = false;
}

VideoEncoder::~VideoEncoder()
{
#ifdef USE_FFMPEG
  if ( mFrame )
    av_free(mFrame);
  if ( mFrameRGB )
    av_free(mFrameRGB);
  if ( mCodecContext )
    avcodec_close(mCodecContext);
  if ( mFormatContext )
    avformat_close_input(&mFormatContext);
  if ( mBuffer )
    free(mBuffer);
#endif
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

  if ( extension == "webm" || extension == "vpx" )
    mVPXProcessing = true;

  if ( mVPXProcessing )
  {
#ifdef USE_VPX
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

    if ( !vpx_img_alloc(&mVPXRawImage, VPX_IMG_FMT_I420, mWidth, mHeight, 1) )
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
#else
    retVal = false;
#endif
  }
  else
  {
#ifdef USE_FFMPEG
#if 0
    mCodecID = AV_CODEC_ID_H264;
#else
    mCodecID = AV_CODEC_ID_MPEG1VIDEO;
#endif
    mCodec = avcodec_find_encoder(mCodecID);
    if ( !mCodec )
    {
      fprintf(stderr, "VideoEncoder::Open - codec not found\n");
      return false;
    }

    mCodecContext = avcodec_alloc_context3(mCodec);
    if ( !mCodecContext )
    {
      fprintf(stderr, "VideoEncoder::Open - avcodec_alloc_context3 failed\n");
      return false;
    }

    mCodecContext->bit_rate = 400000;
    mCodecContext->width = width;
    mCodecContext->height = height;
    mCodecContext->time_base.num = 1;
    mCodecContext->time_base.den = fps;
    mCodecContext->gop_size = 10;
    mCodecContext->max_b_frames = 1;
    mCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (mCodecID == AV_CODEC_ID_H264)
      av_opt_set(mCodecContext->priv_data, "preset", "slow", 0);

    if ( avcodec_open2(mCodecContext, mCodec, NULL) < 0 )
    {
      fprintf(stderr, "VideoEncoder::Open - avcodec_open2 failed\n");
      return false;
    }

    mOutFile = fopen(filename, "wb");
    if ( !mOutFile )
    {
      fprintf(stderr, "VideoEncoder::Open - Failed opening %s\n", filename);
      return false;
    }

    mFrame = av_frame_alloc();
    if ( !mFrame )
    {
      fprintf(stderr, "VideoEncoder::Open - av_frame_alloc failed\n");
      return false;
    }

    mFrame->format = mCodecContext->pix_fmt;
    mFrame->width = width;
    mFrame->height = height;

    if ( av_image_alloc(mFrame->data, mFrame->linesize, width, height, mCodecContext->pix_fmt, 32) < 0 )
    {
      fprintf(stderr, "VideoEncoder::Open - av_image_alloc failed\n");
      exit(1);
    }
#else
    retVal = false;
#endif
  }

  return retVal;
}

bool VideoEncoder::Close()
{
  bool retVal = true;

  if ( mVPXProcessing )
  {
#ifdef USE_VPX
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
#else
    retVal = false;
#endif
    mVPXProcessing = false;
  }
  else
  {
#ifdef USE_FFMPEG
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    int delayedIndex = 1;
    int gotOutput = 1;
    while ( gotOutput )
    {
      fflush(stdout);
      if ( avcodec_encode_video2(mCodecContext, &mPacket, NULL, &gotOutput) < 0 )
      {
        fprintf(stderr, "VideoEncoder::Close - avcodec_encode_video2 failed\n");
        exit(1);
      }
      if (gotOutput)
      {
        printf("Write frame %3d (size=%5d)\n", delayedIndex++, mPacket.size);
        fwrite(mPacket.data, 1, mPacket.size, mOutFile);
        av_free_packet(&mPacket);
      }
    }
    fwrite(endcode, 1, sizeof(endcode), mOutFile);
    fclose(mOutFile);
    mOutFile = 0;

    avcodec_close(mCodecContext);
    av_free(mCodecContext);
    mCodecContext = 0;

    av_freep(&mFrame->data[0]);
    av_frame_free(&mFrame);
    mFrame = 0;
#else
    retVal = false;
#endif
  }

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

  if ( mVPXProcessing )
  {
#ifdef USE_VPX
    unsigned char* data = image->GetI420Buffer();
    if ( !data )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - Failed getting image buffer\n");
      return false;
    }
    memcpy(mVPXRawImage.planes[0], data, mWidth * mHeight * 3 / 2);
    retVal = WriteVPXFrameData(false);
#else
    retVal = false;
#endif
  }
  else
  {
#ifdef USE_FFMPEG
    av_init_packet(&mPacket);
    mPacket.data = NULL;    // packet data will be allocated by the encoder
    mPacket.size = 0;
    fflush(stdout);

    unsigned char* srcBuffer = image->GetI420Buffer();
    if ( !srcBuffer )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - Failed getting YUV buffer\n");
      return false;
    }

    int x, y;
    for (y = 0; y < mHeight; y++)
      for (x = 0; x < mWidth; x++)
        mFrame->data[0][y * mFrame->linesize[0] + x] = srcBuffer[y * mWidth + x];

    int halfWidth = mWidth / 2;
    int halfHeight = mHeight / 2;
    unsigned char* uBuffer = srcBuffer + mWidth * mHeight;
    unsigned char* vBuffer = uBuffer + halfWidth * halfHeight;

    for (y = 0; y < halfHeight; y++)
      for (x = 0; x < halfWidth; x++)
      {
        mFrame->data[1][y * mFrame->linesize[1] + x] = uBuffer[y * halfWidth + x];
        mFrame->data[2][y * mFrame->linesize[2] + x] = vBuffer[y * halfWidth + x];
      }
#if 0
    int x, y;
    /* prepare a dummy image */
    /* Y */
    for (y = 0; y < mCodecContext->height; y++)
    {
      for (x = 0; x < mCodecContext->width; x++)
      {
        mFrame->data[0][y * mFrame->linesize[0] + x] = x + y + (mNumFrames % 25) * 3;
      }
    }
    /* Cb and Cr */
    for (y = 0; y < mCodecContext->height/2; y++)
    {
      for (x = 0; x < mCodecContext->width/2; x++)
      {
        mFrame->data[1][y * mFrame->linesize[1] + x] = 128 + y + (mNumFrames % 25) * 2;
        mFrame->data[2][y * mFrame->linesize[2] + x] = 64 + x + (mNumFrames % 25) * 5;
      }
    }
#endif
    mFrame->pts = mNumFrames;

    int gotOutput;
    if ( avcodec_encode_video2(mCodecContext, &mPacket, mFrame, &gotOutput) < 0 )
    {
      fprintf(stderr, "VideoEncoder::AddFrame - avcodec_encode_video2 failed\n");
      return false;
    }

    if (gotOutput)
    {
      printf("Write frame %3d (size=%5d)\n", mNumFrames, mPacket.size);
      fwrite(mPacket.data, 1, mPacket.size, mOutFile);
      av_free_packet(&mPacket);
    }
#else
    retVal = false;
#endif
  }

  if ( retVal )
    mNumFrames++;

  return retVal;
}

static void video_encode_example(const char *filename, enum AVCodecID codec_id)
{
  AVCodec *codec;
  AVCodecContext *c= NULL;
  int i, ret, x, y, got_output;
  FILE *f;
  AVFrame *frame;
  AVPacket pkt;
  uint8_t endcode[] = { 0, 0, 1, 0xb7 };
  printf("Encode video file %s\n", filename);
  /* find the mpeg1 video encoder */
  codec = avcodec_find_encoder(codec_id);
  if (!codec)
  {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }
  c = avcodec_alloc_context3(codec);
  if (!c)
  {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }
  /* put sample parameters */
  c->bit_rate = 400000;
  /* resolution must be a multiple of two */
  c->width = 352;
  c->height = 288;
  /* frames per second */
  c->time_base.num = 1;
  c->time_base.den = 25;
  c->gop_size = 10; /* emit one intra frame every ten frames */
  c->max_b_frames = 1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;
  if (codec_id == AV_CODEC_ID_H264)
    av_opt_set(c->priv_data, "preset", "slow", 0);
  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0)
  {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }
  f = fopen(filename, "wb");
  if (!f)
  {
    fprintf(stderr, "Could not open %s\n", filename);
    exit(1);
  }
  frame = av_frame_alloc();
  if (!frame)
  {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }
  frame->format = c->pix_fmt;
  frame->width  = c->width;
  frame->height = c->height;
  /* the image can be allocated by any means and av_image_alloc() is
  * just the most convenient way if av_malloc() is to be used */
  ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
  if (ret < 0)
  {
    fprintf(stderr, "Could not allocate raw picture buffer\n");
    exit(1);
  }
  /* encode 1 second of video */
  for (i = 0; i < 25; i++)
  {
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    fflush(stdout);
    /* prepare a dummy image */
    /* Y */
    for (y = 0; y < c->height; y++)
    {
      for (x = 0; x < c->width; x++)
      {
        frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
      }
    }
    /* Cb and Cr */
    for (y = 0; y < c->height/2; y++)
    {
      for (x = 0; x < c->width/2; x++)
      {
        frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
        frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
      }
    }
    frame->pts = i;
    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0)
    {
      fprintf(stderr, "Error encoding frame\n");
      exit(1);
    }
    if (got_output)
    {
      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_free_packet(&pkt);
    }
  }
  /* get the delayed frames */
  for (got_output = 1; got_output; i++)
  {
    fflush(stdout);
    ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
    if (ret < 0)
    {
      fprintf(stderr, "Error encoding frame\n");
      exit(1);
    }
    if (got_output)
    {
      printf("Write frame %3d (size=%5d)\n", i, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_free_packet(&pkt);
    }
  }
  /* add sequence end code to have a real mpeg file */
  fwrite(endcode, 1, sizeof(endcode), f);
  fclose(f);
  avcodec_close(c);
  av_free(c);
  av_freep(&frame->data[0]);
  av_frame_free(&frame);
  printf("\n");
}

#ifdef USE_VPX
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
#endif
