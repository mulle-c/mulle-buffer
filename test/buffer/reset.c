#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


// just fishing for leaks
static void   test( void)
{
   struct mulle_buffer  *buffer;

   buffer = mulle_buffer_create( NULL);
   mulle_buffer_reset( buffer);
   mulle_buffer_add_string( buffer, "VfL Bochum 1848");
   mulle_buffer_reset( buffer);
   mulle_buffer_destroy( buffer);
}


int  main()
{
   test();
   return( 0);
}

