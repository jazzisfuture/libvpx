/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_MEM_H_
#define TEST_MEM_H_

namespace libvpx_test {

template<typename T, int align>
T* AlignAddr(T* const base) {
  const uintptr_t base_addr = reinterpret_cast<uintptr_t>(base);
  return reinterpret_cast<T*>((base_addr + align - 1) & -align);
}

}  // namespace libvpx_test

#endif  // TEST_MEM_H_
