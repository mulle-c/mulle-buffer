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


//
// the initial capacity contract: `init`'s capacity argument is the first
// allocation size, and get_capacity reports it before and after materialization
//
static int   test_capacity_contract()
{
   struct mulle_buffer         buffer;
   struct mulle_allocator      *allocator;
   size_t                      capacity;

   allocator = &mulle_stdlib_allocator;

   // reported capacity equals requested capacity before first write
   mulle_buffer_init( &buffer, 128, allocator);
   capacity = mulle_buffer_get_capacity( &buffer);
   if( capacity != 128)
      return( 1);
   // first allocation is the requested capacity, so reported capacity
   // stays 128 after materialization
   mulle_buffer_add_byte( &buffer, 'V');
   capacity = mulle_buffer_get_capacity( &buffer);
   if( capacity != 128)
      return( 2);
   mulle_buffer_done( &buffer);

   // default init uses MULLE_BUFFER_DEFAULT_CAPACITY as the first allocation
   mulle_buffer_init_default( &buffer);
   capacity = mulle_buffer_get_capacity( &buffer);
   if( capacity != MULLE_BUFFER_DEFAULT_CAPACITY)
      return( 3);
   mulle_buffer_add_byte( &buffer, 'f');
   capacity = mulle_buffer_get_capacity( &buffer);
   if( capacity != MULLE_BUFFER_DEFAULT_CAPACITY)
      return( 4);
   mulle_buffer_done( &buffer);

   return( 0);
}


int  main()
{
   coverage();
   return( test_capacity_contract());
}

