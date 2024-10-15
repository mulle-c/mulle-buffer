
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>

static char   text[] = "VfL Bochum 1848";

static void   example()
{
   struct mulle_buffer   *buf;
   char                  *s;

   buf = mulle_buffer_create( NULL);
   mulle_buffer_init_with_const_bytes( buf, text, sizeof( text));
   mulle_buffer_set_seek( buf, 0, MULLE_BUFFER_SEEK_END);
   printf( "%.*s\n", (int) mulle_buffer_get_length( buf), (char *) mulle_buffer_get_bytes( buf));
   mulle_buffer_destroy( buf);
}


int  main()
{
   fprintf( stderr, "test start: mulle_default_allocator (%p)\n", &mulle_default_allocator);

   example();

   return( 0);
}
