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
/**
 * The `mulle_flexbuffer` macro defines a flexible buffer that can be used to
 * store data. It takes two arguments:
 *
 * 1. `name`: The name of the buffer variable.
 * 2. `stackcount`: The size of the stack-based buffer in bytes.
 *
 * The macro defines a stack-based buffer of the given size, and a pointer
 * variable that can be used to access the buffer. The buffer is initially
 * allocated on the stack, but can be dynamically resized using the
 * `mulle_flexbuffer_alloc`, `mulle_flexbuffer_realloc`, and
 * `mulle_flexbuffer_done` macros.
 *
 * The `mulle_flexbuffer` macro is useful when you need a buffer that can
 * grow dynamically, but you want to avoid dynamic memory allocation for
 * small buffers. This can improve performance and reduce memory usage.
 */

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
                            MULLE__BUFFER_FLEXIBLE_DATA( name ## __storage, \
                                                         (stackcount))

/**
 * Allocates memory for the `name` flexbuffer and returns a pointer to the
 * allocated memory.
 *
 * This macro guarantees that the `name` flexbuffer has at least `actualcount`
 * elements. If the current buffer is not large enough, it will be reallocated
 * to the required size.
 *
 * @param name The name of the flexbuffer to allocate memory for.
 * @param actualcount The minimum number of elements to allocate for the
 *                    flexbuffer.
 * @return A pointer to the allocated memory for the flexbuffer.
 */
#define mulle_flexbuffer_alloc( name, actualcount)                          \
      name = _mulle__buffer_guarantee( &name ## __buffer,                   \
                                       (actualcount),                       \
                                       &mulle_default_allocator)

/**
 * Reallocates the memory for the `name` flexbuffer to the specified `count`.
 *
 * This macro guarantees that the `name` flexbuffer has at least `count`
 * elements. If the current buffer is not large enough, it will be reallocated
 * to the required size.
 *
 * @param name The name of the flexbuffer to reallocate memory for.
 * @param count The new minimum number of elements to allocate for the
 *              flexbuffer.
 * @return A pointer to the reallocated memory for the flexbuffer.
 */
#define mulle_flexbuffer_realloc( name, count)                              \
   name = (void *) (                                                        \
             _mulle__buffer_set_length(                                     \
                &name ## __buffer,                                          \
                (count) * sizeof( name ## __storage[ 0]),                   \
                &mulle_default_allocator),                                  \
             name ## __buffer._storage                                      \
          )

/**
 * Frees the memory allocated for the `name` flexbuffer and sets the pointer to
 * `NULL`.
 *
 * This macro should be called when the flexbuffer is no longer needed to
 * release the memory used by the buffer.
 *
 * @param name The name of the flexbuffer to free.
 */
#define mulle_flexbuffer_done( name)                                        \
   do                                                                       \
   {                                                                        \
      _mulle__buffer_done( &name ## __buffer, &mulle_default_allocator);    \
      name = NULL;                                                          \
   }                                                                        \
   while( 0)



/**
 * Frees the memory allocated for the `name` flexbuffer and returns the specified
 * `value`.
 *
 * This macro should be called when the flexbuffer is no longer needed to
 * release the memory used by the buffer and return a value.
 *
 * @param name The name of the flexbuffer to free.
 * @param value The value to return.
 * @return The specified `value`.
 */
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

/**
 * Frees the memory allocated for the `name` flexbuffer and returns.
 *
 * This macro should be called when the flexbuffer is no longer needed to
 * release the memory used by the buffer and return.
 *
 * @param name The name of the flexbuffer to free.
 */
#define _mulle_flexbuffer_return_void( name)                                \
   do                                                                       \
   {                                                                        \
      mulle_flexbuffer_done( name);                                         \
      return;                                                               \
   }                                                                        \
   while( 0)

/**
 * Defines a flexible buffer that can be used to dynamically allocate and manage
 * memory for a variable-sized data structure.
 *
 * This macro creates a flexible buffer with a fixed-size stack-allocated
 * storage area, and a dynamically allocated heap-based storage area that can
 * grow as needed. The `name` parameter is used to generate unique variable
 * names for the buffer and its associated state.
 *
 * The macro defines a loop that allows you to use the flexible buffer within a
 * block of code. The loop ensures that the buffer is properly initialized,
 * used, and freed when the block is exited, either normally or via a `break`
 * statement.
 *
 * @param name The name to use for the flexible buffer and its associated
 *             variables.
 * @param stackcount The size of the fixed-size stack-allocated storage area
 *                   for the buffer.
 * @param count The initial size of the dynamically allocated heap-based
 *              storage area for the buffer.
 */
//
// we have to keep storage out of the for loop, but we can zero the
// pointer, so that a "too late" access is catchable
//
#define mulle_flexbuffer_do( name, stackcount, count)                         \
   char   name ## __storage[ stackcount], *name;                              \
   for( struct mulle__buffer                                                  \
           name ## __buffer =                                                 \
              MULLE__BUFFER_FLEXIBLE_DATA( name ## __storage, stackcount),    \
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
