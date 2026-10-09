#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>
#include <string.h>


static int   test_grow( struct mulle_buffer *buffer, size_t len)
{
   char     *s;
   size_t   i;

   mulle_buffer_grow( buffer, len);
   mulle_buffer_memset( buffer, '*', len);

   if( mulle_buffer_has_overflown( buffer))
      return( 0);

   s = mulle_buffer_get_string( buffer);
   if( ! s)
   {
      fprintf( stderr, "buffer string is NULL for length %zu\n", len);
      return( 1);
   }

   if( strlen( s) != len)
   {
      fprintf( stderr, "buffer has length %zu instead of %zu\n", strlen( s), len);
      return( 1);
   }

   for( i = 0; i < len; i++)
   {
      if( s[ i] != '*')
      {
         fprintf( stderr, "byte %zu is not '*' for length %zu\n", i, len);
         return( 1);
      }
   }
   return( 0);
}


static int   test_default( size_t len)
{
   struct mulle_buffer   *buffer;
   int                   rval;

   buffer = mulle_buffer_create_default();
   {
      rval = test_grow( buffer, len);
   }
   mulle_buffer_destroy( buffer);

   return( rval);
}


static int   test_allocated_bytes( size_t len)
{
   struct mulle_buffer   *buffer;
   void                  *storage;
   size_t                length;
   int                   rval;

   length  = 60;
   storage = mulle_malloc( length);

   buffer = mulle_buffer_alloc_default();
   mulle_buffer_init_with_allocated_bytes( buffer, storage, length, NULL);
   {
      rval = test_grow( buffer, len);
   }
   mulle_buffer_destroy( buffer);

   return( rval);
}


static int   test_static_bytes( size_t len)
{
   struct mulle_buffer   *buffer;
   size_t                length;
   char                  storage[ MULLE_BUFFER_DEFAULT_CAPACITY];
   int                   rval;

   length  = sizeof( storage);

   buffer = mulle_buffer_alloc_default();
   mulle_buffer_init_with_static_bytes( buffer, storage, length, &mulle_default_allocator);
   {
      rval = test_grow( buffer, len);
   }
   mulle_buffer_destroy( buffer);

   return( rval);
}



int  main()
{
   unsigned int  i;
   int           rval;

   rval = 0;
   for( i = 0; i < 200; i++)
   {
      rval |= test_default( i);
      rval |= test_static_bytes( i);
      rval |= test_allocated_bytes( i);
   }
   return( rval ? 1 : 0);
}
