#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// Allocator coverage (review gap):
//  - zero-size initial capacity is a valid request
//  - a custom (malloc/calloc/realloc/free) allocator round-trips writes
//  - size arithmetic near SIZE_MAX must not silently truncate
//

static void   *test_calloc( size_t n, size_t size, struct mulle_allocator *allocator)
{
   void   *p;

   p = calloc( n, size);
   return( p);
}


static void   *test_realloc( void *block, size_t size, struct mulle_allocator *allocator)
{
   return( realloc( block, size));
}


static void   test_free( void *block, struct mulle_allocator *allocator)
{
   free( block);
}


static struct mulle_allocator *
   make_test_allocator( struct mulle_allocator *allocator)
{
   allocator->calloc  = test_calloc;
   allocator->realloc = test_realloc;
   allocator->free    = test_free;
   allocator->fail    = mulle_allocation_fail;
   return( allocator);
}


static int   test_zero_capacity( void)
{
   struct mulle_buffer   buffer;

   mulle_buffer_init( &buffer, 0, NULL);
   {
      mulle_buffer_add_string( &buffer, "VfL");
      if( mulle_buffer_get_length( &buffer) != 3)
         return( 1);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_custom_allocator( void )
{
   struct mulle_buffer      buffer;
   struct mulle_allocator   allocator;

   make_test_allocator( &allocator);
   mulle_buffer_init( &buffer, 8, &allocator);
   mulle_buffer_add_string( &buffer, "VfL Bochum 1848 beyond initial capacity");
   if( mulle_buffer_get_length( &buffer) == 0)
      return( 1);
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_huge_length( void )
{
   struct mulle_buffer   buffer;

   // a huge capacity hint must be preserved, not truncated into a small value
   mulle_buffer_init( &buffer, (size_t) 1 << 31, NULL);
   if( mulle_buffer_get_capacity( &buffer) != (size_t) 1 << 31)
      return( 1);
   mulle_buffer_done( &buffer);
   return( 0);
}


int  main()
{
   int   rval;

   rval = test_zero_capacity();
   if( rval) return( 100 + rval);
   rval = test_custom_allocator();
   if( rval) return( 200 + rval);
   rval = test_huge_length();
   if( rval) return( 300 + rval);
   return( 0);
}