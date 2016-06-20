/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_SNAPSHOT_H_
#define TEST_SNAPSHOT_H_

#include <map>

#include "test/array.h"

namespace libvpx_test {

//
// Allows capturing and retrieving snapshots of arbitrary blobs of memory,
// blob size is based on compile time type information. Handles instances
// of 'Array' naturally.
//
// Example:
//   void example() {
//     Snapshot snapshot;
//
//     int foo = 4;
//
//     snapshot(foo);
//
//     foo = 10;
//
//     assert(snapshot.Get(foo) == 4);     // Pass
//     assert(snapshot.Get(foo) == foo);   // Fail (4 != 10)
//
//     char bar[10][10];
//     memset(bar, 3, sizeof(bar));
//
//     snapshot(bar);
//
//     memset(bar, 8, sizeof(bar));
//
//     assert(sum(bar) == 800);                 // Pass
//     assert(sum(snapshot.Get(bar)) == 300);   // Pass
//   }
//
class Snapshot {
 public:
  virtual ~Snapshot() {
    for (array_map_t::iterator it = array_snapshots_.begin();
         it != array_snapshots_.end(); ++it) {
      delete it->second;
    }

    for (blob_map_t::iterator it = blob_snapshots_.begin();
         it != blob_snapshots_.end(); ++it) {
      delete[] it->second;
    }
  }

  // Take new snapshot for object.
  template<typename E>
  void Take(const E &e) {
    const void *const key = reinterpret_cast<const void*>(&e);
    blob_map_t::iterator it = blob_snapshots_.find(key);

    if (it != blob_snapshots_.end())
      delete[] it->second;

    char *const snapshot = new char[sizeof(E)];
    memcpy(snapshot, &e, sizeof(E));
    blob_snapshots_[key] = snapshot;
  }

  // Take new snapshot of 'Array'
  template<typename T, int A>
  void Take(const Array<T, A> &a) {
    const void *const key = reinterpret_cast<const void*>(&a);
    array_map_t::iterator it = array_snapshots_.find(key);

    if (it != array_snapshots_.end())
      delete it->second;

    Array<T, A> *const snapshot = new Array<T, A>;
    memcpy(snapshot->AddrOf(), a.AddrOf(), a.SizeOf());
    array_snapshots_[key] = snapshot;
  }

  // Same as 'take'
  template<typename E>
  void operator()(const E &e) {
    Take(e);
  }

  // Retrieve last snapshot for object. Crashes if snapshot has not been
  // taken for this object before.
  template<typename E>
  const E& Get(const E &e) const {
    const void *const key = reinterpret_cast<const void*>(&e);
    blob_map_t::const_iterator it = blob_snapshots_.find(key);

    assert(it != blob_snapshots_.end());
    return *reinterpret_cast<const E*>(it->second);
  }

  // Retrieve last snaphot of 'Array'. Crashes if snapshot has not been
  // taken for this 'Array' before.
  template<typename T, int A>
  const Array<T, A>& Get(const Array<T, A> &a) const {
    const void *const key = reinterpret_cast<const void*>(&a);
    array_map_t::const_iterator it = array_snapshots_.find(key);

    assert(it != array_snapshots_.end());
    return *reinterpret_cast<const Array<T, A>*>(it->second);
  }

 private:
  typedef std::map<const void*, const ArrayBase*> array_map_t;
  typedef std::map<const void*, const char*> blob_map_t;

  array_map_t array_snapshots_;
  blob_map_t blob_snapshots_;
};

}   // namespace libvpx_test

#endif  // TEST_SNAPSHOT_H_
