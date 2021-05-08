#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif
#include "DynamicDll.h"
#include "DllAvUtil.h"
#include "utils/log.h"

extern "C" {
#ifndef HAVE_MMX
#define HAVE_MMX
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __GNUC__
#pragma warning(disable:4244)
#endif

#if (defined USE_EXTERNAL_FFMPEG)
  #if (defined HAVE_LIBAVCODEC_AVCODEC_H)
    #include <libavcodec/avcodec.h>
    #if (defined HAVE_LIBAVCODEC_OPT_H)
      #include <libavcodec/opt.h>
    #endif
    #if (defined AVPACKET_IN_AVFORMAT)
      #include <libavformat/avformat.h>
    #endif
  #elif (defined HAVE_FFMPEG_AVCODEC_H)
    #include <ffmpeg/avcodec.h>
    #include <ffmpeg/opt.h>
    #if (defined AVPACKET_IN_AVFORMAT)
      #include <ffmpeg/avformat.h>
    #endif
  #endif
#else
  #include "libavcodec/avcodec.h"
#endif
}

//#include "threads/SingleLock.h"

class DllAvCodecInterface
{
public:
  virtual ~DllAvCodecInterface() {}
  virtual void avcodec_flush_buffers(AVCodecContext *avctx)=0;
  virtual int avcodec_open2_dont_call(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options)=0;
  virtual const AVCodec *avcodec_find_decoder(enum AVCodecID id)=0;
  virtual int avcodec_close_dont_call(AVCodecContext *avctx)=0;
  virtual AVFrame *av_frame_alloc(void)=0;
  virtual int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)=0;
  virtual int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt)=0;
  virtual AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)=0;
  virtual void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode)=0;
  virtual AVCodecParserContext *av_parser_init(int codec_id)=0;
  virtual int av_parser_parse2(AVCodecParserContext *s,AVCodecContext *avctx, uint8_t **poutbuf, int *poutbuf_size,
                    const uint8_t *buf, int buf_size,
                    int64_t pts, int64_t dts, int64_t pos)=0;
  virtual void av_parser_close(AVCodecParserContext *s)=0;
  virtual void av_packet_unref(AVPacket *pkt)=0;
  virtual enum AVPixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum AVPixelFormat *fmt)=0;
  virtual int avcodec_default_get_buffer2(AVCodecContext *s, AVFrame *pic, int flags)=0;
  virtual void av_init_packet(AVPacket *pkt)=0;
};

#if (defined USE_EXTERNAL_FFMPEG) || (defined TARGET_DARWIN)

// Use direct layer
class DllAvCodec : public DllDynamic, DllAvCodecInterface
{
public:
  virtual ~DllAvCodec() {}
  virtual void avcodec_flush_buffers(AVCodecContext *avctx) { ::avcodec_flush_buffers(avctx); }
  virtual int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
  {
    return ::avcodec_open2(avctx, codec, options);
  }
  virtual int avcodec_open2_dont_call(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options) { *(int *)0x0 = 0; return 0; }
  virtual int avcodec_close_dont_call(AVCodecContext *avctx) { *(int *)0x0 = 0; return 0; }
  virtual const AVCodec *avcodec_find_decoder(enum AVCodecID id) { return ::avcodec_find_decoder(id); }
  virtual int avcodec_close(AVCodecContext *avctx)
  {
    return ::avcodec_close(avctx);
  }
  virtual AVFrame *av_frame_alloc() { return ::av_frame_alloc(); }

  virtual int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt) {
    int ret;
    *got_frame = 0;

    if (pkt) {
      ret = avcodec_send_packet(avctx, pkt);
      // In particular, we don't expect AVERROR(EAGAIN), because we read all
      // decoded frames with avcodec_receive_frame() until done.
      if (ret < 0 && ret != AVERROR_EOF) {
        return ret;
      }
    }

    ret = avcodec_receive_frame(avctx, frame);

    if (ret < 0 && ret != AVERROR(EAGAIN)) {
      return ret;
    }

    if (ret >= 0) {
      *got_frame = 1;
    }

    return 0;
  }

  virtual int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt) { return ::avcodec_decode_subtitle2(avctx, sub, got_sub_ptr, avpkt); }
  virtual AVCodecContext *avcodec_alloc_context3(const AVCodec *codec) { return ::avcodec_alloc_context3(codec); }
  virtual void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode) { ::avcodec_string(buf, buf_size, enc, encode); }

  virtual AVCodecParserContext *av_parser_init(int codec_id) { return ::av_parser_init(codec_id); }
  virtual int av_parser_parse2(AVCodecParserContext *s,AVCodecContext *avctx, uint8_t **poutbuf, int *poutbuf_size,
                    const uint8_t *buf, int buf_size,
                    int64_t pts, int64_t dts, int64_t pos)
  {
    return ::av_parser_parse2(s, avctx, poutbuf, poutbuf_size, buf, buf_size, pts, dts, pos);
  }
  virtual void av_parser_close(AVCodecParserContext *s) { ::av_parser_close(s); }

  virtual void av_packet_unref(AVPacket *pkt) { ::av_packet_unref(pkt); }
  virtual int avcodec_default_get_buffer2(AVCodecContext *s, AVFrame *pic, int flags) { return ::avcodec_default_get_buffer2(s, pic, flags); }
  virtual enum AVPixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum AVPixelFormat *fmt) { return ::avcodec_default_get_format(s, fmt); }

  virtual void av_init_packet(AVPacket *pkt) { return ::av_init_packet(pkt); }

  // DLL faking.
  virtual bool ResolveExports() { return true; }
  virtual bool Load() {
    CLog::Log(LOGDEBUG, "DllAvCodec: Using libavcodec system library");
    return true;
  }
  virtual void Unload() {}
};
#else
class DllAvCodec : public DllDynamic, DllAvCodecInterface
{
  DECLARE_DLL_WRAPPER(DllAvCodec, DLL_PATH_LIBAVCODEC)
  DEFINE_FUNC_ALIGNED1(void, __cdecl, avcodec_flush_buffers, AVCodecContext*)
  DEFINE_FUNC_ALIGNED3(int, __cdecl, avcodec_open2_dont_call, AVCodecContext*, AVCodec *, AVDictionary **)
  DEFINE_FUNC_ALIGNED2(int, __cdecl, avcodec_send_packet, AVCodecContext*, AVPacket*)
  DEFINE_FUNC_ALIGNED2(int, __cdecl, avcodec_receive_frame, AVCodecContext*, AVFrame*)
  DEFINE_FUNC_ALIGNED4(int, __cdecl, avcodec_decode_subtitle2, AVCodecContext*, AVSubtitle*, int*, AVPacket*)
  DEFINE_FUNC_ALIGNED1(AVCodecContext*, __cdecl, avcodec_alloc_context3, AVCodec *)
  DEFINE_FUNC_ALIGNED1(AVCodecParserContext*, __cdecl, av_parser_init, int)
  DEFINE_FUNC_ALIGNED9(int, __cdecl, av_parser_parse2, AVCodecParserContext*,AVCodecContext*, uint8_t**, int*, const uint8_t*, int, int64_t, int64_t, int64_t)
  DEFINE_METHOD1(void, av_init_packet, (AVPacket *p1))

  LOAD_SYMBOLS();

  DEFINE_METHOD1(AVCodec*, avcodec_find_decoder, (enum AVCodecID p1))
  DEFINE_METHOD1(int, avcodec_close_dont_call, (AVCodecContext *p1))
  DEFINE_METHOD0(AVFrame*, av_frame_alloc)
  DEFINE_METHOD4(void, avcodec_string, (char *p1, int p2, AVCodecContext *p3, int p4))
  DEFINE_METHOD1(void, av_parser_close, (AVCodecParserContext *p1))
  DEFINE_METHOD1(void, av_packet_unref, (AVPacket *p1))
  DEFINE_METHOD2(int, avcodec_default_get_buffer2, (AVCodecContext *p1, AVFrame *p2, int flags))
  DEFINE_METHOD2(enum AVPixelFormat, avcodec_default_get_format, (struct AVCodecContext *p1, const enum AVPixelFormat *p2))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(avcodec_flush_buffers)
    RESOLVE_METHOD_RENAME(avcodec_open2,avcodec_open2_dont_call)
    RESOLVE_METHOD_RENAME(avcodec_close,avcodec_close_dont_call)
    RESOLVE_METHOD(avcodec_find_decoder)
    RESOLVE_METHOD(av_frame_alloc)
    RESOLVE_METHOD(avcodec_decode_subtitle2)
    RESOLVE_METHOD(avcodec_alloc_context3)
    RESOLVE_METHOD(avcodec_string)
    RESOLVE_METHOD(av_parser_init)
    RESOLVE_METHOD(av_parser_parse2)
    RESOLVE_METHOD(av_parser_close)
    RESOLVE_METHOD(av_packet_unref)
    RESOLVE_METHOD(avcodec_default_get_buffer2)
    RESOLVE_METHOD(avcodec_default_get_format)
    RESOLVE_METHOD(av_init_packet)
  END_METHOD_RESOLVE()

  /* dependencies of libavcodec */
  DllAvUtil m_dllAvUtil;
  // DllAvUtil loaded implicitely by m_dllAvCore

public:
    int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
    {
      return avcodec_open2_dont_call(avctx,codec,options);
    }
    int avcodec_close(AVCodecContext *avctx)
    {
      return avcodec_close_dont_call(avctx);
    }
    virtual bool Load()
    {
      if (!m_dllAvUtil.Load())
	return false;
      return DllDynamic::Load();
    }
};

#endif
