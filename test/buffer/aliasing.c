#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


//
// Aliasing coverage (review gap):
//  - string-family adds whose source points into the buffer's own storage are
//    detected and reported via `mulle_buffer_has_overflown`; nothing is
//    appended. This is the defined runtime result (debug and release).
//

static int   test_string_self_reference( void)
{
   struct mulle_buffer   buffer;
   char                  storage[ 64];

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage), NULL);
   {
      char   *s;

      mulle_buffer_add_string( &buffer, "VfL Bochum 1848");
      if( mulle_buffer_has_overflown( &buffer))
         return( 1);

      s = mulle_buffer_get_string( &buffer);
      // source points into own storage: must be detected, not crash
      mulle_buffer_add_string( &buffer, s);
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 2);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_chars_self_reference( void)
{
   struct mulle_buffer   buffer;
   char                  storage[ 64];

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage), NULL);
   {
      char   *s;

      mulle_buffer_add_c_chars( &buffer, "VfL", 3);
      if( mulle_buffer_has_overflown( &buffer))
         return( 1);

      s = (char *) mulle_buffer_get_bytes( &buffer);
      mulle_buffer_add_c_chars( &buffer, s, mulle_buffer_get_length( &buffer));
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 2);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


int  main()
{
   int   rval;

   rval = test_string_self_reference();
   if( rval) return( 100 + rval);
   rval = test_chars_self_reference();
   if( rval) return( 200 + rval);
   return( 0);
}