# mulle-buffer

#### ↗️ A growable C char array and also a stream

A growable array of C `unsigned char`. This can be used to construct arbitrary
long binary data. It can be used to implement NSMutableData. But it can also be
used as a stream.


| Release Version
|-----------------------------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-buffer.svg?branch=release) [![Build Status](https://github.com/mulle-c/mulle-buffer/workflows/CI/badge.svg?branch=release)](https://github.com/mulle-c/mulle-buffer/actions)

## Examples

### Dynamic C string construction

Here a C string is constructed. You don't have to worry about calculating
the necessary buffer size. It's easy, fast and safe:
The intermediate `mulle_buffer` is removed.

```
#include <mulle-buffer/mulle-buffer.h>

void  test( void)
{
   unsigned int          i;
   struct mulle_buffer   buffer;

   mulle_buffer_init( &buffer, NULL);

   for( i = 0; i < 10; i++)
      mulle_buffer_add_byte( &buffer, 'a' + i % 26);

   mulle_buffer_memset( &buffer, 'z', 10);
   mulle_buffer_add_string( &buffer, "hello");

   printf( "%s\n", mulle_buffer_get_string( buffer));

   mulle_buffer_done( &buffer);
}
```

### Dynamic File reader

Read a file into a malloced memory buffer. The buffer is efficiently
grown as to minimize reallocs. The returned buffer is sized to fit.
The intermediate `mulle_buffer` is removed.


```
static struct mulle_data    read_file( FILE *fp)
{
   struct mulle_buffer   buffer;
   struct mulle_data     data;
   void                  *ptr;
   size_t                length;
   size_t                size;

   mulle_buffer_init( &buffer, NULL)
   while( ! feof( fp))
   {
      ptr  = mulle_buffer_guarantee( &buffer, 0x1000);
      assert( ptr);  // can't be NULL as we are not a limited buffer
      size = mulle_buffer_guaranteed_size( &buffer);
      assert( size >= 0x1000);   // could also be larger, use it
      length = fread( ptr, 1, size, fp);

      mulle_buffer_advance( &buffer, length);
   }
   mulle_buffer_shrink_to_fit( &buffer);

   data = mulle_buffer_extract_data( &buffer);
   mulle_buffer_done( &buffer);

   return( data);
}
```


## API

File                                 | Description
------------------------------------ | ----------------------------------------
[`mulle_buffer`](dox/API_BUFFER.md)  | The array data structure.


## Add

### Either: link library

Use [mulle-sde](//github.com/mulle-sde) to add mulle-buffer to your project:

```
mulle-sde dependency add --c --github mulle-c mulle-buffer
```

### Or: add Sources

Alternatively you can read [STEAL.md](//github.com/mulle-c11/dox/STEAL.md) on
how to add mulle-c source code into your own projects.


## Install

### mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-buffer and all dependencies:

```
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-c/mulle-buffer/archive/latest.tar.gz
```

### Manual Installation


Install the requirements:

Requirements                                             | Description
---------------------------------------------------------|-----------------------
[mulle-allocator](//github.com/mulle-c/mulle-allocator)  | Memory allocation wrapper
[mulle-data](//github.com/mulle-c/mulle-data)            | Hash code


Install into `/usr/local`:

```
mkdir build 2> /dev/null
(
   cd build ;
   cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
         -DCMAKE_PREFIX_PATH=/usr/local \
         -DCMAKE_BUILD_TYPE=Release .. ;
   make install
)
```


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//github.com/mulle-c/mulle-c11)


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
