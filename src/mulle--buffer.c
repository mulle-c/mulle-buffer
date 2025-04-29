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


void
  _mulle__buffer_init_inflexible_with_static_bytes( struct mulle__buffer *buffer,
                                                    void *storage,
                                                    size_t length)
{
   assert( length != (size_t) -1);

   buffer->_initial_storage  =
   buffer->_storage          =
   buffer->_curr             = storage;
   buffer->_sentinel         = &buffer->_storage[ length];

   //
   // this is for mulle_sprintf, where we have API that gets storage, where
   // we don't know the size. We can't have buffer->_sentinel wrapping
   // around though for our tests. Of course the crazy sentinel value,
   // can't protect us from overwrites (but thats sprintf)
   //
   if( length == INT_MAX)
   {
      while( buffer->_sentinel < buffer->_storage)
      {
         length >>= 1;
         buffer->_sentinel = &buffer->_storage[ length];
      }
      buffer->_type = MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE;
   }
   else
      buffer->_type = MULLE_BUFFER_IS_INFLEXIBLE;
   buffer->_size = length;

   assert( buffer->_sentinel >= buffer->_storage);
}


long  _mulle__buffer_get_seek( struct mulle__buffer *buffer)
{
   long                           len;
   struct mulle_flushablebuffer   *flushable;

   len = (long) _mulle__buffer_get_length( buffer);
   if( ! _mulle__buffer_is_flushable( buffer))
      return( len);

   flushable = (struct mulle_flushablebuffer *) buffer;
   return( (long) (flushable->_flushed + len));
}


int   _mulle__buffer_set_seek( struct mulle__buffer *buffer, long seek, int mode)
{
   unsigned char   *plan;

   if( _mulle__buffer_is_flushable( buffer))
      return( -1);

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

   buffer->_curr = plan;
   return( 0);
}


char   *_mulle__buffer_get_string( struct mulle__buffer *buffer,
                                   struct mulle_allocator *allocator)
{
   if( _mulle__buffer_make_string( buffer, allocator) == 2)
      return( NULL);
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
   int  c;

   c = _mulle__buffer_get_last_byte( buffer);
   if( ! c)
      return( 0);

   if( _mulle__buffer_is_inflexible( buffer))
   {
      assert( ! _mulle__buffer_is_void( buffer));

      if( _mulle__buffer_is_full( buffer))
         return( _mulle__buffer_zero_last_byte( buffer));
   }

   _mulle__buffer_add_byte( buffer, 0, allocator);
   return( 0);
}


char   *_mulle__buffer_extract_string( struct mulle__buffer *buffer,
                                       struct mulle_allocator *allocator)
{
   // afterwards the size of the string will include the zero
   _mulle__buffer_make_string( buffer, allocator);
   // this will do nothing for inflexible data
   _mulle__buffer_size_to_fit( buffer, allocator);
   return( (char *) _mulle__buffer_extract_data( buffer, allocator).bytes);
}

void  _mulle__buffer_set_overflown( struct mulle__buffer *buffer)
{
   assert( ! _mulle__buffer_has_overflown( buffer));

   buffer->_size = _mulle__buffer_get_length( buffer);
   buffer->_curr = buffer->_sentinel + 1;  // set "overflowed"
}


int   _mulle__buffer_flush( struct mulle__buffer *buffer)
{
   struct mulle_flushablebuffer   *ibuffer;

   if( ! _mulle__buffer_is_flushable( buffer))
   {
      _mulle__buffer_set_overflown( buffer);
      return( -1);
   }

   ibuffer = (struct mulle_flushablebuffer *) buffer;
   return( _mulle_flushablebuffer_flush( ibuffer));
}


static size_t   _mulle__buffer_get_new_allocation_length( struct mulle__buffer *buffer,
                                                          size_t growth)
{
   size_t   plus;
   size_t   new_size;

   plus = MULLE_BUFFER_MIN_GROW_SIZE;
   if( growth > plus)
      plus = growth;

   //
   // ! buffer->_curr, buffer->size is capacity, which we should respect
   //
   if( ! buffer->_curr)
   {
      new_size = plus < buffer->_size ? buffer->_size : plus;
   }
   else
   {
      new_size  = _mulle__buffer_get_allocation_length( buffer);
      // at least double buffer->size
      new_size += plus < new_size ? new_size : plus;
   }
   return( new_size);
}


int   _mulle__buffer_grow( struct mulle__buffer *buffer,
                           size_t growth,
                           struct mulle_allocator *allocator)
{
   void     *malloc_block;
   void     *p;
   size_t   new_size;
   size_t   len;
   size_t   desired;

   if( _mulle__buffer_has_overflown( buffer))
      return( -1);

   if( _mulle__buffer_is_inflexible( buffer))
   {
      // stupid test border case
      if( ! growth)
         return( 0);

      desired = _mulle__buffer_get_remaining_length( buffer) + growth;
      //
      // this may or may not work, depending on its's being
      // flushable
      //
      if( _mulle__buffer_flush( buffer))
         return( -1);

      return( buffer->_size >= desired ? 0 : -1);
   }

   malloc_block = NULL;
   if( buffer->_storage != buffer->_initial_storage)
      malloc_block = buffer->_storage;

   //
   // assume realloc is slow enough, to warrant all this code :)
   //

   new_size          = _mulle__buffer_get_new_allocation_length( buffer, growth);
   len               = buffer->_curr - buffer->_storage;
   p                 = mulle_allocator_realloc( allocator, malloc_block, new_size);

   if( ! malloc_block)
      memcpy( p, buffer->_initial_storage, len);

   buffer->_storage  = p;
   buffer->_curr     = &buffer->_storage[ len];
   buffer->_sentinel = &buffer->_storage[ new_size];

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
   _mulle__buffer_init( buffer, MULLE_BUFFER_MIN_GROW_SIZE);
   return( buffer);
}


void   _mulle__buffer_destroy( struct mulle__buffer *buffer,
                               struct mulle_allocator *allocator)
{
   _mulle__buffer_done( buffer, allocator);
   mulle_allocator_free( allocator, buffer);
}


int   _mulle__buffer_set_length( struct mulle__buffer *buffer,
                                 size_t length,
                                 unsigned int options,
                                 struct mulle_allocator *allocator)
{
   long   diff;
   void   *reserved;

   diff = (long) length - (long) _mulle__buffer_get_length( buffer);
   // shrink ?
   if( diff <= 0)
   {
      if( _mulle__buffer_has_overflown( buffer))
         return( -1);
      if( ! diff)
         return( -1);

      buffer->_curr = &buffer->_curr[ diff];
      if( ! (options & 0x1))
         _mulle__buffer_size_to_fit( buffer, allocator);
      return( 0);
   }

   // grow
   reserved = _mulle__buffer_advance( buffer, (size_t) diff, allocator);
   if( ! reserved)
      return( -1);

   if( ! (options & 0x2))
      memset( reserved, 0, (size_t) diff);
   return( 0);
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
                                  void *dst)
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
//#ifndef __WIN32
//   case '\e' : _mulle__buffer_add_escaped_char( buffer, 'e', allocator); return;
//#endif
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

