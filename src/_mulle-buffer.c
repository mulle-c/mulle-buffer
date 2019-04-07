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

#include "_mulle-buffer.h"

#include "include-private.h"


#if DEBUG
# define MULLE_BUFFER_MIN_GROW_SIZE    4        // minimum realloc increment
#else
# define MULLE_BUFFER_MIN_GROW_SIZE    64
#endif


size_t  _mulle_buffer_get_seek( struct _mulle_buffer *buffer)
{
   size_t                          len;
   struct _mulle_flushablebuffer   *flushable;

   len = _mulle_buffer_get_length( buffer);
   if( ! _mulle_buffer_is_flushable( buffer))
      return( len);

   flushable = (struct _mulle_flushablebuffer *) buffer;
   return( flushable->_flushed + len);
}


int   _mulle_buffer_set_seek( struct _mulle_buffer *buffer, int mode, size_t seek)
{
   unsigned char   *plan;

   switch( mode)
   {
   case MULLE_BUFFER_SEEK_SET :
      plan = &buffer->_storage[ seek];
      break;

   case MULLE_BUFFER_SEEK_CUR :
      if( _mulle_buffer_has_overflown( buffer))
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

   assert( ! _mulle_buffer_is_flushable( buffer));

   buffer->_curr = plan;
   return( 0);
}


void   *_mulle_buffer_extract_all( struct _mulle_buffer *buffer,
                                   struct mulle_allocator *allocator)
{
   size_t   size;
   void     *block;

   block = buffer->_storage;

   if( block && block == buffer->_initial_storage)
   {
      size  = _mulle_buffer_get_length( buffer);
      block = _mulle_allocator_malloc( allocator, size);
      memcpy( block, buffer->_storage, size);

      buffer->_curr    =
      buffer->_storage = buffer->_initial_storage;

      return( block);
   }

   buffer->_storage          =
   buffer->_curr             =
   buffer->_sentinel         =
   buffer->_initial_storage  = NULL;

   return( block);
}


void    _mulle_buffer_size_to_fit( struct _mulle_buffer *buffer,
                                   struct mulle_allocator *allocator)
{
   size_t   length;
   void     *p;

   if( _mulle_buffer_is_inflexible( buffer))
      return;

   if( buffer->_storage == buffer->_initial_storage)
      return;

   length  = _mulle_buffer_get_length( buffer);

   p  = _mulle_allocator_realloc( allocator, buffer->_storage, sizeof( unsigned char) * length);

   buffer->_storage  = p;
   buffer->_curr     = &buffer->_storage[ length];
   buffer->_sentinel = &buffer->_storage[ length];
   buffer->_size     = length;
}


static inline void  _mulle_buffer_set_overflown( struct _mulle_buffer *buffer)
{
   if( buffer->_curr <= buffer->_sentinel)
      buffer->_curr = buffer->_sentinel + 1;  // set "overflowed"
}

int  _mulle_flushablebuffer_flush( struct _mulle_flushablebuffer *ibuffer)
{
   size_t   len;

   len = ibuffer->_curr - ibuffer->_storage;
   if( ! len)
      return( 0);

   if( len != (*ibuffer->_flusher)( ibuffer->_storage, 1, len, ibuffer->_userinfo))
   {
      _mulle_buffer_set_overflown( (struct _mulle_buffer *) ibuffer);

      ibuffer->_curr = ibuffer->_sentinel + 1;  // set "overflowed"
      return( -2);
   }

   ibuffer->_flushed += len;
   ibuffer->_curr     = ibuffer->_storage;

   return( 0);
}


int   _mulle_buffer_flush( struct _mulle_buffer *buffer)
{
   struct _mulle_flushablebuffer  *ibuffer;

   if( ! _mulle_buffer_is_flushable( buffer))
   {
      _mulle_buffer_set_overflown( buffer);
      return( -1);
   }

   ibuffer = (struct _mulle_flushablebuffer *) buffer;
   return( _mulle_flushablebuffer_flush( ibuffer));
}


static size_t   _mulle_buffer_grow_size( struct _mulle_buffer *buffer,
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


int   _mulle_buffer_grow( struct _mulle_buffer *buffer,
                          size_t min_amount,
                          struct mulle_allocator *allocator)
{
   void     *malloc_block;
   void     *p;
   size_t   new_size;
   size_t   len;

   assert( min_amount);
   if( _mulle_buffer_is_inflexible( buffer))
   {
      if( _mulle_buffer_get_size( buffer) < min_amount)
         return( -1);

      //
      // this may or may not work, depending on its's being
      // flushable
      //
      assert( buffer->_curr); // for the analyzer
      return( _mulle_buffer_flush( buffer));
   }

   malloc_block = NULL;
   if( buffer->_storage != buffer->_initial_storage)
      malloc_block = buffer->_storage;

   //
   // assume realloc is slow enough, to warrant all this code :)
   //

   new_size          = _mulle_buffer_grow_size( buffer, min_amount);
   len               = buffer->_curr - buffer->_storage;
   p                 = _mulle_allocator_realloc( allocator, malloc_block, new_size);

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
void   _mulle_buffer_make_inflexible( struct _mulle_buffer *buffer,
                                      void *buf,
                                      size_t length,
                                      struct mulle_allocator *allocator)
{
   if( buffer->_storage != buffer->_initial_storage)
      _mulle_allocator_free( allocator, buffer->_storage);

   buffer->_storage         =
   buffer->_initial_storage = buf;

   buffer->_curr            =
   buffer->_sentinel        = &buffer->_storage[ length];
   buffer->_size            = length;
   buffer->_type            = MULLE_BUFFER_IS_INFLEXIBLE;
}


void   _mulle_buffer_done( struct _mulle_buffer *buffer,
                           struct mulle_allocator *allocator)
{
   if( buffer->_storage != buffer->_initial_storage)
      _mulle_allocator_free( allocator, buffer->_storage);
}


struct _mulle_buffer   *_mulle_buffer_create( struct mulle_allocator *allocator)
{
   struct _mulle_buffer  *buffer;

   buffer = _mulle_allocator_malloc( allocator, sizeof( struct _mulle_buffer));
   _mulle_buffer_init( buffer);
   return( buffer);
}


void   _mulle_buffer_destroy( struct _mulle_buffer *buffer,
                           struct mulle_allocator *allocator)
{
   _mulle_buffer_done( buffer, allocator);
   _mulle_allocator_free( allocator, buffer);
}


size_t   _mulle_buffer_set_length( struct _mulle_buffer *buffer,
                                   size_t length,
                                   struct mulle_allocator *allocator)
{
   long   diff;

   diff = (long) length - (long) _mulle_buffer_get_length( buffer);
   if( diff <= 0)
   {
      if( ! _mulle_buffer_has_overflown( buffer))
      {
         buffer->_curr = &buffer->_curr[ diff];
         _mulle_buffer_size_to_fit( buffer, allocator);
      }
      return( 0);
   }

   _mulle_buffer_memset( buffer, 0, (size_t) diff, allocator);
   return( (size_t) diff);
}


void   _mulle_buffer_zero_to_length( struct _mulle_buffer *buffer,
                                    size_t length,
                                    struct mulle_allocator *allocator)
{
   long   diff;

   diff = _mulle_buffer_set_length( buffer, length, allocator);
   memset( &buffer->_curr[ -diff], 0, (size_t) diff);
}


void   _mulle_buffer_add_buffer_range( struct _mulle_buffer *buffer,
                                       struct _mulle_buffer *other,
                                       size_t offset,
                                       size_t length,
                                       struct mulle_allocator *allocator)
{
   assert( buffer != other);

   unsigned char   *start;
   unsigned char   *sentinel;

   start    = _mulle_buffer_get_bytes( other);
   sentinel = &start[ _mulle_buffer_get_length( other)];

   start    = &start[ offset];
   if( &start[ length] <= sentinel)
      _mulle_buffer_add_bytes( buffer,
                               start,
                               length,
                               allocator);
}



void   _mulle_buffer_copy_range( struct _mulle_buffer *buffer,
                                 size_t offset,
                                 size_t length,
                                 unsigned char *dst)
{
   unsigned char   *start;
   unsigned char   *sentinel;

   start    = _mulle_buffer_get_bytes( buffer);
   sentinel = &start[ _mulle_buffer_get_length( buffer)];

   start    = &start[ offset];
   if( &start[ length] <= sentinel)
      memmove( dst, start, length);
}
