#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>



static void   example( char *text, size_t i)
{
   char   tmp[ 8]; // = { 0 }; // 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x' };

   mulle_buffer_do_flexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_c_chars( buffer, text, i);
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}


int  main()
{
   char     *s;
   size_t   len;
   size_t   i;

   s   = "VfL Bochum 1848";
   len = strlen( s);
   for( i = 0; i <= len; i++)
      example( s, i);

   return( 0);
}
