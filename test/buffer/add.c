#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>


#include <stdio.h>



static void   simple_fill_test()
{
   struct mulle_buffer   *buffer;
   unsigned int           i;
   size_t                 len;

   buffer = mulle_buffer_create( NULL);

   for( i = 0; i < 100000; i++)
      mulle_buffer_add_byte( buffer, 'a' + i % 26);
   printf( "simple_fill_test: %ld\n", mulle_buffer_get_length( buffer));

   mulle_buffer_memset( buffer, 'z', 1000);
   mulle_buffer_zero_last_byte( buffer);

   len = strlen( mulle_buffer_get_bytes( buffer));
   printf( "simple_fill_test: %ld\n", len);

//   printf( "simple_fill_test: %lx\n", mulle_hash( mulle_buffer_get_bytes( buffer),
//                                                  len));

   mulle_buffer_add_bytes( buffer, "hello", 5);

   printf( "simple_fill_test: %ld\n", mulle_buffer_get_length( buffer));

   printf( "simple_fill_test: %ld\n", mulle_buffer_get_staticlength( buffer));

   mulle_buffer_reset( buffer);
   printf( "simple_fill_test: %ld\n", mulle_buffer_get_length( buffer));

   mulle_buffer_destroy( buffer);
}


static void   simple_static_test()
{
   struct mulle_buffer   buffer;

   // this can't grow
   mulle_buffer_init_inflexible_with_static_bytes( &buffer,
                                                   "hello world",
                                                   12);
   // skip over first 11 bytes
   mulle_buffer_advance( &buffer, 12);

   // test that we can't write to it
   mulle_buffer_add_byte( &buffer, 'x');
   mulle_buffer_memset( &buffer, 'y', 3);

   printf( "simple_static_test: %ld\n", mulle_buffer_get_length( &buffer));
   printf( "simple_static_test: %s\n", (char *) mulle_buffer_get_bytes( &buffer));
   printf( "simple_static_test: %ld\n", mulle_buffer_get_staticlength( &buffer));

   mulle_buffer_reset( &buffer);
   printf( "simple_fill_test: %ld\n", mulle_buffer_get_length( &buffer));

   mulle_buffer_done( &buffer);
}

static void   if_empty_test( void)
{
   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string_if_not_empty( buffer, "A");
      mulle_buffer_add_string_if_empty( buffer, "B");
      mulle_buffer_add_string_if_not_empty( buffer, "C");
      mulle_buffer_add_string_if_empty( buffer, "D");
      mulle_buffer_make_string( buffer);
      printf( "if_empty_test: %s\n", (char *) mulle_buffer_get_bytes( buffer));
   }
}



int  main()
{
   simple_fill_test();
   mulle_testallocator_reset();
   simple_static_test();
   if_empty_test();

   return( 0);
}

