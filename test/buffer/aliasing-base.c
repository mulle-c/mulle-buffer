//
//  aliasing-base.c
//
//  Direct regression test for the nested-member layout the buffer family
//  switched to. `struct mulle_buffer` must embed its `struct mulle__buffer`
//  as a real member named `base` at offset 0, and `struct
//  mulle_flushablebuffer` must embed a `struct mulle_buffer` the same way.
//
//  The failure this guards against: the old layout-compatible-prefix cast
//
//      (struct mulle__buffer *) buffer
//
//  is strict-aliasing undefined behavior (C11 6.5p7). At -O2/-O3 the
//  optimizer is then free to assume that a store through a `struct
//  mulle__buffer` lvalue cannot alias a load of a `struct mulle_buffer`
//  member (and vice versa), and it hoists the load out of the store. That is
//  exactly what made `buffer/intersect.c` fail at -O2, and it is why the
//  wrappers now hand out `&buffer->base` instead of casting.
//
//  The `static inline` accessors below present the memory as `struct
//  mulle__buffer *` while the caller still talks `struct mulle_buffer`, and
//  the public `mulle_buffer_intersects_bytes` wrapper is itself a `static
//  inline` that reaches through `&buffer->base`. They are small enough to be
//  inlined into the test, so the optimizer has to decide whether the stores
//  and loads alias. The hard guarantee here is the layout: the `offsetof`
//  asserts and the `&buffer->base` accesses below stop compiling if the
//  member is removed or moved, i.e. if the prefix pun comes back. The
//  interleaved store/load pairs and the inline `mulle_buffer_intersects_bytes`
//  wrapper then exercise the exact access pattern that the old cast got wrong
//  at -O2, under whatever optimization level the build uses.
//
#include <mulle-buffer/mulle-buffer.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


// `base` is the first member, so the outer address and the base address
// coincide. These hold at compile time; a layout regression fails the build.
_Static_assert( offsetof( struct mulle_buffer, base) == 0,
                "mulle_buffer.base must be the first member");
_Static_assert( offsetof( struct mulle_flushablebuffer, base) == 0,
                "mulle_flushablebuffer.base must be the first member");
_Static_assert( offsetof( struct mulle_flushablebuffer, base) +
                offsetof( struct mulle_buffer, base) == 0,
                "mulle_flushablebuffer.base.base must be at offset 0");
_Static_assert( sizeof( struct mulle_buffer) >= sizeof( struct mulle__buffer),
                "mulle_buffer must be at least as large as its base");


// The callee view: `struct mulle__buffer *`. These helpers are `static
// inline` so the optimizer reasons about the aliasing within one TU.
static inline void   _base_set_type( struct mulle__buffer *base,
                                     unsigned int type)
{
   base->_type = type;
}

static inline unsigned int   _base_get_type( struct mulle__buffer *base)
{
   return( base->_type);
}

static inline void   _base_set_storage( struct mulle__buffer *base,
                                        void *storage,
                                        void *sentinel)
{
   base->_storage  = (unsigned char *) storage;
   base->_sentinel = (unsigned char *) sentinel;
}


//
// `&buffer` and `&buffer->base` are the same address, and the accessors
// agree with the members.
//
static int   test_pointer_identity( void)
{
   struct mulle_buffer   buffer;
   struct mulle_buffer   *p;

   memset( &buffer, 0, sizeof( buffer));
   p = &buffer;

   if( (void *) p != (void *) &p->base)
      return( 1);
   if( mulle_buffer_as__buffer( p) != &p->base)
      return( 2);
   if( mulle_buffer_as__buffer( p) != (struct mulle__buffer *) p)
      return( 3);
   return( 0);
}


//
// Store through the outer handle, read through the inner type (and back).
// The interleaved stores matter: the last store must be observed by the
// differently-typed load, which is what a type pun would forbid.
//
static int   test_store_load_roundtrip( void)
{
   struct mulle_buffer   buffer;

   // raw object: only the members are touched here, so no storage to free
   memset( &buffer, 0, sizeof( buffer));

   // outer store, then inner store, then outer load
   buffer.base._type = MULLE_BUFFER_IS_READONLY;
   _base_set_type( &buffer.base, MULLE_BUFFER_IS_WRITEONLY);
   if( buffer.base._type != MULLE_BUFFER_IS_WRITEONLY)
      return( 1);

   // inner store, then outer store, then inner load
   _base_set_type( &buffer.base, MULLE_BUFFER_IS_READONLY);
   buffer.base._type = MULLE_BUFFER_IS_WRITEONLY;
   if( _base_get_type( &buffer.base) != MULLE_BUFFER_IS_WRITEONLY)
      return( 2);

   // same for pointer members
   buffer.base._storage = (unsigned char *) (uintptr_t) 0x10;
   _base_set_storage( &buffer.base, (void *) (uintptr_t) 0x30,
                                    (void *) (uintptr_t) 0x40);
   if( buffer.base._storage != (unsigned char *) (uintptr_t) 0x30)
      return( 3);
   if( buffer.base._sentinel != (unsigned char *) (uintptr_t) 0x40)
      return( 4);
   if( _base_get_type( &buffer.base) != MULLE_BUFFER_IS_WRITEONLY)
      return( 5);

   return( 0);
}


//
// The public `mulle_buffer_intersects_bytes` wrapper is `static inline` and
// computes through `&buffer->base`. Fabricate storage/sentinel exactly as
// `buffer/intersect.c` does and check the exact answers, so a hoisted load
// of `_storage`/`_sentinel` shows up as a wrong result rather than a diff.
//
static int   test_intersect_through_base( void)
{
   struct mulle_buffer   buffer;

   memset( &buffer, 0, sizeof( buffer));

   // mixed: one member stored through the outer handle, one through an
   // inner-typed inline setter
   buffer.base._storage = (unsigned char *) (uintptr_t) 0x10;
   _base_set_storage( &buffer.base, (void *) (uintptr_t) 0x10,
                                    (void *) (uintptr_t) 0x20);
   buffer.base._type    = MULLE_BUFFER_IS_FLEXIBLE;

   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x0,
                                      0x10) != 0) return( 1);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x0,
                                      0x11) != 1) return( 2);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x10,
                                      0x10) != 1) return( 3);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x10,
                                      0x11) != 1) return( 4);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x1F,
                                      0x10) != 1) return( 5);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x20,
                                      0x10) != 0) return( 6);
   if( mulle_buffer_intersects_bytes( &buffer, (void *) (uintptr_t) 0x11,
                                      0x0)  != 0) return( 7);
   return( 0);
}


//
// A live buffer keeps working when the same object is poked through the
// inner type: length accounting through the outer API must see the base
// mutation.
//
static int   test_live_mutation( void)
{
   struct mulle_buffer   buffer;
   char                  storage[ 64];
   size_t                length;

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage),
                                        NULL);
   mulle_buffer_add_string( &buffer, "VfL Bochum 1848");

   length = (size_t) ( buffer.base._curr - buffer.base._storage);
   if( length != mulle_buffer_get_length( &buffer))
      return( 1);

   // rewind through the inner type and let the outer API observe it
   buffer.base._curr = buffer.base._storage;
   _mulle__buffer_set_overflown( &buffer.base);
   if( ! mulle_buffer_has_overflown( &buffer))
      return( 2);

   mulle_buffer_done( &buffer);
   return( 0);
}


//
// Same nested-member contract for `struct mulle_flushablebuffer`, which
// embeds a `struct mulle_buffer` (and therefore a `struct mulle__buffer`) at
// offset 0.
//
static int   test_flushable_identity( void)
{
   struct mulle_flushablebuffer   buffer;
   struct mulle_flushablebuffer   *p;

   memset( &buffer, 0, sizeof( buffer));
   p = &buffer;

   if( (void *) p != (void *) &p->base)
      return( 1);
   if( (void *) p != (void *) &p->base.base)
      return( 2);
   if( mulle_flushablebuffer_as_buffer( p) != &p->base)
      return( 3);
   if( mulle_buffer_as__buffer( &p->base) != &p->base.base)
      return( 4);

   // a write through the innermost member is visible to the outermost
   // accessor, and vice versa
   p->base.base._type = MULLE_BUFFER_IS_READONLY;
   _base_set_type( &p->base.base, MULLE_BUFFER_IS_WRITEONLY);
   if( p->base.base._type != MULLE_BUFFER_IS_WRITEONLY)
      return( 5);
   if( mulle_flushablebuffer_as_buffer( p)->base._type
       != MULLE_BUFFER_IS_WRITEONLY)
      return( 6);
   return( 0);
}


int   main( void)
{
   int   rval;

   if( (rval = test_pointer_identity()))
   {
      printf( "test_pointer_identity: %d\n", rval);
      return( 100 + rval);
   }

   if( (rval = test_store_load_roundtrip()))
   {
      printf( "test_store_load_roundtrip: %d\n", rval);
      return( 200 + rval);
   }

   if( (rval = test_intersect_through_base()))
   {
      printf( "test_intersect_through_base: %d\n", rval);
      return( 300 + rval);
   }

   if( (rval = test_live_mutation()))
   {
      printf( "test_live_mutation: %d\n", rval);
      return( 400 + rval);
   }

   if( (rval = test_flushable_identity()))
   {
      printf( "test_flushable_identity: %d\n", rval);
      return( 500 + rval);
   }

   return( 0);
}
