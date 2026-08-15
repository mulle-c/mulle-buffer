#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   test_flexible()
{
   char   *s;
   char   storage[ 128];

   mulle_buffer_do_flexible( buf, storage, sizeof( storage))
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
}


static void   test_inflexible()
{
   char   *s;
   char   storage[ 128];

   mulle_buffer_do_inflexible( buf, storage, sizeof( storage))
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
}



static void   test_dynamic()
{
   char                  *s;
   struct mulle_buffer   *buf;

   buf = mulle_buffer_create( NULL);
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
   mulle_buffer_destroy( buf);
}


//
// the empty extraction rule: all three extraction functions return an
// empty/NULL result for an empty buffer, no allocation
//
static int   test_empty_extraction()
{
   struct mulle_buffer   buf;
   struct mulle_allocator   *allocator;
   struct mulle_data      data;
   char                   *s;
   void                   *bytes;

   mulle_buffer_init_default( &buf);

   data  = mulle_buffer_extract_data( &buf);
   if( data.bytes != NULL || data.length != 0)
      return( 1);
   s = mulle_buffer_extract_string( &buf);
   if( s != NULL)
      return( 2);
   bytes = mulle_buffer_extract_bytes( &buf);
   if( bytes != NULL)
      return( 3);

   // buffer must be reusable after empty extraction
   mulle_buffer_add_string( &buf, "VfL");
   data = mulle_buffer_extract_data( &buf);
   if( data.length != 3)
      return( 4);
   mulle_free( data.bytes);

   // empty over a stack-backed buffer
   mulle_buffer_init_with_static_bytes( &buf, "x", 1, NULL);
   s = mulle_buffer_extract_string( &buf);
   if( s != NULL)
      return( 5);

   allocator = &mulle_stdlib_allocator;
   mulle_buffer_init_with_allocated_bytes( &buf, NULL, 0, allocator);
   s = mulle_buffer_extract_string( &buf);
   if( s != NULL)
      return( 6);

   return( 0);
}


int  main()
{
   test_flexible();
   test_inflexible();
   test_dynamic();
   return( test_empty_extraction());
}

