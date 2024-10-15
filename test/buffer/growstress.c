#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


static void   test_grow( struct mulle_buffer *buffer, size_t len, char *name)
{
   char   *s;

   mulle_buffer_grow( buffer, len);
   mulle_buffer_memset( buffer, '*',
      _mulle__buffer_get_allocation_length( (struct mulle__buffer *) buffer));

   s = mulle_buffer_get_string( buffer);
   printf( "%19s %3td%s: %s\n", name + 5, mulle_buffer_get_length( buffer),
                                          mulle_buffer_has_overflown( buffer) ? " OVERFLOW" : "",
                                          s ? s : "NULL");
}


static void   test_default( size_t len)
{
   struct mulle_buffer   *buffer;
   char                  *s;

   buffer = mulle_buffer_create_default();
   {
      test_grow( buffer, len, (char *) __FUNCTION__);
   }
   mulle_buffer_destroy( buffer);
}


static void   test_allocated_bytes( size_t len)
{
   struct mulle_buffer   *buffer;
   void                  *storage;
   size_t                length;

   length  = 60;
   storage = mulle_malloc( length);

   buffer = mulle_buffer_alloc_default();
   mulle_buffer_init_with_allocated_bytes( buffer, storage, length, NULL);
   {
      test_grow( buffer, len, (char *) __FUNCTION__);
   }
   mulle_buffer_destroy( buffer);
}


static void   test_static_bytes( size_t len)
{
   struct mulle_buffer   *buffer;
   size_t                length;
   char                  storage[ MULLE_BUFFER_DEFAULT_CAPACITY];

   length  = sizeof( storage);

   buffer = mulle_buffer_alloc_default();
   mulle_buffer_init_with_static_bytes( buffer, storage, length, &mulle_default_allocator);
   {
      test_grow( buffer, len, (char *) __FUNCTION__);
   }
   mulle_buffer_destroy( buffer);
}



int  main()
{
   unsigned int  i;

   for( i = 0; i < 200; i++)
   {
      test_default( i);
      test_static_bytes( i);
      test_allocated_bytes( i);
   }
   return( 0);
}

