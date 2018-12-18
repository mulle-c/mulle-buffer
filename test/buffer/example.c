
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

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
   mulle_testallocator_initialize();
   mulle_default_allocator = mulle_testallocator;

   example();

   mulle_testallocator_reset();
   return( 0);
}
