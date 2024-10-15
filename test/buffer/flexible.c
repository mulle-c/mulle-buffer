#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>



static void   example( char *text, size_t i, char *tmp, size_t tmp_len)
{
   struct mulle_buffer  buffer = MULLE_BUFFER_FLEXIBLE_DATA(  tmp, tmp_len, NULL);
   mulle_buffer_add_c_chars( &buffer, text, i);
   printf( "%s\n", mulle_buffer_get_string( &buffer));
   mulle_buffer_done( &buffer);
}


int  main()
{
   char     *s;
   size_t   len;
   size_t   i;
   size_t   j;
   char     tmp[ 16]  = { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
                          'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'
                        };

   s   = "VfL Bochum 1848";
   len = strlen( s);
   for( j = 0; j <= sizeof( tmp); j++)
   {
      for( i = 0; i <= len; i++)
         example( s, i, tmp, j);
   }

   return( 0);
}
