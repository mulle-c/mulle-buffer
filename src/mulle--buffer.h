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

#ifndef mulle__buffer_h__
#define mulle__buffer_h__

#include "include.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


struct mulle_allocator;


enum
{
  MULLE_BUFFER_IS_FLEXIBLE           = 0,
  MULLE_BUFFER_IS_INFLEXIBLE         = 1,
  MULLE_BUFFER_IS_FLUSHABLE          = 2,
  MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE = 3  // sentinel is not trustworthy
};


//
// this is the base for MULLE_BUFFER, but it does not contain an allocator
// and none of the functions check for NULL pointers
//
// _size is there to keep "capacity" for initial allocation (could be
// optimized away, if we don't need capacity) or remember what the size of
// _initial_storage is. It is never modified unless you run into the overflow
// condition, then the length before the overflow is kept there.
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

#define MULLE__BUFFER_FLEXIBLE_DATA( data, len)    \
   ((struct mulle__buffer)                         \
   {                                               \
      (unsigned char *) data,                      \
      (unsigned char *) data,                      \
      &((unsigned char *) data)[ (len)],           \
      (unsigned char *) data,                      \
      (len),                                       \
      MULLE_BUFFER_IS_FLEXIBLE                     \
   })

#define MULLE__BUFFER_INFLEXIBLE_DATA( data, len)  \
   ((struct mulle__buffer)                         \
   {                                               \
      (unsigned char *) data,                      \
      (unsigned char *) data,                      \
      &((unsigned char *) data)[ (len)],           \
      (unsigned char *) data,                      \
      (len),                                       \
      MULLE_BUFFER_IS_INFLEXIBLE                   \
   })



#pragma mark - creation destruction

MULLE__BUFFER_GLOBAL
struct mulle__buffer   *_mulle__buffer_create( struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
void                   _mulle__buffer_destroy( struct mulle__buffer *buffer,
                                               struct mulle_allocator *allocator);


#pragma mark - initialization


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

   assert( buffer->_sentinel >= buffer->_storage);
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

   assert( buffer->_sentinel >= buffer->_storage);
}


static inline void   _mulle__buffer_init( struct mulle__buffer *buffer,
                                          size_t capacity)
{
   buffer->_initial_storage  =
   buffer->_curr             =
   buffer->_storage          =
   buffer->_sentinel         = NULL;
   buffer->_size             = capacity >> 1;  // will grow to double _size
   buffer->_type             = MULLE_BUFFER_IS_FLEXIBLE;
}


MULLE__BUFFER_GLOBAL
MULLE_C_NONNULL_FIRST
void
  _mulle__buffer_init_inflexible_with_static_bytes( struct mulle__buffer *buffer,
                                                    void *storage,
                                                    size_t length);


MULLE__BUFFER_GLOBAL
MULLE_C_NONNULL_FIRST
void
  _mulle__buffer_init_with_const_bytes( struct mulle__buffer *buffer,
                                        const void *storage,
                                        size_t length);


static inline void   _mulle__buffer_done( struct mulle__buffer *buffer,
                                          struct mulle_allocator *allocator)
{
   if( buffer->_storage != buffer->_initial_storage)
      mulle_allocator_free( allocator, buffer->_storage);
#ifdef DEBUG
   mulle_memset_uint32( buffer, 0xDEADDEAD, sizeof( struct mulle__buffer));
#endif
}


// _initial_storage will be lost
static inline void   _mulle__buffer_reset( struct mulle__buffer *buffer,
                                           struct mulle_allocator *allocator)
{
   _mulle__buffer_done( buffer, allocator);
   _mulle__buffer_init( buffer, 0);
}


#pragma mark - change buffer type

MULLE__BUFFER_GLOBAL
void   _mulle__buffer_make_inflexible( struct mulle__buffer *buffer,
                                      void *storage,
                                      size_t length,
                                      struct mulle_allocator *allocator);


static inline int   _mulle__buffer_has_overflown( struct mulle__buffer *buffer)
{
   return( buffer->_curr > buffer->_sentinel);
}


/**
 * Sets the buffer's overflown state.
 *
 * This internal function is used to mark the buffer as overflown, indicating that
 * its capacity has been exceeded. It sets the current pointer to a position past
 * the sentinel, signaling that the buffer is in an overflown state.
 *
 * @param buffer The buffer to set as overflown.
 */
MULLE__BUFFER_GLOBAL
MULLE_C_NONNULL_FIRST
void  _mulle__buffer_set_overflown( struct mulle__buffer *buffer);



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
int   _mulle__buffer_set_length( struct mulle__buffer *buffer,
                                 size_t length,
                                 unsigned int options,
                                 struct mulle_allocator *allocator);


static inline void   *_mulle__buffer_guarantee( struct mulle__buffer *buffer,
                                                size_t length,
                                                struct mulle_allocator *allocator)
{
   ptrdiff_t   missing;

   if( _mulle__buffer_has_overflown( buffer))
      return( NULL);

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

// clears overflown
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


static inline size_t   _mulle__buffer_get_allocation_length( struct mulle__buffer *buffer)
{
   return( buffer->_sentinel - buffer->_storage);
}


static inline size_t   _mulle__buffer_get_remaining_length( struct mulle__buffer *buffer)
{
   return( _mulle__buffer_has_overflown( buffer) ? 0 : buffer->_sentinel - buffer->_curr);
}


static inline int   _mulle__buffer_is_big_enough( struct mulle__buffer *buffer,
                                                  size_t len)
{
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


static inline size_t   _mulle__buffer_get_length( struct mulle__buffer *buffer)
{
   return( _mulle__buffer_has_overflown( buffer)
           ? buffer->_size
           : (size_t) (buffer->_curr - buffer->_storage));
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
   MULLE_BUFFER_SEEK_END = 2     // will dial to "size", hardly ever use
};

MULLE__BUFFER_GLOBAL
long   _mulle__buffer_get_seek( struct mulle__buffer *buffer);

MULLE__BUFFER_GLOBAL
int      _mulle__buffer_set_seek( struct mulle__buffer *buffer, long seek, int mode);



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


static inline void   *_mulle__buffer_get_bytes( struct mulle__buffer *buffer)
{
   return( buffer->_storage);
}


static inline struct mulle_data   _mulle__buffer_get_data( struct mulle__buffer *buffer)
{
   struct mulle_data   data;

   data.bytes  = _mulle__buffer_get_bytes( buffer);
   data.length = _mulle__buffer_get_length( buffer);

   return( data);
}


static inline size_t   _mulle__buffer_get_staticlength( struct mulle__buffer *buffer)
{
   return( buffer->_storage == buffer->_initial_storage
          ? _mulle__buffer_get_length( buffer)
          : 0);
}


static inline size_t   _mulle__buffer_get_staticsize( struct mulle__buffer *buffer)
{
   return( buffer->_storage == buffer->_initial_storage
          ? buffer->_size
          : 0);
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



static inline void    _mulle__buffer_remove_last_byte( struct mulle__buffer *buffer)
{
   assert( ! _mulle__buffer_is_empty( buffer));
   assert( ! _mulle__buffer_has_overflown( buffer));

   --buffer->_curr;
}


static inline int    _mulle__buffer_pop_byte( struct mulle__buffer *buffer,
                                              struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator); // future
   // no longer overflown I guess
   if( _mulle__buffer_has_overflown( buffer))
      buffer->_curr = buffer->_sentinel;
   if( _mulle__buffer_is_empty( buffer))
      return( -1);

   --buffer->_curr;
   return( *buffer->_curr);
}


void   _mulle__buffer_remove_in_range( struct mulle__buffer *buffer,
                                       struct mulle_range range);


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

   // so if the buffer is MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE (which it NEVER
   // should be except when we are doing actually a sprintf implementation)
   // the concept of intersection becomes meaningless, as the sentinel is
   // way off
   if( ! length || (buffer->_type & MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE) == MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE)
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



//
// produces C escape code but does not wrap in ""
//
MULLE__BUFFER_GLOBAL
void   _mulle__buffer_add_c_char( struct mulle__buffer *buffer,
                                  char c,
                                  struct mulle_allocator *allocator);

//
// produces C escape codes but does not wrap in ""
//
static inline void   _mulle__buffer_add_c_chars( struct mulle__buffer *buffer,
                                                 char *s,
                                                 size_t length,
                                                 struct mulle_allocator *allocator)
{
   char   *sentinel;

   assert( ! _mulle__buffer_intersects_bytes( buffer, s, length));

   sentinel = &s[ length];
   while( s < sentinel)
      _mulle__buffer_add_c_char( buffer, *s++, allocator);
}


//
// produces C escape codes and wraps everything in ""
//
MULLE__BUFFER_GLOBAL
void   _mulle__buffer_add_c_string( struct mulle__buffer *buffer,
                                    char *s,
                                    struct mulle_allocator *allocator);

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



//
// a bit weird, but it is used to truncate or a append a 0 string
// but size is not adjusted. Useful when retrieving the buffer and
// use it as cString, but the buffer is fixed size. You can then print
// the strlen or print the contents. You can then easily just append
// another string.
//
// returns 0 : last byte is zeroed no loss
//         1 : last byte zeroed (lossy!, potentially corrupting UTF8)
//         2 : is NULL
//
static inline int   _mulle__buffer_zero_last_byte( struct mulle__buffer *buffer)
{
   unsigned char   *p;
   int             rval;

   if( _mulle__buffer_is_void( buffer))
      return( 2); // NULL

   rval = 0;
   p    = buffer->_curr;

   if( _mulle__buffer_has_overflown( buffer))
      p = &buffer->_storage[ buffer->_size];

   if( p == buffer->_sentinel)
   {
      --p;
      rval = 1;  // truncate
   }
   *p = 0;
   return( rval);
}


static inline int   _mulle__buffer_zero_last_byte_no_truncate( struct mulle__buffer *buffer)
{
   unsigned char   *p;
   int             rval;

   if( _mulle__buffer_is_void( buffer))
      return( 2); // NULL

   rval = 0;
   p    = buffer->_curr;

   if( _mulle__buffer_has_overflown( buffer))
      p = &buffer->_storage[ buffer->_size];

   if( p == buffer->_sentinel)
      return( 1);

   *p = 0;
   return( rval);
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
                                        struct mulle_range range,
                                        struct mulle_allocator *allocator);


//
// returns 0 : no truncation
//         1 : truncated! (e.g. possibly UTF8 corruption
//         2 : buffer is void (NULL)
//
MULLE__BUFFER_GLOBAL
int   _mulle__buffer_make_string( struct mulle__buffer *buffer,
                                  struct mulle_allocator *allocator);

MULLE__BUFFER_GLOBAL
int  _mulle__buffer_flush( struct mulle__buffer *buffer);



#pragma mark - reading


// returns -1 if not a byte
static inline int   _mulle__buffer_get_byte( struct mulle__buffer *buffer, unsigned int index)
{
   if( &buffer->_storage[ index] >= buffer->_sentinel)
      return( -1);
   return( ((uint8_t *) buffer->_storage)[ index]);
}


// returns -1 if not a word
static inline int   _mulle__buffer_get_uint16( struct mulle__buffer *buffer, unsigned int index)
{
   // we access 1 additional bytes after index, so gotta check that as well
   if( &((uint16_t *) buffer->_storage)[ index * sizeof( uint32_t)] > &((uint16_t *) buffer->_sentinel)[ -1])
      return( -1);
   return( ((uint16_t *) buffer->_storage)[ index * sizeof( uint32_t)]);
}


// returns -1 if not a word
static inline int64_t   _mulle__buffer_get_uint32( struct mulle__buffer *buffer, unsigned int index)
{
   // we access 3 additional bytes after index, so gotta check that as well
   if( &((uint32_t *) buffer->_storage)[ index * sizeof( uint32_t)] > &((uint32_t *) buffer->_sentinel)[ -1])
      return( -1);
   return( ((uint32_t *) buffer->_storage)[ index * sizeof( uint32_t)]);
}



static inline int    _mulle__buffer_get_last_byte( struct mulle__buffer *buffer)
{
   size_t   length;

   length = _mulle__buffer_get_length( buffer);
   if( ! length)
      return( -1);

   return( buffer->_storage[ length - 1]);
}

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

   assert( buf || ! len);

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


static inline int   _mulle__buffer_memcmp( struct mulle__buffer *buffer,
                                           void  *bytes,
                                           size_t length)
{
   if( length > _mulle__buffer_get_length( buffer))
      return( 1);

   return( memcmp( buffer->_storage, bytes, length));
}



/* like a seek forwards, if something found */
static inline long   _mulle__buffer_seek_byte( struct mulle__buffer *buffer,
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

// or name get_bytes_in_range ?
MULLE__BUFFER_GLOBAL
void   _mulle__buffer_copy_range( struct mulle__buffer *buffer,
                                  struct mulle_range range,
                                  void *dst);


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

