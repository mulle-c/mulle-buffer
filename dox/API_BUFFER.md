# `mulle_buffer`

`mulle_buffer` is a multi-purpose **`unsigned char`** array. It can be
useful as a stream, as a string buffer and to parse simple things. It's
interface is quite complex.

It has a companion container called `mulle_flushablebuffer`. This a non
growing array, that will flush its contents to a callback when it is full.
This is useful for acting like a "write" stream.

## Buffer Modes and States

Term            | Description
----------------|-----------------------------
`inflexable`    | a buffer that does not grow
`static`        | storage is on the stack or in .bss / .data segement
`flushable`     | buffer contents can be "flushed" out to a consumer, freeing it up to hold more contents. This goes hand in hand with being inflexable.
`overflown`     | the inflexable buffer had to truncate content, or a buffer read couldn't be done.


## Examples

### `mulle_buffer` as a dynamic array creator


``` c
struct mulle_buffer   *buf;
char                  *s;

buf = mulle_buffer_create( NULL);
mulle_buffer_add_string( buf, "VfL");
mulle_buffer_add_char( buf, 0);
s = mulle_buffer_extract_string( buf);
mulle_buffer_destroy( buf);

printf( "%s\n", s);
mulle_free( s);
```

### `mulle_buffer` as overflow protection


Initialize a stack array. `mulle_buffer` ensures that no data is written out
of bounds:

``` c
{
   struct mulle_buffer   buffer;
   auto                  string[ 64];

   mulle_buffer_init_inflexable_with_static_bytes( &buffer, string, sizeof( string));

   // operations on &buffer can not overflow `string` now
   for( i = 0; i < 100; i++)
      mulle_buffer_add( &buffer, i);
   mulle_buffer_done( &buffer);
}
```

### `mulle_buffer` as a string stream reader

Read characters with an offset from a string:

``` c
{
   struct mulle_buffer   buffer;
   static char           string[] = "VfL Bochum 1848\n";

   mulle_buffer_init_inflexable_with_static_bytes( &buffer, string, sizeof( string));

   mulle_buffer_set_seek( &buffer, SEEK_SET, 4);
   while( c == mulle_buffer_get_char( &buffer))
      putchar( c);
   mulle_buffer_done( &buffer);
}
```

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



## Type

``` c
struct mulle_buffer
```

## Verbs


* `create`
* `destroy`
* `init`
* `done`

* `set_allocator`


## Functions

Since programs will probably use a lot of `mulle_buffer_add` calls it would be inefficient to test for success or failure for ever call.
Instead calls should proceed optimistically until the buffer is queried. Then they check if it is still in a valid state with `mulle_buffer_has_overflown`. (For non-growing buffers or read buffers)


### `mulle_buffer_init_with_allocated_bytes`

``` c
void    mulle_buffer_init_with_allocated_bytes( struct mulle_buffer *buffer,
                                                void *storage,
                                                size_t length,
                                                struct mulle_allocator *allocator)
```

Initialize a growing buffer, that starts with a memory region `storage` of
`length` bytes, allocated by the user using `allocator`. This is useful if you
already malloced something and dont't need it anymore.
The storage is overwritten from the start.


### `mulle_buffer_init_with_static_bytes`


``` c
void    mulle_buffer_init_with_static_bytes( struct mulle_buffer *buffer,
                                             void *storage,
                                             size_t length,
                                             struct mulle_allocator *allocator)
```

Initialize a growing buffer, that starts with a non freeable memory region,
allocated by the user. Useful if you want to supply some stack memory for
`mulle_buffer` so that it may not need to malloc, if the contents remain small
enough. The storage is overwritten from the start.



### `mulle_buffer_init_with_capacity`

``` c
void    mulle_buffer_init_with_capacity( struct mulle_buffer *buffer,
                                         size_t capacity,
                                         struct mulle_allocator *allocator)
```

Like `mulle_buffer_init` but pre-allocates at least `capacity` bytes.


### `mulle_buffer_init_inflexable_with_static_bytes`

``` c
void    mulle_buffer_init_inflexable_with_static_bytes( struct mulle_buffer *buffer,
                                                        void *storage,
                                                        size_t length)
```

Initialize a buffer that does not grow. The backing buffer `storage` of
`length` bytes has to be supplied. It will not be freed.
The intial length of the buffer will be 0. The storage can be read with
`mulle_buffer_next` if so desired.

This is useful for guaranteeing that a buffer can not be overrun.
The storage is overwritten from the start.


### `mulle_buffer_size_to_fit`

``` c
void   mulle_buffer_size_to_fit( struct mulle_buffer *buffer)
```

Release all unneeded memory. Useful to call before
`mulle_buffer_extract_data` when using larger buffers.


### `mulle_buffer_make_inflexable`

``` c
void   mulle_buffer_make_inflexable( struct mulle_buffer *buffer,
                                     void *storage,
                                     size_t length)
```

Turn a regular buffer into an inflexable one. This is extremely hackish and
rarely useful.


### `mulle_buffer_zero_to_length`

``` c
void   mulle_buffer_zero_to_length( struct mulle_buffer *buffer,
                                    size_t length)
```

Fill up buffer with zeroes until it contains `length` characters. This does not
truncate.


### `mulle_buffer_set_length`

``` c
size_t   mulle_buffer_set_length( struct mulle_buffer *buffer,
                                  size_t length)
```

Truncate or widen a buffer to `length` size. If the buffer is expanded, the new bytes
will be set to zero.


### `mulle_buffer_extract_data`

``` c
struct mulle_data   mulle_buffer_extract_data( struct mulle_buffer *buffer)
```

Retrieve the internal storage. The buffer is empty afterwards. Here is
an example how to concatenate strings and retrieve them in a `malloc` (stdlib)
allocated memory region):

``` c
struct mulle_buffer   buf;
struct mulle_data     data;

mulle_buffer_init( &buf, &mulle_stdlib_allocator);
mulle_buffer_add_string( &buf, "VfL");
mulle_buffer_add_char( &buf, ' ');
mulle_buffer_add_string( &buf, "Bochum");
data = mulle_buffer_extract_data( &buf);
mulle_buffer_done( &buf);
printf( "%s\n", data.characters);
free( data.characters);
```

In general it is preferable to use `NULL` instead of `&mulle_stdlib_allocator` and
then `mulle_free` instead of `free`. In the example it is assumed, that `malloc`
must be used.


### `mulle_buffer_extract_string`

``` c
char *   mulle_buffer_extract_string( struct mulle_buffer *buffer)
```

A convenience around `mulle_buffer_extract_data`. It ensures that there is a
trailing zero and also resizes the allocated memory to fit the string size.

``` c
struct mulle_buffer   buf;
char                  *s;

mulle_buffer_init( &buf, &mulle_stdlib_allocator);
mulle_buffer_add_string( &buf, "VfL");
mulle_buffer_add_char( &buf, ' ');
mulle_buffer_add_string( &buf, "Bochum");
s = mulle_buffer_extract_string( &buf);
mulle_buffer_done( &buf);
printf( "%s\n", s);
free( s);
```


### `mulle_buffer_remove_all`

``` c
void   mulle_buffer_remove_all( struct mulle_buffer *buffer)
```

Sets the length of `buffer` back to zero clearing possible overflow. Does not
free or shrink memory.


### `mulle_buffer_get_bytes`

``` c
void   *mulle_buffer_get_bytes( struct mulle_buffer *buffer)
```

Accessor to the internal storage.



### `mulle_buffer_get_length`

``` c
size_t   mulle_buffer_get_length( struct mulle_buffer *buffer)
```

Get the number of characters contained in the buffer. Will be initially zero
for inflexable buffers.


### `mulle_buffer_get_staticlength`

``` c
size_t   mulle_buffer_get_staticlength( struct mulle_buffer *buffer)
```

Get the length in bytes of the static backing storage. Will be zero for
buffers not intialized with static bytes.


### `mulle_buffer_set_seek`

``` c
int   mulle_buffer_set_seek( struct mulle_buffer *buffer, int mode, size_t seek)
```

Treat `buffer` as a stream and seek around. Just like `lseek`.


### `mulle_buffer_get_seek`

``` c
size_t   mulle_buffer_get_seek( struct mulle_buffer *buffer)
```

Get the current `seek` position.


### `mulle_buffer_advance`


``` c
void   *mulle_buffer_advance( struct mulle_buffer *buffer,
                              size_t length)
```

Move ahead `length` bytes. They may well generate garbage content in a
growable writer buffer. When writing it is recommended to use
`mulle_buffer_memset`.


### `mulle_buffer_is_inflexable`

``` c
int   mulle_buffer_is_inflexable( struct mulle_buffer *buffer)
```

Returns 1 if `buffer` is inflexable.


### `mulle_buffer_is_flushable`

``` c
int   mulle_buffer_is_flushable( struct mulle_buffer *buffer)
```

Returns 1 if `buffer` is flushable.


### `mulle_buffer_has_overflown`

``` c
int   mulle_buffer_has_overflown( struct mulle_buffer *buffer)
```

An inflexable buffer may overflow. If that is the case, many operations on
 the buffer will have no effect (like adding characters).


### `mulle_buffer_guarantee`

``` c
static inline void   *mulle_buffer_guarantee( struct mulle_buffer *buffer,
                                             size_t length)
```

Increase capacity of `buffer` so that `length` bytes can be added next time
without needing to realloc. Returns a pointer to the reserved area (of size
length) on success. Otherwise NULL. NULL can only happen if you are dealing
with a non-flexable buffer.


### `mulle_buffer_remove_last_byte`

``` c
void    mulle_buffer_remove_last_byte( struct mulle_buffer *buffer)
```

Remove the last byte from the buffer.


### `mulle_buffer_add_byte`

``` c
void    mulle_buffer_add_byte( struct mulle_buffer *buffer,
                               int c)
```

Add 'c' to the buffer. This therefore adds 1 byte to the buffer.


### `mulle_buffer_add_character`

``` c
void    mulle_buffer_add_character( struct mulle_buffer *buffer,
                                    int c)
```

Add character 'c' to the buffer. This therefore adds 1 byte to the buffer.


### `mulle_buffer_add_uint16`

``` c
void    mulle_buffer_add_uint16( struct mulle_buffer *buffer,
                                 uint16_t c)
{
   _mulle__buffer_add_uint16( (struct mulle__buffer *) buffer, c, buffer->_allocator);
}
```

Add `c` in network order to buffer. This is the binary representation of `c`,
not the string representation. This therefore adds 2 bytes to the buffer.


### `mulle_buffer_add_uint32`

``` c
void    mulle_buffer_add_uint32( struct mulle_buffer *buffer,
                                 uint32_t c)
```

Add `c` in network order to buffer. This is the binary representation of `c`,
not the string representation. This therefore adds 4 bytes to the buffer.



### `mulle_buffer_add_bytes`

``` c
void   mulle_buffer_add_bytes( struct mulle_buffer *buffer,
                               void *bytes,
                               size_t length)
```

Add `length` `bytes` to `buffer`. The buffer may be set to overflown,
 if it is inflexable.


### `mulle_buffer_add_string`

``` c
void   mulle_buffer_add_string( struct mulle_buffer *buffer,
                                char *s)
```

Add a \0 terminated C-string `s` to `buffer`.


### `mulle_buffer_add_string_with_maxlength`

``` c
size_t   mulle_buffer_add_string_with_maxlength( struct mulle_buffer *buffer,
                                                 char *s,
                                                 size_t length)
```

Add up to `length` characters from the \0 terminated C-string `s`to `buffer`.
Returns the actually copied number of characters.


### `mulle_buffer_memset`

``` c
void   mulle_buffer_memset( struct mulle_buffer *buffer,
                            int c,
                            size_t length)
```

Fill `buffer` from the current position with `length` characters.


### `mulle_buffer_zero_last_byte`

``` c
void   mulle_buffer_zero_last_byte( struct mulle_buffer *buffer)
```

Terminate buffer with \0. If the buffer is inflexable and full, the last character
will be overwritten, effectively truncating the contents.


### `mulle_buffer_add_buffer`

``` c
void    mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                 struct mulle_buffer *other)
```

Add the contents of `other` to `buffer`.


### `mulle_buffer_next_bytes`

``` c
int   mulle_buffer_next_bytes( struct mulle_buffer *buffer,
                               void *buf,
                               size_t len)
```

Read `len` bytes from `buffer` into `buf`. The return value indicates success
with 0.


### `mulle_buffer_next_byte`

``` c
int   mulle_buffer_next_byte( struct mulle_buffer *buffer)
```

Read the next byte from `buffer`. -1 indicates failure.



### `mulle_buffer_next_character`

``` c
int   mulle_buffer_next_character( struct mulle_buffer *buffer)
```

Read the next char from `buffer`. INT_MAX indicates failure.




# `mulle_flushablebuffer`

Here is an example of `mulle_flushablebuffer` as a flushing string stream writer:

``` c
   struct mulle_flushablebuffer   _buf;
   struct mulle_buffer            *buf;
   char                           storage[ 8];

   mulle_flushablebuffer_init( &_buf,
                               storage,
                               sizeof( storage),
                               (mulle_flushablebuffer_flusher_t) fwrite,
                               stdout);

   buf = &_buf;
   mulle_buffer_add_string( buf, "VfL Bochum 1848\n");
   mulle_flushablebuffer_done( &_buf);
```


## Functions

### `mulle_flushablebuffer_init`


``` c
void    mulle_flushablebuffer_init( struct mulle_flushablebuffer *buffer,
                                    void *storage,
                                    size_t length,
                                    mulle_flushablebuffer_flusher_t flusher,
                                    void *userinfo)
```

Initialize `buffer` with `flusher` and `userinfo`. `flusher` is a callback
function with the following signature:

``` c
typedef size_t   mulle__flushablebuffer_flusher( void *userinfo, size_t len, size_t, void *buffer);
```

which is actually the quite the same signature as `fwrite`.

The `mulle_flushablebuffer` does not manage its own backing buffer, you have to
pass in your own `storage` of `length` bytes, which should exist as long as
`buffer` is used.



### `mulle_flushablebuffer_flush`


``` c
int   mulle_flushablebuffer_flush( struct mulle_flushablebuffer *buffer)
```

This flushes out the accumulated data so far. Will return 0 on success.


### `mulle_flushablebuffer_done`


``` c
int   mulle_flushablebuffer_done( struct mulle_flushablebuffer *buffer)
```

This flushes the buffer before calling `mulle_buffer_done`. Will return 0
on success. If this is unsuccessful, the buffer remains alive!


### `mulle_flushablebuffer_destroy`

``` c
int   mulle_flushablebuffer_destroy( struct mulle_flushablebuffer *buffer)
```

This flushes the buffer before calling `mulle_buffer_destroy`. Will return 0
on success. If this is unsuccessful, the buffer remains alive!

## Convenient quaffer / flusher implementations

These are not part of the library as they use system calls.

``` c
size_t   _mulle_flushablebuffer_write_flusher( void *buf, size_t one, size_t len, void *userinfo)
{
   ssize_t  written_len;

   written_len = write( (int) (intptr_t) userinfo, buf, len);
   if( written_len != -1)
      return( written_len);
   return( 0);
}
```

``` c
size_t   _mulle_quaffingbuffer_read_quaffer( void *buf, size_t one, size_t len, void *userinfo)
{
   ssize_t  read_len;

   read_len = read( (int) (intptr_t) userinfo, buf, len);
   if( read_len != -1)
      return( read_len);
   return( 0);
}
```




