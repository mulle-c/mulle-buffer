//
//  mulle_buffer.c
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
#include "mulle-buffer.h"

#include "include-private.h"

#include <ctype.h>


struct mulle_buffer   *mulle_buffer_create( struct mulle_allocator *allocator)
{
   struct mulle_buffer  *buffer;

   buffer = mulle_buffer_alloc( allocator);
   mulle_buffer_init( buffer, MULLE_BUFFER_DEFAULT_CAPACITY, allocator);
   return( buffer);
}


void   mulle_buffer_reset( struct mulle_buffer *buffer)
{
   struct mulle_allocator   *allocator;

   allocator = mulle_buffer_get_allocator( buffer);
   mulle_buffer_done( buffer);
   mulle_buffer_init( buffer, 0, allocator);
}

int   _mulle_buffer_zero_last_byte( struct mulle_buffer *buffer)
{
   return( _mulle__buffer_zero_last_byte( (struct mulle__buffer *) buffer));
}


void    _mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                  struct mulle_buffer *other)
{
   _mulle__buffer_add_buffer( (struct mulle__buffer *) buffer,
                              (struct mulle__buffer *) other,
                              mulle_buffer_get_allocator( buffer));
}



void
  mulle_buffer_init_with_const_bytes( struct mulle_buffer *buffer,
                                      const void *storage,
                                      size_t length)
{
   assert( length != (size_t) -1);

   buffer->_initial_storage  =
   buffer->_storage          =
   buffer->_curr             = (void *) storage;
   buffer->_sentinel         = &buffer->_storage[ length];

   // no special case for mulle_sprintf
   assert( length != INT_MAX);

   buffer->_size             = length;
   buffer->_type             = MULLE_BUFFER_IS_INFLEXIBLE|MULLE_BUFFER_IS_READONLY;
   buffer->_allocator        = NULL;

   assert( buffer->_sentinel >= buffer->_storage);
}


void   mulle_buffer_add_bytes_callback( void *buffer,
                                        void *bytes,
                                        size_t length)
{
   mulle_buffer_add_bytes( buffer, bytes, length);
}


void   mulle_buffer_add_c_chars_callback( void *buffer,
                                          void *bytes,
                                          size_t length)
{
   mulle_buffer_add_c_chars( buffer, bytes, length);
}



//
// hex dump
//

static inline unsigned int   hex( unsigned int c)
{
   assert( c <= 0xf);
   return( c >= 0xa ? c + 'a' - 0xa : c + '0');
}


void   mulle_buffer_hexdump_line( struct mulle_buffer *buffer,
                                  void *bytes,
                                  unsigned int n,
                                  size_t counter,
                                  unsigned int options)
{
   uint8_t        *memo;
   uint8_t        *s;
#ifndef NDEBUG
   uint8_t        *sentinel;
#endif
   unsigned int   i;
   unsigned int   value;
   uint8_t        *p;
   uint32_t       adr;     // limited to 32 bit currently

   memo = bytes;
   p    = bytes;
   adr  = (uint32_t) counter;

   if( ! (options & mulle_buffer_hexdump_no_offset))
   {
      s = mulle_buffer_advance( buffer, 10);
      assert( s);
      if( s)
      {
#ifndef NDEBUG
         sentinel = &s[ 10];
#endif
         *s++ = (uint8_t) hex( adr >> 28 & 0xF);
         *s++ = (uint8_t) hex( adr >> 24 & 0xF);
         *s++ = (uint8_t) hex( adr >> 20 & 0xF);
         *s++ = (uint8_t) hex( adr >> 16 & 0xF);

         *s++ = (uint8_t) hex( adr >> 12 & 0xF);
         *s++ = (uint8_t) hex( adr >> 8 & 0xF);
         *s++ = (uint8_t) hex( adr >> 4 & 0xF);
         *s++ = (uint8_t) hex( adr >> 0 & 0xF);

         *s++ = ' ';
         *s++ = ' ';

         assert( s == sentinel);
      }
   }

   if( ! (options & mulle_buffer_hexdump_no_hex))
   {
      i = 0;
      s = mulle_buffer_advance( buffer, 3 * 8);
      assert( s);
      if( s)
      {
#ifndef NDEBUG
         sentinel = &s[ 24];
#endif
         for( ; i < 8; i++)
         {
            if( i < n)
            {
               value = *p++;
               *s++ = (uint8_t) hex( value >> 4);
               *s++ = (uint8_t) hex( value & 0xF);
            }
            else
            {
               if( options & mulle_buffer_hexdump_no_ascii)
               {
                  buffer->_curr = s;
                  return;
               }
               *s++ = ' ';
               *s++ = ' ';
            }
            *s++ = ' ';
         }

         assert( s == sentinel);
      }

      mulle_buffer_add_byte( buffer, ' ');

      s = mulle_buffer_advance( buffer, 3 * 8);
      assert( s);
      if( s)
      {
#ifndef NDEBUG
         sentinel = &s[ 24];
#endif
         for( ; i < 16; i++)
         {
            if( i < n)
            {
               value = *p++;

               *s++ = (uint8_t) hex( value >> 4);
               *s++ = (uint8_t) hex( value & 0xF);
            }
            else
            {
               if( options & mulle_buffer_hexdump_no_ascii)
               {
                  buffer->_curr = s;
                  return;
               }

               *s++ = ' ';
               *s++ = ' ';
            }
            *s++ = ' ';
         }

         assert( s == sentinel);
      }
   }

   if( ! (options & mulle_buffer_hexdump_no_ascii))
   {
      mulle_buffer_add_byte( buffer, ' ');
      mulle_buffer_add_byte( buffer, '|');

      p = memo;

      for( i = 0; i < 16; i++)
      {
         if( i < n)
         {
            value = *p++;
            if( ! isprint( value))
               value = '.';
            mulle_buffer_add_byte( buffer, (uint8_t) value);
         }
         else
            mulle_buffer_add_byte( buffer, ' ');
      }

      mulle_buffer_add_byte( buffer, '|');
   }
}


void  mulle_buffer_hexdump( struct mulle_buffer *buffer,
                            void *bytes,
                            size_t length,
                            size_t counter,
                            unsigned int options)
{
   size_t   lines;
   size_t   full_lines;
   size_t   remainder;
   size_t   i;
   uint8_t  *p;

   lines      = (length + 15) / 16;
   full_lines = length / 16;
   p          = bytes;

   for( i = 0; i < full_lines; i++)
   {
      mulle_buffer_hexdump_line( buffer, p, 16, counter, options);
      mulle_buffer_add_byte( buffer, '\n');
      counter += 16;
      p       += 16;
   }

   if( i < lines)
   {
      remainder = length - full_lines * 16;
      mulle_buffer_hexdump_line( buffer, p, (unsigned int) remainder, counter, options);
      mulle_buffer_add_byte( buffer, '\n');
   }
}

