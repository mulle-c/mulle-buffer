#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>


int   main( void)
{
   printf( "sizeof( struct mulle__buffer) = %td\n", sizeof( struct mulle__buffer));
   printf( "sizeof( struct mulle_buffer) = %td\n", sizeof( struct mulle_buffer));
   printf( "sizeof( struct mulle_flushablebuffer) = %td\n", sizeof( struct mulle_flushablebuffer));
   return( 0);
}

