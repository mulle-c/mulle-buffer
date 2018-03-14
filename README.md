# mulle-buffer

A growable array of C `unsigned char`. This can be used to construct arbitrary
long binary data. It can be used to implement NSMutableData. But it can also be
used as a stream.


> This library could benefit from more tests. Do not assume, that it
> is completely bug free.


Fork      |  Build Status | Release Version
----------|---------------|-----------------------------------
[Mulle kybernetiK](//github.com/mulle-nat/mulle-buffer) | [![Build Status](https://travis-ci.org/mulle-nat/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-nat/mulle-buffer) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-nat/mulle-buffer.svg) [![Build Status](https://travis-ci.org/mulle-nat/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-nat/mulle-buffer)


## Install

Install the prerequisites first:

| Prerequisites                                           |
|---------------------------------------------------------|
| [mulle-allocator](//github.com/mulle-c/mulle-allocator) |

Then build and install

```
mkdir build 2> /dev/null
(
   cd build ;
   cmake .. ;
   make install
)
```

Or let [mulle-sde](//github.com/mulle-sde) do it all for you.


## Example

```
#include <mulle_buffer/mulle_buffer.h>

void  test( void)
{
   unsigned int          i;
   struct mulle_buffer   *buffer;

   buffer = mulle_buffer_create( NULL);

   for( i = 0; i < 10; i++)
      mulle_buffer_add_byte( buffer, 'a' + i % 26);

   mulle_buffer_memset( buffer, 'z', 10);
   mulle_buffer_add_string( buffer, "hello");

   printf( "%s\n", mulle_buffer_get_bytes( buffer));

   mulle_buffer_destroy( buffer);
}
```


## API

File                                 | Description
------------------------------------ | ----------------------------------------
[`mulle_buffer`](dox/API_BUFFER.md)  | The array data structure.


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//github.com/mulle-c/mulle-c11) and
[mulle-thread](//github.com/mulle-c/mulle-thread).


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
