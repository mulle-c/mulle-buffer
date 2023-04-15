# mulle-buffer

#### ↗️ A growable C char array and also a stream

A growable array of C `unsigned char`. This can be used to construct arbitrary
long binary data. It can be used to implement NSMutableData. But it can also be
used as a stream.

| Release Version                                       | Release Notes
|-------------------------------------------------------|--------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-buffer.svg?branch=release) [![Build Status](https://github.com/mulle-c/mulle-buffer/workflows/CI/badge.svg?branch=release)](//github.com/mulle-c/mulle-buffer/actions)| [RELEASENOTES](RELEASENOTES.md) |


## API

File                                 | Description
------------------------------------ | ----------------------------------------
[`mulle_buffer`](dox/API_BUFFER.md)  | The array data structure.






## Examples

### Dynamic C string construction

Here a C string is constructed. You don't have to worry about calculating
the necessary buffer size. It's easy, fast and safe:
The intermediate `mulle_buffer` will be removed once `mulle_buffer_done`
executes.

``` c
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

   printf( "%s\n", mulle_buffer_get_string( &buffer));

   mulle_buffer_done( &buffer);
}
```

### Convenience macros

The convenience macro `mulle_buffer_do` saves you from typing the
variable declaration and the `_init` and `_done` calls. The scope of the
`mulle_buffer` is restricted to the blockquote following the `mulle_buffer_do`.
Also note that, though the actual "mulle_buffer" is stack based, within the
"do" scope, you are accessing the "mulle_buffer" with a pointer, to save a
bit more type work:


``` c
// ...

void  test( void)
{
   // ...

   mulle_buffer_do( buffer)
   {
      // ...
      mulle_buffer_add_string( buffer, "hello");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

You can use `break` inside the `mulle_buffer_do` block to leave it. If you use
`return`, you will risk a memory leak. Use `mulle_buffer_return` instead. Or
explicitly delete the buffer with `mulle_buffer_done` before issuing `return`.


### Convenience macros for small strings

If you expect the string to be small in most circumstances, you can use
`mulle_buffer_do_flexible` to keep character storage on the stack:


``` c
void  test( void)
{
   char   tmp[ 256];

   mulle_buffer_do_flexible( buffer, tmp)
   {
      mulle_buffer_add_string( buffer, "hello");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

If you don't want the produced string to ever exceed the initial storage length
you can use `mulle_buffer_do_inflexible`

``` c
void  test( void)
{
   char   tmp[ 4];

   mulle_buffer_do_inflexible( buffer, tmp)
   {
      mulle_buffer_add_string( buffer, "hello");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

This should print "hel", as a trailing zero will be needed for the last
character.


### Convenience macro for creating allocated strings

To construct a dynamically allocated string, you can use the
`mulle_buffer_do_string` convenience macro. It's similar to `mulle_buffer_do`,
but takes two more arguments. The second argument is the allocator to use for
the string. Use NULL for the default allocator or `&mulle_stdlib_allocator` for
the standard C allocator. The third parameter is the `char *`  variable name
that will hold the resultant C string:



``` c
void  test( void)
{
   unsigned int   i;
   char           *s;

   mulle_buffer_do_string( buffer, NULL, s)
   {
      for( i = 0; i < 10; i++)
         mulle_buffer_add_byte( buffer, 'a' + i % 26);

      mulle_buffer_memset( buffer, 'z', 10);
      mulle_buffer_add_string( buffer, "hello");
   }

   printf( "%s\n", s);
   mulle_free( s);         // this time its allocated
}
```

You will have to `mulle_free` the constructed string "s".



> #### Tip
>
> Use the companion project [mulle-sprintf](//github.com/mulle-core/mulle-sprintf) to
> print data with format strings a la `sprintf` into a mulle-buffer.
>


### Dynamic File reader

Read a file into a malloced memory buffer. The buffer is efficiently
grown as to minimize reallocs. The returned buffer is sized to fit.
The intermediate `mulle_buffer` is removed.


``` c
struct mulle_data    read_file( FILE *fp)
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


### Output stream to file handle


``` c
void  dump( FILE *fp, void *bytes, size_t length)
{
   struct mulle_flushablebuffer   flushable_buffer;
   struct mulle_buffer            *buffer;
   char                           storage[ 1024];  // storage for buffer

   mulle_flushablebuffer_init( &flushable_buffer,
                               storage,
                               sizeof( storage),
                               (mulle_flushablebuffer_flusher_t) fwrite,
                               fp);
   {
      buffer = mulle_flushablebuffer_as_buffer( &flushable_buffer);
      mulle_buffer_add_string( buffer, "---\n");
      mulle_buffer_hexdump( buffer, bytes, length, 0, mulle_buffer_hexdump_default);
      mulle_buffer_add_string( buffer, "---\n");
   }
   mulle_flushablebuffer_done( &flushable_buffer);
}
```

##### flexbuffer, a replacement for alloca

The `mulle_flexbuffer` can be used as an replacement for `alloca`. The problem
with `alloca` is always two-fold. 1.) It's non-standard and not available on
all platforms. 2.) The amount of memory to `alloca` may exceed the available
stack space. The `mulle_flexbuffer` solves this problem by using a small amount
of stack space for low memory scenarios and moving to `malloc`, when it's
needed.

Example:

``` c
 void  foo( char *s)
{
   size_t   n;

   n = strlen( s) + 1;
   mulle_flexbuffer_do( copy, 32, n)
   {
      strcpy( copy, s);
   }
}
```


A `char *` named "copy" is created. "copy" either points to stack memory or to
a malloced area. `mulle_flexbuffer_do` defines the maximum amout of memory
to be stored on the stack. In this case its `char[ 32]`. The actual amount
used is determined by `n`. The flexbuffer will be valid in the scope of the
`mulle_flexbuffer_do` block statement only.

> `mulle_flexbuffer` is actually a macro for `mulle_buffer`.
> Due to a C language limitations, the symbol "copy" will be available outside
> the block, but it will be reset to NULL.

If you use a `return` statement in a `mulle_flexbuffer_do` block you risk a
leak unless you issue `mulle_flexbuffer_done` before the return or use
`mulle_flexbuffer_return` (which needs the compiler extension `__typeof__` to
work though).

``` c
   mulle_flexbuffer_do( copy, int, 32, n)
   {
      memset( copy, 0, sizeof( int) * n);
      if( n == 1)
         mulle_flexbuffer_return( copy, copy[ 0]);
      if( n == 2)
      {
         tmp = copy[ 0];                // rescue return value
         mulle_flexbuffer_done( copy);   // "copy" is invalid after done
         return( tmp);   // possible!
      }
   }
```

Using `break` to break out of `mulle_flexbuffer_do` is not a problem. A
`continue` statement in `mulle_flexbuffer_do` will do the same thing as
`break` though and is therefore only confusing:


``` c
   for( n = 0; n < 2; n++)
   {
      mulle_flexbuffer_do( copy, int, 16, n)
      {
         continue;  // affects mulle_flexbuffer_do, not for
      }
   }
```


### You are here

![Overview](overview.dot.svg)





## Add

Use [mulle-sde](//github.com/mulle-sde) to add mulle-buffer to your project:

``` sh
mulle-sde add github:mulle-c/mulle-buffer
```

To only add the sources of mulle-buffer with dependency
sources use [clib](https://github.com/clibs/clib):


``` sh
clib install --out src/mulle-c mulle-c/mulle-buffer
```

Add `-isystem src/mulle-c` to your `CFLAGS` and compile all the sources that were downloaded with your project.


## Install

### Install with mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-buffer and all dependencies:

``` sh
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-c/mulle-buffer/archive/latest.tar.gz
```

### Manual Installation

Install the requirements:

| Requirements                                 | Description
|----------------------------------------------|-----------------------
| [mulle-allocator](https://github.com/mulle-c/mulle-allocator)             | 🔄 Flexible C memory allocation scheme
| [mulle-data](https://github.com/mulle-c/mulle-data)             | #️⃣ A collection of hash functions

Install **mulle-buffer** into `/usr/local` with [cmake](https://cmake.org):

``` sh
cmake -B build \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release &&
cmake --install build --config Release
```


## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK



