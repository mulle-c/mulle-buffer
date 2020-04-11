# mulle-buffer

↗️ A growable C char array and also a stream

A growable array of C `unsigned char`. This can be used to construct arbitrary
long binary data. It can be used to implement NSMutableData. But it can also be
used as a stream.


> This library could benefit from more tests. Do not assume, that it
> is completely bug free.


Build Status | Release Version
-------------|-----------------------------------
[![Build Status](https://travis-ci.org/mulle-c/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-buffer) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-buffer.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-buffer)

## Example

```
#include <mulle-buffer/mulle-buffer.h>

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


## Add 

Use [mulle-sde](//github.com/mulle-sde) to add mulle-buffer to your project:

```
mulle-sde dependency add --c --github mulle-c mulle-buffer
```

## Install

### mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-thread and all dependencies:

```
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-c/mulle-buffer/archive/latest.tar.gz
```

### Manual Installation


Install the requirements:

Requirements                                             | Description
---------------------------------------------------------|-----------------------
[mulle-allocator](//github.com/mulle-c/mulle-allocator)  | Memory allocation wrapper


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
[mulle-c11](//github.com/mulle-c/mulle-c11) and
[mulle-thread](//github.com/mulle-c/mulle-thread).


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
