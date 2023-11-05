
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>


static void   example()
{
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf,
                               storage,
                               sizeof( storage),
                               (mulle_flushablebuffer_flusher_t *) fwrite,
                               stdout);


   buf = (struct mulle_buffer  *) &_buf;
   mulle_buffer_add_string( buf, "VfL");
   mulle_buffer_add_char( buf, ' ');
   mulle_buffer_add_string( buf, "Bochum");
   mulle_buffer_add_char( buf, ' ');
   mulle_buffer_add_string( buf, "1848");
   mulle_buffer_add_char( buf, '\n');

   // call this or the last flush won't happen
   mulle_flushablebuffer_done( &_buf);
}


static void   example2()
{
   struct mulle_flushablebuffer   *flush_buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];
   size_t                         seek;

   flush_buf = mulle_flushablebuffer_create( sizeof( storage),
                                             (mulle_flushablebuffer_flusher_t *) fwrite,
                                             stdout,
                                             NULL);

   buf = (struct mulle_buffer *) flush_buf;
   mulle_buffer_add_string( buf, "VfL Bochum 1848\n");
   seek = mulle_buffer_get_seek( buf);
   assert( seek == 16);

   mulle_flushablebuffer_destroy( flush_buf);

   // for coverage
   mulle_flushablebuffer_destroy( NULL);
}


static void   coverage1()
{
   struct mulle_flushablebuffer   *fbuf;
   struct mulle_buffer            *buf;

   fbuf = mulle_flushablebuffer_create( 8,
                                        (mulle_flushablebuffer_flusher_t *) fwrite,
                                        stdout,
                                        NULL);

   // for coverage
   mulle_flushablebuffer_flush( fbuf);

   buf = (struct mulle_buffer  *) fbuf;
   mulle_buffer_add_string( buf, "VfL");
   mulle_buffer_add_char( buf, ' ');

   _mulle__buffer_set_overflown( (struct mulle__buffer *) buf);

   mulle_buffer_add_string( buf, "Bochum");
   mulle_buffer_add_char( buf, ' ');
   mulle_buffer_add_string( buf, "1848");
   mulle_buffer_add_char( buf, '\n');

   // as we have overflown this will not be happy
   if( mulle_flushablebuffer_destroy( fbuf))
      mulle_buffer_destroy( buf);
}



static size_t   fail_flush( void *buf, size_t one, size_t len, void *userinfo)
{
   // did not flush
   return( 0);
}


static void   coverage2()
{
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf,
                               storage,
                               sizeof( storage),
                               fail_flush,
                               stdout);

   // for coverage
   mulle_flushablebuffer_flush( &_buf);

   buf = (struct mulle_buffer  *) &_buf;
   mulle_buffer_add_string( buf, "VfL Bochum 1848");

   // as we have overflown this will not be happy
   if( mulle_flushablebuffer_done( &_buf))
      mulle_buffer_done( (struct mulle_buffer *) &_buf);
}


int  main()
{
   example();
   example2();
   coverage1();
   coverage2();
   return( 0);
}
