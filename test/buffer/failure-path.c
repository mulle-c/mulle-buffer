#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


//
// Failure-path coverage (review gap):
//  - a flusher that writes fewer bytes than asked must mark the buffer
//    overflown, and a flush retry must stay failed
//  - overflow followed by every mutating operation stays overflown
//  - extraction on an overflown buffer
//

static size_t   short_flusher( void *buf, size_t one, size_t len, void *userinfo)
{
   return( 0);   // report a short/failed write
}


static int   test_failed_flush( void)
{
   struct mulle_flushablebuffer   fb;
   struct mulle_buffer            *buffer;
   char                           storage[ 16];

   mulle_flushablebuffer_init_with_static_bytes( &fb, storage, sizeof( storage),
                                                 short_flusher, NULL, NULL);
   buffer = mulle_flushablebuffer_as_buffer( &fb);
   // fill past the 16-byte capacity to force a flush; the flusher reports a
   // short write, which must mark the buffer overflown
   mulle_buffer_add_bytes( buffer, "0123456789ABCDEFGHIJ", 20);
   if( ! mulle_buffer_has_overflown( buffer))
      return( 1);
   // a flush attempt after failure stays failed
   if( mulle_flushablebuffer_flush( &fb) != -2)
      return( 2);
   return( 0);
}


static int   test_overflow_sticky( void)
{
   struct mulle_buffer   buffer;
   char                  storage[ 4];

   mulle_buffer_init_inflexible_with_static_bytes( &buffer, storage, sizeof( storage));
   {
      mulle_buffer_add_bytes( &buffer, "0123", 4);
      if( mulle_buffer_has_overflown( &buffer))
         return( 1);
      // overflow on the 5th byte
      mulle_buffer_add_byte( &buffer, '4');
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 2);

      // subsequent mutating operations must not clear the sticky flag
      mulle_buffer_add_bytes( &buffer, "more", 4);
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 3);
      mulle_buffer_remove_all( &buffer);
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 4);
      mulle_buffer_set_seek( &buffer, 0, MULLE_BUFFER_SEEK_SET);
      if( ! mulle_buffer_has_overflown( &buffer))
         return( 5);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_extract_overflown( void)
{
   struct mulle_buffer    buffer;
   char                   storage[8];
   char                   *s;

   mulle_buffer_init_inflexible_with_static_bytes( &buffer, storage, sizeof( storage));
   mulle_buffer_add_string( &buffer, "VfL");
   mulle_buffer_add_string( &buffer, " Bochum 1848");   // overflows
   if( ! mulle_buffer_has_overflown( &buffer))
      return( 1);

   // extraction preserves the pre-overflow content (the "VfL Bochum 1848"
   // prefix that fit into storage)
   s = mulle_buffer_extract_string( &buffer);
   if( ! s)
      return( 2);
   mulle_free( s);
   return( 0);
}


int  main()
{
   int   rval;

   rval = test_failed_flush();
   if( rval) return( 100 + rval);
   rval = test_overflow_sticky();
   if( rval) return( 200 + rval);
   rval = test_extract_overflown();
   if( rval) return( 300 + rval);
   return( 0);
}