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


//
// this is fairly conveniently, just like fwrite(  _storage, len, nElems, fp)
// though a non-buffering write could be better
//
typedef size_t   mulle_flushablebuffer_flusher_t( void *buf,
                                                  size_t one,
                                                  size_t len,
                                                  void *userinfo);

// _size will be -2 for a flushable buffer (always inflexible)

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


#define MULLE_FLUSHABLEBUFFER_INIT_STATIC( xstorage, xlength,                     \
                                           xflusher, xuserinfo)                   \
   ((struct mulle_flushablebuffer)                                                \
   {                                                                              \
      ._initial_storage = (xstorage),                                             \
      ._curr            = (xstorage),                                             \
      ._storage         = (xstorage),                                             \
      ._sentinel        = &((unsigned char *)(xstorage))[ (xlength)],             \
      ._size            = (xlength),                                              \
      ._type            = MULLE_BUFFER_IS_INFLEXIBLE | MULLE_BUFFER_IS_FLUSHABLE, \
      ._flusher         = (mulle_flushablebuffer_flusher_t *) (xflusher),         \
      ._userinfo        = (xuserinfo)                                             \
   })                                                                             \

#define MULLE_FLUSHABLEBUFFER_INIT_ALLOCATED( xstorage, xlength, xflusher,        \
                                              xuserinfo, xallocator)              \
   ((struct mulle_flushablebuffer)                                                \
   {                                                                              \
      ._curr            = (xstorage),                                             \
      ._storage         = (xstorage),                                             \
      ._sentinel        = &((char *)(xstorage))[ (xlength)],                      \
      ._size            = (xlength),                                              \
      ._type            = MULLE_BUFFER_IS_INFLEXIBLE | MULLE_BUFFER_IS_FLUSHABLE, \
      ._flusher         = (mulle_flushablebuffer_flusher_t *) (xflusher),         \
      ._userinfo        = (xuserinfo),                                            \
      ._allocator       = (xallocator)                                            \
   })


static inline struct mulle_buffer   *
   mulle_flushablebuffer_as_buffer( struct mulle_flushablebuffer *buffer)
{
  return( (struct mulle_buffer *) buffer);
}



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
   buffer->_type            = MULLE_BUFFER_IS_INFLEXIBLE | MULLE_BUFFER_IS_FLUSHABLE;

   buffer->_flusher         = flusher;
   buffer->_userinfo        = userinfo;
   buffer->_allocator       = allocator;
}


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
   buffer->_type            = MULLE_BUFFER_IS_INFLEXIBLE | MULLE_BUFFER_IS_FLUSHABLE;

   buffer->_flusher         = flusher;
   buffer->_userinfo        = userinfo;
   buffer->_allocator       = allocator;
}


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


static inline int
   mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer)
{
   if( ! buffer)
      return( 0);
   return( _mulle_flushablebuffer_done( buffer));
}


MULLE__BUFFER_GLOBAL
struct mulle_flushablebuffer   *
   mulle_flushablebuffer_create( size_t length,
                                 mulle_flushablebuffer_flusher_t *flusher,
                                 void *userinfo,
                                 struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
int   mulle_flushablebuffer_destroy( struct mulle_flushablebuffer *buffer);


//
// Flush to FILE *
//
#define mulle_buffer_do_FILE( name, fp)                                                \
   unsigned char   name ## __buf[ 128];                                                \
   struct mulle_flushablebuffer                                                        \
      name ## __storage = MULLE_FLUSHABLEBUFFER_INIT_STATIC( name ## __buf,            \
                                                             sizeof( name ## __buf),   \
                                                             fwrite,                   \
                                                             (fp));                    \
   for( struct mulle_buffer                                                            \
          *name = (struct mulle_buffer *) &name ## __storage,                          \
          *name ## __i = NULL;                                                         \
        ! name ## __i;                                                                 \
        name ## __i = ( mulle_flushablebuffer_done( &name ## __storage), (void *) 0x1) \
      )                                                                                \
                                                                                       \
      for( int  name ## __j = 0;    /* break protection */                             \
           name ## __j < 1;                                                            \
           name ## __j++)

#endif
