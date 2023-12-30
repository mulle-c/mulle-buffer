//
//  mulle-flexbuffer.h
//  mulle-container
//
//  Created by Nat! on 22/08/16.
//  Copyright (c) 2022 Nat! - Mulle kybernetiK.
//  Copyright (c) 2022 Codeon GmbH.
//  All rights reserved.
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

#ifndef mulle_flexbuffer_h__
#define mulle_flexbuffer_h__

#include "include.h"

// use mulle-c11 for that
//#include <stdalign.h>

// A mulle-flexbuffer is useful in functions, when you prefer to do the work
// in a stack based (auto) buffer, but must remain flexible to use malloc
// for heavier workloads. Use mulle_flexarray if you need aligned memory!
//
// e.g. conventional code:
//
// void  print_uppercase( char *s)
// {
//    char     tmp[ 32];
//    char     *copy = tmp;
//    size_t   i;
//    size_t   len;
//
//    len = strlen( s) + 1;
//    if( n > 32)
//       copy = malloc( n);
//
//    for( i = 0; i < len; i++)
//      copy[ i] = toupper( s[ i]);
//
//    printf( "%s\n", copy);
//    if( copy != copy_data)
//       free( copy);
// }
//
//
// mulle-flexbuffer code:
//
// void  print_uppercase( char *s)
// {
//    size_t   i;
//    size_t   len;
//
//    len = strlen( s) + 1;
//    mulle_flexbuffer_do( copy, 32, len)
//    {
//       for( i = 0; i < len; i++)
//          copy[ i] = toupper( s[ i]);
//
//       printf( "%s\n", copy);
//    }
// }

#define mulle_flexbuffer( name, stackcount)                                 \
   char                   name ## __storage[ (stackcount)], *name;          \
   struct mulle__buffer   name ## __buffer =                                \
                            MULLE__BUFFER_INIT_FLEXIBLE( name ## __storage, \
                                                         (stackcount))

#define mulle_flexbuffer_alloc( name, actualcount)                          \
      name = _mulle__buffer_guarantee( &name ## __buffer,                   \
                                       (actualcount),                       \
                                       &mulle_default_allocator)

#define mulle_flexbuffer_realloc( name, count)                              \
   name = (                                                                 \
             _mulle__buffer_set_length(                                     \
                &name ## __buffer,                                          \
                (count) * sizeof( name ## __storage[ 0]),                   \
                &mulle_default_allocator),                                  \
             name ## __buffer._storage                                      \
          )

#define mulle_flexbuffer_done( name)                                        \
   do                                                                       \
   {                                                                        \
      _mulle__buffer_done( &name ## __buffer, &mulle_default_allocator);    \
      name = NULL;                                                          \
   }                                                                        \
   while( 0)



//
// these macros are only useful, if you are not nesting your flexbuffer_dos
// I just might throw these two macros out again
//
#define _mulle_flexbuffer_return( name, value)                              \
   do                                                                       \
   {                                                                        \
      __typeof__( *name) name ## __tmp = (value);                           \
      mulle_flexbuffer_done( name);                                         \
      return( name ## __tmp);                                               \
   }                                                                        \
   while( 0)

#define _mulle_flexbuffer_return_void( name)                                \
   do                                                                       \
   {                                                                        \
      mulle_flexbuffer_done( name);                                         \
      return;                                                               \
   }                                                                        \
   while( 0)

//
// we have to keep storage out of the for loop, but we can zero the
// pointer, so that a "too late" access is catchable
//
#define mulle_flexbuffer_do( name, stackcount, count)                         \
   char   name ## __storage[ stackcount], *name;                              \
   for( struct mulle__buffer                                                  \
           name ## __buffer =                                                 \
              MULLE__BUFFER_INIT_FLEXIBLE( name ## __storage, stackcount),    \
           name ## __i =                                                      \
           {                                                                  \
              ( mulle_flexbuffer_alloc( name, count), (void *) 0 )            \
           };                                                                 \
        ! name ## __i._storage;                                               \
        name = NULL,                                                          \
        name ## __i._storage =                                                \
        (                                                                     \
           _mulle__buffer_done( &name ## __buffer, &mulle_default_allocator), \
           (void *) 0x1                                                       \
        )                                                                     \
      )                                                                       \
                                                                              \
      for( int  name ## __j = 0;    /* break protection */                    \
           name ## __j < 1;                                                   \
           name ## __j++)

#endif
