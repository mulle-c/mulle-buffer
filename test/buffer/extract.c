#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   test_flexible()
{
   char   *s;
   char   storage[ 128];

   mulle_buffer_do_flexible( buf, storage, sizeof( storage))
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
}


static void   test_inflexible()
{
   char   *s;
   char   storage[ 128];

   mulle_buffer_do_inflexible( buf, storage, sizeof( storage))
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
}



static void   test_dynamic()
{
   char                  *s;
   struct mulle_buffer   *buf;

   buf = mulle_buffer_create( NULL);
   {
      mulle_buffer_add_string( buf, "VfL Bochum 1848");
      s = mulle_buffer_extract_string( buf);
      printf( "%s\n", s);
      mulle_free( s);
   }
   mulle_buffer_destroy( buf);
}



int  main()
{
   test_flexible();
   test_inflexible();
   test_dynamic();
   return( 0);
}

