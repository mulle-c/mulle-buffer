
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example( void)
{
   char   tmp[ 256];

   mulle_buffer_do_flexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_remove_in_range( buffer, mulle_range_make_all());
      printf( "1: \"%s\"\n", mulle_buffer_get_string( buffer));

      // get_string will introduce a trailing \0
      mulle_buffer_remove_all( buffer);
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_remove_in_range( buffer, mulle_range_make( 1, mulle_buffer_get_length( buffer) - 2));
      printf( "2: \"%s\"\n", mulle_buffer_get_string( buffer));

      mulle_buffer_remove_all( buffer);
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_remove_in_range( buffer, mulle_range_make( mulle_buffer_get_length( buffer) - 1, 1));
      printf( "3: \"%s\"\n", mulle_buffer_get_string( buffer));

      mulle_buffer_remove_all( buffer);
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_remove_in_range( buffer, mulle_range_make( 0, 1));
      printf( "4: \"%s\"\n", mulle_buffer_get_string( buffer));

      mulle_buffer_remove_all( buffer);
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_remove_in_range( buffer, mulle_range_make( 0, 100));
      printf( "5: \"%s\"\n", mulle_buffer_get_string( buffer));
   }
}


int  main()
{
   example();

   return( 0);
}
