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
#include "mulle-flushablebuffer.h"

#include "include-private.h"


int  _mulle_flushablebuffer_flush( struct mulle_flushablebuffer *ibuffer)
{
   size_t   len;

   if( _mulle__buffer_has_overflown( (struct mulle__buffer *) ibuffer))
      return( -2);

   len = ibuffer->_curr - ibuffer->_storage;
   if( ! len)
      return( 0);

   // if flush fails, we are failed (forever)
   if( len != (*ibuffer->_flusher)( ibuffer->_storage, 1, len, ibuffer->_userinfo))
   {
      _mulle__buffer_set_overflown( (struct mulle__buffer *) ibuffer);
      return( -2);
   }

   ibuffer->_flushed += len;
   ibuffer->_curr     = ibuffer->_storage;

   return( 0);
}


int   _mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer)
{
   int   rval;

   rval = _mulle_flushablebuffer_flush( buffer);
   if( rval)
      return( rval);

   if( ! buffer->_initial_storage)
      mulle_allocator_free( buffer->_allocator, buffer->_storage);
#ifdef DEBUG
   memset( buffer, 0xFD, sizeof( *buffer));
#endif
   return( 0);
}



struct mulle_flushablebuffer   *
   mulle_flushablebuffer_create( size_t length,
                                 mulle_flushablebuffer_flusher_t *flusher,
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
   int                      rval;
   struct mulle_allocator   *allocator;

   if( ! buffer)
      return( 0);

   allocator = buffer->_allocator;
   rval      = mulle_flushablebuffer_done( buffer);
   if( ! rval)
      mulle_allocator_free( allocator, buffer);
   return( rval);
}


