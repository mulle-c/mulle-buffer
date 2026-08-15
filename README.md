# mulle-buffer

#### ↗️  A growable C char array - on stack and heap

mulle-buffer can construct arbitrary long binary data dynamically or in static
storage. You don't have to worry about calculating the necessary buffer size.
It's easy, fast and safe and it is used to implement NSMutableData.
mulle-buffer has functions to create hexdumps and quoted C string output.

The `mulle_flushablebuffer` variant is a stream-like output buffer: it flushes
long output to a sink (like `stdout`) to keep memory bounded. A plain
`mulle_buffer` is an append-first byte builder, not a general file-like
stream (see the contract below).

With [mulle-fprintf](//github.com/mulle-core/mulle_fprintf), you can use
`printf` style formatting and create string concatenations without
having to worry about memory management.


## Contract (read this first)

`mulle_buffer` is an append-first byte builder, not a general file-like
stream. Everything below follows from that:

- Writing is the primary operation; reading is a secondary convenience.
- The buffer has a single cursor used for writing, reading, and seeking.
- Seeking changes the logical length (`mulle_buffer_get_length` follows the
  cursor).
- `MULLE_BUFFER_SEEK_END` is relative to the allocation capacity, not the
  written length.
- Reads consume the same cursor used for writing.
- Allocation uses a mulle allocator under the no-fail allocator contract:
  it never returns `NULL`.
- Write functions are intentionally `void`; failure is reported through
  `mulle_buffer_has_overflown`.
- Callers must check `mulle_buffer_has_overflown` after writing to a
  fixed-size or flushable buffer.
- const/read-only buffers must not be written and write-only buffers must
  not be read; both are enforced by assertions in debug builds.
- Self-aliasing (appending a slice of the buffer's own storage) is not
  supported.

See [dox/DESIGN.md](dox/DESIGN.md) for the detailed design decisions.



| Release Version                                       | Release Notes  | AI Documentation
|-------------------------------------------------------|----------------|---------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-buffer.svg) [![Build Status](https://github.com/mulle-c/mulle-buffer/workflows/CI/badge.svg)](//github.com/mulle-c/mulle-buffer/actions) ![Coverage](https://img.shields.io/badge/coverage-95%25%C2%A0-seagreen) | [RELEASENOTES](RELEASENOTES.md) | [DeepWiki for mulle-buffer](https://deepwiki.com/mulle-c/mulle-buffer)


## API

| Data Structure                                        | Description
| ------------------------------------------------------| ----------------------------------------
| [`mulle-buffer`](dox/API_BUFFER.md)                   | A resizable buffer that grows to the heap if needed
| [`mulle-flushablebuffer`](dox/API_FLUSHABLEBUFFER.md) | Useful for dumps and other longer output



## Documentation & Guides

* [API Summary](asset/dox/api/toc)



## Examples


### Dynamic C string construction

Here a C string is constructed that is valid inside the `mulle_buffer_do`
block:


``` c
void  test( void)
{
   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "hello");
      mulle_buffer_add_string( buffer, " ");
      mulle_buffer_add_string( buffer, "world");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

As soon as the `mulle_buffer_do` block is exited, the buffer will be invalid.

> #### Note
>
> A `break` inside the block is OK and does not leak, but a `return` will
> leak. Use `mulle_buffer_return` instead.


### Use explicit stack memory for small strings

`mulle_buffer_do` will create a default sized stack buffer of at least 128 bytes.

If you want to specify the amount of stack space yourself, you can use
`mulle_buffer_do_flexible`. If the stack storage is exhausted, the string will be copied
to dynamically allocated memory:


``` c
void  test( void)
{
   char   tmp[ 256];

   mulle_buffer_do_flexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "hello");
      mulle_buffer_add_string( buffer, " ");
      mulle_buffer_add_string( buffer, "world");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

![alloca](pix/mulle-buffer-alloca.svg)



If you don't want the string to ever exceed the initial storage length
you can use `mulle_buffer_do_inflexible`:

``` c
void  test( void)
{
   char   tmp[ 8];  // space for seven characters and trailing zero

   mulle_buffer_do_inflexible( buffer, tmp, sizeof( tmp))
   {
      mulle_buffer_add_string( buffer, "VfL_");
      mulle_buffer_add_string( buffer, "Bochum");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}
```

![overflow](pix/mulle-buffer-overflow.svg)


This should print "VfL_Boc", as the overflow preserves the pre-overflow content
and `get_string` zero-terminates the last byte.

### Error model

Write operations return `void`, so failures are reported silently through two
channels:

- `mulle_buffer_has_overflown( buffer)` is the official write contract and the
  primary error channel. The flag is set exactly when a fixed-size buffer runs
  out of capacity, a flushable buffer's flusher fails, a string add is given a
  self-referencing source, or a flush is attempted on a non-flushable buffer.
  Once set it stays set: the buffer keeps its pre-overflow content and further
  write operations become no-ops until the buffer is re-initialized.
- A `mulle_flushablebuffer` reports failures through the return value of its
  flusher (and `mulle_flushablebuffer_flush`): `0` bytes written means the
  flusher failed, and the buffered data is retained.

Because writes are silent, check `mulle_buffer_has_overflown` whenever you write
to a fixed-size or flushable buffer whose capacity you did not size explicitly:

``` c
mulle_buffer_add_bytes( buffer, data, length);
mulle_buffer_add_string( buffer, suffix);

if( mulle_buffer_has_overflown( buffer))
   return( FAILURE);
```

The convenience macro `mulle_buffer_return_if_overflown( buffer, rval)` applies
this exact pattern in one line.


### Convenience macro for creating allocated strings

To construct a dynamically allocated string, that you can use outside of the
`mulle_buffer_do` block, use the `mulle_buffer_do_string` convenience macro.
It's similar to `mulle_buffer_do`, but takes two more arguments.
The second argument is the allocator to use for
the string. Use NULL for the default allocator or use `&mulle_stdlib_allocator`
for the standard C allocator. The third parameter is the `char *` variable name
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

### Permanent mulle-buffer

If the mulle-buffer should live longer than the current function, create
and destroy a buffer manually:

``` c
buffer = mulle_buffer_create_default();
...
mulle_buffer_destroy( buffer);
```

or

``` c
mulle_buffer_init_default( &buffer);
...
mulle_buffer_done( buffer);
```

> #### Tip
>
> Use the companion project [mulle-sprintf](//github.com/mulle-core/mulle-sprintf) to
> print data with format strings a la `sprintf` into a mulle-buffer.
>


### You are here

![Overview](overview.dot.svg)





## Add

mulle-buffer is a component of the [mulle-core](//github.com/mulle-core/mulle-core) library. So in your code include the mulle-core umbrella header:

``` c
#include <mulle-core/mulle-core.h>
```

### Add mulle-core to a cmake and git project

``` bash
git submodule add https://github.com/mulle-core/mulle-core.git mulle-core
```

Add this to your `CMakeLists.txt`:

``` cmake
add_subdirectory( mulle-core)
target_link_libraries( ${PROJECT_NAME} PRIVATE mulle-core)
```


### Add mulle-core to a mulle-sde project

``` sh
mulle-sde add github:mulle-core/mulle-core
```

### Embed mulle-buffer with clib

``` sh
clib install --out src mulle-c/mulle-buffer
```

Append `src` to your include path (e.g. add `-isystem src`  to your `CFLAGS`)
and compile all the sources that were downloaded.




## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



