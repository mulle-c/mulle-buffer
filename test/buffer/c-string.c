
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example( void)
{
   char   *s;

   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle_buffer_add_c_string( buffer, "VfL\n\"Bochum\"\n1848");
      mulle_buffer_add_string( buffer, "\n");
      mulle_buffer_add_c_string( buffer, "\\\a\b\e\f\t\n\r\vWhat ???\017\322-");
      break;
   }

   printf( "%s\n", s);
   mulle_free( s);
}


int  main()
{
   example();

   return( 0);
}
