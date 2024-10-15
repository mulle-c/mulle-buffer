#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   test_normal()
{
   struct mulle_buffer    *buffer;
   unsigned int           i;
   size_t                 len;

   buffer = mulle_buffer_create( NULL);

   printf( "test_zero: %d\n", (int) mulle_buffer_get_length( buffer));

   mulle_buffer_set_length( buffer, 20, MULLE_BUFFER_SHRINK_OR_ZEROFILL);
   printf( "test_zero: %d\n", (int) mulle_buffer_get_length( buffer));

   mulle_buffer_set_length( buffer, 8, MULLE_BUFFER_SHRINK_OR_ZEROFILL);
   printf( "test_zero: %d\n", (int) mulle_buffer_get_length( buffer));

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

   // this should be fine
   mulle_buffer_set_length( &buffer, 8, MULLE_BUFFER_NO_ZEROFILL);
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));

   // can't be more than 12
   mulle_buffer_set_length( &buffer, 20, MULLE_BUFFER_NO_ZEROFILL);
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));

   // has overflown will stick to 12
   mulle_buffer_set_length( &buffer, 8, MULLE_BUFFER_NO_ZEROFILL);
   printf( "test_inflexible: %d\n", (int) mulle_buffer_get_length( &buffer));


   mulle_buffer_done( &buffer);
}



int  main()
{
   test_normal();
   test_inflexible();
   return( 0);
}

