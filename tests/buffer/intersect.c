#include <mulle_buffer/mulle_buffer.h>
#include <stdio.h>


int  main()
{
   struct mulle_buffer   buffer;

   buffer._storage  = (void *) 0x10;
   buffer._sentinel = (void *) 0x20;

    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x0, 0x10));

    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x0,  0x11));
    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x10, 0x10));
    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x10, 0x11));
    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x1F, 0x10));

    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x20, 0x10));
    printf( "%d\n", mulle_buffer_intersects_bytes( &buffer, (void *) 0x11, 0x0));
}

