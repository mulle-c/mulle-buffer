//
//  mulle-buffer-c23.h
//  mulle-buffer
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
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
#ifndef mulle_buffer_c23_h__
#define mulle_buffer_c23_h__


//
// This macro needs __typeof__, which is not C11. It is a GNU extension (also
// understood by clang and tcc) that only became standard as `typeof` with C23.
// It therefore lives here, so that mulle-buffer.h itself stays C11 clean.
//
// mulle-buffer.h includes this header automatically when MULLE_C_HAS_TYPEOF
// (from mulle-c11/mulle-c11-feature.h) is set. Including it directly on a
// compiler without typeof is an error by design.
//
#ifndef mulle_buffer_h__
# include "mulle-buffer.h"
#endif

#include <mulle-c11/mulle-c23-feature.h>


/**
 * Safely returns a value from a block of code that uses a `mulle_buffer`.
 *
 * This macro is used to return a value from a block of code that uses a
 * `mulle_buffer`. It ensures that the `mulle_buffer` is properly cleaned up
 * before returning the value, preventing memory leaks.
 *
 * It stores `value` in a temporary of the same type, so the value is computed
 * before the buffer is done. Any compiler that provides `typeof` is supported:
 * GCC, Clang, and any C23 compiler. On compilers without `typeof` (MSVC in
 * C11 mode) the macro is not defined, so using it is a compile-time error;
 * return `value` manually after calling `mulle_buffer_done` instead.
 *
 * @param name The name of the `mulle_buffer` variable used in the block.
 * @param value The value to be returned.
 */
//
// Caveats: don't use return in the block, or you will leak, use mulle_buffer_return
//          don't pre-initialize the buffer
//
#define mulle_buffer_return( name, value)                 \
   do                                                     \
   {                                                      \
      MULLE_C_TYPE_OF( value) name ## __tmp = (value);    \
                                                          \
      mulle_buffer_done( &name ## __storage);             \
      return( name ## __tmp);                             \
   }                                                      \
   while( 0)


#endif
