#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>


#include <stdio.h>


int  main()
{
   struct mulle_buffer      buffer = { 0 };
   struct mulle_allocator   *allocator;

   mulle_buffer_set_allocator( NULL, NULL);
   mulle_buffer_set_allocator( &buffer, NULL);
   allocator = mulle_buffer_get_allocator( &buffer);
   if( allocator != &mulle_default_allocator)
      return( 2);
   allocator = mulle_buffer_get_allocator( NULL);
   if( allocator != &mulle_default_allocator)
      return( 1);

   // some coverage stuff
   mulle_buffer_init( NULL, 0, NULL);
   mulle_buffer_add_bytes_callback( NULL, NULL, 0);
   mulle_buffer_done( NULL);

   return( 0);
}

