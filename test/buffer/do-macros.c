//
//  do-macros.c
//
//  Coverage for the `mulle_buffer_do_*` family. Three members had no call
//  site anywhere in the project and had therefore never been compiled:
//
//     mulle_buffer_do_allocator
//     mulle_buffer_do_flexible_filled
//     mulle_buffer_do_inflexible_filled
//
//  The rest of the family plus the `_default` creators are instantiated here
//  as well, so a regression in any of them is caught in one place.
//
#ifdef NDEBUG
# undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <mulle-buffer/mulle-buffer.h>


static int   example_return( void)
{
   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_return( buffer, (int) mulle_buffer_get_length( buffer));
   }

   return( -1);
}


static int   example_return_if_overflown( void)
{
   char   tmp[ 8];

   mulle_buffer_do_inflexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      mulle_buffer_return_if_overflown( buffer, -2);
   }

   return( 0);
}


static void   test_do( void)
{
   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      assert( mulle_buffer_get_length( buffer) == 15);
      assert( ! strcmp( mulle_buffer_get_string( buffer), "VfL Bochum 1848"));
   }
}


static void   test_do_allocator( void)
{
   mulle_buffer_do_allocator( buffer, &mulle_default_allocator)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      assert( mulle_buffer_get_length( buffer) == 15);
      assert( ! strcmp( mulle_buffer_get_string( buffer), "VfL Bochum 1848"));
   }
}


static void   test_do_flexible( void)
{
   char   tmp[ 8];

   mulle_buffer_do_flexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      assert( ! strcmp( mulle_buffer_get_string( buffer), "VfL Bochum 1848"));
   }
}


static void   test_do_flexible_filled( void)
{
   char   storage[ 32];

   memcpy( storage, "VfL", 3);

   mulle_buffer_do_flexible_filled( buffer, storage, 3)
   {
      assert( mulle_buffer_get_length( buffer) == 3);
      assert( ! memcmp( mulle_buffer_get_bytes( buffer), "VfL", 3));

      // grows past the initial storage
      mulle_buffer_add_c_chars( buffer, " Bochum 1848", 12);
      assert( mulle_buffer_get_length( buffer) == 15);
      assert( ! mulle_buffer_has_overflown( buffer));
      assert( ! memcmp( mulle_buffer_get_bytes( buffer), "VfL Bochum 1848", 15));
   }
}


static void   test_do_inflexible( void)
{
   char   tmp[ 8];

   mulle_buffer_do_inflexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_c_chars( buffer, "VfL", 3);
      assert( ! mulle_buffer_has_overflown( buffer));
      assert( ! memcmp( mulle_buffer_get_bytes( buffer), "VfL", 3));
   }
}


static void   test_do_inflexible_filled( void)
{
   char   storage[ 8];

   memcpy( storage, "VfL", 3);

   mulle_buffer_do_inflexible_filled( buffer, storage, 3)
   {
      assert( mulle_buffer_get_length( buffer) == 3);
      assert( ! memcmp( mulle_buffer_get_bytes( buffer), "VfL", 3));

      // no room left, this must overflow and not crash
      mulle_buffer_add_c_chars( buffer, "Bochum", 6);
      assert( mulle_buffer_has_overflown( buffer));
   }
}


static void   test_do_string( void)
{
   char   *s;

   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      break;
   }

   assert( s != NULL);
   assert( ! strcmp( s, "VfL Bochum 1848"));
   mulle_free( s);
}


static void   test_default_creators( void)
{
   struct mulle_buffer   buffer;
   struct mulle_buffer   *created;

   mulle_buffer_init_default( &buffer);
   mulle_buffer_add_string( &buffer, "VfL");
   assert( mulle_buffer_get_length( &buffer) == 3);
   mulle_buffer_done( &buffer);

   created = mulle_buffer_create_default();
   assert( created != NULL);
   mulle_buffer_add_string( created, "Bochum");
   assert( mulle_buffer_get_length( created) == 6);
   mulle_buffer_destroy( created);

   created = mulle_buffer_alloc_default();
   assert( created != NULL);
   mulle_buffer_init_default( created);
   mulle_buffer_add_string( created, "1848");
   assert( mulle_buffer_get_length( created) == 4);
   mulle_buffer_destroy( created);
}


int   main( void)
{
   test_do();
   test_do_allocator();
   test_do_flexible();
   test_do_flexible_filled();
   test_do_inflexible();
   test_do_inflexible_filled();
   test_do_string();
   test_default_creators();

   assert( example_return() == 15);
   assert( example_return_if_overflown() == -2);

   return( 0);
}
