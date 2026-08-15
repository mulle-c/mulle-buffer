#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static size_t   fwrite_stdout( void *buf, size_t one, size_t len, void *userinfo)
{
   return( fwrite( buf, one, len, (FILE *) userinfo));
}


static void   example( void)
{
   unsigned int   i;

   mulle_buffer_do( buffer)
   {
      for( i = 0; i < 7; i++)
         mulle_buffer_add_byte( buffer, i);

      {
         struct mulle_flushablebuffer   output_small;
         char                           storage_small[ 256];

         mulle_flushablebuffer_init_with_static_bytes( &output_small,
                                                       storage_small,
                                                       sizeof( storage_small),
                                                       fwrite_stdout,
                                                       stdout,
                                                       NULL);
         mulle_buffer_hexdump( mulle_flushablebuffer_as_buffer( &output_small),
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0,
                               mulle_buffer_hexdump_no_ascii);
         mulle_flushablebuffer_done( &output_small);
      }

      for( ; i < 246; i++)
         mulle_buffer_add_byte( buffer, i);

      {
         struct mulle_flushablebuffer   output;
         char                           storage[ 256];

         mulle_flushablebuffer_init_with_static_bytes( &output,
                                                       storage,
                                                       sizeof( storage),
                                                       fwrite_stdout,
                                                       stdout,
                                                       NULL);
         mulle_buffer_hexdump( mulle_flushablebuffer_as_buffer( &output),
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0,
                               0);
         for(; i < 254; i++)
            mulle_buffer_add_byte( buffer, i);
         mulle_buffer_hexdump( mulle_flushablebuffer_as_buffer( &output),
                               mulle_buffer_get_bytes( buffer),
                               mulle_buffer_get_length( buffer),
                               0x100,
                               mulle_buffer_hexdump_no_ascii);
         mulle_flushablebuffer_done( &output);
      }
   }
}


int  main()
{
   example();

   return( 0);
}