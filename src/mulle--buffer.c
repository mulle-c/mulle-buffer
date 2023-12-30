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

#include "mulle--buffer.h"

#include "include-private.h"

#include <ctype.h>

#include "mulle-flushablebuffer.h"


#if DEBUG
# define MULLE_BUFFER_MIN_GROW_SIZE    4        // minimum realloc increment
#else
# define MULLE_BUFFER_MIN_GROW_SIZE    64
#endif

long  _mulle__buffer_get_seek( struct mulle__buffer *buffer)
{
   long                           len;
   struct mulle_flushablebuffer   *flushable;

   len = _mulle__buffer_get_length( buffer);
   if( ! _mulle__buffer_is_flushable( buffer))
      return( len);

   flushable = (struct mulle_flushablebuffer *) buffer;
   return( flushable->_flushed + len);
}


int   _mulle__buffer_set_seek( struct mulle__buffer *buffer, int mode, long seek)
{
   unsigned char   *plan;

   switch( mode)
   {
   case MULLE_BUFFER_SEEK_SET :
      plan = &buffer->_storage[ seek];
      break;

   case MULLE_BUFFER_SEEK_CUR :
      if( _mulle__buffer_has_overflown( buffer))
         seek -= 1;
      plan = &buffer->_curr[ seek];
      break;

   case MULLE_BUFFER_SEEK_END :
      plan = &buffer->_sentinel[ -seek];
      break;

   default :
      return( -1);
   }

   if( plan > buffer->_sentinel || plan < buffer->_storage)
      return( -1);

   assert( ! _mulle__buffer_is_flushable( buffer));

   buffer->_curr = plan;
   return( 0);
}


char   *_mulle__buffer_get_string( struct mulle__buffer *buffer,
                                   struct mulle_allocator *allocator)
{
   _mulle__buffer_make_string( buffer, allocator);
   return( (char *) _mulle__buffer_get_bytes( buffer));
}


void    _mulle__buffer_size_to_fit( struct mulle__buffer *buffer,
                                    struct mulle_allocator *allocator)
{
   size_t   length;
   void     *p;

   if( _mulle__buffer_is_inflexible( buffer))
      return;

   if( buffer->_storage == buffer->_initial_storage)
      return;

   length = _mulle__buffer_get_length( buffer);
   p      = mulle_allocator_realloc_strict( allocator,
                                            buffer->_storage,
                                            sizeof( unsigned char) * length);

   buffer->_storage  = p;
   buffer->_curr     = &buffer->_storage[ length];
   buffer->_sentinel = &buffer->_storage[ length];
   buffer->_size     = length;
}


struct mulle_data   _mulle__buffer_extract_data( struct mulle__buffer *buffer,
                                                 struct mulle_allocator *allocator)
{
   struct mulle_data   data;

   data.bytes  = NULL;
   data.length = _mulle__buffer_get_length( buffer);

   if( data.length)
   {
      data.bytes = buffer->_storage;
      if( data.bytes == buffer->_initial_storage)
      {
         data.bytes = mulle_allocator_malloc( allocator, data.length);
         memcpy( data.bytes, buffer->_storage, data.length);

         buffer->_curr    =
         buffer->_storage = buffer->_initial_storage;

         return( data);
      }
   }

   buffer->_storage         =
   buffer->_curr            =
   buffer->_sentinel        =
   buffer->_initial_storage = NULL;

   return( data);
}


int   _mulle__buffer_make_string( struct mulle__buffer *buffer,
                                  struct mulle_allocator *allocator)
{
   if( _mulle__buffer_is_inflexible( buffer))
      return( _mulle__buffer_zero_last_byte( buffer));
   if( _mulle__buffer_is_empty( buffer) || _mulle__buffer_get_last_byte( buffer))
      _mulle__buffer_add_byte( buffer, 0, allocator);
   return( 0);
}


char   *_mulle__buffer_extract_string( struct mulle__buffer *buffer,
                                       struct mulle_allocator *allocator)
{
  _mulle__buffer_make_string( buffer, allocator);
  _mulle__buffer_size_to_fit( buffer, allocator);
  return( (char *) _mulle__buffer_extract_data( buffer, allocator).bytes);
}


int   _mulle__buffer_flush( struct mulle__buffer *buffer)
{
   struct mulle_flushablebuffer  *ibuffer;

   if( ! _mulle__buffer_is_flushable( buffer))
   {
      _mulle__buffer_set_overflown( buffer);
      return( -1);
   }

   ibuffer = (struct mulle_flushablebuffer *) buffer;
   return( _mulle_flushablebuffer_flush( ibuffer));
}


static size_t   _mulle__buffer_grow_size( struct mulle__buffer *buffer,
                                          size_t min_amount)
{
   size_t   plus;
   size_t   new_size;

   plus = MULLE_BUFFER_MIN_GROW_SIZE;
   if( min_amount > plus)
      plus = min_amount;

   //
   // ! buffer->_curr,  buffer->size is capacity, which we should respect
   //
   if( ! buffer->_curr)
   {
      if( plus < buffer->_size)
         new_size = buffer->_size;
      else
         new_size = plus;
   }
   else
   {
      // at least double buffer->size
      if( plus < buffer->_size)
         plus = buffer->_size;
      new_size = buffer->_size + plus;
   }
   assert( new_size >= min_amount);
   return( new_size);
}


int   _mulle__buffer_grow( struct mulle__buffer *buffer,
                          size_t min_amount,
                          struct mulle_allocator *allocator)
{
   void     *malloc_block;
   void     *p;
   size_t   new_size;
   size_t   len;

   if( _mulle__buffer_has_overflown( buffer))
      return( -1);

   assert( min_amount);
   if( _mulle__buffer_is_inflexible( buffer))
   {
      if( _mulle__buffer_get_size( buffer) < min_amount)
      {
         buffer->_curr = buffer->_sentinel + 1; // mark as overflown
         return( -1);
      }

      //
      // this may or may not work, depending on its's being
      // flushable
      //
      assert( buffer->_curr); // for the analyzer
      return( _mulle__buffer_flush( buffer));
   }

   malloc_block = NULL;
   if( buffer->_storage != buffer->_initial_storage)
      malloc_block = buffer->_storage;

   //
   // assume realloc is slow enough, to warrant all this code :)
   //

   new_size          = _mulle__buffer_grow_size( buffer, min_amount);
   len               = buffer->_curr - buffer->_storage;
   p                 = mulle_allocator_realloc( allocator, malloc_block, new_size);

   if( ! malloc_block)
      memcpy( p, buffer->_initial_storage, len);

   buffer->_storage  = p;
   buffer->_curr     = &buffer->_storage[ len];
   buffer->_sentinel = &buffer->_storage[ new_size];
   buffer->_size     = new_size;

   assert( buffer->_sentinel >= buffer->_curr);

   return( 0);
}


//
// this transforms a buffer
// into a inflexibleBuffer
//
void   _mulle__buffer_make_inflexible( struct mulle__buffer *buffer,
                                      void *buf,
                                      size_t length,
                                      struct mulle_allocator *allocator)
{
   if( buffer->_storage != buffer->_initial_storage)
      mulle_allocator_free( allocator, buffer->_storage);

   buffer->_storage         =
   buffer->_initial_storage = buf;

   buffer->_curr            =
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_BUFFER_IS_INFLEXIBLE;
}


struct mulle__buffer   *_mulle__buffer_create( struct mulle_allocator *allocator)
{
   struct mulle__buffer  *buffer;

   buffer = mulle_allocator_malloc( allocator, sizeof( struct mulle__buffer));
   _mulle__buffer_init( buffer);
   return( buffer);
}


void   _mulle__buffer_destroy( struct mulle__buffer *buffer,
                               struct mulle_allocator *allocator)
{
   _mulle__buffer_done( buffer, allocator);
   mulle_allocator_free( allocator, buffer);
}


size_t   _mulle__buffer_set_length( struct mulle__buffer *buffer,
                                    size_t length,
                                    struct mulle_allocator *allocator)
{
   long   diff;

   diff = (long) length - (long) _mulle__buffer_get_length( buffer);
   if( diff <= 0)
   {
      if( ! _mulle__buffer_has_overflown( buffer))
      {
         buffer->_curr = &buffer->_curr[ diff];
         _mulle__buffer_size_to_fit( buffer, allocator);
      }
      return( 0);
   }

   if( _mulle__buffer_advance( buffer, (size_t) diff, allocator))
      return( (size_t) diff);
   return( 0);
}


void   _mulle__buffer_zero_to_length( struct mulle__buffer *buffer,
                                      size_t length,
                                      struct mulle_allocator *allocator)
{
   long   diff;

   diff = _mulle__buffer_set_length( buffer, length, allocator);
   memset( &buffer->_curr[ -diff], 0, (size_t) diff);
}


void   _mulle__buffer_add_buffer_range( struct mulle__buffer *buffer,
                                        struct mulle__buffer *other,
                                        struct mulle_range range,
                                        struct mulle_allocator *allocator)
{
   assert( buffer != other);  // you could do it though, but whats the point

   unsigned char   *start;

   range = mulle_range_validate_against_length( range, _mulle__buffer_get_length( other));
   start = _mulle__buffer_get_bytes( other);
   start = &start[ range.location];
   _mulle__buffer_add_bytes( buffer, start, range.length, allocator);
}


void   _mulle__buffer_copy_range( struct mulle__buffer *buffer,
                                  struct mulle_range range,
                                  unsigned char *dst)
{
   unsigned char   *start;

   range = mulle_range_validate_against_length( range, _mulle__buffer_get_length( buffer));
   start = _mulle__buffer_get_bytes( buffer);
   start = &start[ range.location];
   memmove( dst, start, range.length);
}


void   _mulle__buffer_remove_in_range( struct mulle__buffer *buffer,
                                       struct mulle_range range)
{
   unsigned char   *start;
   unsigned char   *end;

   range = mulle_range_validate_against_length( range, _mulle__buffer_get_length( buffer));
   if( ! range.length)
      return;

   start = _mulle__buffer_get_bytes( buffer);
   start = &start[ range.location];
   end   = &start[ range.length];

   if( end != buffer->_curr)
      memmove( start, end, buffer->_sentinel - end);

   buffer->_curr -= range.length;
}



void   _mulle__buffer_add_string_if_empty( struct mulle__buffer *buffer,
                                           char *bytes,
                                           struct mulle_allocator *allocator)
{
   if( ! _mulle__buffer_get_length( buffer))
      _mulle__buffer_add_string( buffer, bytes, allocator);
}


void   _mulle__buffer_add_string_if_not_empty( struct mulle__buffer *buffer,
                                               char *bytes,
                                               struct mulle_allocator *allocator)
{
   if( _mulle__buffer_get_length( buffer))
      _mulle__buffer_add_string( buffer, bytes, allocator);
}





static inline char   tooct( int v)
{
   return( '0' + v);
}


//
// everything a little more complicated, because I prefer hex code
// but if you have 0xfe and then a hex char follows, the compiler sees this
// as more hex code :( If this happens use octal instead.
//
static void  _mulle__buffer_add_octal( struct mulle__buffer *buffer,
                                       char c,
                                       struct mulle_allocator *allocator)
{
   _mulle__buffer_add_byte( buffer, '\\', allocator);
   _mulle__buffer_add_byte( buffer, tooct( ((unsigned char) c >> 6) & 0x7), allocator);
   _mulle__buffer_add_byte( buffer, tooct( ((unsigned char) c >> 3) & 0x7), allocator);
   _mulle__buffer_add_byte( buffer, tooct( (unsigned char) c & 0x7), allocator);
}


static void   _mulle__buffer_add_escaped_char( struct mulle__buffer *buffer,
                                               char c,
                                               struct mulle_allocator *allocator)
{
   _mulle__buffer_add_byte( buffer, '\\', allocator);
   _mulle__buffer_add_byte( buffer, c, allocator);
}


void   _mulle__buffer_add_c_char( struct mulle__buffer *buffer,
                                  char c,
                                  struct mulle_allocator *allocator)
{
   switch( c)
   {
   case '\r' : _mulle__buffer_add_escaped_char( buffer, 'r', allocator); return;
   case '\n' : _mulle__buffer_add_escaped_char( buffer, 'n', allocator); return;
   case '"'  : _mulle__buffer_add_escaped_char( buffer, '"', allocator); return;
   case '?'  : _mulle__buffer_add_escaped_char( buffer, '?', allocator); return;
   case '\\' : _mulle__buffer_add_escaped_char( buffer, '\\', allocator); return;
   case '\a' : _mulle__buffer_add_escaped_char( buffer, 'a', allocator); return;
   case '\b' : _mulle__buffer_add_escaped_char( buffer, 'b', allocator); return;
   case '\e' : _mulle__buffer_add_escaped_char( buffer, 'e', allocator); return;
   case '\f' : _mulle__buffer_add_escaped_char( buffer, 'f', allocator); return;
   case '\t' : _mulle__buffer_add_escaped_char( buffer, 't', allocator); return;
   case '\v' : _mulle__buffer_add_escaped_char( buffer, 'v', allocator); return;
   default   :
      ;
   }
   if( isprint( c))
      _mulle__buffer_add_char( buffer, c, allocator);
   else
      _mulle__buffer_add_octal( buffer, c, allocator);
}


void   _mulle__buffer_add_c_string( struct mulle__buffer *buffer,
                                    char *bytes,
                                   struct mulle_allocator *allocator)
{
   char   c;

   assert( ! _mulle__buffer_intersects_bytes( buffer, bytes, strlen( bytes)));

   _mulle__buffer_add_byte( buffer, '"', allocator);
   while( (c = *bytes++))
      _mulle__buffer_add_c_char( buffer, c, allocator);
   _mulle__buffer_add_byte( buffer, '"', allocator);
}

