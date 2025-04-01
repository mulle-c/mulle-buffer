
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example( void)
{
   unsigned int   i;

   mulle_buffer_do( buffer)
   {
      for( i = 0; i < 7; i++)
         mulle_buffer_add_byte( buffer, i);

      mulle_flushablebuffer_do_FILE( output_small, stdout)
      {
         mulle_buffer_hexdump( output_small,
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0,
                               mulle_buffer_hexdump_no_ascii);
      }

      for( ; i < 246; i++)
         mulle_buffer_add_byte( buffer, i);

      mulle_flushablebuffer_do_FILE( output, stdout)
      {
         mulle_buffer_hexdump( output,
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0,
                               0);
         for(; i < 254; i++)
            mulle_buffer_add_byte( buffer, i);
         mulle_buffer_hexdump( output,
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0x100,
                               mulle_buffer_hexdump_no_ascii);
      }
   }
}


int  main()
{
   example();

   return( 0);
}
