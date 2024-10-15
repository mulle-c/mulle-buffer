#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


#define CAPACITY  8

void   test( size_t len)
{
   struct mulle_buffer   *buffer;
   size_t                length;
   char                  storage[ CAPACITY];
   char                  *s;

   memset( storage, 'x',  sizeof( storage));
   length = sizeof( storage);
   buffer = mulle_buffer_alloc_default();
   mulle_buffer_init_inflexible_with_static_bytes( buffer, storage, length);
   {
      mulle_buffer_grow( buffer, len);
      mulle_buffer_memset( buffer, '*',
         _mulle__buffer_get_allocation_length( (struct mulle__buffer *) buffer));

      s = mulle_buffer_get_string( buffer);
      printf( "%3td%s: %s\n", mulle_buffer_get_length( buffer),
                              mulle_buffer_has_overflown( buffer) ? " OVERFLOW" : "",
                              s ? s : "NULL");
   }
   mulle_buffer_destroy( buffer);
}


int main( void)
{
   test( 0);
   test( 1);
   test( CAPACITY / 2);
   test( CAPACITY);
   test( CAPACITY * 2);

   return( 0);
}