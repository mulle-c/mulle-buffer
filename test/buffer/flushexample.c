
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example()
{
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf, storage, sizeof( storage), fwrite, stdout);

   buf = (struct mulle_buffer  *) &_buf;
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

   buf = (struct mulle_buffer  *) &_buf;
   mulle_buffer_add_string( buf, "VfL Bochum 1848\n");
   mulle_flushablebuffer_done( &_buf);
}


int  main()
{
   example();
   example2();

   return( 0);
}
