//
//  mulle-flushablebuffer.h
//  mulle-buffer
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_flushablebuffer_h__
#define mulle_flushablebuffer_h__


#include "mulle-buffer.h"


// TODO: if we also defined a mulle_flushablebuffer_sucker_t we could
//       also wedge fread (and possibly also fseek) into this datastructure.
//       Then we could wrap FILE into mulle_buffer and use this to
//       do file I/O. This would be nice for callbacks (like in mulle-utf)
//       maybe ? Other idea. Base mulle_buffer on fmemopen, if we can
//       ascertain thats available on all platforms ? but FILE is opaque
//       hmm..
//
// this is fairly conveniently, just like fwrite(  _storage, len, nElems, fp)
// though a non-buffering write could be better
//
// Like fwrite, the flusher is responsible for retrying partial writes
// internally: it must keep writing until all `len` bytes are delivered
// and then return `len`. A short return therefore signals a genuine
// delivery failure, not a merely interrupted write. The buffer treats a
// short return as fatal: it marks itself overflown and retains the
// undelivered bytes only for inspection. Do not retry a flusher that
// reported a short return, or the already-delivered prefix would be
// duplicated.
typedef size_t   mulle_flushablebuffer_flusher_t( void *buf,
                                                  size_t one,
                                                  size_t len,
                                                  void *userinfo);

// since we want to cast this to mulle_buffer eventually, the allocator
// must be in here (argh). It's not used by mulle--buffer code though
// and it's initialized to the default allocator
#define MULLE_FLUSHABLEBUFFER_BASE                 \
   MULLE_BUFFER_BASE;                              \
   mulle_flushablebuffer_flusher_t   *_flusher;    \
   void                              *_userinfo;   \
   size_t                            _flushed

/*
 * The flushable buffer is inflexible and occasionally
 * flushes out data to make room.
 * It's easy to stream data to stdout with a flushable
 * buffer.
 *
 * **NOT THREAD-SAFE**: All instances must be used from a single thread.
 */
struct mulle_flushablebuffer
{
   MULLE_FLUSHABLEBUFFER_BASE;
};


#define MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY  (sizeof( void *) * 32) // 256 bytes on 64 bit

// the min capacity is needed so a mulle_buffer can guarantee at least 8 bytes
#define MULLE_FLUSHABLEBUFFER_MIN_CAPACITY      (sizeof( double))

#define MULLE_FLUSHABLEBUFFER_TYPE  \
   (MULLE_BUFFER_IS_FLUSHABLE | MULLE_BUFFER_IS_WRITEONLY)

/**
 * Initializes a `mulle_flushablebuffer` struct with a static storage buffer.
 *
 * @param xstorage    The static storage buffer to use for the buffer.
 * @param xlength     The length of the static storage buffer.
 * @param xflusher    The flusher function to use for the buffer.
 * @param xuserinfo   The user-provided information to pass to the flusher function.
 *
 * @return A `mulle_flushablebuffer` struct initialized with the provided parameters.
 */
// MULLE_FLUSHABLEBUFFER_STATIC_DATA is a compound literal initializer. It
// exists for static/global data-section variables (and other constant-expression
// contexts), where a function call is not allowed. It evaluates `xstorage`,
// `xlength`, `xflusher` and `xuserinfo` more than once, so it accepts only
// side-effect-free expressions.
//
// Inside actual code, use the single-evaluation initializers
// `mulle_flushablebuffer_init_with_static_bytes` (on a stack struct) or
// `mulle_flushablebuffer_init`, which evaluate their arguments exactly once.
//
#define MULLE_FLUSHABLEBUFFER_STATIC_DATA( xstorage, xlength,              \
                                           xflusher, xuserinfo)            \
   ((struct mulle_flushablebuffer)                                         \
   {                                                                       \
      ._initial_storage = (unsigned char *) (xstorage),                    \
      ._curr            = (unsigned char *) (xstorage),                    \
      ._storage         = (unsigned char *) (xstorage),                    \
      ._sentinel        = (xstorage) ? &((unsigned char *)(xstorage))[ (xlength)] : NULL, \
      ._size            = (xlength),                                       \
      ._type            = MULLE_FLUSHABLEBUFFER_TYPE,                      \
      ._flusher         = (mulle_flushablebuffer_flusher_t *) (xflusher),  \
      ._userinfo        = (xuserinfo)                                      \
   })                                                                      \

/**
 * Initializes a `mulle_flushablebuffer` struct with an allocated storage buffer.
 *
 * @param xstorage    The allocated storage buffer to use for the buffer.
 * @param xlength     The length of the allocated storage buffer.
 * @param xflusher    The flusher function to use for the buffer.
 * @param xuserinfo   The user-provided information to pass to the flusher function.
 * @param xallocator  The allocator to use for the buffer.
 *
 * @return A `mulle_flushablebuffer` struct initialized with the provided parameters.
 */
// Same caveat as `MULLE_FLUSHABLEBUFFER_STATIC_DATA`: for static data-section
// initialization only; use `mulle_flushablebuffer_init_with_allocated_bytes`
// in code.
#define MULLE_FLUSHABLEBUFFER_ALLOCATED_DATA( xstorage, xlength, xflusher, \
                                              xuserinfo, xallocator)       \
   ((struct mulle_flushablebuffer)                                         \
   {                                                                       \
      ._curr            = (unsigned char *) (xstorage),                    \
      ._storage         = (unsigned char *) (xstorage),                    \
      ._sentinel        = (xstorage) ? &((unsigned char *)(xstorage))[ (xlength)] : NULL, \
      ._size            = (xlength),                                       \
      ._type            = MULLE_FLUSHABLEBUFFER_TYPE,                      \
      ._flusher         = (mulle_flushablebuffer_flusher_t *) (xflusher),  \
      ._userinfo        = (xuserinfo),                                     \
      ._allocator       = (xallocator)                                     \
   })


/**
 * Single-evaluation variant of `MULLE_FLUSHABLEBUFFER_STATIC_DATA` for use in
 * code (for example by the `mulle_flushablebuffer_do_FILE_*` macros), where a
 * struct-returning expression, not a function call on a pre-declared struct, is
 * convenient. `storage`, `length`, `flusher` and `userinfo` are evaluated
 * exactly once, so expressions with side effects are safe.
 *
 * @param storage   The static storage buffer to initialize with.
 * @param length    The length of the static storage buffer.
 * @param flusher   The flusher function to use.
 * @param userinfo  The user information to pass to the flusher function.
 * @return A `mulle_flushablebuffer` struct initialized with the parameters.
 */
static inline struct mulle_flushablebuffer
   _mulle_flushablebuffer_static_data( void *storage,
                                       size_t length,
                                       mulle_flushablebuffer_flusher_t *flusher,
                                       void *userinfo)
{
   return( MULLE_FLUSHABLEBUFFER_STATIC_DATA( storage, length, flusher, userinfo));
}


/**
 * Single-evaluation variant of `MULLE_FLUSHABLEBUFFER_ALLOCATED_DATA` for use
 * in code. `storage`, `length`, `flusher`, `userinfo` and `allocator` are
 * evaluated exactly once, so expressions with side effects are safe.
 *
 * @param storage   The allocated storage buffer to initialize with.
 * @param length    The length of the allocated storage buffer.
 * @param flusher   The flusher function to use.
 * @param userinfo  The user information to pass to the flusher function.
 * @param allocator The allocator to use for the storage.
 * @return A `mulle_flushablebuffer` struct initialized with the parameters.
 */
static inline struct mulle_flushablebuffer
   _mulle_flushablebuffer_allocated_data( void *storage,
                                          size_t length,
                                          mulle_flushablebuffer_flusher_t *flusher,
                                          void *userinfo,
                                          struct mulle_allocator *allocator)
{
   return( MULLE_FLUSHABLEBUFFER_ALLOCATED_DATA( storage, length, flusher, userinfo, allocator));
}


/**
 * Returns the `mulle_buffer` representation of a `mulle_flushablebuffer`.
 *
 * @param buffer The `mulle_flushablebuffer` to convert.
 * @return The `mulle_buffer` representation of the provided `mulle_flushablebuffer`.
 */
static inline struct mulle_buffer   *
   mulle_flushablebuffer_as_buffer( struct mulle_flushablebuffer *buffer)
{
  return( (struct mulle_buffer *) buffer);
}



/**
 * Initializes a `mulle_flushablebuffer` struct with a static storage buffer.
 * For technical reasons the storage must be at least
 * @param buffer     The `mulle_flushablebuffer` struct to initialize.
 * @param storage    The static storage buffer to use for the buffer.
 * @param length     The length of the static storage buffer.
 * @param flusher    The flusher function to use for the buffer.
 * @param userinfo   The user-provided information to pass to the flusher function.
 * @param allocator  The allocator to use for the buffer.
 */
// the storage is
static inline void
   _mulle_flushablebuffer_init_with_static_bytes( struct mulle_flushablebuffer *buffer,
                                                  void *storage,
                                                  size_t length,
                                                  mulle_flushablebuffer_flusher_t *flusher,
                                                  void *userinfo,
                                                  struct mulle_allocator *allocator)
{
   assert( storage && length && flusher);
   assert( length >= MULLE_FLUSHABLEBUFFER_MIN_CAPACITY);

   memset( buffer, 0, sizeof( *buffer));

   buffer->_initial_storage =
   buffer->_curr            =
   buffer->_storage         = storage;
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_FLUSHABLEBUFFER_TYPE;

   buffer->_flusher         = flusher;
   buffer->_userinfo        = userinfo;
   buffer->_allocator       = allocator;
}


/**
 * Initializes a `mulle_flushablebuffer` struct with an allocated storage buffer.
 *
 * @param buffer     The `mulle_flushablebuffer` struct to initialize.
 * @param storage    The allocated storage buffer to use for the buffer.
 * @param length     The length of the allocated storage buffer.
 * @param flusher    The flusher function to use for the buffer.
 * @param userinfo   The user-provided information to pass to the flusher function.
 * @param allocator  The allocator to use for the buffer.
 */
static inline void
   _mulle_flushablebuffer_init_with_allocated_bytes( struct mulle_flushablebuffer *buffer,
                                                     void *storage,
                                                     size_t length,
                                                     mulle_flushablebuffer_flusher_t *flusher,
                                                     void *userinfo,
                                                     struct mulle_allocator *allocator)
{
   assert( storage && length && flusher);
   assert( length >= MULLE_FLUSHABLEBUFFER_MIN_CAPACITY);

   memset( buffer, 0, sizeof( *buffer));

   buffer->_curr            =
   buffer->_storage         = storage;
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_FLUSHABLEBUFFER_TYPE;

   buffer->_flusher         = flusher;
   buffer->_userinfo        = userinfo;
   buffer->_allocator       = allocator;
}


/**
 * Initializes a `mulle_flushablebuffer` struct with a static storage buffer.
 *
 * This function is provided for backwards compatibility. It is recommended to use
 * `mulle_flushablebuffer_init_with_static_bytes` instead, which provides more
 * explicit parameter names.
 *
 * @param buffer     The `mulle_flushablebuffer` struct to initialize.
 * @param storage    The static storage buffer to use for the buffer.
 * @param length     The length of the static storage buffer.
 * @param flusher    The flusher function to use for the buffer.
 * @param userinfo   The user-provided information to pass to the flusher function.
 */
// backwards compatibility
MULLE_C_NONNULL_SECOND_FOURTH
static inline void
mulle_flushablebuffer_init( struct mulle_flushablebuffer *buffer,
                               void *storage,
                               size_t length,
                               mulle_flushablebuffer_flusher_t *flusher,
                               void *userinfo)
{
   if( ! buffer)
      return;

   _mulle_flushablebuffer_init_with_static_bytes( buffer,
                                                   storage,
                                                   length,
                                                   (mulle_flushablebuffer_flusher_t *) flusher,
                                                   userinfo,
                                                   &mulle_default_allocator);
}


/**
 * Initializes a `mulle_flushablebuffer` struct with a static storage buffer.
 *
 * This function is provided for backwards compatibility. It is recommended to use
 * `mulle_flushablebuffer_init_with_allocated_bytes` instead, which provides more
 * explicit parameter names.
 *
 * @param buffer     The `mulle_flushablebuffer` struct to initialize.
 * @param storage    The static storage buffer to use for the buffer.
 * @param length     The length of the static storage buffer.
 * @param flusher    The flusher function to use for the buffer.
 * @param userinfo   The user-provided information to pass to the flusher function.
 * @param allocator  The allocator to use for the buffer (can be NULL).
 */
MULLE_C_NONNULL_SECOND_FOURTH
static inline void
   mulle_flushablebuffer_init_with_static_bytes( struct mulle_flushablebuffer *buffer,
                                                 void *storage,
                                                 size_t length,
                                                 mulle_flushablebuffer_flusher_t flusher,
                                                 void *userinfo,
                                                 struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   _mulle_flushablebuffer_init_with_static_bytes( buffer,
                                                  storage,
                                                  length,
                                                  (mulle_flushablebuffer_flusher_t *) flusher,
                                                  userinfo,
                                                  allocator);
}


/**
 * Initializes a `mulle_flushablebuffer` struct with an allocated storage buffer.
 *
 * This function is used to initialize a `mulle_flushablebuffer` struct with a
 * dynamically allocated storage buffer. The buffer will be allocated using the
 * provided `mulle_allocator` instance.
 *
 * @param buffer     The `mulle_flushablebuffer` struct to initialize.
 * @param storage    The allocated storage buffer to use for the buffer.
 * @param length     The length of the allocated storage buffer.
 * @param flusher    The flusher function to use for the buffer.
 * @param userinfo   The user-provided information to pass to the flusher function.
 * @param allocator  The allocator to use for the buffer.
 */
MULLE_C_NONNULL_SECOND_FOURTH
static inline void
   mulle_flushablebuffer_init_with_allocated_bytes( struct mulle_flushablebuffer *buffer,
                                                    void *storage,
                                                    size_t length,
                                                    mulle_flushablebuffer_flusher_t flusher,
                                                    void *userinfo,
                                                    struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   _mulle_flushablebuffer_init_with_allocated_bytes( buffer,
                                                     storage,
                                                     length,
                                                     (mulle_flushablebuffer_flusher_t *) flusher,
                                                     userinfo,
                                                     allocator);
}


MULLE__BUFFER_GLOBAL
MULLE_C_NONNULL_FIRST
int  _mulle_flushablebuffer_flush( struct mulle_flushablebuffer *ibuffer);


static inline int
   mulle_flushablebuffer_flush( struct mulle_flushablebuffer *buffer)
{
   if( ! buffer)
      return( -1);
   return( _mulle_flushablebuffer_flush( buffer));
}

// if != 0, the flush didn't succeed and the buffer is still alive!
MULLE__BUFFER_GLOBAL
MULLE_C_NONNULL_FIRST
int   _mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer);


/**
 * Finalizes and destroys a `mulle_flushablebuffer` instance.
 *
 * This function will flush any remaining data in the buffer and then destroy the
 * buffer instance. If the flush operation fails, the function will return a
 * non-zero value to indicate that the buffer is still alive and valid.
 *
 * @param buffer The `mulle_flushablebuffer` instance to finalize and destroy.
 * @return 0 if the buffer was successfully destroyed, non-zero if the flush
 *         operation failed and the buffer is still alive.
 */
static inline int
   mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer)
{
   if( ! buffer)
      return( 0);
   return( _mulle_flushablebuffer_done( buffer));
}


/**
 * Creates a new `mulle_flushablebuffer` instance.
 *
 * This function creates a new `mulle_flushablebuffer` instance with the specified
 * length, flusher function, user information, and memory allocator. The created
 * buffer can be used to efficiently write data and automatically flush it to the
 * provided flusher function.
 *
 * @param length The initial length of the buffer.
 * @param flusher The function to use for flushing the buffer contents.
 * @param userinfo An opaque pointer that will be passed to the flusher function.
 * @param allocator The memory allocator to use for the buffer.
 * @return A new `mulle_flushablebuffer` instance, or `NULL` if the creation failed.
 */
MULLE__BUFFER_GLOBAL
struct mulle_flushablebuffer   *
   mulle_flushablebuffer_create( size_t length,
                                 mulle_flushablebuffer_flusher_t *flusher,
                                 void *userinfo,
                                 struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
int   mulle_flushablebuffer_destroy( struct mulle_flushablebuffer *buffer);

/**
 * Releases a `mulle_flushablebuffer` created by `mulle_flushablebuffer_create`.
 *
 * This function flushes any remaining data and frees both the storage and the
 * heap object. If the flush fails, the buffer and its retained undelivered
 * bytes are still discarded; call `mulle_flushablebuffer_flush` manually
 * first if the remaining data must be delivered before the buffer is freed.
 *
 * @param buffer The `mulle_flushablebuffer` instance to destroy.
 * @return 0 on success, or the flush error if the final flush failed.
 */

#endif
