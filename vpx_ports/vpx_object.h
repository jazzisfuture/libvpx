/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef VPX_OBJECT_H_
#define VPX_OBJECT_H_

struct vpx_arena;

/* \brief Allocates an pool of reference counted opaque objects
 *
 * \param[in]  n      Number of objects in the pool
 * \param[in]  size   Size of each object
 * \param[in]  align  Minimum alignment of each object
 *
 * \return a pointer to the arena, or NULL if one could not e allocated
 */
struct vpx_arena *vpx_object_arena_alloc(size_t n, size_t size, size_t align);

/* \brief Frees a previously allocated arena
 *
 * All outstanding references to objects in this arena will be invalidated.
 *
 * \param[in]  arena  Pointer to the arena to free
 */
void vpx_object_arena_free(struct vpx_arena *arena);

/* \brief Acquires an unreferenced object
 *
 * Returns a reference to a free object from the arena. Every call to
 * vpx_object_acquire_new() must be matched with a call to vpx_object_release().
 *
 * \param[in]  arena  Arena to acquire an object from
 *
 * \return A pointer to the new object
 */
void *vpx_object_acquire_new(struct vpx_arena *arena);

/* \brief Atomically acquires an additional reference to a referenced object.
 *
 * Increments the reference count on the object. Every call to
 * vpx_object_acquire() must be matched with a call to vpx_object_release().
 *
 * \param[in]  object  Object to acquire a reference to
 *
 * \return returns its input
 */
void *vpx_object_acquire(void *object);

/* \brief Atomically releases a held reference
 *
 * Decrements the reference count on the object.
 *
 * \param[in]  object  Object to release
 */
void vpx_object_release(void *object);

/* \brief Exchanges a reference to one object for another.
 *
 * Increments the reference count on the new_object and decrements it on
 * the old_object.
 *
 * \param[in]  new_object  Object to acquire
 * \param[in]  old_object  Object to release
 *
 * \return returns the new_object
 */
void *vpx_object_exchange(void *new_object, void *old_object);

#endif