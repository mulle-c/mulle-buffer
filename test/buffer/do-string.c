
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example( void)
{
   char   *s;

   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      break;
   }

   printf( "%s\n", s);
   mulle_free( s);
}



//static void   example2( void)
//{
//   struct mulle_buffer   buffer;
//   char                  *s;
//
//   mulle_buffer_do( buffer, 8, s)
//   {
//      mulle_buffer_add_string( &buffer, "VfL Bochum 1848");
//      break;
//   }
//
//   printf( "%s\n", s);
//   mulle_free( s);
//}
//

int  main()
{
   example();

   return( 0);
}
