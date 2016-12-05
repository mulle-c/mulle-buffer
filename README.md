# mulle-buffer

A growable array of C `unsigned char`. This can be used to construct arbitrary
long binary data. It can be used to implement NSMutableData. But it can also be
used as a stream.


> This library could benefit from more tests. Do not assume, that it
> is completely bug free.


Fork      |  Build Status | Release Version
----------|---------------|-----------------------------------
[Mulle kybernetiK](//github.com/mulle-nat/mulle-buffer) | [![Build Status](https://travis-ci.org/mulle-nat/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-nat/mulle-buffer) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-nat/mulle-buffer.svg) [![Build Status](https://travis-ci.org/mulle-nat/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-nat/mulle-buffer)

<!--- [Community](https://github.com/mulle-objc/mulle-buffer/tree/release) | [![Build Status](https://travis-ci.org/mulle-objc/mulle-buffer.svg)](https://travis-ci.org/mulle-objc/mulle-buffer) | ![Community tag](https://img.shields.io/github/tag/mulle-objc/mulle-buffer.svg) [![Build Status](https://travis-ci.org/mulle-objc/mulle-buffer.svg?branch=release)](https://travis-ci.org/mulle-objc/mulle-buffer) -->


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


## Install

On OS X and Linux you can use [homebrew](//brew.sh), respectively
[linuxbrew](//linuxbrew.sh) to install the library:

```
brew tap mulle-kybernetik/software/mulle-buffer
```

On other platforms you can use **mulle-install** from
[mulle-build](//github.com/mulle-nat/mulle-build) to install the library:

```
mulle-install --prefix /usr/local --branch release https://github.com/mulle-nat/mulle-buffer
```

Otherwise read:

* [How to Build](dox/BUILD.md)


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//www.mulle-kybernetik.com/software/git/mulle-c11/)


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
