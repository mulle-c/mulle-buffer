# 4.0.0



feat: improve buffer flexibility and safety

* Breaking changes to buffer length handling
  - Save pre-overflow length for safer data extraction
  - Integrate length control with zero filling via options
  - Change seek parameter order to match standard C

* Enhanced buffer initialization and creation
  - Add capacity parameter to `mulle_init`
  - Add `_default` variants for simpler allocator handling
  - Remove deprecated mulle-flexbuffer functionality

* Improved buffer access control
  - Add readonly/writeonly buffer modes with assertions
  - Add byte-level operations `(get_byte,` `pop_byte)`
  - Fix buffer overflow edge cases

* Build system improvements
  - Add CMake package configuration support
  - Improve 32-bit compatibility
  - Update project version to 4.0.0

* **BREAKING** ``mulle_buffer_zero_to_length`` is now done in ``mulle_buffer_set_length``
* **BREAKING** the length of the buffer before the overflow happened is now saved, so string and data extraction will provide the date before the overflow happened
* **BREAKING** ``mulle_buffer_set_seek`` has change parameter order to match `lseek` and `fseek`
* **BREAKING** mulle-flexbuffer is dead, use mulle-c11 mulle-alloca instead
* **BREAKING** ``mulle_init`` now has a capacity parameter just like the container init functions, maybe use ``mulle_init_default`` instead
* **BREAKING** `mulle-buffer-set-length` now has an `options` argument, which determines zerofilling and size fitting
* new ``_default`` functions like ``mulle_buffer_create_default`` allow the user to leave out an allocator argument, lessening the cognitive load
* `mulle-buffer-do` gains a default `alloca` so you have to do even less work to get better performing code
* mulle-buffer can now be set to readonly or writeonly (in addition to the default read/write)

* mulle-buffer gained readonly and writeonly assertion checks. you can mark a buffer as readonly or writeonly. the default is read/write (no asserts)

*** BREAKING ``_INIT_`` initializers are now called ``_DATA`` to comply with other mulle code
* ``mulle_buffer_get_byte`` does now exist

* added mulle-buffer-pop-byte function

* changed signature type of `_mulle__buffer_copy_range` dst to void *


### 3.5.1

* fix 32 bit problem for mulle-sprintf (special handling of `INT_MAX`)
* fix inflexible buffer not properly adjusting length for added zero

## 3.5.0

* changed `..._quoted_string` to `_c_string,` because it fits better
* fixes for 16bit scenarips
* **BREAKING CHANGE** misnamed ``mulle_buffer_find_byte`` is now ``mulle_buffer_seek_byte``
* **BREAKING CHANGE** ``mulle_flushablebuffer_flusher_t`` is now a function, not longer a function pointer to be in line with other mulle libraries
* **BREAKING CHANGE** `mulle__flushablebuffer` is now gone and completely replaces by `mulle_flushablebuffer.` `mulle_flushablebuffer` is an improved version that supports `mulle_buffer_do_FILE`
* added `_mulle__buffer_add_quoted_string` to output C double quoted strings with proper escape codes
* **BREAKING CHANGE** functions that were dealing with ranges but used offset/length now use struct `mulle_range` with proper validation (you can now use -1, for implicit strlen)
* the overflown condition is now better checked throughout the library
* rework flushable buffer
* fix explicit mulle-flexarray calls
* **BREAKING CHANGE** `mulle_flexarray_define` no longer exists.
* **BREAKING CHANGE** `mulle_flexarray()` now also defines the `<name>` pointer and alloc, realloc assign to it


## 3.4.0

* make string and zero functions to return int to detect truncation with the static bytes
* remove package.json as it conflicts with clib.json
* added `mulle_buffer_strcpy` synonym for `mulle_buffer_add_string`
* added convenience `mulle_buffer_do_allocator`


## 3.3.0

* convenience macros for mulle-buffer and flexbuffer support


## 3.2.0

* add `mulle_buffer_hexdump_default` constant


## 3.1.0

* ``mulle_buffer_make_string`` added
* ``MULLE_BUFFER_INIT`` added


# 3.0.0

* added `mulle_buffer_add_string_if_empty` and `mulle_buffer_add_string_if_not_empty`
* changed type for hexdump memory from `uint8_t *` to `void *`, which is more convenient
* added `mulle_flushablebuffer_as_buffer` casting function
    add stdthreads library for FreeBSD


## 2.2.0

* ``mulle_buffer_extract/mulle_buffer_extract_all`` is now `mulle_buffer_extract_data` or ``mulle_buffer_extract_string``
* ``mulle_buffer_guarantee`` now returns a non-null `void *` on success instead of an `int`
* added ``mulle_buffer_size_to_fit``
* reorder `mulle_buffer` ``_initial_storage`` field to below ``_sentinel``
* add some string functions for easier and less errorprone c-string construction
* added `mulle_buffer_get_last_byte`
* added `struct `mulle_data`` for benefit of *NSData*
* destroy buffer contents in ``_mulle__buffer_done`` if DEBUG is defined


### 2.1.1

* new mulle-sde project structure

## 2.1.0

* added `mulle_buffer_memcmp` function


# 2.0.0

* renamed struct `_mulle_buffer` to struct `mulle__buffer,` which cleans up the underscore prefixed function mess


### 1.2.1

* Various small improvements

## 1.2.0

* fix problem with static/dynamic mixed mode
* hex options are now inverted, making 0 the default and not -1


### 1.1.5

* modernized to mulle-sde with .mulle folder

### 1.1.4

* fix badge

### 1.1.3

* add missing file

### 1.1.2

* fix test a bit

### 1.1.1

* Various small improvements

## 1.1.0

* added `add_buffer` range functions


### 1.0.5

* remove obsolete file

### 1.0.4

* fix mingw, update sde

### 1.0.3

* modernized mulle-sde

### 1.0.2

* Various small improvements

### 1.0.1

* fix travis.yml

# 1.0.0

* migrated to mulle-sde
* made headernames hyphenated
* no longer distributed as a homebrew package

## 0.6.1

* rename mulle_buffer_dump_hex to mulle_buffer_hexdump and related


### 0.5.9

* support new mulle-tests

### 0.5.7

* fixed scion wrapper command

### 0.5.5

* follow mulle-configuration 3.1 changes and move .travis.yml to trusty

### 0.5.3

* Modernize cmake

### 0.4.9

* make cmake "C" project

### 0.4.7

* modernized project

## 0.4.1-5

* renamed inflexable methods to inflexible, but also kept old functions. So
there is no breaking API change. Marked old functions as deprecated. Need
new mulle_c11 for this.

### 0.3.1

* better asserts
* allow _mulle__buffer_done to run on an initialized buffer (still got to be
zeroed though)

### 0.2.1-7

* improve documentation
* rename some functions for release and for orthogonality with future releases
* call flush on done
* made `mulle_buffer_get_allocator` a non null returning function

### 0.1.1-3

* documentation improved somewhat

# 0.1

* moved it out of mulle-container, because it didn't belong there
