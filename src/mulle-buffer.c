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

   if( ! allocator)
      allocator = &mulle_default_allocator;

   buffer = mulle_allocator_malloc( allocator, sizeof( struct mulle_buffer));
   mulle_buffer_init( buffer, allocator);
   return( buffer);
}


struct mulle_flushablebuffer   *
   mulle_flushablebuffer_create( size_t length,
                                 mulle_flushablebuffer_flusher_t flusher,
                                 void *userinfo,
                                 struct mulle_allocator *allocator)
{
   struct mulle_flushablebuffer  *buffer;
   void                          *storage;

   if( ! allocator)
      allocator = &mulle_default_allocator;

   buffer  = mulle_allocator_malloc( allocator, sizeof( struct mulle_flushablebuffer));
   storage = mulle_allocator_malloc( allocator, length);
   mulle_flushablebuffer_init_with_allocated_bytes( buffer,
                                                    storage,
                                                    length,
                                                    flusher,
                                                    userinfo,
                                                    allocator);
   return( buffer);
}



int   mulle_flushablebuffer_destroy( struct mulle_flushablebuffer *buffer)
{
   int   rval;

   if( ! buffer)
      return( 0);

   rval = mulle_flushablebuffer_done( buffer);
   if( ! rval)
      mulle_allocator_free( buffer->_allocator, buffer);
   return( 0);
}



static inline unsigned int   hex( unsigned int c)
{
   assert( c >= 0 && c <= 0xf);
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
   unsigned int   i;
   unsigned int   value;
   uint8_t        *p;

   memo = bytes;
   p    = bytes;

   if( ! (options & mulle_buffer_hexdump_no_offset))
   {
      s     = mulle_buffer_advance( buffer, 10);
      if( s)
      {
         *s++ = (uint8_t) hex( counter >> 28 & 0xF);
         *s++ = (uint8_t) hex( counter >> 24 & 0xF);
         *s++ = (uint8_t) hex( counter >> 20 & 0xF);
         *s++ = (uint8_t) hex( counter >> 16 & 0xF);

         *s++ = (uint8_t) hex( counter >> 12 & 0xF);
         *s++ = (uint8_t) hex( counter >> 8 & 0xF);
         *s++ = (uint8_t) hex( counter >> 4 & 0xF);
         *s++ = (uint8_t) hex( counter >> 0 & 0xF);

         *s++ = ' ';
         *s++ = ' ';
      }
   }

   if( ! (options & mulle_buffer_hexdump_no_hex))
   {
      i = 0;
      s = mulle_buffer_advance( buffer, 3 * 8);
      if( s)
      {
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
      }

      mulle_buffer_add_byte( buffer, ' ');

      s = mulle_buffer_advance( buffer, 3 * 8);
      if( s)
      {
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

