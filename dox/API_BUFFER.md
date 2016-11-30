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


```
struct mulle_buffer   *buf;
char                  *s;

buf = mulle_buffer_create( NULL);
mulle_buffer_add_string( buf, "VfL");
mulle_buffer_add_char( buf, 0);
s = mulle_buffer_extract( buf);
mulle_buffer_destroy( buf);

printf( "%s\n", s);
mulle_free( s);
```

### `mulle_buffer` as overflow protection


Initialize a stack array. `mulle_buffer` ensures that no data is written out
of bounds:

```
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

```
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

## Type

```
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

```
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


```
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

```
void    mulle_buffer_init_with_capacity( struct mulle_buffer *buffer,
                                         size_t capacity,
                                         struct mulle_allocator *allocator)
```

Like `mulle_buffer_init` but pre-allocates at least `capacity` bytes.


### `mulle_buffer_init_inflexable_with_static_bytes`

```
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

```
void   mulle_buffer_size_to_fit( struct mulle_buffer *buffer)
```

Release all unneeded memory. Useful to call before `mulle_buffer_extract_all` when using larger buffers.


### `mulle_buffer_make_inflexable`

```
void   mulle_buffer_make_inflexable( struct mulle_buffer *buffer,
                                     void *storage,
                                     size_t length)
```

Turn a regular buffer into an inflexable one. This is extremely hackish and
rarely useful.


### `mulle_buffer_zero_to_length`

```
void   mulle_buffer_zero_to_length( struct mulle_buffer *buffer,
                                    size_t length)
```

Fill up buffer with zeroes until it contains `length` characters. This does not
truncate.


### `mulle_buffer_set_length`

```
size_t   mulle_buffer_set_length( struct mulle_buffer *buffer,
                                  size_t length)
```

Truncate or widen a buffer to `length` size. If the buffer is expanded, the new bytes
will be set to zero.


### `mulle_buffer_extract_bytes`

```
void   *mulle_buffer_extract_bytes( struct mulle_buffer *buffer)
```

Retrieve the internal storage. The buffer is empty afterwards. Here is
an example how to concatenate strings and retrieve them in a `malloc` (stdlib)
allocated memory region):

```
struct mulle_buffer   buf;
char                  *s;

mulle_buffer_init( &buf, &mulle_stdlib_allocator);
mulle_buffer_add_string( &buf, "VfL");
mulle_buffer_add_char( &buf, ' ');
mulle_buffer_add_string( &buf, "Bochum");
mulle_buffer_size_to_fit( &buf);
s = mulle_buffer_extract( &buf);
mulle_buffer_done( &buf);
printf( "%s\n", s);
free( s);
```

In general it is preferable to use `NULL` instead of `&mulle_stdlib_allocator` and
then `mulle_free` instead of `free`. In the example it is assumed, that `malloc` 
must be used.


### `mulle_buffer_remove_all`

```
void   mulle_buffer_remove_all( struct mulle_buffer *buffer)
```

Sets the length of `buffer` back to zero clearing possible overflow. Does not
free or shrink memory.


### `mulle_buffer_get_bytes`

```
void   *mulle_buffer_get_bytes( struct mulle_buffer *buffer)
```

Accessor to the internal storage.



### `mulle_buffer_get_length`

```
size_t   mulle_buffer_get_length( struct mulle_buffer *buffer)
```

Get the number of characters contained in the buffer. Will be initially zero
for inflexable buffers.


### `mulle_buffer_get_staticlength`

```
size_t   mulle_buffer_get_staticlength( struct mulle_buffer *buffer)
```

Get the length in bytes of the static backing storage. Will be zero for
buffers not intialized with static bytes.


### `mulle_buffer_set_seek`

```
int   mulle_buffer_set_seek( struct mulle_buffer *buffer, int mode, size_t seek)
```

Treat `buffer` as a stream and seek around. Just like `lseek`.


### `mulle_buffer_get_seek`

```
size_t   mulle_buffer_get_seek( struct mulle_buffer *buffer)
```

Get the current `seek` position.


### `mulle_buffer_advance`


```
void   *mulle_buffer_advance( struct mulle_buffer *buffer,
                              size_t length)
```

Move ahead `length` bytes. They may well generate garbage content in a
growable writer buffer. When writing it is recommended to use
`mulle_buffer_memset`.


### `mulle_buffer_is_inflexable`

```
int   mulle_buffer_is_inflexable( struct mulle_buffer *buffer)
```

Returns 1 if `buffer` is inflexable.


### `mulle_buffer_is_flushable`

```
int   mulle_buffer_is_flushable( struct mulle_buffer *buffer)
```

Returns 1 if `buffer` is flushable.


### `mulle_buffer_has_overflown`

```
int   mulle_buffer_has_overflown( struct mulle_buffer *buffer)
```

An inflexable buffer may overflow. If that is the case, many operations on
 the buffer will have no effect (like adding characters).


### `mulle_buffer_guarantee`

```
static inline int   mulle_buffer_guarantee( struct mulle_buffer *buffer,
                                            size_t length)
```

Increase capacity of `buffer` so that `length` bytes can be added next time
without needing to realloc. Returns 0 on success.


### `mulle_buffer_remove_last_byte`

```
void    mulle_buffer_remove_last_byte( struct mulle_buffer *buffer)
```

Remove the last byte from the buffer.


### `mulle_buffer_add_byte`

```
void    mulle_buffer_add_byte( struct mulle_buffer *buffer,
                               int c)
```

Add 'c' to the buffer. This therefore adds 1 byte to the buffer.


### `mulle_buffer_add_character`

```
void    mulle_buffer_add_character( struct mulle_buffer *buffer,
                                    int c)
```

Add character 'c' to the buffer. This therefore adds 1 byte to the buffer.


### `mulle_buffer_add_uint16`

```
void    mulle_buffer_add_uint16( struct mulle_buffer *buffer,
                                 uint16_t c)
{
   _mulle_buffer_add_uint16( (struct _mulle_buffer *) buffer, c, buffer->_allocator);
}
```

Add `c` in network order to buffer. This is the binary representation of `c`,
not the string representation. This therefore adds 2 bytes to the buffer.


### `mulle_buffer_add_uint32`

```
void    mulle_buffer_add_uint32( struct mulle_buffer *buffer,
                                 uint32_t c)
```

Add `c` in network order to buffer. This is the binary representation of `c`,
not the string representation. This therefore adds 4 bytes to the buffer.



### `mulle_buffer_add_bytes`

```
void   mulle_buffer_add_bytes( struct mulle_buffer *buffer,
                               void *bytes,
                               size_t length)
```

Add `length` `bytes` to `buffer`. The buffer may be set to overflown,
 if it is inflexable.


### `mulle_buffer_add_string`

```
void   mulle_buffer_add_string( struct mulle_buffer *buffer,
                                char *s)
```

Add a \0 terminated C-string `s` to `buffer`.


### `mulle_buffer_add_string_with_maxlength`

```
size_t   mulle_buffer_add_string_with_maxlength( struct mulle_buffer *buffer,
                                                 char *s,
                                                 size_t length)
```

Add up to `length` characters from the \0 terminated C-string `s`to `buffer`.
Returns the actually copied number of characters.


### `mulle_buffer_memset`

```
void   mulle_buffer_memset( struct mulle_buffer *buffer,
                            int c,
                            size_t length)
```

Fill `buffer` from the current position with `length` characters.


### `mulle_buffer_zero_last_byte`

```
void   mulle_buffer_zero_last_byte( struct mulle_buffer *buffer)
```

Terminate buffer with \0. If the buffer is inflexable and full, the last character
will be overwritten, effectively truncating the contents.


### `mulle_buffer_add_buffer`

```
void    mulle_buffer_add_buffer( struct mulle_buffer *buffer,
                                 struct mulle_buffer *other)
```

Add the contents of `other` to `buffer`.


### `mulle_buffer_next_bytes`

```
int   mulle_buffer_next_bytes( struct mulle_buffer *buffer,
                               void *buf,
                               size_t len)
```

Read `len` bytes from `buffer` into `buf`. The return value indicates success
with 0.


### `mulle_buffer_next_byte`

```
int   mulle_buffer_next_byte( struct mulle_buffer *buffer)
```

Read the next byte from `buffer`. -1 indicates failure.



### `mulle_buffer_next_character`

```
int   mulle_buffer_next_character( struct mulle_buffer *buffer)
```

Read the next char from `buffer`. INT_MAX indicates failure.




# `mulle_flushablebuffer`

## `mulle_flushablebuffer` as a flushing string stream writer

```
```


## Functions

### `mulle_flushablebuffer_init`


```
void    mulle_flushablebuffer_init( struct mulle_flushablebuffer *buffer,
                                    void *storage,
                                    size_t length,
                                    mulle_flushablebuffer_flusher *flusher,
                                    void *userinfo)
```

Initialize `buffer` with `flusher` and `userinfo`. `flusher` is a callback
function with the following signature:

```
typedef size_t   _mulle_flushablebuffer_flusher( void *userinfo, size_t len, size_t, void *buffer);
```

which is actually the same signature as `fwrite`.

The mulle_flushablebuffer does not manage its own backing buffer, you have to
pass in your own `storage` of `length` bytes, which should exit as long as
`buffer` is used.



