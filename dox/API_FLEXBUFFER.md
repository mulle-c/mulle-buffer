# mulle-flexbuffer

The flexbuffer has been superseded by **mulle-alloca**, to be
found in [mulle-allocator](//mulle-c/mulle-allocator).


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


## mulle_flexbuffer

```c
#define mulle_flexbuffer( name, stackcount)
```

The `mulle_flexbuffer` macro is used to define a `mulle_buffer` object that uses a flexible buffer. The macro takes two arguments: `name` is the name of the `mulle_buffer` object to define, and `stackcount` is the size of the pre-allocated buffer that will be used if the `mulle_buffer` object is small. If the `mulle_buffer` object grows beyond the size of the pre-allocated buffer, it will automatically switch to using a dynamically allocated buffer.


### Example

```c
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mulle-buffer/mulle_buffer.h>

int   foo( char *s1, char *s2) 
{
   mulle_flexbuffer( copy, 32);
   size_t n;

   n = strlen(s1) + 1;
   mulle_flexbuffer_alloc( copy, n);

   strcpy(copy, s1);
   copy[ 0] = toupper( copy[ 0]);
   printf("Copied string: %s\n", copy);
   if( copy[ 0] == 'Q')
      mulle_flexbuffer_return( copy, 1);

   mulle_flexbuffer_realloc( copy, n + strlen( s2));
   strcat(copy, s2);

   printf("Copied string: %s\n", copy);

   mulle_flexbuffer_done( copy);
   return( 0);
}

int main() 
{
   return( foo( "hello", " world!"));
}
```


## mulle_flexbuffer_alloc

```c
#define mulle_flexbuffer_alloc( name, actualcount)
```

The `mulle_flexbuffer_alloc` macro is used to allocate memory for a `mulle_buffer` object that was defined using the `mulle_flexbuffer` macro. The macro takes two arguments: `name` is the name of the `mulle_buffer` object to allocate memory for, and `actualcount` is the size of the buffer.

## mulle_flexbuffer_realloc

```c
#define mulle_flexbuffer_realloc( name, count)
```

The `mulle_flexbuffer_realloc` macro is used to resize the buffer of a `mulle_buffer` object that was defined using the `mulle_flexbuffer` macro. The macro takes two arguments: `name` is the name of the `mulle_buffer` object to resize, and `count` is the new size of the buffer in bytes. If the buffer needs to be resized, the macro will automatically switch from using the pre-allocated buffer to using a dynamically allocated buffer. It can not switch back from the a dynamically allocated buffer to the pre-allocated buffer though.

## mulle_flexbuffer_done

```c
#define mulle_flexbuffer_done( name)
```

The `mulle_flexbuffer_done` macro is used to release the memory used by a `mulle_buffer` object that was defined using the `mulle_flexbuffer` macro. The macro takes one argument: `name` is the name of the `mulle_buffer` object to release. After calling this macro, the `mulle_buffer` object is no longer valid and should not be used.


## mulle_flexbuffer_return

```c
#define mulle_flexbuffer_return( name, value)
```

The `mulle_flexbuffer_return` macro is used to return a value from a function that uses a `mulle_buffer` object that was defined using the `mulle_flexbuffer` macro. The macro takes two arguments: `name` is the name of the `mulle_buffer` object to release, and `value` is the value to return from the function. After calling this macro, the `mulle_buffer` object is no longer valid and should not be used.


## mulle_flexbuffer_do

```c
#define mulle_flexbuffer_do( p, size, block)
```

The `mulle_flexbuffer_do` macro is used to execute a block of code with a `mulle_buffer` object that uses a flexible buffer. The macro takes three arguments: `p` is a pointer to the `mulle_buffer` object to use, `size` is the size of the buffer to use if the `mulle_buffer` object is small, and `block` is the code block to execute. If the `mulle_buffer` object grows beyond the size of the pre-allocated buffer, it will automatically switch to using a dynamically allocated buffer. After the block of code is executed, the `mulle_buffer` object is automatically released.

### Example

```c
#include <stdio.h>
#include <string.h>
#include <mulle-buffer/mulle_buffer.h>

void foo(char *s) {
    size_t n = strlen(s) + 1;
    mulle_flexbuffer_do( copy, 32, n)
    {
        strcpy(copy, s);
        printf("Copied string: %s\n", copy);
    };
}

int main() 
{
    char *s = "Hello, world!";

    foo(s);
    return 0;
}
```

When the `foo` function is called with `s` equal to `"Hello, world!"`, the output of the program will be:

```
Copied string: Hello, world!
```

