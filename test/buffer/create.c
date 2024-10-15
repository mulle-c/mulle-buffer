#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   coverage()
{
   struct mulle__buffer  *buffer;
   struct mulle__buffer  *other;
   char                  *s;

   buffer = _mulle__buffer_create( NULL);
   _mulle__buffer_get_capacity( buffer);

   other  = _mulle__buffer_create( NULL);
   _mulle__buffer_add_bytes( other, "Whatever", 8, NULL);
   _mulle__buffer_add_buffer_range( buffer, other, mulle_range_make_all(), NULL);
   _mulle__buffer_add_string_with_maxlength( buffer, "!", 200, NULL);
   _mulle__buffer_set_length( other, 10, 0, NULL);
   _mulle__buffer_add_string_if_empty( buffer, "", NULL);
   _mulle__buffer_add_string_if_not_empty( buffer, "", NULL);
   _mulle__buffer_add_c_string( buffer, "x", NULL);
   s = _mulle__buffer_extract_string( buffer, NULL);
   printf( "%s\n", s);
   mulle_free( s);

   _mulle__buffer_intersects_bytes( buffer, "foo", 3);
   _mulle__buffer_get_staticlength( buffer);
   _mulle__buffer_get_staticsize( buffer);

   _mulle__buffer_destroy( other, NULL);
   _mulle__buffer_reset( buffer, NULL);
   _mulle__buffer_destroy( buffer, NULL);
}


int  main()
{
   coverage();
   return( 0);
}

