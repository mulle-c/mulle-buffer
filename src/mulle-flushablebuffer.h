//
//  mulle-flushablebuffer.c
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


// TODO: if we also defined a mulle_flushablebuffer_sucket_t we could
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
 */
struct mulle_flushablebuffer
{
   MULLE_FLUSHABLEBUFFER_BASE;
};


#define MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY  256

#define MULLE_FLUSHABLEBUFFER_TYPE  \
   (MULLE_BUFFER_IS_INFLEXIBLE | MULLE_BUFFER_IS_FLUSHABLE | MULLE_BUFFER_IS_WRITEONLY)

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
#define MULLE_FLUSHABLEBUFFER_STATIC_DATA( xstorage, xlength,              \
                                           xflusher, xuserinfo)            \
   ((struct mulle_flushablebuffer)                                         \
   {                                                                       \
      ._initial_storage = (unsigned char *) (xstorage),                    \
      ._curr            = (unsigned char *) (xstorage),                    \
      ._storage         = (unsigned char *) (xstorage),                    \
      ._sentinel        = &((unsigned char *)(xstorage))[ (xlength)],      \
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
#define MULLE_FLUSHABLEBUFFER_ALLOCATED_DATA( xstorage, xlength, xflusher, \
                                              xuserinfo, xallocator)       \
   ((struct mulle_flushablebuffer)                                         \
   {                                                                       \
      ._curr            = (unsigned char *) (xstorage),                    \
      ._storage         = (unsigned char *) (xstorage),                    \
      ._sentinel        = &((unsigned char *)(xstorage))[ (xlength)],      \
      ._size            = (xlength),                                       \
      ._type            = MULLE_FLUSHABLEBUFFER_TYPE,                      \
      ._flusher         = (mulle_flushablebuffer_flusher_t *) (xflusher),  \
      ._userinfo        = (xuserinfo),                                     \
      ._allocator       = (xallocator)                                     \
   })


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
 *
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
                                                  NULL);
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
 * Defines a macro that creates a `mulle_flushablebuffer` instance and a loop to use it.
 *
 * This macro creates a static `mulle_flushablebuffer` instance with a 128-byte internal
 * buffer, and a `fwrite` flusher function that writes to the provided `FILE*`. It then
 * defines a loop that uses this buffer, flushing it when the loop completes.
 *
 * @param name The name to use for the `mulle_flushablebuffer` instance and loop variables.
 * @param fp The `FILE*` to write the buffer contents to.
 */
//
// Flush to FILE *
//
#define _mulle_flushablebuffer_chars_to_struct( len) \
   ((len + sizeof( struct mulle_flushablebuffer) - 1) / sizeof( struct mulle_flushablebuffer))

#define mulle_buffer_do_FILE( name, fp)                                                                       \
   for( struct mulle_flushablebuffer                                                                          \
          name ## __alloca[ _mulle_flushablebuffer_chars_to_struct( MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY)], \
          name ## __storage = MULLE_FLUSHABLEBUFFER_STATIC_DATA( name ## __alloca,                            \
                                                                 sizeof( name ## __alloca),                   \
                                                                 fwrite,                                      \
                                                                 (fp)),                                       \
          *name ## __i = NULL;                                                                                \
        ! name ## __i;                                                                                        \
        name ## __i = ( mulle_flushablebuffer_done( &name ## __storage), (void *) 0x1)                        \
      )                                                                                                       \
                                                                                                              \
      for( struct mulle_buffer                                                                                \
            *name = (struct mulle_buffer *) &name ## __storage,                                               \
            *name ## __j = 0;    /* break protection */                                                       \
            name ## __j < (struct mulle_buffer *) 1;                                                          \
            name ## __j++)

#endif
