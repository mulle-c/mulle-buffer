
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example()
{
   struct mulle_buffer   *buf;
   char                  *s;

   buf = mulle_buffer_create( NULL);
   mulle_buffer_add_string( buf, "VfL");
   s = mulle_buffer_extract_string( buf);
   mulle_buffer_destroy( buf);

   printf( "%s\n", s);
   mulle_free( s);
}


int  main()
{
   fprintf( stderr, "test start: mulle_default_allocator (%p)\n", &mulle_default_allocator);

   example();

   return( 0);
}
