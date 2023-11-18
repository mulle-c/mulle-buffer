#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static size_t   flusher( void *buf, size_t one, size_t len, void *userinfo)
{
   return( 0);
}


int  main()
{
   // for coverage
   mulle_flushablebuffer_init_with_allocated_bytes( NULL, (void *) 1, 0, flusher, NULL, NULL);
   mulle_flushablebuffer_done( NULL);

   mulle_buffer_init( NULL, NULL);
   mulle_buffer_add_byte( NULL, 0);
   mulle_buffer_add_bytes( NULL, NULL, 0);
   mulle_buffer_advance( NULL, 0);
   mulle_buffer_done( NULL);

   return( 0);
}

