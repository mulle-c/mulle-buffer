
#include <mulle_buffer/mulle_buffer.h>
#include <mulle_test_allocator/mulle_test_allocator.h>
#include <stdio.h>


static void   example()
{
   struct mulle_buffer   *buf;
   char                  *s;

   buf = mulle_buffer_create( NULL);
   mulle_buffer_add_string( buf, "VfL");
   mulle_buffer_add_character( buf, 0);
   s = mulle_buffer_extract_all( buf);
   mulle_buffer_destroy( buf);

   printf( "%s\n", s);
   mulle_free( s);
}


int  main()
{
   mulle_test_allocator_initialize();
   mulle_default_allocator = mulle_test_allocator;

   example();

   mulle_test_allocator_reset();
   return( 0);
}
