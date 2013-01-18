/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_CODEC_FACTORY_H_
#define TEST_CODEC_FACTORY_H_
#include "test/decode_test_driver.h"
#include "test/encode_test_driver.h"

namespace libvpx_test {

class CodecFactory {
 public:
  // In theory, these should be pure virtuals, but the template magic
  // in gtest doesn't want to play along.
  virtual Decoder* CreateDecoder(vpx_codec_dec_cfg_t cfg,
                                 unsigned long deadline) const = 0; //{return NULL;}

  virtual Encoder* CreateEncoder(vpx_codec_enc_cfg_t cfg,
                                 unsigned long deadline,
                                 const unsigned long init_flags,
                                 TwopassStatsStore *stats) const = 0; //{return NULL;}
  virtual ~CodecFactory() {}
};

class VP8Decoder : public Decoder {
 public:
  VP8Decoder(vpx_codec_dec_cfg_t cfg, unsigned long deadline) :
      Decoder(cfg, deadline) {}

 protected:
  virtual const vpx_codec_iface_t* CodecInterface() const {
#if CONFIG_VP8_DECODER
    return &vpx_codec_vp8_dx_algo;
#else
    return NULL;
#endif
  }
};

class VP8Encoder : public Encoder {
 public:
  VP8Encoder(vpx_codec_enc_cfg_t cfg, unsigned long deadline,
             const unsigned long init_flags, TwopassStatsStore *stats) :
      Encoder(cfg, deadline, init_flags, stats) {}

 protected:
  virtual const vpx_codec_iface_t* CodecInterface() const {
#if CONFIG_VP8_ENCODER
    return &vpx_codec_vp8_cx_algo;
#else
    return NULL;
#endif
  }
};

class VP8CodecFactory : public CodecFactory {
  virtual Decoder* CreateDecoder(vpx_codec_dec_cfg_t cfg,
                                 unsigned long deadline) const {
    return new VP8Decoder(cfg, deadline);
  }

  virtual Encoder* CreateEncoder(vpx_codec_enc_cfg_t cfg,
                                 unsigned long deadline,
                                 const unsigned long init_flags,
                                 TwopassStatsStore *stats) const {
    return new VP8Encoder(cfg, deadline, init_flags, stats);
  }
};

static const libvpx_test::VP8CodecFactory kVP8;

#define VP8_INSTANTIATE_TEST_CASE(test, params)\
  INSTANTIATE_TEST_CASE_P(VP8, test,\
    ::testing::Combine(::testing::Values(\
      static_cast<const libvpx_test::CodecFactory*>(&libvpx_test::kVP8)),\
                       params))

}  // namespace libvpx_test

#endif  // TEST_CODEC_FACTORY_H_