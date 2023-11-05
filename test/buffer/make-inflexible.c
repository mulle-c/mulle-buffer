#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>


int  main()
{
   struct mulle_buffer   *buffer;
   char                  tmp[ 32];

   buffer = mulle_buffer_create( NULL);
   mulle_buffer_add_string( buffer, "VfL Bochum 1848");
   printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                     (char *) mulle_buffer_get_bytes( buffer));

   mulle_buffer_copy_range( buffer, mulle_range_make( 4, 6), tmp);
   printf( "%.*s\n", 6,
                     tmp);

   mulle_buffer_make_inflexible( buffer, tmp, sizeof( tmp));

   mulle_buffer_destroy( buffer);

   return( 0);
}

