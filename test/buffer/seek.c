#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   test_seek()
{
   char   buf[ 32];
   int    rc;

   mulle_buffer_do_inflexible( buffer, buf, sizeof( buf))
   {
      rc = mulle_buffer_set_seek( buffer, 33, MULLE_BUFFER_SEEK_SET);
      assert( rc);

      mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_SET);
      assert( mulle_buffer_get_seek( buffer) == 0);
      mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_CUR);
      assert( mulle_buffer_get_seek( buffer) == 0);
      mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_END);
      assert( mulle_buffer_get_seek( buffer) == 32);

      rc = mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_SET);
      assert( ! rc);
      mulle_buffer_memset( buffer, '.', 32);
      printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                        (char *) mulle_buffer_get_bytes( buffer));

      rc = mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_SET);
      assert( ! rc);
      mulle_buffer_add_string( buffer, "012345678");
      printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                        (char *) mulle_buffer_get_bytes( buffer));

      rc = mulle_buffer_set_seek( buffer, 4, MULLE_BUFFER_SEEK_SET);
      assert( ! rc);
      assert( mulle_buffer_get_seek( buffer) == 4);
      mulle_buffer_add_string( buffer, "X");
      printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                        (char *) mulle_buffer_get_bytes( buffer));

      rc = mulle_buffer_set_seek( buffer, -2, MULLE_BUFFER_SEEK_CUR);
      assert( ! rc);
      mulle_buffer_add_string( buffer, "Y");
      printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                        (char *) mulle_buffer_get_bytes( buffer));

      rc = mulle_buffer_set_seek( buffer, 1, MULLE_BUFFER_SEEK_END);
      assert( ! rc);
      mulle_buffer_add_string( buffer, "Z");
      printf( "%.*s\n", (int) mulle_buffer_get_length( buffer),
                        (char *) mulle_buffer_get_bytes( buffer));
   }
}


int  main()
{
   test_seek();
   return( 0);
}

