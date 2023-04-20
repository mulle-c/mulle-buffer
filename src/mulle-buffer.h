//
//  mulle-buffer.h
//  mulle-buffer
//
//  Created by Nat! on 04/11/15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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

#ifndef mulle_buffer__h__
#define mulle_buffer__h__

#define MULLE_BUFFER_VERSION  ((3 << 20) | (4 << 8) | 0)

#include "include.h"
#include "mulle--buffer.h"


// stupidities to fix:
// non "_" prefixed functions should check for NULL buffer
//
#define MULLE_BUFFER_BASE              \
   MULLE__BUFFER_BASE;                 \
   struct mulle_allocator  *_allocator

//
// _size will be -1 for a non-growing buffer
// this is like a possibly growing memory block (->NSData)
//
struct mulle_buffer
{
   MULLE_BUFFER_BASE;
};


#define MULLE_BUFFER_INIT( allocator) ((struct mulle_buffer)   \
{                                                              \
   0, 0, 0, 0,                                                 \
   0,                                                          \
   MULLE_BUFFER_IS_FLEXIBLE,                                   \
   allocator ? allocator : &mulle_default_allocator            \
})


//   unsigned char   *_storage;
//   unsigned char   *_curr;
//   unsigned char   *_sentinel;
//   unsigned char   *_initial_storage;
//   size_t          _size;
//   unsigned int    _type
//

//
//
//
#define MULLE_BUFFER_INIT_FLEXIBLE( data, len, allocator)  \
   ((struct mulle_buffer)                                  \
   {                                                       \
      (unsigned char *) data,                              \
      (unsigned char *) data,                              \
      &((unsigned char *) data)[ (len)],                   \
      (unsigned char *) data,                              \
      (len),                                               \
      MULLE_BUFFER_IS_FLEXIBLE,                            \
      allocator ? allocator : &mulle_default_allocator     \
   })


#define MULLE_BUFFER_INIT_FLEXIBLE_FILLED( data, len, allocator)  \
   ((struct mulle_buffer)                                  \
   {                                                       \
      (unsigned char *) data,                              \
      &((unsigned char *) data)[ (len)],                   \
      &((unsigned char *) data)[ (len)],                   \
      (unsigned char *) data,                              \
      (len),                                               \
      MULLE_BUFFER_IS_FLEXIBLE,                            \
      allocator ? allocator : &mulle_default_allocator     \
   })


#define MULLE_BUFFER_INIT_INFLEXIBLE( data, len, allocator) \
   ((struct mulle_buffer)                                   \
   {                                                        \
      (unsigned char *) data,                               \
      (unsigned char *) data,                               \
      &((unsigned char *) data)[ (len)],                    \
      (unsigned char *) data,                               \
      (len),                                                \
      MULLE_BUFFER_IS_INFLEXIBLE,                           \
      allocator ? allocator : &mulle_default_allocator      \
   })


//
// we take a some static data, but we assume its already filled
// with data.
//
#define MULLE_BUFFER_INIT_INFLEXIBLE_FILLED( data, len, allocator) \
   ((struct mulle_buffer)                                          \
   {                                                               \
      (unsigned char *) data,                                      \
      &((unsigned char *) data)[ (len)],                           \
      &((unsigned char *) data)[ (len)],                           \
      (unsigned char *) data,                                      \
      (len),                                                       \
      MULLE_BUFFER_IS_INFLEXIBLE,                                  \
      allocator ? allocator : &mulle_default_allocator             \
   })



static inline struct mulle__buffer   *
   mulle_buffer_as__buffer( struct mulle_buffer *buffer)
{
  return( (struct mulle__buffer *) buffer);
}


MULLE_C_NONNULL_RETURN static inline struct mulle_allocator  *
   mulle_buffer_get_allocator( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( &mulle_default_allocator);

   assert( _mulle__buffer_is_inflexible( (struct mulle__buffer *) buffer) ||
           buffer->_allocator);
   return( buffer->_allocator);
}


# pragma mark - initialization and destruction

MULLE_BUFFER_GLOBAL
struct mulle_buffer   *mulle_buffer_create( struct mulle_allocator *allocator);


static inline void   mulle_buffer_destroy( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   _mulle__buffer_destroy( (struct mulle__buffer *) buffer,
                           mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_done( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;
   // it's ok to call _done even if not inited
   _mulle__buffer_done( (struct mulle__buffer *) buffer, buffer->_allocator);
}


static inline void
   mulle_buffer_set_allocator( struct mulle_buffer *buffer,
                               struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;
   // always have an allocator
   buffer->_allocator = allocator ? allocator : &mulle_default_allocator;
}


/* a growing buffer, that starts with a memory region, allocated by the user.
   Useful if you already malloced something and dont't need it anymore.
   The storage is overwritten from the start.
*/
static inline void
   mulle_buffer_init_with_allocated_bytes( struct mulle_buffer *buffer,
                                           void *storage,
                                           size_t length,
                                           struct mulle_allocator *allocator)
{
   if( ! buffer || ! storage)
      return;

   _mulle__buffer_init_with_allocated_bytes( (struct mulle__buffer *) buffer,
                                            storage,
                                            length);
   mulle_buffer_set_allocator( buffer, allocator);
}


/* a growing buffer, that starts with a non freeable memory region, allocated
   by the user. Useful if you want to supply some stack memory for mulle_buffer
   so that it need not necessarily malloc, if the contents remain small enough.
   The storage is overwritten from the start.
 */
static inline void
   mulle_buffer_init_with_static_bytes( struct mulle_buffer *buffer,
                                        void *storage,
                                        size_t length,
                                        struct mulle_allocator *allocator)
{
   if( ! buffer || ! storage)
      return;

   _mulle__buffer_init_with_static_bytes( (struct mulle__buffer *) buffer, storage, length);
   mulle_buffer_set_allocator( buffer, allocator);
}


//
// The allocator you pass in will provide the alignment guarantees of
// the internal buffer. So if it is malloc based and doesn't change the
// address the "return[ed] ... memory ... is suitably aligned for any
// built-in type"
//
static inline void   mulle_buffer_init( struct mulle_buffer *buffer,
                                        struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   _mulle__buffer_init( (struct mulle__buffer *) buffer);
   mulle_buffer_set_allocator( buffer, allocator);
}


static inline void   mulle_buffer_init_with_capacity( struct mulle_buffer *buffer,
                                                      size_t capacity,
                                                      struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   _mulle__buffer_init_with_capacity( (struct mulle__buffer *) buffer, capacity);
   mulle_buffer_set_allocator( buffer, allocator);
}


/* this buffer does not grow. storage is supplied. This is useful for
   guaranteeing that a buffer can not be overrun.
   The storage is overwritten from the start.
 */
static inline void   mulle_buffer_init_inflexible_with_static_bytes( struct mulle_buffer *buffer,
                                                                     void *storage,
                                                                     size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_init_inflexible_with_static_bytes( (struct mulle__buffer *) buffer,
                                                     storage,
                                                     length);
   mulle_buffer_set_allocator( buffer, NULL);
}


#pragma mark - sizing

static inline int   mulle_buffer_grow( struct mulle_buffer *buffer,
                                       size_t min_amount)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_grow( (struct mulle__buffer *) buffer,
                                min_amount,
                                mulle_buffer_get_allocator( buffer)));
}


static inline void   mulle_buffer_size_to_fit( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   _mulle__buffer_size_to_fit( (struct mulle__buffer *) buffer,
                              mulle_buffer_get_allocator( buffer));
}


/* the given storage replaces the previous contents.
   the buffer is not flexable anymore.
   The insertion point is moved to &storage[length]
 */
static inline void   mulle_buffer_make_inflexible( struct mulle_buffer *buffer,
                                                   void *storage,
                                                   size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_make_inflexible( (struct mulle__buffer *) buffer,
                                  storage,
                                  length,
                                  mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_zero_to_length( struct mulle_buffer *buffer,
                                                  size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_zero_to_length( (struct mulle__buffer *) buffer,
                                 length,
                                 mulle_buffer_get_allocator( buffer));
}


static inline size_t   mulle_buffer_set_length( struct mulle_buffer *buffer,
                                                size_t length)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_set_length( (struct mulle__buffer *) buffer,
                                       length,
                                       mulle_buffer_get_allocator( buffer)));
}


//
// you only do this once!, because you now own the malloc block
// TODO: rename to extract only ?
//static inline void   *mulle_buffer_extract_all( struct mulle_buffer *buffer)
//{
//   if( ! buffer)
//      return( NULL);
//   return( _mulle__buffer_extract_all( (struct mulle__buffer *) buffer,
//                                       mulle_buffer_get_allocator( buffer)));
//}


static inline struct mulle_data   mulle_buffer_extract_data( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( mulle_data_make_invalid());

   return( _mulle__buffer_extract_data( (struct mulle__buffer *) buffer,
                                         mulle_buffer_get_allocator( buffer)));
}


static inline void   *mulle_buffer_extract_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_extract_string( (struct mulle__buffer *) buffer,
                                           mulle_buffer_get_allocator( buffer)));
}


static inline void   *mulle_buffer_extract_bytes( struct mulle_buffer *buffer)
{
   struct mulle_data   data;

   if( ! buffer)
      return( NULL);

   data = _mulle__buffer_extract_data( (struct mulle__buffer *) buffer,
                                        mulle_buffer_get_allocator( buffer));
   return( data.bytes);
}


static inline void   mulle_buffer_remove_all( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   _mulle__buffer_remove_all( (struct mulle__buffer *) buffer);
}


static inline void   mulle_buffer_remove_in_range( struct mulle_buffer *buffer,
                                                   size_t offset,
                                                   size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_remove_in_range( (struct mulle__buffer *) buffer, offset, length);
}


static inline void
   mulle_buffer_set_initial_capacity( struct mulle_buffer *buffer,
                                      size_t capacity)
{
   if( ! buffer)
      return;
   _mulle__buffer_set_initial_capacity( (struct mulle__buffer *) buffer, capacity);
}


#pragma mark - accessors


static inline struct mulle_data   mulle__buffer_get_data( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( mulle_data_make_invalid());
   return( _mulle__buffer_get_data( (struct mulle__buffer *) buffer));
}

static inline void   *mulle_buffer_get_bytes( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);
   return( _mulle__buffer_get_bytes( (struct mulle__buffer *) buffer));
}


static inline char   *mulle_buffer_get_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_get_string( (struct mulle__buffer *) buffer, buffer->_allocator));
}


static inline int    mulle_buffer_get_last_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_get_last_byte( (struct mulle__buffer *) buffer));
}


static inline size_t   mulle_buffer_get_length( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_length( (struct mulle__buffer *) buffer));
}


static inline size_t   mulle_buffer_get_capacity( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_capacity( (struct mulle__buffer *) buffer));
}


static inline size_t
   mulle_buffer_get_staticlength( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_staticlength( (struct mulle__buffer *) buffer));
}


static inline int
    mulle_buffer_set_seek( struct mulle_buffer *buffer, int mode, size_t seek)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_set_seek( (struct mulle__buffer *) buffer, mode, seek));
}


static inline size_t   mulle_buffer_get_seek( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_seek( (struct mulle__buffer *) buffer));
}


static inline void   *mulle_buffer_advance( struct mulle_buffer *buffer,
                                            size_t length)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_advance( (struct mulle__buffer *) buffer,
                                  length,
                                  mulle_buffer_get_allocator( buffer)));
}


#pragma mark - copy out

static inline void   mulle_buffer_copy_range( struct mulle_buffer *buffer,
                                              size_t offset,
                                              size_t length,
                                              unsigned char *dst)
{
   if( ! buffer || ! dst)
      return;

   _mulle__buffer_copy_range( (struct mulle__buffer *) buffer,
                              offset,
                              length,
                              dst);
}


#pragma mark - query

static inline int   mulle_buffer_is_inflexible( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_inflexible( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_is_flushable( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_is_flushable( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_is_full( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_full( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_is_big_enough( struct mulle_buffer *buffer,
                                                size_t len)
{
   if( ! buffer)
      return( len == 0);

   return( _mulle__buffer_is_big_enough( (struct mulle__buffer *) buffer, len));
}


// same as mulle_buffer_get_length( buffer) == 0
static inline int   mulle_buffer_is_empty( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_empty( (struct mulle__buffer *) buffer));
}


// a void buffer's backing storage can't hold any content
static inline int   mulle_buffer_is_void( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_void( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_has_overflown( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);  // or 0 ?

   return( _mulle__buffer_has_overflown( (struct mulle__buffer *) buffer));
}


//
// check if bytes of length, are actually buffer contents
//
static inline int   mulle_buffer_intersects_bytes( struct mulle_buffer *buffer,
                                                   void *bytes,
                                                   size_t length)
{
   if( ! buffer)
      return( length == 0);

   return( _mulle__buffer_intersects_bytes( (struct mulle__buffer *) buffer,
                                           bytes,
                                           length));
}


#pragma mark - additions

//
// returns NULL, if length can not be guaranteed. Otherwise a pointer to
// the unused area. Use mulle_buffer_advance to forward the current
// pointer after writing into the area.
//
static inline void   *mulle_buffer_guarantee( struct mulle_buffer *buffer,
                                              size_t length)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_guarantee( (struct mulle__buffer *) buffer,
                                     length,
                                     mulle_buffer_get_allocator( buffer)));
}


static inline void   mulle_buffer_add_byte( struct mulle_buffer *buffer,
                                            unsigned char c)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_byte( (struct mulle__buffer *) buffer,
                            c,
                            mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_remove_last_byte( struct mulle_buffer *buffer)
{
   _mulle__buffer_remove_last_byte( (struct mulle__buffer *) buffer);
}


static inline void   mulle_buffer_add_char( struct mulle_buffer *buffer,
                                                 int c)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_char( (struct mulle__buffer *) buffer,
                            c,
                            mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_add_uint16( struct mulle_buffer *buffer,
                                              uint16_t c)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_uint16( (struct mulle__buffer *) buffer,
                              c,
                              mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_add_uint32( struct mulle_buffer *buffer,
                                              uint32_t c)
{
   _mulle__buffer_add_uint32( (struct mulle__buffer *) buffer,
                              c,
                              mulle_buffer_get_allocator( buffer));
}


#pragma mark - add memory ranges

// MEMO: need to stay compatible with: mulle_utf_add_bytes_function_t
static inline void   mulle_buffer_add_bytes( struct mulle_buffer *buffer,
                                             void *bytes,
                                             size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_bytes( (struct mulle__buffer *) buffer,
                              bytes,
                              length,
                              mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_add_string( struct mulle_buffer *buffer,
                                              char *bytes)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_string( (struct mulle__buffer *) buffer,
                              bytes,
                              mulle_buffer_get_allocator( buffer));
}


// just a synonym
static inline void   mulle_buffer_strcpy( struct mulle_buffer *buffer,
                                         char *bytes)
{
   mulle_buffer_add_string( buffer, bytes);
}


static inline void   mulle_buffer_add_string_if_empty( struct mulle_buffer *buffer,
                                                       char *bytes)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_string_if_empty( (struct mulle__buffer *) buffer,
                                       bytes,
                                       mulle_buffer_get_allocator( buffer));
}


static inline void   mulle_buffer_add_string_if_not_empty( struct mulle_buffer *buffer,
                                                           char *bytes)
{
   if( ! buffer)
      return;

   _mulle__buffer_add_string_if_not_empty( (struct mulle__buffer *) buffer,
                                           bytes,
                                           mulle_buffer_get_allocator( buffer));
}



static inline size_t
   mulle_buffer_add_string_with_maxlength( struct mulle_buffer *buffer,
                                           char *bytes,
                                           size_t length)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_add_string_with_maxlength( (struct mulle__buffer *) buffer,
                                                     bytes,
                                                     length,
                                                     mulle_buffer_get_allocator( buffer)));
}


static inline void   mulle_buffer_memset( struct mulle_buffer *buffer,
                                          int c,
                                          size_t length)
{
   if( ! buffer)
      return;

   _mulle__buffer_memset( (struct mulle__buffer *) buffer,
                         c,
                         length,
                         mulle_buffer_get_allocator( buffer));
}


static inline int   mulle_buffer_memcmp( struct mulle_buffer *buffer,
                                         void  *bytes,
                                         size_t length)
{
   if( ! buffer)
      return( +1);

   return( _mulle__buffer_memcmp( (struct mulle__buffer *) buffer,
                                  bytes,
                                  length));
}


static inline int   mulle_buffer_zero_last_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_zero_last_byte( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_make_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_make_string( (struct mulle__buffer *) buffer,
                                        mulle_buffer_get_allocator( buffer)));
}


static inline void    mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                               struct mulle_buffer *other)
{
   if( ! buffer || ! other)
      return;

   _mulle__buffer_add_buffer( (struct mulle__buffer *) buffer,
                             (struct mulle__buffer *) other,
                             mulle_buffer_get_allocator( buffer));
}


static inline void
   mulle_buffer_add_buffer_range( struct mulle_buffer *buffer,
                                  struct mulle__buffer *other,  // sic
                                  size_t offset,
                                  size_t length)
{
   if( ! buffer || ! other)
      return;

   _mulle__buffer_add_buffer_range( (struct mulle__buffer *) buffer,
                                   other,
                                   offset,
                                   length,
                                   mulle_buffer_get_allocator( buffer));
}



// _initial_storage storage will be lost
static inline void   mulle_buffer_reset( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   _mulle__buffer_reset( (struct mulle__buffer *) buffer,
                          mulle_buffer_get_allocator( buffer));
}


#pragma mark - reading

/*
 * Limited read support for buffers
 * -1 == no more bytes
 */
static inline int   mulle_buffer_next_bytes( struct mulle_buffer *buffer,
                                             void *buf,
                                             size_t len)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_next_bytes( (struct mulle__buffer *) buffer, buf, len));
}


static inline void   *
   mulle_buffer_reference_bytes( struct mulle_buffer *buffer, size_t len)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_reference_bytes( (struct mulle__buffer *) buffer, len));
}


static inline int   mulle_buffer_next_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_next_byte( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_peek_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_peek_byte( (struct mulle__buffer *) buffer));
}



static inline int   mulle_buffer_next_character( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_next_character( (struct mulle__buffer *) buffer));
}


static inline int   mulle_buffer_find_byte( struct mulle_buffer *buffer,
                                            unsigned char byte)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_find_byte( (struct mulle__buffer *) buffer, byte));
}


#pragma mark - hexdump fixed to 16 bytes per line

// actually they are bits to be ORed
enum mulle_buffer_hexdump_options
{
   mulle_buffer_hexdump_default   = 0x0,
   mulle_buffer_hexdump_no_offset = 0x1,
   mulle_buffer_hexdump_no_hex    = 0x2,
   mulle_buffer_hexdump_no_ascii  = 0x4
};


// dumps only for n >= 1 && n <= 16
MULLE_BUFFER_GLOBAL
void  mulle_buffer_hexdump_line( struct mulle_buffer *buffer,
                                 void *bytes,
                                 unsigned int n,
                                 size_t counter,
                                 unsigned int options);

// dumps all, does not append a \0
MULLE_BUFFER_GLOBAL
void  mulle_buffer_hexdump( struct mulle_buffer *buffer,
                            void *bytes,
                            size_t length,
                            size_t counter,
                            unsigned int options);

#pragma mark - mulle_flushablebuffer

#define MULLE_FLUSHABLEBUFFER_BASE    \
MULLE__FLUSHABLEBUFFER_BASE;          \
struct mulle_allocator  *_allocator

struct mulle_flushablebuffer
{
   MULLE_FLUSHABLEBUFFER_BASE;
};

typedef mulle__flushablebuffer_flusher   mulle_flushablebuffer_flusher;
typedef mulle__flushablebuffer_flusher   *mulle_flushablebuffer_flusher_t;

static inline void
   mulle_flushablebuffer_init( struct mulle_flushablebuffer *buffer,
                               void *storage,
                               size_t length,
                               mulle_flushablebuffer_flusher_t flusher,
                               void *userinfo)
{
   if( ! buffer || ! storage || ! flusher)
      return;

   _mulle__flushablebuffer_init( (struct mulle__flushablebuffer *) buffer,
                                storage,
                                length,
                                flusher,
                                userinfo);
}


static inline int
   mulle_flushablebuffer_flush( struct mulle_flushablebuffer *buffer)
{
   if( ! buffer)
      return( -1);
   return( _mulle__flushablebuffer_flush( (struct mulle__flushablebuffer *) buffer));
}

MULLE_BUFFER_GLOBAL
int   mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer);

MULLE_BUFFER_GLOBAL
int   mulle_flushablebuffer_destroy( struct mulle_flushablebuffer *buffer);


static inline struct mulle_buffer   *
   mulle_flushablebuffer_as_buffer( struct mulle_flushablebuffer *buffer)
{
   return( (struct mulle_buffer *) buffer);
}


#pragma mark - backwards compatibility

MULLE_C_DEPRECATED
static inline int   mulle_buffer_is_inflexable( struct mulle_buffer *buffer)
{
   return( mulle_buffer_is_inflexible( buffer));
}


MULLE_C_DEPRECATED static inline void
   mulle_buffer_init_inflexable_with_static_bytes( struct mulle_buffer *buffer,
                                                   void *storage,
                                                   size_t length)
{
   mulle_buffer_init_inflexible_with_static_bytes( buffer, storage, length);
}


MULLE_C_DEPRECATED static inline void
   mulle_buffer_make_inflexable( struct mulle_buffer *buffer,
                                 void *storage,
                                 size_t length)
{
   mulle_buffer_make_inflexible( buffer, storage, length);
}


// convenience:
// static void   example( void)
// {
//    char   *s;
//
//    mulle_buffer_do_string( buffer, NULL, s)
//    {
//       mulle_buffer_add_string( &buffer, "VfL Bochum 1848");
//       break;
//    }
//
//    printf( "%s\n", s);
//    mulle_free( s);
// }
//
// Caveats: don't use return in the block, or you will leak, use mulle_buffer_return
//          don't pre-initialize the buffer
//
#define mulle_buffer_return( name, value)           \
   do                                               \
   {                                                \
      __typeof__( value) name ## __tmp = (value);   \
                                                    \
      mulle_buffer_done( &name ## __storage);       \
      return( value);                               \
   }                                                \
   while( 0)

#define mulle_buffer_do_string( name, allocator, s)                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT( allocator), \
                            *name = &name ## __storage,                        \
                            name ## __i = { 0 };                               \
                                                                               \
        s = (name ## __i._storage)                                             \
               ? mulle_buffer_extract_string( &name ## __storage)              \
               : NULL,                                                         \
        ! name ## __i._storage;                                                \
                                                                               \
        name ## __i._storage = (void *) 0x1                                    \
      )                                                                        \
                                                                               \
      for( int  name ## __j = 0;    /* break protection */                     \
           name ## __j < 1;                                                    \
           name ## __j++)


//
// have a buffer inside the block, will self destruct when the block
// is exited (properly)
//
#define mulle_buffer_do( name)                                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT( NULL), \
                            *name = &name ## __storage,                   \
                            name ## __i = { 0 };                          \
        ! name ## __i._storage;                                           \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),  \
                                 (void *) 0x1)                            \
      )                                                                   \
                                                                          \
      for( int  name ## __j = 0;    /* break protection */                \
           name ## __j < 1;                                               \
           name ## __j++)

//
// As above but the buffer storage allocator can be changed from the
// default allocator
//
#define mulle_buffer_do_allocator( name, allocator)                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT( allocator), \
                            *name = &name ## __storage,                        \
                            name ## __i = { 0 };                               \
        ! name ## __i._storage;                                                \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),       \
                                 (void *) 0x1)                                 \
      )                                                                        \
                                                                               \
      for( int  name ## __j = 0;    /* break protection */                     \
           name ## __j < 1;                                                    \
           name ## __j++)


//
// Create a buffer with some static/auto storage preset. If that is
// exhausted the buffer will start mallocing. Nice to avoid an initial malloc
// for small workloads.
//
#define mulle_buffer_do_flexible( name, data, len)                                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT_FLEXIBLE( data, len, NULL), \
                            *name = &name ## __storage,                                       \
                            name ## __i = { 0 };                                              \
        ! name ## __i._storage;                                                               \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),                      \
                                 (void *) 0x1)                                                \
      )                                                                                       \
                                                                                              \
      for( int  name ## __j = 0;    /* break protection */                                    \
           name ## __j < 1;                                                                   \
           name ## __j++)


//
// Create a buffer with some static/auto storage preset. This storage already
// contains bytes for the buffer. Otherwise as above.
//
#define mulle_buffer_do_flexible_filled( name, data, len)                                     \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT_FLEXIBLE_FILLED( data, len, NULL), \
                            *name = &name ## __storage,                                       \
                            name ## __i = { 0 };                                              \
        ! name ## __i._storage;                                                               \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),                      \
                                 (void *) 0x1)                                                \
      )                                                                                       \
                                                                                              \
      for( int  name ## __j = 0;    /* break protection */                                    \
           name ## __j < 1;                                                                   \
           name ## __j++)


//
// Like mulle_buffer_do_flexible but when the buffer is full, there won't be
// a malloc.
//
#define mulle_buffer_do_inflexible( name, data, len)                                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT_INFLEXIBLE( data, len, NULL), \
                            *name = &name ## __storage,                                         \
                            name ## __i = { 0 };                                                \
        ! name ## __i._storage;                                                                 \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),                        \
                                 (void *) 0x1)                                                  \
      )                                                                                         \
                                                                                                \
      for( int  name ## __j = 0;    /* break protection */                                      \
           name ## __j < 1;                                                                     \
           name ## __j++)

//
// Like above, but the buffer is already preset with data. So nothing can
// be added (but it can be read).
//
#define mulle_buffer_do_inflexible_filled( name, data, len)                                            \
   for( struct mulle_buffer name ## __storage = MULLE_BUFFER_INIT_INFLEXIBLE_FILLED( data, len, NULL), \
                            *name = &name ## __storage,                                                \
                            name ## __i = { 0 };                                                       \
        ! name ## __i._storage;                                                                        \
        name ## __i._storage = ( mulle_buffer_done( &name ## __storage),                               \
                                 (void *) 0x1)                                                         \
      )                                                                                                \
                                                                                                       \
      for( int  name ## __j = 0;    /* break protection */                                             \
           name ## __j < 1;                                                                            \
           name ## __j++)


// TODO:
//#define mulle_buffer_do_FILE( name, FP)


#include "mulle-flexbuffer.h"


#ifdef __has_include
# if __has_include( "_mulle-buffer-versioncheck.h")
#  include "_mulle-buffer-versioncheck.h"
# endif
#endif


#endif /* mulle_buffer_h */
