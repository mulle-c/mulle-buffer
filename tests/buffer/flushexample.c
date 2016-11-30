
#include <mulle_buffer/mulle_buffer.h>
#include <mulle_test_allocator/mulle_test_allocator.h>
#include <stdio.h>


static void   example()
{
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf, storage, sizeof( storage), fwrite, stdout);

   buf = &_buf;
   mulle_buffer_add_string( buf, "VfL");
   mulle_buffer_add_character( buf, ' ');
   mulle_buffer_add_string( buf, "Bochum");
   mulle_buffer_add_character( buf, ' ');
   mulle_buffer_add_string( buf, "1848");
   mulle_buffer_add_character( buf, '\n');

   // call this or the last flush won't happen
   mulle_flushablebuffer_done( &_buf);
}

static void   example2()
{
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf, storage, sizeof( storage), fwrite, stdout);

   buf = &_buf;
   mulle_buffer_add_string( buf, "VfL Bochum 1848\n");
   mulle_flushablebuffer_done( &_buf);
}


int  main()
{
   mulle_test_allocator_initialize();
   mulle_default_allocator = mulle_test_allocator;

   example();
   example2();

   mulle_test_allocator_reset();
   return( 0);
}
