#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>


//
// Semantic regression coverage for the single-cursor model:
//  - get_length follows the cursor, not a record of written bytes
//  - reads after a rewind are bounded by the allocation, not written length
//  - remove_all resets the cursor but does not clear overflow
//  - SEEK_END is relative to capacity on flexible AND fixed buffers
//
static int   test_length_after_rewind( void)
{
   struct mulle_buffer   buffer;
   char                  storage[ 16];

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage), NULL);
   {
      mulle_buffer_add_bytes( &buffer, "0123456789", 10);
      if( mulle_buffer_get_length( &buffer) != 10)
         return( 1);

      // rewind changes the reported length (single cursor!)
      mulle_buffer_set_seek( &buffer, 4, MULLE_BUFFER_SEEK_SET);
      if( mulle_buffer_get_length( &buffer) != 4)
         return( 2);

      // appending after rewind overwrites and shortens logical data
      mulle_buffer_add_string( &buffer, "XY");
      if( mulle_buffer_get_length( &buffer) != 6)
         return( 3);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_reads_bounded_by_capacity( void)
{
   struct mulle_buffer   buffer;
   char                  storage[16];
   int                    c;

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage), NULL);
   {
      mulle_buffer_add_bytes( &buffer, "ABCD", 4);
      // rewind to start and read; reads consume the same cursor
      mulle_buffer_set_seek( &buffer, 0, MULLE_BUFFER_SEEK_SET);
      c = mulle_buffer_next_byte( &buffer);
      if( c != 'A')
         return( 1);
      c = mulle_buffer_next_byte( &buffer);
      if( c != 'B')
         return( 2);
      // the read cursor now consumed 2 of the 4 written bytes; the reported
      // length follows the cursor (single-cursor model)
      if( mulle_buffer_get_length( &buffer) != 2)
         return( 3);

      mulle_buffer_remove_all( &buffer);
      if( mulle_buffer_get_length( &buffer) != 0)
         return( 4);
   }
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_seek_end_flexible( void)
{
   struct mulle_buffer   buffer;
   char                  storage[16];

   mulle_buffer_init_with_static_bytes( &buffer, storage, sizeof( storage), NULL);
   mulle_buffer_add_string( &buffer, "VfL");
   // SEEK_END is relative to capacity, not written data
   mulle_buffer_set_seek( &buffer, 0, MULLE_BUFFER_SEEK_END);
   if( mulle_buffer_get_seek( &buffer) != sizeof( storage))
      return( 1);
   mulle_buffer_done( &buffer);
   return( 0);
}


static int   test_seek_end_fixed( void)
{
   struct mulle_buffer   buffer;
   char                  storage[16];

   mulle_buffer_init_inflexible_with_static_bytes( &buffer, storage, sizeof( storage));
   mulle_buffer_add_string( &buffer, "VfL");
   mulle_buffer_set_seek( &buffer, 0, MULLE_BUFFER_SEEK_END);
   if( mulle_buffer_get_seek( &buffer) != sizeof( storage))
      return( 1);
   mulle_buffer_done( &buffer);
   return( 0);
}


int  main()
{
   int   rval;

   rval = test_seek_end_flexible();
   if( rval) return( 100 + rval);

   rval = test_seek_end_fixed();
   if( rval) return( 200 + rval);

   rval = test_length_after_rewind();
   if( rval) return( 300 + rval);

   rval = test_reads_bounded_by_capacity();
   if( rval) return( 400 + rval);

   return 0;
}