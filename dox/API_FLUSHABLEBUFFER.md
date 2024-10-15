

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



## A flusher implementations

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



