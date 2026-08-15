//
//  mulle-buffer.h
//  mulle-buffer
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
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
/** @file */

#ifndef mulle_buffer_h__
#define mulle_buffer_h__

#define MULLE__BUFFER_VERSION  ((5UL << 20) | (1 << 8) | 1)

#include "include.h"
#include "mulle--buffer.h"


// ## Contract (read this first)
//
// `mulle_buffer` is an append-first byte builder, not a general file-like
// stream. Everything below follows from that:
//
// - Writing is the primary operation; reading is a secondary convenience.
// - The buffer has a single cursor used for writing, reading, and seeking.
// - Seeking changes the logical length (`mulle_buffer_get_length` follows
//   the cursor).
// - `MULLE_BUFFER_SEEK_END` is relative to the allocation capacity, not the
//   written length.
// - Reads consume the same cursor used for writing.
// - Allocation uses a mulle allocator under the no-fail allocator contract:
//   it never returns `NULL`.
// - Write functions are intentionally `void`; failure is reported through
//   `mulle_buffer_has_overflown`.
// - Callers must check `mulle_buffer_has_overflown` after writing to a
//   fixed-size or flushable buffer.
// - const/read-only buffers must not be written and write-only buffers must
//   not be read; both are enforced by assertions in debug builds.
// - Self-aliasing (appending a slice of the buffer's own storage) is not
//   supported.


// stupidities to fix:
// all non "_" prefixed functions should check for NULL buffer
// inline code from mulle--buffer that is too large should be called from
// a mulle-buffer.c function (see: _mulle_buffer_zero_last_byte)
//
// 64 bit:
// sizeof( struct mulle__buffer) = 48
// sizeof( struct mulle_buffer) = 56
// sizeof( struct mulle_flushablebuffer) = 80
//
// 32 bit:
// sizeof( struct mulle__buffer) = 24
// sizeof( struct mulle_buffer) = 28
// sizeof( struct mulle_flushablebuffer) = 40

#define MULLE_BUFFER_BASE              \
   MULLE__BUFFER_BASE;                 \
   struct mulle_allocator  *_allocator

//
// This is like a possibly growing memory block (->NSData)
//
// **NOT THREAD-SAFE**: All instances must be used from a single thread.
//
struct mulle_buffer
{
   MULLE_BUFFER_BASE;
};


//
// Default capacity for heap-initialized buffers (mulle_buffer_init_default,
// mulle_buffer_create_default) and the guaranteed minimum stack backing for
// mulle_buffer_do. Chosen as a power of two, large enough for a line of text
// and most small string operations without hitting the heap.
//
#define MULLE_BUFFER_DEFAULT_CAPACITY  128

// value added by mulle-buffer
// as we override assert for testing, these tests can only be done in the
// header
enum
{
  MULLE_BUFFER_IS_READONLY   = 0x40,
  MULLE_BUFFER_IS_WRITEONLY  = 0x80,
  MULLE_BUFFER_IS_TEXT       = 0x100, // used in mulle_buffer_stdio only
  MULLE_BUFFER_IS_BINARY     = 0x0    // used in mulle_buffer_stdio only
};

#ifndef mulle_buffer_assert_readable
# define mulle_buffer_assert_readable( buffer) \
   assert( ! (buffer->_type & MULLE_BUFFER_IS_WRITEONLY))
#endif

#ifndef mulle_buffer_assert_writeable
# define mulle_buffer_assert_writeable( buffer) \
   assert( ! (buffer->_type & MULLE_BUFFER_IS_READONLY))
#endif


/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * initializes it with default values.
 *
 * This macro creates a new `mulle_buffer` instance and initializes it with
 * default values. The buffer is marked as flexible, meaning its underlying
 * storage can be resized. The data is assumed to be empty.
 *
 * @param allocator The allocator to use for the `mulle_buffer` instance. If
 *                  `NULL`, the default allocator `mulle_default_allocator` will
 *                  be used.
 * @return A new `mulle_buffer` instance initialized with default values.
 */
#define MULLE_BUFFER_DATA( allocator)                               \
((struct mulle_buffer)                                              \
{                                                                   \
   0, 0, 0, 0,                                                      \
   MULLE_BUFFER_DEFAULT_CAPACITY / 2,                               \
   MULLE_BUFFER_IS_FLEXIBLE,                                        \
   allocator ? allocator : &mulle_default_allocator                 \
})


/**
 * Single-evaluation variant of `MULLE_BUFFER_DATA` for use by the
 * `mulle_buffer_do_*` macros. `allocator` is evaluated exactly once, so
 * expressions with side effects are safe.
 */
static inline struct mulle_buffer   _mulle_buffer_data( struct mulle_allocator *allocator)
{
   return( MULLE_BUFFER_DATA( allocator));
}


//   unsigned char   *_storage;
//   unsigned char   *_curr;
//   unsigned char   *_sentinel;
//   unsigned char   *_initial_storage;
//   size_t          _size;
//   unsigned int    _type
//

//
//
//
/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * initializes it with the provided data.
 *
 * This macro creates a new `mulle_buffer` instance and initializes it with the
 * given data buffer, length, and allocator. The buffer is marked as flexible,
 * meaning its underlying storage can be resized. The data is assumed to be
 * scratch.
 *
 * @param data The data buffer to initialize the `mulle_buffer` with.
 * @param len The length of the data buffer.
 * @param allocator The allocator to use for the `mulle_buffer` instance. If
 *                  `NULL`, the default allocator `mulle_default_allocator` will
 *                  be used.
 * @return A new `mulle_buffer` instance initialized with the provided data.
 */
// MULLE_BUFFER_FLEXIBLE_DATA is a compound literal initializer. It exists for
// static/global data-section variables (and other constant-expression
// contexts), where a function call is not allowed. It evaluates `data`, `len`
// and `allocator` more than once, so it accepts only side-effect-free
// expressions.
//
// Inside actual code, use the single-evaluation static inline wrapper
// `_mulle_buffer_flexible_data` (or the `mulle_buffer_do_*` macros), which
// evaluate their arguments exactly once.
#define MULLE_BUFFER_FLEXIBLE_DATA( data, len, allocator)          \
   ((struct mulle_buffer)                                          \
   {                                                               \
      (unsigned char *) data,                                      \
      (unsigned char *) data,                                      \
      data ? &((unsigned char *) data)[ (len)] : NULL,             \
      (unsigned char *) data,                                      \
      (len),                                                       \
      MULLE_BUFFER_IS_FLEXIBLE,                                    \
      allocator ? allocator : &mulle_default_allocator             \
   })


/**
 * Single-evaluation variant of `MULLE_BUFFER_FLEXIBLE_DATA` for use by the
 * `mulle_buffer_do_*` macros. `data`, `len` and `allocator` are evaluated
 * exactly once, so expressions with side effects are safe.
 */
static inline struct mulle_buffer   _mulle_buffer_flexible_data( void *data,
                                                                 size_t len,
                                                                 struct mulle_allocator *allocator)
{
   return( MULLE_BUFFER_FLEXIBLE_DATA( data, len, allocator));
}


/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * initializes it with the provided data.
 *
 * This macro creates a new `mulle_buffer` instance and initializes it with the
 * given data buffer, length, and allocator. The buffer is marked as flexible,
 * meaning its underlying storage can be resized. The data is assumed to be
 * already filled with content.
 *
 * @param data The data buffer to initialize the `mulle_buffer` with.
 * @param len The length of the data buffer.
 * @param allocator The allocator to use for the `mulle_buffer` instance. If
 *                  `NULL`, the default allocator `mulle_default_allocator` will
 *                  be used.
 * @return A new `mulle_buffer` instance initialized with the provided data.
 */
// Same caveat as `MULLE_BUFFER_FLEXIBLE_DATA`: for static data-section
// initialization only; use `_mulle_buffer_flexible_filled_data` in code.
#define MULLE_BUFFER_FLEXIBLE_FILLED_DATA( data, len, allocator)   \
   ((struct mulle_buffer)                                          \
   {                                                               \
      (unsigned char *) data,                                      \
      data ? &((unsigned char *) data)[ (len)] : NULL,             \
      data ? &((unsigned char *) data)[ (len)] : NULL,             \
      (unsigned char *) data,                                      \
      (len),                                                       \
      MULLE_BUFFER_IS_FLEXIBLE,                                    \
      allocator ? allocator : &mulle_default_allocator             \
   })


/**
 * Single-evaluation variant of `MULLE_BUFFER_FLEXIBLE_FILLED_DATA` for use by
 * the `mulle_buffer_do_*` macros. `data`, `len` and `allocator` are evaluated
 * exactly once, so expressions with side effects are safe.
 */
static inline struct mulle_buffer   _mulle_buffer_flexible_filled_data( void *data,
                                                                       size_t len,
                                                                       struct mulle_allocator *allocator)
{
   return( MULLE_BUFFER_FLEXIBLE_FILLED_DATA( data, len, allocator));
}


/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * initializes it with the provided data.
 *
 * This macro creates a new `mulle_buffer` instance and initializes it with the
 * given data buffer, length, and allocator. The buffer is marked as inflexible,
 * meaning its underlying storage cannot be resized. The data is assumed to be
 * scratch.
 *
 * @param data The data buffer to initialize the `mulle_buffer` with.
 * @param len The length of the data buffer.
 * @param allocator The allocator to use for the `mulle_buffer` instance. If
 *                  not `NULL`, the data will be freed on done.
 * @return A new `mulle_buffer` instance initialized with the provided data.
 */
// Same caveat as `MULLE_BUFFER_FLEXIBLE_DATA`: for static data-section
// initialization only; use `_mulle_buffer_inflexible_data` in code.
#define MULLE_BUFFER_INFLEXIBLE_DATA( data, len, allocator)        \
   ((struct mulle_buffer)                                          \
   {                                                               \
      (unsigned char *) data,                                      \
      (unsigned char *) data,                                      \
      data ? &((unsigned char *) data)[ (len)] : NULL,             \
      (unsigned char *) data,                                      \
      (len),                                                       \
      MULLE_BUFFER_IS_INFLEXIBLE,                                  \
      allocator                                                    \
   })


/**
 * Single-evaluation variant of `MULLE_BUFFER_INFLEXIBLE_DATA` for use by the
 * `mulle_buffer_do_*` macros. `data`, `len` and `allocator` are evaluated
 * exactly once, so expressions with side effects are safe.
 */
static inline struct mulle_buffer   _mulle_buffer_inflexible_data( void *data,
                                                                  size_t len,
                                                                  struct mulle_allocator *allocator)
{
   return( MULLE_BUFFER_INFLEXIBLE_DATA( data, len, allocator));
}


/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * initializes it with the provided data.
 *
 * This macro creates a new `mulle_buffer` instance and initializes it with the
 * given data buffer, length, and allocator. The buffer is marked as inflexible,
 * meaning its underlying storage cannot be resized. The data is assumed to be
 * already filled with content.
 *
 * @param data The data buffer to initialize the `mulle_buffer` with.
 * @param len The length of the data buffer.
 * @param allocator The allocator to use for the `mulle_buffer` instance. If
 *                  not `NULL`, the data will be freed on done.
 * @return A new `mulle_buffer` instance initialized with the provided data.
 */
//
// we take some static data, but we assume its already filled
// with data.
//
// Same caveat as `MULLE_BUFFER_FLEXIBLE_DATA`: for static data-section
// initialization only; use `_mulle_buffer_inflexible_filled_data` in code.
#define MULLE_BUFFER_INFLEXIBLE_FILLED_DATA( data, len, allocator)  \
   ((struct mulle_buffer)                                           \
   {                                                                \
      (unsigned char *) data,                                       \
      data ? &((unsigned char *) data)[ (len)] : NULL,            \
      data ? &((unsigned char *) data)[ (len)] : NULL,            \
      (unsigned char *) data,                                       \
      (len),                                                        \
      MULLE_BUFFER_IS_INFLEXIBLE,                                   \
      allocator                                                     \
   })


/**
 * Single-evaluation variant of `MULLE_BUFFER_INFLEXIBLE_FILLED_DATA` for use by
 * the `mulle_buffer_do_*` macros. `data`, `len` and `allocator` are evaluated
 * exactly once, so expressions with side effects are safe.
 */
static inline struct mulle_buffer   _mulle_buffer_inflexible_filled_data( void *data,
                                                                         size_t len,
                                                                         struct mulle_allocator *allocator)
{
   return( MULLE_BUFFER_INFLEXIBLE_FILLED_DATA( data, len, allocator));
}



/**
 * Returns the `mulle__buffer` instance underlying the given `mulle_buffer`.
 *
 * This function provides direct access to the `mulle__buffer` instance that
 * backs the `mulle_buffer`. This is useful for low-level operations that need
 * to interact with the buffer's internal representation.
 *
 * @param buffer The `mulle_buffer` instance to get the underlying `mulle__buffer` for.
 * @return The `mulle__buffer` instance underlying the given `mulle_buffer`.
 */
static inline struct mulle__buffer   *
   mulle_buffer_as__buffer( struct mulle_buffer *buffer)
{
  return( (struct mulle__buffer *) buffer);
}


/**
 * Returns the allocator associated with the given `mulle_buffer` instance.
 *
 * This function returns the allocator that was used to create the `mulle_buffer`
 * instance. If no allocator was provided when the buffer was created, the default
 * allocator `mulle_default_allocator` is returned.
 *
 * A `NULL` return value is intentional: a buffer initialized over static or
 * `const` storage (see `mulle_buffer_do_inflexible`) has no allocator, which
 * means the buffer does not own its storage and must not free it.
 *
 * The returned allocator is subject to the no-fail allocator contract: it must
 * never return `NULL` from an allocation. `mulle-buffer` is intentionally
 * no-fail with respect to allocation; supplying a conforming mulle allocator
 * is a precondition of the API.
 *
 * @param buffer The `mulle_buffer` instance to get the allocator for.
 * @return The allocator associated with the `mulle_buffer` instance, or NULL if
 *         the buffer was created over static storage and owns no allocator.
 */
static inline struct mulle_allocator  *
   mulle_buffer_get_allocator( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( &mulle_default_allocator);

   assert( _mulle__buffer_is_inflexible( (struct mulle__buffer *) buffer) ||
           buffer->_allocator);
   return( buffer->_allocator);
}


# pragma mark - initialization and destruction

/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * the default capacity.
 *
 * This function creates a new `mulle_buffer` instance and initializes it with the
 * provided `mulle_allocator`. If no allocator is provided, the default allocator
 * `mulle_default_allocator` will be used.
 *
 * The allocator is subject to the no-fail allocator contract: it must never
 * return `NULL` from an allocation. `mulle-buffer` is intentionally no-fail
 * with respect to allocation; supplying a conforming mulle allocator is a
 * precondition of the API.
 *
 * @param allocator The allocator to use for the buffer's storage, or NULL to use the default allocator.
 * @return A new `mulle_buffer` instance.
 */
static inline struct mulle_buffer   *mulle_buffer_alloc( struct mulle_allocator *allocator)
{
   return( mulle_allocator_malloc( allocator, sizeof( struct mulle_buffer)));
}

#define mulle_buffer_alloc_default() \
   mulle_buffer_alloc( NULL)

/**
 * Creates a new `mulle_buffer` instance with the specified allocator and
 * the default capacity.
 *
 * This function creates a new `mulle_buffer` instance and initializes it with the
 * provided `mulle_allocator`. If no allocator is provided, the default allocator
 * `mulle_default_allocator` will be used.
 *
 * The allocator is subject to the no-fail allocator contract: it must never
 * return `NULL` from an allocation. `mulle-buffer` is intentionally no-fail
 * with respect to allocation; supplying a conforming mulle allocator is a
 * precondition of the API.
 *
 * @param allocator The allocator to use for the buffer's storage, or NULL to use the default allocator.
 * @return A new `mulle_buffer` instance.
 */
MULLE__BUFFER_GLOBAL
struct mulle_buffer   *mulle_buffer_create( struct mulle_allocator *allocator);

/**
 * Creates a new `mulle_buffer` instance with the default allocator and
 * the default capacity.
 *
 * @return A new `mulle_buffer` instance.
 */
#define mulle_buffer_create_default() \
   mulle_buffer_create( NULL)

/**
 * Destroys a buffer and releases any resources it holds.
 *
 * This function destroys a `mulle_buffer` and releases any resources it holds,
 * such as the memory region used for its storage. It also frees the
 * `struct mulle_buffer` object itself, so it must only be used on a buffer
 * allocated by `mulle_buffer_create`. For an embedded or stack-allocated
 * buffer, use `mulle_buffer_done` instead.
 *
 * @param buffer The buffer to destroy.
 */
static inline void   mulle_buffer_destroy( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   _mulle__buffer_destroy( (struct mulle__buffer *) buffer,
                           mulle_buffer_get_allocator( buffer));
}


/**
 * Finalizes a buffer and releases any resources it holds.
 *
 * This function finalizes a `mulle_buffer` and releases any resources it holds,
 * such as the memory region used for its storage. It is safe to call this
 * function on a buffer that was not initialized, as long as it is
 * zero-initialized (a static/global variable, `calloc`ed memory, or an
 * Objective-C ivar). An uninitialized automatic `struct mulle_buffer` has
 * indeterminate field values and must be initialized first.
 *
 * @param buffer The buffer to finalize.
 */
static inline void   mulle_buffer_done( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;
   // it's ok to call _done even if not inited
   _mulle__buffer_done( (struct mulle__buffer *) buffer, buffer->_allocator);
}


/**
 * Sets the allocator to use for the buffer's storage.
 *
 * This function sets the allocator to use for the buffer's storage. If no allocator
 * is provided, the default allocator `mulle_default_allocator` will be used.
 *
 * @param buffer The buffer to set the allocator for.
 * @param allocator The allocator to use for the buffer's storage, or NULL to use the default allocator.
 */
static inline void
   mulle_buffer_set_allocator( struct mulle_buffer *buffer,
                               struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;
   // always have an allocator
   buffer->_allocator = allocator ? allocator : &mulle_default_allocator;
}


/**
 * Initializes a buffer with a memory region allocated by the user.
 *
 * This function initializes a `mulle_buffer` with a memory region provided by the
 * user. This is useful if you have already allocated some memory and don't need it
 * anymore. The storage is overwritten from the start.
 *
 * @param buffer The buffer to initialize.
 * @param storage The memory region to use for the buffer's storage.
 * @param length The length of the memory region.
 * @param allocator The allocator to use for the buffer's storage.
 */
/* a growing buffer, that starts with a memory region, allocated by the user.
   Useful if you already malloced something and dont't need it anymore.
   The storage is overwritten from the start.
*/
static inline void
   mulle_buffer_init_with_allocated_bytes( struct mulle_buffer *buffer,
                                           void *storage,
                                           size_t length,
                                           struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   // NULL storage with a non-zero length is invalid: in a debug build this
   // is a hard precondition. In a release build the object is still
   // initialized to a valid, empty flexible buffer, so a later `done` is
   // always well-defined.
   assert( storage || ! length);

   _mulle__buffer_init_with_allocated_bytes( (struct mulle__buffer *) buffer,
                                            storage,
                                            length);
   mulle_buffer_set_allocator( buffer, allocator);
}


/**
 * Initializes a buffer with a non-freeable memory region allocated by the user.
 *
 * This function initializes a `mulle_buffer` with a memory region provided by the
 * user. This is useful if you want to supply some stack memory for `mulle_buffer`
 * so that it does not necessarily need to malloc, if the contents remain small
 * enough. The storage is overwritten from the start.
 *
 * @param buffer The buffer to initialize.
 * @param storage The memory region to use for the buffer's storage.
 * @param length The length of the memory region.
 * @param allocator The allocator to use for the buffer's storage.
 */
/* a growing buffer, that starts with a non freeable memory region, allocated
   by the user. Useful if you want to supply some stack memory for mulle_buffer
   so that it need not necessarily malloc, if the contents remain small enough.
   The storage is overwritten from the start.
 */
static inline void
   mulle_buffer_init_with_static_bytes( struct mulle_buffer *buffer,
                                        void *storage,
                                        size_t length,
                                        struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   // NULL storage with a non-zero length is invalid: in a debug build this
   // is a hard precondition. In a release build the object is still
   // initialized to a valid, empty flexible buffer, so a later `done` is
   // always well-defined.
   assert( storage || ! length);

   _mulle__buffer_init_with_static_bytes( (struct mulle__buffer *) buffer, storage, length);
   mulle_buffer_set_allocator( buffer, allocator);
}


/**
 * Initializes a buffer with a desired initial capacity and the provided
 * allocator.
 *
 * This function initializes a `mulle_buffer` with the desired initial capacity
 * that can be expanded as needed. The buffer will be allocated using the
 * provided `mulle_allocator` when the need arises.
 *
 * The allocator is subject to the no-fail allocator contract: it must never
 * return `NULL` from an allocation. `mulle-buffer` is intentionally no-fail
 * with respect to allocation; supplying a conforming mulle allocator is a
 * precondition of the API.
 *
 * @param buffer The buffer to initialize.
 * @param capacity
 * @param allocator The allocator to use for the buffer's storage.
 */
//
// The allocator you pass in will provide the alignment guarantees of
// the internal buffer. So if it is malloc based and doesn't change the
// address the "return[ed] ... memory ... is suitably aligned for any
// built-in type"
//
static inline void   mulle_buffer_init( struct mulle_buffer *buffer,
                                        size_t capacity,
                                        struct mulle_allocator *allocator)
{
   if( ! buffer)
      return;

   _mulle__buffer_init( (struct mulle__buffer *) buffer, capacity);
   mulle_buffer_set_allocator( buffer, allocator);
}


MULLE_C_DEPRECATED
static inline void   mulle_buffer_init_with_capacity( struct mulle_buffer *buffer,
                                                      size_t capacity,
                                                      struct mulle_allocator *allocator)
{
   mulle_buffer_init( buffer, capacity, allocator);
}


/**
 * Initializes a buffer with the default initial capacity and the default
 * allocator.
 *
 * @param buffer The buffer to initialize.
 */
#define mulle_buffer_init_default( buffer) \
   mulle_buffer_init( buffer, MULLE_BUFFER_DEFAULT_CAPACITY, NULL)


/**
 * Initializes a buffer with a fixed-size, non-growable storage region.
 *
 * This function initializes a `mulle_buffer` with a fixed-size storage region
 * that cannot be resized. The storage is provided by the caller and is
 * overwritten from the start. This is useful for guaranteeing that the buffer
 * cannot be overrun.
 *
 * @param buffer The buffer to initialize.
 * @param storage The storage region to use for the buffer.
 * @param length The length of the storage region.
 */
/* this buffer does not grow. storage is supplied. This is useful for
   guaranteeing that a buffer can not be overrun.
   The storage is overwritten from the start.
 */
static inline void   mulle_buffer_init_inflexible_with_static_bytes( struct mulle_buffer *buffer,
                                                                     void *storage,
                                                                     size_t length)
{
   if( ! buffer)
      return;

   // NULL storage with a non-zero length is invalid: in a debug build this
   // is a hard precondition. In a release build the underlying initializer
   // still produces a safe, empty fixed-size buffer.
   assert( storage || ! length);

   _mulle__buffer_init_inflexible_with_static_bytes( (struct mulle__buffer *) buffer,
                                                     storage,
                                                     length);

   // the storage belongs to the caller, so there is no allocator associated
   // with this buffer (mulle_buffer_get_allocator will return NULL). this is
   // consistent with mulle_buffer_do_inflexible and init_with_const_bytes.
   buffer->_allocator = NULL;
}


/**
 * Initializes a buffer with a fixed-size, non-growable storage region that
 * can not be written too.
 *
 * This can be useful to parse compiled in data or strings.
 * This function initializes a `mulle_buffer` with a fixed-size storage region
 * that cannot be resized.
 *
 * @param buffer The buffer to initialize.
 * @param storage The storage region to use for the buffer.
 * @param length The length of the storage region.
 */
MULLE__BUFFER_GLOBAL
void   mulle_buffer_init_with_const_bytes( struct mulle_buffer *buffer,
                                           const void *storage,
                                           size_t length);

#pragma mark - sizing

/**
 * Grows the buffer to hold at least the specified minimum amount of data.
 *
 * This function increases the internal storage of the buffer to ensure it can
 * hold at least the specified minimum amount of data. If the buffer's current
 * capacity is already greater than or equal to the minimum amount, this
 * function does nothing.
 *
 * @param buffer The buffer to grow.
 * @param min_amount The minimum amount of data the buffer should be able to
 *                   hold.
 * @return 0 on success, a negative value on failure.
 */
static inline int   mulle_buffer_grow( struct mulle_buffer *buffer,
                                       size_t min_amount)
{
   if( ! buffer)
      return( 0);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_grow( (struct mulle__buffer *) buffer,
                                min_amount,
                                mulle_buffer_get_allocator( buffer)));
}


/**
 * Resizes the buffer to the minimum required size to hold its contents.
 *
 * This function reduces the buffer's internal storage to the minimum size
 * required to hold its current contents. This can be useful to reduce the
 * memory footprint of the buffer.
 *
 * @param buffer The buffer to resize.
 */
static inline void   mulle_buffer_size_to_fit( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_size_to_fit( (struct mulle__buffer *) buffer,
                               mulle_buffer_get_allocator( buffer));
}


/**
 * Makes the buffer inflexible, using the provided storage.
 *
 * The given storage replaces the previous contents. The buffer is not flexible
 * anymore. The insertion point is moved to &storage[length].
 *
 * @param buffer The buffer to make inflexible.
 * @param storage The new storage for the buffer.
 * @param length The length of the new storage.
 */
/* the given storage replaces the previous contents.
   the buffer is not flexable anymore.
   The insertion point is moved to &storage[length]
 */
static inline void   mulle_buffer_make_inflexible( struct mulle_buffer *buffer,
                                                   void *storage,
                                                   size_t length)
{
   if( ! buffer)
      return;

   // NULL storage with a non-zero length is invalid: in a debug build this
   // is a hard precondition. In a release build the underlying code still
   // produces a safe, empty fixed-size buffer.
   assert( storage || ! length);

   _mulle__buffer_make_inflexible( (struct mulle__buffer *) buffer,
                                   storage,
                                   length,
                                   mulle_buffer_get_allocator( buffer));
}


#define MULLE_BUFFER_SHRINK_OR_ZEROFILL      0
#define MULLE_BUFFER_NO_SHRINK               1
#define MULLE_BUFFER_NO_ZEROFILL             2
#define MULLE_BUFFER_NO_SHRINK_OR_ZEROFILL   3

/**
 * Sets the length of the buffer to the given length. If the buffer is
 * inflexible, then this can lead to an overflow condition if the new
 * length can't be met. If the length is less than the current length,
 * then the buffer will try to "size to fit". This will either
 * "size to fit" for shrinking or "zero to length".
 *
 * @param buffer The buffer to set the length of.
 * @param length The new length of the buffer.
 * @param options 0=shrink/zerofill,1=no shrink,2=no zerofill,3=neither
 * @return -1 on failure, if the buffer is NULL or the buffer can't be resized
 */
static inline int   mulle_buffer_set_length( struct mulle_buffer *buffer,
                                             size_t length,
                                             unsigned int options)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_set_length( (struct mulle__buffer *) buffer,
                                       length,
                                       options,
                                       mulle_buffer_get_allocator( buffer)));
}


//
// you only do this once!, because you now own the malloc block
// TODO: rename to extract only ?
//static inline void   *mulle_buffer_extract_all( struct mulle_buffer *buffer)
//{
//   if( ! buffer)
//      return( NULL);
//   return( _mulle__buffer_extract_all( (struct mulle__buffer *) buffer,
//                                       mulle_buffer_get_allocator( buffer)));
//}


/**
 * Extracts the data from the buffer and returns it.
 *
 * The caller takes ownership of the returned bytes and must free them with
 * the same allocator the buffer used (`mulle_buffer_get_allocator`). The
 * buffer is consumed: it is left empty with no backing storage and must be
 * re-initialized (`mulle_buffer_reset` / `mulle_buffer_init_*`) before reuse.
 *
 * If the buffer is empty, the returned `mulle_data` has `bytes == NULL` and
 * `length == 0`, and the buffer is still left empty with no backing storage.
 *
 * @param buffer The buffer to extract the data from.
 * @return The data from the buffer, or an invalid mulle_data struct if the
 *         buffer is NULL.
 */
static inline struct mulle_data   mulle_buffer_extract_data( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( mulle_data_make_invalid());

   return( _mulle__buffer_extract_data( (struct mulle__buffer *) buffer,
                                         mulle_buffer_get_allocator( buffer)));
}


/**
 * Extracts the string from the buffer and returns it.
 *
 * The buffer content is made a zero-terminated C string first, then handed to
 * the caller. The caller owns the returned string and must free it with the
 * same allocator the buffer used (`mulle_buffer_get_allocator`). The buffer
 * is consumed: it is left empty with no backing storage and must be
 * re-initialized before reuse.
 *
 * If the buffer is empty, `NULL` is returned and the buffer is still left
 * empty with no backing storage.
 *
 * @param buffer The buffer to extract the string from.
 * @return The string from the buffer, or NULL if the buffer is NULL.
 */
static inline void   *mulle_buffer_extract_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_extract_string( (struct mulle__buffer *) buffer,
                                           mulle_buffer_get_allocator( buffer)));
}


/**
 * Extracts the bytes from the buffer and returns them.
 *
 * Same ownership and reuse contract as `mulle_buffer_extract_data`, returning
 * only the bytes pointer. The caller owns the returned bytes and must free
 * them with the same allocator the buffer used; the buffer is consumed and
 * must be re-initialized before reuse. An empty buffer returns `NULL`.
 *
 * @param buffer The buffer to extract the bytes from.
 * @return The bytes from the buffer, or NULL if the buffer is NULL.
 */
static inline void   *mulle_buffer_extract_bytes( struct mulle_buffer *buffer)
{
   struct mulle_data   data;

   if( ! buffer)
      return( NULL);

   data = _mulle__buffer_extract_data( (struct mulle__buffer *) buffer,
                                        mulle_buffer_get_allocator( buffer));
   return( data.bytes);
}


/**
 * Removes all bytes from the buffer.
 *
 * @param buffer The buffer to remove all bytes from.
 */
static inline void   mulle_buffer_remove_all( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_remove_all( (struct mulle__buffer *) buffer);
}


/**
 * Removes the bytes in the given range from the buffer.
 *
 * @param buffer The buffer to remove the bytes from.
 * @param range The range of bytes to remove from the buffer.
 */
static inline void   mulle_buffer_remove_in_range( struct mulle_buffer *buffer,
                                                   struct mulle_range range)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_remove_in_range( (struct mulle__buffer *) buffer, range);
}


#pragma mark - accessors


/**
 * Returns the buffer contents as a mulle_data struct.
 *
 * @param buffer The buffer to get the data from.
 * @return The buffer contents as a mulle_data struct, or an invalid mulle_data struct if the buffer is invalid.
 */
static inline struct mulle_data   mulle__buffer_get_data( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( mulle_data_make_invalid());
   return( _mulle__buffer_get_data( (struct mulle__buffer *) buffer));
}

/**
 * Returns the buffer contents as a byte array.
 *
 * @param buffer The buffer to get the bytes from.
 * @return The buffer contents as a byte array, or NULL if the buffer is invalid.
 */
static inline void   *mulle_buffer_get_bytes( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_get_bytes( (struct mulle__buffer *) buffer));
}


/**
 * Returns the buffer contents as a null-terminated string.
 *
 * Despite the accessor-like name, this is a mutating operation: it appends a
 * NUL terminator and may grow the buffer's storage to fit it. It is therefore
 * only valid on writeable buffers and is not a read-only operation on a
 * read-only buffer.
 *
 * @param buffer The buffer to get the string from.
 * @return The buffer contents as a null-terminated string, or NULL if the buffer is invalid.
 */
static inline char   *mulle_buffer_get_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( NULL);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_get_string( (struct mulle__buffer *) buffer, buffer->_allocator));
}


/**
 * Returns the length of the buffer.
 *
 * @param buffer The buffer to get the length from.
 * @return The length of the buffer, or 0 if the buffer is invalid.
 */
static inline size_t   mulle_buffer_get_length( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_length( (struct mulle__buffer *) buffer));
}


/**
 * Returns the capacity of the buffer.
 *
 * @param buffer The buffer to get the capacity from.
 * @return The capacity of the buffer, or 0 if the buffer is invalid.
 */
static inline size_t   mulle_buffer_get_capacity( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_capacity( (struct mulle__buffer *) buffer));
}


/**
 * Returns the static length of the buffer.
 *
 * @param buffer The buffer to get the static length from.
 * @return The static length of the buffer, or 0 if the buffer is invalid.
 */
static inline size_t
   mulle_buffer_get_staticlength( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_staticlength( (struct mulle__buffer *) buffer));
}


/**
 * Sets the seek position of the buffer.
 *
 * `seek` is of type `long`, which is 32 bit on platforms with the LLP64 model
 * (e.g. Windows). Seek positions ≥ 2 GB truncate on those platforms. Use
 * `mulle_buffer_lseek` with `off_t` for large-buffer safety.
 *
 * `MULLE_BUFFER_SEEK_END` is relative to the end of the *allocation*, not the
 * written length: offset `0` seeks to the end of the capacity and negative
 * offsets seek towards the start. The new position is `capacity + offset`.
 * This mirrors the POSIX `lseek` semantics used by `mulle_buffer_lseek`.
 *
 * @param buffer The buffer to set the seek position for.
 * @param seek The seek position, relative to the mode.
 * @param mode The seek mode, one of SEEK_SET, SEEK_CUR, or SEEK_END.
 * @return 0 on success, or -1 on failure (invalid buffer, flushable, or
 *         out-of-range). Use `mulle_buffer_lseek` to obtain the new position.
 */
static inline int    mulle_buffer_set_seek( struct mulle_buffer *buffer, long seek, int mode)
{
   if( ! buffer)
      return( -1);

   return( _mulle__buffer_set_seek( (struct mulle__buffer *) buffer, seek, mode));
}


/**
 * Returns the current seek position of the buffer.
 *
 * The return type is `long`. On LLP64 platforms such as Windows, `long` is 32
 * bit and positions ≥ 2 GB truncate. Use `mulle_buffer_get_lseek` for `off_t`
 * positions.
 *
 * @param buffer The buffer to get the seek position from.
 * @return The current seek position of the buffer, or 0 if the buffer is invalid.
 */
static inline long   mulle_buffer_get_seek( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_seek( (struct mulle__buffer *) buffer));
}


/**
 * Returns the current seek position of the buffer as an `off_t`.
 *
 * This is the `off_t` variant of `mulle_buffer_get_seek`. Its value does not
 * truncate positions >= 2 GB on platforms where `long` is 32 bit (e.g. Windows).
 *
 * @param buffer The buffer to get the seek position from.
 * @return The current seek position of the buffer, or -1 if the buffer is invalid.
 */
static inline off_t   mulle_buffer_get_lseek( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( (off_t) -1);

   return( _mulle__buffer_get_lseek( (struct mulle__buffer *) buffer));
}


/**
 * Sets the seek position of the buffer, returning the new position.
 *
 * This is the `off_t` variant of `mulle_buffer_set_seek`, mirroring POSIX
 * lseek(2). `MULLE_BUFFER_SEEK_END` is relative to the end of the buffer:
 * offset `0` seeks to the end, negative offsets seek towards the beginning,
 * and positive offsets would seek past the end (which fails). The new
 * position is `length + offset` where `length` is the buffer capacity.
 *
 * @param buffer The buffer to set the seek position for.
 * @param offset The seek offset, relative to the mode.
 * @param mode The seek mode, one of MULLE_BUFFER_SEEK_SET, MULLE_BUFFER_SEEK_CUR,
 *             or MULLE_BUFFER_SEEK_END.
 * @return The new seek position, or -1 on failure (invalid buffer, flushable, or
 *         out-of-range).
 */
static inline off_t   mulle_buffer_lseek( struct mulle_buffer *buffer,
                                          off_t offset,
                                          int mode)
{
   if( ! buffer)
      return( (off_t) -1);

   return( _mulle__buffer_lseek( (struct mulle__buffer *) buffer,
                                 offset,
                                 mode));
}


/**
 * Advances the buffer by the specified length, returning a pointer to the new
 * position in the buffer.
 *
 * @param buffer The buffer to advance.
 * @param length The number of bytes to advance the buffer by.
 * @return A pointer to the new position in the buffer, or NULL if the buffer
 *         is invalid.
 */
static inline void   *mulle_buffer_advance( struct mulle_buffer *buffer,
                                            size_t length)
{
   if( ! buffer)
      return( NULL);

   return( _mulle__buffer_advance( (struct mulle__buffer *) buffer,
                                  length,
                                  mulle_buffer_get_allocator( buffer)));
}


#pragma mark - read only / write only asserting


static inline int   mulle_buffer_is_readonly( struct mulle_buffer *buffer)
{
   return( buffer ? buffer->_type & MULLE_BUFFER_IS_READONLY : 0);
}


static inline int   mulle_buffer_is_writeonly( struct mulle_buffer *buffer)
{
   return( buffer ? buffer->_type & MULLE_BUFFER_IS_WRITEONLY : 0);
}


static inline void   mulle_buffer_set_readonly( struct mulle_buffer *buffer)
{
   if( buffer)
      buffer->_type |= MULLE_BUFFER_IS_READONLY;
}


static inline void   mulle_buffer_set_writeonly( struct mulle_buffer *buffer)
{
   if( buffer)
      buffer->_type |= MULLE_BUFFER_IS_WRITEONLY;
}



#pragma mark - copy out

/**
 * Copies a range of bytes from the buffer to the provided destination buffer.
 *
 * @param buffer The buffer to copy from.
 * @param range The range of bytes to copy.
 * @param dst The destination buffer to copy the bytes to.
 */
static inline void   mulle_buffer_copy_range( struct mulle_buffer *buffer,
                                              struct mulle_range range,
                                              void *dst)
{
   if( ! buffer || ! dst)
      return;

   mulle_buffer_assert_readable( buffer);

   _mulle__buffer_copy_range( (struct mulle__buffer *) buffer,
                              range,
                              dst);
}


#pragma mark - query

/**
 * Checks if the buffer is inflexible, i.e. cannot be resized or reallocated.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer is inflexible, 0 otherwise.
 */
static inline int   mulle_buffer_is_inflexible( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_inflexible( (struct mulle__buffer *) buffer));
}


/**
 * Checks if the buffer is flushable, i.e. can be flushed to its underlying storage.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer is flushable, 0 otherwise.
 */
static inline int   mulle_buffer_is_flushable( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_is_flushable( (struct mulle__buffer *) buffer));
}


/**
 * Checks if the buffer is full, i.e. has no more capacity to hold additional data.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer is full, 0 otherwise.
 */
static inline int   mulle_buffer_is_full( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_is_full( (struct mulle__buffer *) buffer));
}


static inline size_t   mulle_buffer_remaining_length( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_remaining_length( (struct mulle__buffer *) buffer));
}


/**
 * Checks if the buffer has enough capacity to hold the specified length.
 *
 * @param buffer The buffer to check.
 * @param len The length to check against the buffer's capacity.
 * @return 1 if the buffer has enough capacity, 0 otherwise.
 */
static inline int   mulle_buffer_is_big_enough( struct mulle_buffer *buffer,
                                                size_t len)
{
   if( ! buffer)
      return( len == 0);

   return( _mulle__buffer_is_big_enough( (struct mulle__buffer *) buffer, len));
}


/**
 * Checks if the buffer is empty, i.e. has no content.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer is empty, 0 otherwise.
 */
// same as mulle_buffer_get_length( buffer) == 0
static inline int   mulle_buffer_is_empty( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_empty( (struct mulle__buffer *) buffer));
}


/**
 * Checks if the buffer is a void buffer, i.e. its backing storage cannot hold any content.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer is a void buffer, 0 otherwise.
 */
// a void buffer's backing storage can't hold any content
static inline int   mulle_buffer_is_void( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_is_void( (struct mulle__buffer *) buffer));
}


/**
 * Checks if the buffer has overflown, i.e. an operation could not be
 * completed.
 *
 * This is the official write contract of the library: write functions are
 * `void`, so a failed operation reports itself by setting this sticky flag.
 * The flag is set exactly when:
 *
 * - a fixed-size (inflexible) buffer runs out of capacity and has to
 *   truncate content;
 * - a flushable buffer's flusher reports a short write (failed flush);
 * - a string add function (`mulle_buffer_add_string`, `add_string_with_maxlength`,
 *   `add_c_chars`, `add_c_string`) is given a source that points into the
 *   buffer's own storage (self-reference);
 * - `mulle_buffer_flush` is called on a non-flushable buffer.
 *
 * In debug builds, violations of the read-only/write-only mode contract and
 * self-aliasing in `mulle_buffer_add_bytes` additionally trip assertions, so
 * invalid operations surface early during development.
 *
 * The overflow state is sticky: once set it can not be cleared with
 * `mulle_buffer_remove_all`. The buffer keeps its pre-overflow content, and
 * further write operations become no-ops. It can only be reused by
 * re-initializing it with `mulle_buffer_reset` or `mulle_buffer_done` + init.
 * Use `mulle_buffer_return_if_overflown` to apply the official check pattern.
 *
 * @param buffer The buffer to check.
 * @return 1 if the buffer has overflown, 0 otherwise. A NULL buffer is
 *         considered to have overflown and returns 1.
 */
static inline int   mulle_buffer_has_overflown( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 1);

   return( _mulle__buffer_has_overflown( (struct mulle__buffer *) buffer));
}


//
// Convenience for the official write contract: after a series of `void`
// writes, return `rval` as soon as the buffer has overflown.
//
#define mulle_buffer_return_if_overflown( buffer, rval)  \
   do                                                    \
   {                                                     \
      if( mulle_buffer_has_overflown( buffer))           \
         return( rval);                                  \
   }                                                     \
   while( 0)


/**
 * Checks if the bytes pointed to by `bytes` of length `length` intersect with the
 * contents of the `mulle_buffer` object.
 *
 * @param buffer The `mulle_buffer` object to check.
 * @param bytes A pointer to the bytes to check for intersection.
 * @param length The length of the bytes to check.
 * @return 1 if the bytes intersect with the buffer contents, 0 otherwise.
 */
//
// check if bytes of length, are actually buffer contents
//
static inline int   mulle_buffer_intersects_bytes( struct mulle_buffer *buffer,
                                                   void *bytes,
                                                   size_t length)
{
   if( ! buffer)
      return( length == 0);

   return( _mulle__buffer_intersects_bytes( (struct mulle__buffer *) buffer,
                                             bytes,
                                             length));
}


#pragma mark - additions

/**
 * Guarantees that the buffer has at least the specified length of unused space.
 *
 * This function ensures that the buffer has enough space to accommodate the
 * requested length. If the buffer does not have enough space, it will be
 * reallocated to a larger size.
 *
 * @param buffer The buffer to guarantee space for.
 * @param length The minimum length of unused space required.
 * @return A pointer to the unused area of the buffer, or NULL if the length
 *         cannot be guaranteed. Use `mulle_buffer_advance` to forward the
 *         current pointer after writing into the area.
 */
//
// returns NULL, if length can not be guaranteed. Otherwise a pointer to
// the unused area. Use mulle_buffer_advance to forward the current
// pointer after writing into the area.
//
static inline void   *mulle_buffer_guarantee( struct mulle_buffer *buffer,
                                              size_t length)
{
   if( ! buffer)
      return( NULL);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_guarantee( (struct mulle__buffer *) buffer,
                                     length,
                                     mulle_buffer_get_allocator( buffer)));
}


/**
 * Adds a single byte to the buffer.
 *
 * This function appends the specified byte to the end of the buffer. If the
 * buffer does not have enough space to accommodate the new byte, the buffer
 * will be reallocated to a larger size.
 *
 * @param buffer The buffer to add the byte to.
 * @param c The byte to add to the buffer.
 */
static inline void   mulle_buffer_add_byte( struct mulle_buffer *buffer,
                                            unsigned char c)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_byte( (struct mulle__buffer *) buffer,
                            c,
                            mulle_buffer_get_allocator( buffer));
}


/**
 * Removes the last byte from the buffer.
 *
 * This function removes the last byte from the buffer. If the buffer is empty,
 * this function does nothing.
 *
 * @param buffer The buffer to remove the last byte from.
 */
static inline void   mulle_buffer_remove_last_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_remove_last_byte( (struct mulle__buffer *) buffer);
}



/**
 * Removes and returns the last byte from the buffer.
 *
 * This function removes and returns the last byte from the buffer. If the buffer
 * is empty, this function returns -1.
 *
 * @param buffer The buffer to remove the last byte from.
 * @return The last byte from the buffer, or -1 if the buffer is empty.
 */
static inline int    _mulle_buffer_pop_byte( struct mulle_buffer *buffer)
{
   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_pop_byte( (struct mulle__buffer *) buffer,
                                    buffer->_allocator));
}


/**
 * Removes and returns the last byte from the buffer.
 *
 * This function removes and returns the last byte from the buffer. If the buffer
 * is empty, this function returns -1.
 *
 * @param buffer The buffer to remove the last byte from.
 * @return The last byte from the buffer, or -1 if the buffer is empty.
 */
static inline int    mulle_buffer_pop_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_pop_byte( (struct mulle__buffer *) buffer,
                                    buffer->_allocator));
}


/**
 * Adds a single character to the buffer.
 *
 * This function adds the given character `c` to the end of the buffer. If the
 * buffer is `NULL`, this function does nothing. `c` is expected to be within the
 * range `CHAR_MIN`..`CHAR_MAX` (asserted in debug builds). Use
 * `mulle_buffer_add_byte` to append arbitrary byte values.
 *
 * @param buffer The buffer to add the character to.
 * @param c The character to add to the buffer.
 */
static inline void   mulle_buffer_add_char( struct mulle_buffer *buffer,
                                                  int c)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_char( (struct mulle__buffer *) buffer,
                            c,
                            mulle_buffer_get_allocator( buffer));
}


#pragma mark - add memory ranges


/**
 * Adds a byte array to the buffer.
 *
 * This function adds the given byte array `bytes` of length `length` to the end
 * of the buffer. If the buffer is `NULL`, this function does nothing. `bytes`
 * must not be `NULL`.
 *
 * `bytes` must not point into the buffer's own storage: self-append is not
 * supported. The copy happens after the buffer may have grown (and moved)
 * its storage or flushed it to a sink, so the source would be stale.
 *
 * @param buffer The buffer to add the byte array to.
 * @param bytes The byte array to add to the buffer.
 * @param length The length of the byte array to add.
 */
static inline void   mulle_buffer_add_bytes( struct mulle_buffer *buffer,
                                             void *bytes,
                                             size_t length)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_bytes( (struct mulle__buffer *) buffer,
                             bytes,
                             length,
                             mulle_buffer_get_allocator( buffer));
}


// just as above just type differently
static inline
void   mulle_buffer_add_chars( struct mulle_buffer *buffer,
                               char *s,
                               size_t length)
{
   mulle_buffer_add_bytes( buffer, s, length);
}


/**
 * Adds a byte array to the buffer using a callback function.
 *
 * This function adds the given byte array `bytes` of length `length` to the end
 * of the buffer using the provided `mulle_buffer_add_bytes_callback` function.
 * If the buffer is `NULL`, this function does nothing.
 * Consider using mulle-buffer-stdio-functions instead for more algorithmic
 * flexibility.
 *
 * @param buffer The buffer to add the byte array to.
 * @param bytes The byte array to add to the buffer.
 * @param length The length of the byte array to add.
 */
// MEMO: types are different to stay compatible with:
//       mulle_utf_add_bytes_function_t
//       that's why we use void here
MULLE__BUFFER_GLOBAL
void   mulle_buffer_add_bytes_callback( void *buffer,
                                        void *bytes,
                                        size_t length);


/**
 * Adds a C string to the buffer.
 *
* This function adds the given C string `s` to the end of the buffer. If the
 * buffer is `NULL`, this function does nothing. `s` must not be `NULL`.
 *
 * @param buffer The buffer to add the string to.
 * @param s The C string to add to the buffer.
 */
static inline void   mulle_buffer_add_string( struct mulle_buffer *buffer,
                                              char *s)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_string( (struct mulle__buffer *) buffer,
                              s,
                              mulle_buffer_get_allocator( buffer));
}


// just an alias the LLVMs prefer, so why not
static inline void   mulle_buffer_append_string( struct mulle_buffer *buffer,
                                                 char *s)
{
   mulle_buffer_add_string( buffer, s);
}

/**
 * Adds a single character to the buffer.
 *
 * This function adds the given character `c` to the end of the buffer. If the
 * buffer is `NULL`, this function does nothing.
 *
 * @param buffer The buffer to add the character to.
 * @param c The character to add to the buffer.
 */
static inline void   mulle_buffer_add_c_char( struct mulle_buffer *buffer,
                                              char c)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_c_char( (struct mulle__buffer *) buffer,
                              c,
                              mulle_buffer_get_allocator( buffer));
}


/**
 * Adds a sequence of characters to the buffer.
 *
 * This function adds the given sequence of characters `s` of length `length` to
 * the end of the buffer. If the buffer is `NULL`, this function does nothing.
 *
 * @param buffer The buffer to add the characters to.
 * @param s The sequence of characters to add to the buffer.
 * @param length The number of characters to add from `s`.
 */
static inline void   mulle_buffer_add_c_chars( struct mulle_buffer *buffer,
                                               char *s,
                                               size_t length)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_c_chars( (struct mulle__buffer *) buffer,
                               s,
                               length,
                               mulle_buffer_get_allocator( buffer));
}


/**
 * Adds a sequence of characters to the buffer.
 *
 * This function adds the given sequence of characters `bytes` of length `length`
 * to the end of the buffer. This function is a callback used internally by the
 * `mulle_buffer` implementation.
 *
 * @param buffer The buffer to add the characters to.
 * @param bytes The sequence of characters to add to the buffer.
 * @param length The number of characters to add from `bytes`.
 */
MULLE__BUFFER_GLOBAL
void   mulle_buffer_add_c_chars_callback( void *buffer,
                                          void *bytes,
                                          size_t length);

/**
 * Adds a C-style string to the buffer, escaping any special characters.
 *
 * This function adds the given C-style string `s` to the end of the buffer,
 * escaping any special characters (such as newlines, tabs, etc.) and wrapping
 * the entire string in double quotes. If the buffer is `NULL`, this function
 * does nothing.
 *
 * @param buffer The buffer to add the string to.
 * @param s The C-style string to add to the buffer.
 */
//
// produces C escape codes and wraps everything in ""
//
static inline void   mulle_buffer_add_c_string( struct mulle_buffer *buffer,
                                                char *s)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_c_string( (struct mulle__buffer *) buffer,
                                 s,
                                 mulle_buffer_get_allocator( buffer));
}


/**
 * Appends the given C-style string to the buffer.
 *
 * This function is a synonym for `mulle_buffer_add_string()`, and simply appends
 * the given C-style string `bytes` to the end of the buffer. If the buffer is
 * `NULL`, this function does nothing.
 *
 * @param buffer The buffer to append the string to.
 * @param bytes The C-style string to append to the buffer.
 */
// just a synonym for add_string
static inline void   mulle_buffer_strcat( struct mulle_buffer *buffer,
                                          char *bytes)
{
   mulle_buffer_add_string( buffer, bytes);
}


/**
 * Copies the given C-style string to the buffer, clearing any existing contents.
 *
 * This function first clears the contents of the buffer by setting its length to
 * 0, and then adds the given C-style string `bytes` to the buffer. If the buffer
 * is `NULL`, this function does nothing.
 *
 * @param buffer The buffer to copy the string to.
 * @param bytes The C-style string to copy to the buffer.
 */
// clears the buffer and adds the string
static inline void   mulle_buffer_strcpy( struct mulle_buffer *buffer,
                                          char *bytes)
{
   mulle_buffer_set_length( buffer, 0, MULLE_BUFFER_NO_SHRINK_OR_ZEROFILL);
   mulle_buffer_add_string( buffer, bytes);
}


/**
 * Appends the given C-style string to the buffer, if the buffer is not empty.
 *
 * This function appends the given C-style string `bytes` to the end of the buffer,
 * but only if the buffer is not empty. If the buffer is `NULL`, this function
 * does nothing.
 *
 * @param buffer The buffer to append the string to.
 * @param bytes The C-style string to append to the buffer.
 */
static inline void   mulle_buffer_add_string_if_empty( struct mulle_buffer *buffer,
                                                       char *bytes)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_string_if_empty( (struct mulle__buffer *) buffer,
                                       bytes,
                                       mulle_buffer_get_allocator( buffer));
}


/**
 * Appends the given C-style string to the buffer, if the buffer is not empty.
 *
 * This function appends the given C-style string `bytes` to the end of the buffer,
 * but only if the buffer is not empty. If the buffer is `NULL`, this function
 * does nothing.
 *
 * @param buffer The buffer to append the string to.
 * @param bytes The C-style string to append to the buffer.
 */
static inline void   mulle_buffer_add_string_if_not_empty( struct mulle_buffer *buffer,
                                                           char *bytes)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_string_if_not_empty( (struct mulle__buffer *) buffer,
                                           bytes,
                                           mulle_buffer_get_allocator( buffer));
}


/**
 * Appends a C-style string to the buffer, limiting the maximum length.
 *
 * This function appends the given C-style string `bytes` to the end of the buffer,
 * but limits the maximum length of the string to `length` bytes. If the buffer is
 * `NULL`, this function returns 0.
 *
 * @param buffer The buffer to append the string to.
 * @param bytes The C-style string to append to the buffer.
 * @param length The maximum length of the string to append.
 * @return The number of bytes actually appended to the buffer.
 */
static inline size_t
   mulle_buffer_add_string_with_maxlength( struct mulle_buffer *buffer,
                                           char *bytes,
                                           size_t length)
{
   if( ! buffer)
      return( 0);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_add_string_with_maxlength( (struct mulle__buffer *) buffer,
                                                     bytes,
                                                     length,
                                                     mulle_buffer_get_allocator( buffer)));
}


/**
 * Sets the contents of the buffer to the given byte value.
 *
 * This function sets the contents of the buffer to the given byte value `c` for
 * the first `length` bytes of the buffer. If the buffer is `NULL`, this function
 * does nothing. If the buffer is inflexible, a too large memset with respect
 * to remaining length will lead to an overflow.
 *
 * @param buffer The buffer to set the contents of.
 * @param c The byte value to set the contents of the buffer to.
 * @param length The number of bytes to set to the given value.
 */
static inline void   mulle_buffer_memset( struct mulle_buffer *buffer,
                                          int c,
                                          size_t length)
{
   if( ! buffer)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_memset( (struct mulle__buffer *) buffer,
                         c,
                         length,
                         mulle_buffer_get_allocator( buffer));
}



/**
 * Zeros the last byte of the buffer.
 *
 * This function sets the last byte of the buffer to zero. If the buffer is `NULL`
 * it returns -1. Note that on an empty (but allocated) buffer this writes a zero
 * byte at the first position without advancing the length.
 *
 * @param buffer The buffer to zero the last byte of.
 * @return -1 if the buffer is `NULL`, 0 if the last byte was zeroed without loss,
 *         1 if the buffer was full and the last byte was overwritten with a zero
 *         (lossy, potentially corrupting multi-byte UTF-8), 2 if the buffer is void
 *         (no storage).
 */
static inline int   mulle_buffer_zero_last_byte( struct mulle_buffer *buffer)
{
   MULLE__BUFFER_GLOBAL int
      _mulle_buffer_zero_last_byte( struct mulle_buffer *buffer);

   if( ! buffer)
      return( -1);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle_buffer_zero_last_byte( buffer));
}


/**
 * Converts the buffer to a null-terminated string.
 *
 * This function converts the contents of the buffer to a null-terminated string.
 * If the buffer is `NULL`, this function returns 0. Otherwise, it returns the
 * length of the resulting string.
 *
 * @param buffer The buffer to convert to a string.
 * @return 0 : no truncation
 *         1 : truncated! (e.g. possibly UTF8 corruption
 *         2 : buffer is void (NULL)
 */
static inline int   mulle_buffer_make_string( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( 2);

   mulle_buffer_assert_writeable( buffer);

   return( _mulle__buffer_make_string( (struct mulle__buffer *) buffer,
                                        mulle_buffer_get_allocator( buffer)));
}


/**
 * Adds the contents of one buffer to another.
 *
 * This function adds the contents of the `other` buffer to the `buffer`. If
 * either `buffer` or `other` is `NULL`, this function does nothing.
 *
 * @param buffer The buffer to add the contents of `other` to.
 * @param other The buffer to add to `buffer`.
 */
static inline void    mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                               struct mulle_buffer *other)
{
   MULLE__BUFFER_GLOBAL void
      _mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                struct mulle_buffer *other);
   if( ! buffer || ! other)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle_buffer_add_buffer( buffer, other);
}


/**
 * Adds a range of bytes from another buffer to this buffer.
 *
 * This function adds the bytes in the specified range of the `other` buffer to
 * this buffer. If either `buffer` or `other` is `NULL`, this function does
 * nothing.
 *
 * @param buffer The buffer to add the bytes to.
 * @param other The buffer to copy the bytes from.
 * @param range The range of bytes to copy from `other`.
 */
static inline void
   mulle_buffer_add_buffer_range( struct mulle_buffer *buffer,
                                  struct mulle_buffer *other,  // sic
                                  struct mulle_range range)
{
   if( ! buffer || ! other)
      return;

   mulle_buffer_assert_writeable( buffer);

   _mulle__buffer_add_buffer_range( (struct mulle__buffer *) buffer,
                                    (struct mulle__buffer *) other,
                                    range,
                                    mulle_buffer_get_allocator( buffer));
}


/**
 * Resets the buffer to its initial state.
 *
 * This function resets the buffer to its initial state, discarding any existing
 * contents. The buffer's allocator is preserved.
 *
 * @param buffer The buffer to reset.
 */
// _initial_storage storage will be lost
MULLE__BUFFER_GLOBAL
void   mulle_buffer_reset( struct mulle_buffer *buffer);


#pragma mark - reading


/**
 * Access a byte from the buffer.
 *
 * @param buffer The buffer to get the byte from.
 * @return The byte or -1, if the buffer or index is invalid.
 */
static inline int   mulle_buffer_get_byte( struct mulle_buffer *buffer,
                                           size_t index)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_get_byte( (struct mulle__buffer *) buffer, index));
}


/*
 * Limited read support for buffers
 * -1 == no more bytes
 */
static inline int   mulle_buffer_next_bytes( struct mulle_buffer *buffer,
                                             void *buf,
                                             size_t len)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_next_bytes( (struct mulle__buffer *) buffer, buf, len));
}


/**
 * Returns a reference to the bytes in the buffer.
 *
 * This function returns a pointer to the bytes in the buffer, up to the specified
 * length. The returned pointer is valid until the buffer is modified or
 * destroyed.
 *
 * @param buffer The buffer to reference bytes from.
 * @param len The maximum number of bytes to reference.
 * @return A pointer to the bytes in the buffer, or NULL if the buffer is NULL.
 */
static inline void   *
   mulle_buffer_reference_bytes( struct mulle_buffer *buffer, size_t len)
{
   if( ! buffer)
      return( NULL);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_reference_bytes( (struct mulle__buffer *) buffer, len));
}


/**
 * Returns the last byte of the buffer.
 *
 * @param buffer The buffer to get the last byte from.
 * @return The last byte of the buffer, or -1 if the buffer is invalid.
 */
static inline int    mulle_buffer_get_last_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_get_last_byte( (struct mulle__buffer *) buffer));
}


/**
 * Reads the next byte from the buffer.
 *
 * This function reads the next byte from the buffer and returns it. If the
 * buffer is NULL or there are no more bytes to read, it returns -1.
 *
 * @param buffer The buffer to read from.
 * @return The next byte from the buffer, or -1 if the buffer is NULL or there
 *         are no more bytes to read.
 */
static inline int   mulle_buffer_next_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_next_byte( (struct mulle__buffer *) buffer));
}


/**
 * Peeks at the next byte in the buffer without removing it.
 *
 * This function returns the next byte in the buffer without removing it from the
 * buffer. If the buffer is NULL or there are no more bytes to read, it returns -1.
 *
 * @param buffer The buffer to peek at.
 * @return The next byte in the buffer, or -1 if the buffer is NULL or there are
 *         no more bytes to read.
 */
static inline int   mulle_buffer_peek_byte( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_peek_byte( (struct mulle__buffer *) buffer));
}

/**
 * Reads the next character from the buffer.
 *
 * This function reads the next character from the buffer and returns it. If the
 * buffer is NULL or there are no more characters to read, it returns -1.
 *
 * @param buffer The buffer to read from.
 * @return The next character from the buffer, or -1 if the buffer is NULL or there
 *         are no more characters to read.
 */
static inline int   mulle_buffer_next_character( struct mulle_buffer *buffer)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_next_character( (struct mulle__buffer *) buffer));
}


/**
 * Seeks for the next occurrence of the specified byte in the buffer.
 *
 * This function searches the buffer forward from the current seek position for
 * the next occurrence of the specified byte. If found, it moves the seek
 * position to the byte and returns 0. If the byte is not found (or the buffer
 * has overflown), it returns -1 and leaves the seek position unchanged.
 *
 * @param buffer The buffer to search.
 * @param byte The byte to search for.
 * @return 0 if the byte was found and the seek position advanced to it, or -1 if
 *         the byte is not found, the buffer is invalid, or the buffer has overflown.
 */
static inline long   mulle_buffer_seek_byte( struct mulle_buffer *buffer,
                                             unsigned char byte)
{
   if( ! buffer)
      return( -1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_seek_byte( (struct mulle__buffer *) buffer, byte));
}



/**
 * Compares the contents of the buffer to the given byte array.
 *
 * This function compares the contents of the buffer to the given byte array `bytes`
 * for the first `length` bytes. If the buffer is `NULL`, this function returns +1.
 *
 * @param buffer The buffer to compare to the byte array.
 * @param bytes The byte array to compare to the buffer.
 * @param length The number of bytes to compare.
 * @return A negative value if the buffer is less than the byte array, 0 if they are
 *         equal, and a positive value if the buffer is greater than the byte array.
 */
static inline int   mulle_buffer_memcmp( struct mulle_buffer *buffer,
                                         void  *bytes,
                                         size_t length)
{
   if( ! buffer)
      return( +1);

   mulle_buffer_assert_readable( buffer);

   return( _mulle__buffer_memcmp( (struct mulle__buffer *) buffer,
                                  bytes,
                                  length));
}


#pragma mark - hexdump fixed to 16 bytes per line

/**
 * Enumeration of options for the `mulle_buffer_hexdump` function.
 *
 * These options can be used to control the behavior of the `mulle_buffer_hexdump`
 * function, which dumps the contents of a buffer in a hexadecimal format.
 *
 * - `mulle_buffer_hexdump_default`: The default behavior, which includes the offset,
 *   hexadecimal representation, and ASCII representation.
 * - `mulle_buffer_hexdump_no_offset`: Omits the offset from the output.
 * - `mulle_buffer_hexdump_no_hex`: Omits the hexadecimal representation from the output.
 * - `mulle_buffer_hexdump_no_ascii`: Omits the ASCII representation from the output.
 */
// actually they are bits to be ORed
enum mulle_buffer_hexdump_options
{
   mulle_buffer_hexdump_default   = 0x0,
   mulle_buffer_hexdump_no_offset = 0x1,
   mulle_buffer_hexdump_no_hex    = 0x2,
   mulle_buffer_hexdump_no_ascii  = 0x4
};


/**
 * Dumps a line of hexadecimal data from a buffer.
 *
 * This function dumps a line of hexadecimal data from the specified buffer, with
 * the number of bytes specified by the `n` parameter. The function respects the
 * specified options, which can be used to control the output format.
 *
 * @param buffer The buffer to dump.
 * @param bytes A pointer to the bytes to be dumped.
 * @param n The number of bytes to dump (must be between 1 and 16, inclusive).
 * @param counter The offset of the bytes being dumped.
 * @param options The options to use for the hexadecimal dump.
 */


// dumps only for n >= 1 && n <= 16
MULLE__BUFFER_GLOBAL
void  mulle_buffer_hexdump_line( struct mulle_buffer *buffer,
                                 void *bytes,
                                 unsigned int n,
                                 size_t counter,
                                 unsigned int options);

/**
 * Dumps the contents of a buffer in a hexadecimal format.
 *
 * This function dumps the contents of the specified buffer in a hexadecimal
 * format, with the number of bytes specified by the `length` parameter. The
 * function respects the specified options, which can be used to control the
 * output format.
 *
 * @param buffer The buffer to dump.
 * @param bytes A pointer to the bytes to be dumped.
 * @param length The number of bytes to dump.
 * @param counter The offset of the bytes being dumped.
 * @param options The options to use for the hexadecimal dump.
 */
// dumps all, does not append a \0
MULLE__BUFFER_GLOBAL
void  mulle_buffer_hexdump( struct mulle_buffer *buffer,
                            void *bytes,
                            size_t length,
                            size_t counter,
                            unsigned int options);


#pragma mark - backwards compatibility

MULLE_C_DEPRECATED
static inline int   mulle_buffer_is_inflexable( struct mulle_buffer *buffer)
{
   return( mulle_buffer_is_inflexible( buffer));
}


MULLE_C_DEPRECATED static inline void
   mulle_buffer_init_inflexable_with_static_bytes( struct mulle_buffer *buffer,
                                                   void *storage,
                                                   size_t length)
{
   mulle_buffer_init_inflexible_with_static_bytes( buffer, storage, length);
}


MULLE_C_DEPRECATED static inline void
   mulle_buffer_make_inflexable( struct mulle_buffer *buffer,
                                 void *storage,
                                 size_t length)
{
   mulle_buffer_make_inflexible( buffer, storage, length);
}




// convenience:
// static void   example( void)
// {
//    char   *s;
//
//    mulle_buffer_do_string( buffer, NULL, s)
//    {
//       mulle_buffer_add_string( buffer, "VfL Bochum 1848");
//       break;
//    }
//
//    printf( "%s\n", s);
//    mulle_free( s);
// }

/**
 * Provides a convenience macro `mulle_buffer_do_string` that allows you to
 * easily work with a `mulle_buffer` and extract the resulting string.
 *
 * The macro creates a local `mulle_buffer` instance, performs some operations
 * on it, and then extracts the resulting string. The string is assigned to the
 * `s` variable provided in the macro invocation. The buffer is automatically
 * cleaned up when the block is exited.
 *
 * Example usage:
 *
 *
 * char *s;
 * mulle_buffer_do_string(buffer, NULL, s) {
 *    mulle_buffer_add_string( buffer, "VfL Bochum 1848");
 *    break;
 * }
 * printf("%s\n", s);
 * mulle_free(s);
 *
 *
 * Caveats:
 * - Do not use `return` in the block, or you will leak. Use `mulle_buffer_return`
 *   instead.
 * - Do not pre-initialize the buffer.
 */
#define mulle_buffer_do_string( name, allocator, s)               \
   for( struct mulle_buffer                                       \
           name ## __storage = _mulle_buffer_data( allocator),    \
           *name = &name ## __storage,                            \
           *name ## __i = NULL;                                   \
                                                                  \
        s = (name ## __i)                                         \
               ? mulle_buffer_extract_string( &name ## __storage) \
               : NULL,                                            \
        ! name ## __i;                                            \
                                                                  \
        name ## __i = (void *) 0x1                                \
      )                                                           \
                                                                  \
      MULLE_C_CONFINED_LOOP                                       \
      for( int  name ## __j = 0;    /* break protection */        \
           name ## __j < 1;                                       \
           name ## __j++)


/**
 * Safely creates and uses a `mulle_buffer` within a block.
 *
 * This macro creates a `mulle_buffer` that is initialized with the default
 * allocator. The buffer can be modified and used within the block. The macro
 * ensures that the `mulle_buffer` is properly cleaned up before exiting the
 * block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 */
//
// have a buffer inside the block, will self destruct when the block
// is exited (properly)
//
#define _mulle_buffer_chars_to_struct( len) \
   ((len + sizeof( struct mulle_buffer) - 1) / sizeof( struct mulle_buffer))

#define mulle_buffer_do( name)                                                              \
   for( struct mulle_buffer                                                                 \
           name ## __alloca[ _mulle_buffer_chars_to_struct( MULLE_BUFFER_DEFAULT_CAPACITY)], \
           name ## __storage = _mulle_buffer_flexible_data( name ## __alloca,               \
                                                           sizeof( name ## __alloca),       \
                                                           NULL),                           \
           *name = &name ## __storage,                                                       \
           *name ## __i = NULL;                                                              \
         ! name ## __i;                                                                      \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1)               \
      )                                                                                     \
                                                                                            \
      MULLE_C_CONFINED_LOOP                                                                 \
      for( int  name ## __j = 0;    /* break protection */                                  \
           name ## __j < 1;                                                                 \
           name ## __j++)

/**
 * Safely creates and uses a `mulle_buffer` with a custom allocator.
 *
 * This macro creates a `mulle_buffer` that is initialized with the provided
 * `allocator` parameter. The buffer can be modified and used within the block.
 * The macro ensures that the `mulle_buffer` is properly cleaned up before
 * exiting the block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 * @param allocator The custom allocator to use for the `mulle_buffer`.
 */
//
// As above but the buffer storage allocator can be changed from the
// default allocator
//
#define mulle_buffer_do_allocator( name, allocator)                                        \
   for( struct mulle_buffer                                                                \
           name ## __alloca[ _mulle_buffer_chars_to_struct(MULLE_BUFFER_DEFAULT_CAPACITY)], \
           name ## __storage = _mulle_buffer_flexible_data( name ## __alloca,              \
                                                           sizeof( name ## __alloca),      \
                                                           allocator),                     \
           *name = &name ## __storage,                                                      \
           *name ## __i = NULL;                                                             \
         ! name ## __i;                                                                     \
         name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1)              \
       )                                                                                    \
                                                                                            \
       MULLE_C_CONFINED_LOOP                                                                \
       for( int  name ## __j = 0;    /* break protection */                                 \
            name ## __j < 1;                                                                \
            name ## __j++)

/**
 * Safely creates and uses a `mulle_buffer` backed by user-provided storage.
 *
 * This macro creates a `mulle_buffer` that uses `data` of size `len` as its
 * initial storage. The buffer starts empty and can be written to. If the
 * content exceeds `len`, the buffer will malloc additional storage.
 * The macro ensures that the `mulle_buffer` is properly cleaned up before
 * exiting the block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 * @param data The storage to use as the initial backing memory.
 * @param len The size of the storage in bytes.
 */
//
// Create a buffer with some static/auto storage preset. If that is
// exhausted the buffer will start mallocing. Nice to avoid an initial malloc
// for small workloads.
//
#define mulle_buffer_do_flexible( name, data, len)                            \
   for( struct mulle_buffer                                                   \
          name ## __storage = _mulle_buffer_flexible_data( data, len, NULL),      \
          *name = &name ## __storage,                                             \
          *name ## __i = NULL;                                                    \
        ! name ## __i;                                                        \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1) \
      )                                                                       \
                                                                              \
      MULLE_C_CONFINED_LOOP                                                   \
      for( int  name ## __j = 0;    /* break protection */                    \
           name ## __j < 1;                                                   \
           name ## __j++)


/**
 * Safely creates and uses a `mulle_buffer` pre-filled with existing data.
 *
 * This macro creates a `mulle_buffer` that is initialized with the provided
 * `data` and `len` parameters as pre-existing content. The buffer can grow
 * beyond `len` via malloc. The macro ensures that the `mulle_buffer` is
 * properly cleaned up before exiting the block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 * @param data The data that pre-fills the `mulle_buffer`.
 * @param len The length of the pre-filled data.
 */
//
// Create a buffer with some static/auto storage preset. This storage already
// contains bytes for the buffer. Otherwise as above.
//
#define mulle_buffer_do_flexible_filled( name, data, len)                          \
   for( struct mulle_buffer                                                        \
          name ## __storage = _mulle_buffer_flexible_filled_data( data, len, NULL),      \
          *name = &name ## __storage,                                                   \
          *name ## __i = NULL;                                                          \
        ! name ## __i;                                                             \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1)      \
      )                                                                            \
                                                                                   \
      MULLE_C_CONFINED_LOOP                                                        \
      for( int  name ## __j = 0;    /* break protection */                         \
           name ## __j < 1;                                                        \
           name ## __j++)


/**
 * Safely creates and uses a fixed-size `mulle_buffer` that will not malloc.
 *
 * This macro creates a `mulle_buffer` backed by `data` of size `len`. The
 * buffer starts empty and can be written to, but will not grow beyond `len`.
 * If the content would exceed the storage, the buffer enters the "overflown"
 * state. The macro ensures cleanup before exiting the block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 * @param data The storage to use as backing memory.
 * @param len The size of the storage in bytes.
 */
//
// Like mulle_buffer_do_flexible but when the buffer is full, there won't be
// a malloc.
//
#define mulle_buffer_do_inflexible( name, data, len)                          \
   for( struct mulle_buffer                                                   \
          name ## __storage = _mulle_buffer_inflexible_data( data, len, NULL),      \
          *name = &name ## __storage,                                             \
          *name ## __i = NULL;                                                    \
        ! name ## __i;                                                        \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1) \
      )                                                                       \
                                                                              \
      MULLE_C_CONFINED_LOOP                                                   \
      for( int  name ## __j = 0;    /* break protection */                    \
           name ## __j < 1;                                                   \
           name ## __j++)

/**
 * Safely creates and uses a `mulle_buffer` that is already filled with data.
 *
 * This macro creates a fixed-size `mulle_buffer` over `data` of length `len`,
 * where the storage is already preset with content. The buffer does not set the
 * read-only flag: it is a writable, fixed-size buffer, so writes overwrite the
 * pre-filled content and anything beyond `len` puts the buffer into the
 * "overflown" state. The macro ensures that the `mulle_buffer` is properly
 * cleaned up before exiting the block.
 *
 * @param name The name of the `mulle_buffer` variable to be used in the block.
 * @param data The data to initialize the `mulle_buffer` with.
 * @param len The length of the data to initialize the `mulle_buffer` with.
 */
//
// Like above, but the buffer is already preset with data.
//
#define mulle_buffer_do_inflexible_filled( name, data, len)                          \
   for( struct mulle_buffer                                                          \
          name ## __storage = _mulle_buffer_inflexible_filled_data( data, len, NULL),      \
          *name = &name ## __storage,                                                   \
          *name ## __i = NULL;                                                          \
        ! name ## __i;                                                               \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1)        \
      )                                                                              \
                                                                                     \
      MULLE_C_CONFINED_LOOP                                                          \
      for( int  name ## __j = 0;    /* break protection */                           \
           name ## __j < 1;                                                          \
           name ## __j++)


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
#ifdef __has_builtin
# if __has_builtin(__builtin_types_compatible_p)
#  define MULLE__BUFFER_HAVE_TYPEOF
# endif
#endif

#ifndef MULLE__BUFFER_HAVE_TYPEOF
# if defined( __GNUC__) || defined( __clang__)
#  define MULLE__BUFFER_HAVE_TYPEOF
# endif
#endif

#ifdef MULLE__BUFFER_HAVE_TYPEOF

#define mulle_buffer_return( name, value)           \
   do                                               \
   {                                                \
      __typeof__( value) name ## __tmp = (value);   \
                                                    \
      mulle_buffer_done( &name ## __storage);       \
      return( name ## __tmp);                       \
   }                                                \
   while( 0)

#endif

//
// TODO:
//#define mulle_buffer_do_FILE( name, FP)


#include "mulle-flushablebuffer.h"


#ifdef __has_include
# if __has_include( "_mulle-buffer-versioncheck.h")
#  include "_mulle-buffer-versioncheck.h"
# endif
#endif


#endif /* mulle_buffer_h */
