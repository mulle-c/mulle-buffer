#include <mulle_buffer/mulle_buffer.h>
#include <mulle_test_allocator/mulle_test_allocator.h>
#include <stdio.h>


static void   test_normal()
{
   struct mulle_buffer   *buffer;
   unsigned int           i;
   size_t                 len;

   buffer = mulle_buffer_create( NULL);

   printf( "test_normal: %d\n", (int) mulle_buffer_get_length( buffer));

   mulle_buffer_set_length( buffer, 20);
   printf( "test_normal: %d\n", (int) mulle_buffer_get_length( buffer));

   mulle_buffer_set_length( buffer, 8);
   printf( "test_normal: %d\n", (int) mulle_buffer_get_length( buffer));

   mulle_buffer_destroy( buffer);
}


static void   test_inflexible()
{
   struct mulle_buffer   buffer;

   mulle_buffer_init_inflexible_with_static_bytes( &buffer,
                                                   "hello world",
                                                   12);

   // 0, because nothing has been read yet
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));

   // can't be more than 12
   mulle_buffer_set_length( &buffer, 20);
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));

   // has overflown will stick to 12
   mulle_buffer_set_length( &buffer, 8);
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));

   mulle_buffer_done( &buffer);
}



int  main()
{
   mulle_test_allocator_initialize();
   mulle_default_allocator = mulle_test_allocator;

   test_normal();
   test_inflexible();

   mulle_test_allocator_reset();
}

