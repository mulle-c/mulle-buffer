//
//  Copyright (C) 2011 Nat!, Mulle kybernetiK.
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

#ifndef mulle__buffer__h__
#define mulle__buffer__h__

#include "include.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


struct mulle_allocator;


enum
{
  MULLE_BUFFER_IS_FLEXIBLE      = 0,
  MULLE_BUFFER_IS_INFLEXIBLE    = 1,
  MULLE_BUFFER_IS_FLUSHABLE     = 2
};


//
// this is the base for MULLE_BUFFER, but it does not contain an allocator
// and none of the functions check for NULL pointers
// _size is there to keep "capacity" for initial allocation (could be
// optimized away, if we don't need capacity)
//
#define MULLE__BUFFER_BASE            \
   unsigned char   *_storage;         \
   unsigned char   *_curr;            \
   unsigned char   *_sentinel;        \
   unsigned char   *_initial_storage; \
   size_t          _size;             \
   unsigned int    _type


// NSData/NSMutableData
//
// _size will be -1 for a non-growing buffer
// this is like a possibly growing memory block (->NSData)
//
struct mulle__buffer
{
   MULLE__BUFFER_BASE;
};

#define MULLE__BUFFER_INIT_FLEXIBLE( data, len)    \
   ((struct mulle__buffer)                         \
   {                                               \
      (unsigned char *) data,                      \
      (unsigned char *) data,                      \
      &((unsigned char *) data)[ (len)],           \
      (unsigned char *) data,                      \
      (len),                                       \
      MULLE_BUFFER_IS_FLEXIBLE                     \
   })

#define MULLE__BUFFER_INIT_INFLEXIBLE( data, len)  \
   ((struct mulle__buffer)                         \
   {                                               \
      (unsigned char *) data,                      \
      (unsigned char *) data,                      \
      &((unsigned char *) data)[ (len)],           \
      (unsigned char *) data,                      \
      (len),                                       \
      MULLE_BUFFER_IS_FLEXIBLE                     \
   })

//
// this is fairly conveniently, just like fwrite(  _storage, len, nElems, fp)
// though a non-buffering write could be better
//
typedef size_t   mulle__flushablebuffer_flusher( void *buf,
                                                 size_t one,
                                                 size_t len,
                                                 void *userinfo);

// _size will be -2 for a flushable buffer (always inflexible)

// since we want to cast this to mulle_buffer eventually, the allocator
// must be in here (argh). It's not used by mulle--buffer code though
// and it's initialized to the default allocator
#define MULLE__FLUSHABLEBUFFER_BASE                         \
   MULLE__BUFFER_BASE;                                      \
   struct mulle_allocator           *_allocator;            \
   mulle__flushablebuffer_flusher   *_flusher;              \
   size_t                           _flushed;               \
   void                             *_userinfo

/*
 * The flushable buffer is inflexible and occasionally
 * flushes out data to make room.
 * It's easy to stream data to stdout with a flushable
 * buffer.
 */
struct mulle__flushablebuffer
{
   MULLE__FLUSHABLEBUFFER_BASE;
};

static inline struct mulle__buffer   *
   _mulle_flushablebuffer_as_buffer( struct mulle__flushablebuffer *buffer)
{
  return( (struct mulle__buffer *) buffer);
}

#pragma mark - creation destruction

MULLE__BUFFER_GLOBAL
struct mulle__buffer   *_mulle__buffer_create( struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
void                   _mulle__buffer_destroy( struct mulle__buffer *buffer,
                                               struct mulle_allocator *allocator);


#pragma mark - initialization

// the storage is
static inline void   _mulle__flushablebuffer_init_with_static_bytes( struct mulle__flushablebuffer *buffer,
                                                                     void *storage,
                                                                     size_t length,
                                                                     mulle__flushablebuffer_flusher *flusher,
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
   buffer->_allocator       = allocator ? allocator : &mulle_default_allocator;
}


static inline void   _mulle__flushablebuffer_init_with_allocated_bytes( struct mulle__flushablebuffer *buffer,
                                                                        void *storage,
                                                                        size_t length,
                                                                        mulle__flushablebuffer_flusher *flusher,
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
   buffer->_allocator       = allocator ? allocator : &mulle_default_allocator;
}

// if != 0, the flush didn't succeed and the buffer is still alive!
MULLE__BUFFER_GLOBAL
int   _mulle__flushablebuffer_done( struct mulle__flushablebuffer *buffer);


static inline void   _mulle__buffer_init_with_allocated_bytes( struct mulle__buffer *buffer,
                                                               void *storage,
                                                               size_t length)
{
   assert( length != (size_t) -1);

   buffer->_initial_storage = NULL;
   buffer->_curr            =
   buffer->_storage         = storage;
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_BUFFER_IS_FLEXIBLE;
}


static inline void   _mulle__buffer_init_with_static_bytes( struct mulle__buffer *buffer,
                                                            void *storage,
                                                            size_t length)
{
   assert( length != (size_t) -1);

   buffer->_initial_storage =
   buffer->_curr            =
   buffer->_storage         = storage;
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_BUFFER_IS_FLEXIBLE;
}


static inline void    _mulle__buffer_init( struct mulle__buffer *buffer)
{
   memset( buffer, 0, sizeof( struct mulle__buffer));
}


static inline void   _mulle__buffer_set_initial_capacity( struct mulle__buffer *buffer,
                                                          size_t capacity)
{
   assert( buffer);
   assert( buffer->_storage  == 0);
   assert( buffer->_size == 0);
   buffer->_size =  capacity >> 1;
}


static inline void   _mulle__buffer_init_with_capacity( struct mulle__buffer *buffer,
                                                        size_t capacity)
{
   buffer->_initial_storage  =
   buffer->_curr             =
   buffer->_storage          =
   buffer->_sentinel         = NULL;
   buffer->_size             = capacity >> 1;  // will grow to double _size
   buffer->_type             = MULLE_BUFFER_IS_FLEXIBLE;
}


static inline void
  _mulle__buffer_init_inflexible_with_static_bytes( struct mulle__buffer *buffer,
                                                    void *storage,
                                                    size_t length)
{
   assert( length != (size_t) -1);

   buffer->_initial_storage  =
   buffer->_storage          =
   buffer->_curr             = storage;
   buffer->_sentinel         = &buffer->_storage[ length];
   buffer->_size             = length;
   buffer->_type             = MULLE_BUFFER_IS_INFLEXIBLE;
}


static inline void   _mulle__buffer_done( struct mulle__buffer *buffer,
                                          struct mulle_allocator *allocator)
{
   if( buffer->_storage != buffer->_initial_storage)
      _mulle_allocator_free( allocator, buffer->_storage);
#ifdef DEBUG
   memset( buffer, 0xFD, sizeof( struct mulle__buffer));
#endif
}


// _initial_storage storage will be lost
static inline void   _mulle__buffer_reset( struct mulle__buffer *buffer,
                                           struct mulle_allocator *allocator)
{
   _mulle__buffer_done( buffer, allocator);
   _mulle__buffer_init( buffer);
}


#pragma mark - change buffer type

MULLE__BUFFER_GLOBAL
void   _mulle__buffer_make_inflexible( struct mulle__buffer *buffer,
                                      void *storage,
                                      size_t length,
                                      struct mulle_allocator *allocator);

#pragma mark - resize

MULLE__BUFFER_GLOBAL
int    _mulle__buffer_grow( struct mulle__buffer *buffer,
                            size_t min_amount,
                            struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
void   _mulle__buffer_size_to_fit( struct mulle__buffer *buffer,
                                   struct mulle_allocator *allocator);

// this zeroes, when advancing, shrinks otherwise
MULLE__BUFFER_GLOBAL
void   _mulle__buffer_zero_to_length( struct mulle__buffer *buffer,
                                      size_t length,
                                      struct mulle_allocator *allocator);


// this zeroes, when advancing, shrinks otherwise
MULLE__BUFFER_GLOBAL
size_t   _mulle__buffer_set_length( struct mulle__buffer *buffer,
                                    size_t length,
                                    struct mulle_allocator *allocator);


static inline void   *_mulle__buffer_guarantee( struct mulle__buffer *buffer,
                                                size_t length,
                                                struct mulle_allocator *allocator)
{
   ptrdiff_t   missing;

   missing = &buffer->_curr[ length] - buffer->_sentinel;
   if( missing > 0)
      if( _mulle__buffer_grow( buffer, (size_t) missing, allocator))
         return( NULL);

   assert( ! missing || buffer->_curr); // for the analyzer
   return( buffer->_curr);
}


static inline void   *_mulle__buffer_advance( struct mulle__buffer *buffer,
                                              size_t length,
                                              struct mulle_allocator *allocator)
{
   unsigned char   *reserved;

   reserved = _mulle__buffer_guarantee( buffer, length, allocator);
   if( reserved)
      buffer->_curr = &buffer->_curr[ length];

   return( reserved);
}


#pragma mark - removal

static inline void   _mulle__buffer_remove_all( struct mulle__buffer *buffer)
{
   buffer->_curr = buffer->_storage;
}


#pragma mark - query

static inline int   _mulle__buffer_is_inflexible( struct mulle__buffer *buffer)
{
   return( buffer->_type & MULLE_BUFFER_IS_INFLEXIBLE);
}


static inline int   _mulle__buffer_is_flushable( struct mulle__buffer *buffer)
{
   return( buffer->_type & MULLE_BUFFER_IS_FLUSHABLE);
}


static inline int   _mulle__buffer_is_full( struct mulle__buffer *buffer)
{
   return( buffer->_curr >= buffer->_sentinel);
}


static inline int   _mulle__buffer_is_big_enough( struct mulle__buffer *buffer,
                                                  size_t len)
{
   assert( len);
   return( &buffer->_curr[ len] <= buffer->_sentinel);
}


static inline int   _mulle__buffer_is_empty( struct mulle__buffer *buffer)
{
   return( buffer->_curr == buffer->_storage);
}


static inline int   _mulle__buffer_is_void( struct mulle__buffer *buffer)
{
   return( buffer->_storage == buffer->_sentinel);
}


static inline int   _mulle__buffer_has_overflown( struct mulle__buffer *buffer)
{
   return( buffer->_curr > buffer->_sentinel); // only ever yes for inflexible buffer
}


static inline size_t   _mulle__buffer_get_length( struct mulle__buffer *buffer)
{
   return( _mulle__buffer_has_overflown( buffer)
          ? buffer->_sentinel - buffer->_storage
          : buffer->_curr - buffer->_storage);
}


static inline size_t  _mulle__buffer_get_capacity( struct mulle__buffer *buffer)
{
   if( buffer->_storage == NULL)
      return( buffer->_size << 1);
   return( _mulle__buffer_get_length( buffer));
}


enum
{
   MULLE_BUFFER_SEEK_SET = 0,
   MULLE_BUFFER_SEEK_CUR = 1,
   MULLE_BUFFER_SEEK_END = 2
};

MULLE__BUFFER_GLOBAL
size_t   _mulle__buffer_get_seek( struct mulle__buffer *buffer);

MULLE__BUFFER_GLOBAL
int      _mulle__buffer_set_seek( struct mulle__buffer *buffer, int mode, size_t seek);


static inline size_t   _mulle__buffer_get_staticlength( struct mulle__buffer *buffer)
{
   return( buffer->_storage == buffer->_initial_storage
          ? _mulle__buffer_get_length( buffer)
          : 0);
}


#pragma mark - retrieval

//
// you only do this once!, because you now own the malloc block
// if the length is zero, then buffer.bytes will also be NULL(!)
//
MULLE__BUFFER_GLOBAL
struct mulle_data   _mulle__buffer_extract_data( struct mulle__buffer *buffer,
                                                 struct mulle_allocator *allocator);

// old name, obsolete now
//static inline void   *_mulle__buffer_extract_all( struct mulle__buffer *buffer,
//                                                  struct mulle_allocator *allocator)
//{
//   return( _mulle__buffer_extract_data( buffer, allocator).bytes);
//}


//
// Like _mulle__buffer_extract_data but guarantees
// it's a C-String and sizes to fit
//
MULLE__BUFFER_GLOBAL
char   *_mulle__buffer_extract_string( struct mulle__buffer *buffer,
                                       struct mulle_allocator *allocator);

//
// Nice when building up a C string dynamically. This will ensure the last
// byte is a zero or append one if necessary. For a static buffer it may
// lop of the last character if the buffer is full
//
MULLE__BUFFER_GLOBAL
char   *_mulle__buffer_get_string( struct mulle__buffer *buffer,
                                   struct mulle_allocator *allocator);


static inline struct mulle_data   _mulle__buffer_get_data( struct mulle__buffer *buffer)
{
   struct mulle_data   data;

   data.bytes  = buffer->_storage;
   data.length = _mulle__buffer_get_length( buffer);

   return( data);
}


static inline void   *_mulle__buffer_get_bytes( struct mulle__buffer *buffer)
{
   return( buffer->_storage);
}


static inline size_t   _mulle__buffer_get_size( struct mulle__buffer *buffer)
{
   return( buffer->_size);
}


#pragma mark - modification

// instead of checking every char, run it for a while and then check
// _mulle__buffer_has_overflown.
static inline void    _mulle__buffer_add_byte( struct mulle__buffer *buffer,
                                               uint8_t c,
                                               struct mulle_allocator *allocator)
{
   if( _mulle__buffer_is_full( buffer))
      if( _mulle__buffer_grow( buffer, 1, allocator))
         return;

   assert( buffer->_curr); // for the analyzer
   *buffer->_curr++ = c;
}


static inline int    _mulle__buffer_get_last_byte( struct mulle__buffer *buffer)
{
   assert( ! _mulle__buffer_is_empty( buffer));

   if( buffer->_curr == buffer->_storage)
      return( -1);
   return( buffer->_curr[ -1]);
}


static inline void    _mulle__buffer_remove_last_byte( struct mulle__buffer *buffer)
{
   assert( ! _mulle__buffer_is_empty( buffer));

   --buffer->_curr;
}


void   _mulle__buffer_remove_in_range( struct mulle__buffer *buffer,
                                       size_t offset,
                                       size_t length);


static inline void    _mulle__buffer_add_char( struct mulle__buffer *buffer,
                                               int c,
                                               struct mulle_allocator *allocator)
{
   assert( c <= CHAR_MAX && c >= CHAR_MIN);
   _mulle__buffer_add_byte( buffer, (unsigned char) c, allocator);
}


static inline void    _mulle__buffer_add_uint16( struct mulle__buffer *buffer,
                                                 uint16_t c,
                                                 struct mulle_allocator *allocator)
{
   unsigned char   lsb;
   unsigned char   msb;

   if( ! _mulle__buffer_guarantee( buffer, 2, allocator))
      return;

   lsb = c & 0xFF;
   c >>= 8;
   msb = c & 0xFF;

   // always use network order
   *buffer->_curr++ = msb;
   *buffer->_curr++ = lsb;
}


static inline void    _mulle__buffer_add_uint32( struct mulle__buffer *buffer,
                                                 uint32_t c,
                                                 struct mulle_allocator *allocator)
{
   unsigned char   lsb;
   unsigned char   nsb;
   unsigned char   qsb;
   unsigned char   msb;

   if( ! _mulle__buffer_guarantee( buffer, 4, allocator))
      return;

   lsb = c & 0xFF;
   c >>= 8;
   nsb = c & 0xFF;
   c >>= 8;
   qsb = c & 0xFF;
   c >>= 8;
   msb = c & 0xFF;

// always use network order
   *buffer->_curr++ = msb;
   *buffer->_curr++ = qsb;
   *buffer->_curr++ = nsb;
   *buffer->_curr++ = lsb;
}


static inline int   _mulle__buffer_intersects_bytes( struct mulle__buffer *buffer,
                                                     void *bytes,
                                                     size_t length)
{
   unsigned char   *start;
   unsigned char   *end;

   if( ! length)
      return( 0);

   start = bytes;
   end   = &start[ length];

   return( end > buffer->_storage && start < buffer->_sentinel);
}


static inline void   _mulle__buffer_add_bytes( struct mulle__buffer *buffer,
                                               void *bytes,
                                               size_t length,
                                               struct mulle_allocator *allocator)
{
   void   *s;

   assert( ! _mulle__buffer_intersects_bytes( buffer, bytes, length));

   s = _mulle__buffer_advance( buffer, length, allocator);
   if( s)
      memmove( s, bytes, length);
}


static inline void   _mulle__buffer_add_string( struct mulle__buffer *buffer,
                                                char *bytes,
                                                struct mulle_allocator *allocator)
{
   char   c;

   assert( ! _mulle__buffer_intersects_bytes( buffer, bytes, strlen( bytes)));

   while( (c = *bytes++))
      _mulle__buffer_add_byte( buffer, c, allocator);
}


MULLE__BUFFER_GLOBAL
void   _mulle__buffer_add_string_if_empty( struct mulle__buffer *buffer,
                                           char *bytes,
                                           struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
void   _mulle__buffer_add_string_if_not_empty( struct mulle__buffer *buffer,
                                               char *bytes,
                                               struct mulle_allocator *allocator);


//
// TODO: move this to mulle-string or so
//
// strnlen is not C, it's POSIX according to Linux...
static inline size_t   _mulle_char_strnlen( char *s, size_t len)
{
   char   *start;
   char   *sentinel;

   start    = s;
   s        = &s[ -1];
   sentinel = &s[ len];

   while( s < sentinel)
      if( ! *++s)
         break;

   return( (size_t) (s - start));
}


static inline size_t
   _mulle__buffer_add_string_with_maxlength( struct mulle__buffer *buffer,
                                             char *bytes,
                                             size_t maxlength,
                                             struct mulle_allocator *allocator)
{
   char   c;
   char   *s;
   char   *sentinel;

   assert( ! _mulle__buffer_intersects_bytes( buffer,
                                              bytes,
                                              _mulle_char_strnlen( bytes, maxlength)));
   s        = bytes;
   sentinel = &s[ maxlength];
   while( s < sentinel)
   {
      if( ! (c = *s))
         break;

      _mulle__buffer_add_byte( buffer, c, allocator);
      ++s;
   }
   return( s - bytes);
}


static inline void   _mulle__buffer_memset( struct mulle__buffer *buffer,
                                            int c,
                                            size_t length,
                                            struct mulle_allocator *allocator)
{
   void   *s;

   s = _mulle__buffer_advance( buffer, length, allocator);
   if( s)
      memset( s, c, length);
}


static inline int   _mulle__buffer_memcmp( struct mulle__buffer *buffer,
                                           void  *bytes,
                                           size_t length)
{
   if( length > _mulle__buffer_get_length( buffer))
      return( 1);

   return( memcmp( buffer->_storage, bytes, length));
}


//
// a bit weird, but it is used to truncate or a append a 0 string
// but size is not adjusted, useful when retrieving the buffer and
// use it as cString, but the buffer is fixed size. You can then print
// the strlen or print the contents. You can then easily just append
// another string.
//
static inline int   _mulle__buffer_zero_last_byte( struct mulle__buffer *buffer)
{
   if( ! _mulle__buffer_is_void( buffer))
   {
      if( _mulle__buffer_has_overflown( buffer))
      {
         buffer->_curr[ -2] = 0;
         return( 2);
      }

      if( _mulle__buffer_is_full( buffer))
      {
         buffer->_curr[ -1] = 0;
         return( 1);
      }
      buffer->_curr[ 0] = 0;
   }
   return( 0);
}


static inline void    _mulle__buffer_add_buffer( struct mulle__buffer *buffer,
                                                 struct mulle__buffer *other,
                                                 struct mulle_allocator *allocator)
{
   assert( buffer != other);

   _mulle__buffer_add_bytes( buffer,
                            _mulle__buffer_get_bytes( other),
                            _mulle__buffer_get_length( other),
                            allocator);
}


MULLE__BUFFER_GLOBAL
void   _mulle__buffer_add_buffer_range( struct mulle__buffer *buffer,
                                        struct mulle__buffer *other,
                                        size_t offset,
                                        size_t length,
                                        struct mulle_allocator *allocator);


// zeroes if inflexible, otherwise adds
// returns number of bytes inflexible buffer truncated 
MULLE__BUFFER_GLOBAL
int   _mulle__buffer_make_string( struct mulle__buffer *buffer,
                                  struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
int  _mulle__buffer_flush( struct mulle__buffer *buffer);

MULLE__BUFFER_GLOBAL
int  _mulle__flushablebuffer_flush( struct mulle__flushablebuffer *ibuffer);


#pragma mark - reading

/*
 * Limited read support for inflexible buffers
 * -1 == no more bytes
 */
static inline int   _mulle__buffer_next_byte( struct mulle__buffer *buffer)
{
   if( _mulle__buffer_is_full( buffer))
      return( -1);
   return( *buffer->_curr++);
}


static inline int   _mulle__buffer_peek_byte( struct mulle__buffer *buffer)
{
   if( _mulle__buffer_is_full( buffer))
      return( -1);
   return( *buffer->_curr);
}


static inline void   *_mulle__buffer_reference_bytes( struct mulle__buffer *buffer,
                                                      size_t len)
{
   void   *start;

   if( ! _mulle__buffer_is_big_enough( buffer, len))
      return( NULL);

   start          = buffer->_curr;
   buffer->_curr += len;

   return( start);
}


static inline int   _mulle__buffer_next_bytes( struct mulle__buffer *buffer,
                                              void *buf,
                                              size_t len)
{
   if( ! _mulle__buffer_is_big_enough( buffer, len))
      return( -1);

   memmove( buf, buffer->_curr, len);
   buffer->_curr += len;
   return( 0);
}


static inline int   _mulle__buffer_next_character( struct mulle__buffer *buffer)
{
   if( _mulle__buffer_is_full( buffer))
      return( INT_MAX);
   return( (char) *buffer->_curr++);
}


/* like a seek forwards, if something found */
static inline int   _mulle__buffer_find_byte( struct mulle__buffer *buffer,
                                             unsigned char byte)
{
   unsigned char  *p;

   p = memchr( buffer->_curr, byte, buffer->_sentinel - buffer->_curr);
   if( ! p)
      return( -1);

   buffer->_curr = p;
   return( 0);
}


#pragma mark - copy out

MULLE__BUFFER_GLOBAL
void   _mulle__buffer_copy_range( struct mulle__buffer *buffer,
                                  size_t offset,
                                  size_t length,
                                  unsigned char *dst);


#pragma mark - backwards compatibility

MULLE_C_DEPRECATED static inline int
  _mulle__buffer_is_inflexable( struct mulle__buffer *buffer)
{
   return( _mulle__buffer_is_inflexible( buffer));
}


MULLE_C_DEPRECATED static inline void
  mulle__buffer_init_inflexable_with_static_bytes( struct mulle__buffer *buffer,
                                                   void *storage,
                                                   size_t length)
{
   _mulle__buffer_init_inflexible_with_static_bytes( buffer, storage, length);
}


MULLE_C_DEPRECATED static inline void
  _mulle__buffer_make_inflexable( struct mulle__buffer *buffer,
                                  void *_storage,
                                  size_t length,
                                  struct mulle_allocator *allocator)
{
   _mulle__buffer_make_inflexible( buffer, _storage, length, allocator);
}

#endif

