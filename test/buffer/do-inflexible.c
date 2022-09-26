
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example( void)
{
   char   tmp[ 8];

   mulle_buffer_do_inflexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}


int  main()
{
   example();

   return( 0);
}
