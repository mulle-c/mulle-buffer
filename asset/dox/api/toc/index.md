# mulle-buffer Library Documentation for AI
<!-- Keywords: buffer, growable, string, flushable, hexdump, seekable, allocator -->

## 1. Introduction & Purpose

**mulle-buffer** provides a growable, resizable byte buffer (`struct mulle_buffer`) for constructing arbitrary-length binary or text data dynamically or in static storage. It solves the problem of not knowing the required buffer size in advance: buffers grow automatically from stack backing onto the heap as needed. It is easy, fast and safe, and is used to implement NSMutableData.

Key capabilities:

- **Dynamic growth**: Buffers start on the stack (via the `mulle_buffer_do` macros) with a guaranteed 128-byte minimum backing and grow to heap-allocated storage when exceeded.
- **Flexible and inflexible modes**: Growable buffers, or fixed-size buffers that report overflow instead of overrunning caller storage.
- **Append-first byte builder**: Writing is the primary operation; a single cursor is used for writing, reading, and seeking.
- **Convenience operations**: String concatenation, C-string escaping (`_c_` functions), quoted output, hexdumps.
- **Stream-like reading**: `next_byte`, `next_character`, `peek_byte`, `seek_byte`, `memcmp`, and offset-based access (POSIX-like `lseek` semantics).
- **Macro-based lifecycle management**: `mulle_buffer_do` and friends ensure automatic initialization/cleanup.
- **`mulle_flushablebuffer`**: A stream-like output variant that flushes long output to a sink (fwrite-like callback) to keep memory bounded.

This library is a foundational component of `mulle-core` and is a dependency of many other mulle data-structure libraries (`mulle-data`, object/container libraries). Its public API is version `5.2.0` (`MULLE__BUFFER_VERSION`).

## 2. Key Concepts & Design Philosophy

### Contract (read this first — matches README)

`mulle_buffer` is an append-first byte builder, **not** a general file-like stream:

- Writing is the primary operation; reading is a secondary convenience.
- The buffer has a single cursor (`_curr`) used for writing, reading, and seeking.
- Seeking changes the logical length; `mulle_buffer_get_length` follows the cursor.
- `MULLE_BUFFER_SEEK_END` is relative to the allocation **capacity**, not the written length.
- Reads consume the same cursor used for writing.
- Allocation uses a mulle allocator under the no-fail allocator contract: it never returns `NULL`.
- Write functions are intentionally `void`; failure is reported through `mulle_buffer_has_overflown`.
- Callers must check `mulle_buffer_has_overflown` after writing to a fixed-size or flushable buffer.
- const/read-only buffers must not be written; write-only buffers must not be read; both are enforced by assertions in debug builds.
- Self-aliasing (appending a slice of the buffer's own storage) is **not** supported; string add functions mark the buffer overflown at runtime, `add_bytes` trips an assertion in debug builds.

### Buffer modes and states

- `flexible`: growable; will malloc when storage is exhausted (`MULLE_BUFFER_IS_FLEXIBLE = 0`).
- `inflexible`: fixed-size; sets the sticky `overflown` flag on excess (`MULLE_BUFFER_IS_INFLEXIBLE = 1`).
- `flushable`: fixed storage; flushes via callback when full (always inflexible, `MULLE_BUFFER_IS_FLUSHABLE = 3`).
- `overflown`: sticky flag (`MULLE_BUFFER_IS_OVERFLOWN = 8`); once set it cannot be cleared with `remove_all`. The pre-overflow length is kept in `_size`, further writes become no-ops; only `mulle_buffer_reset` or `done`+init reuses the buffer.
- `readonly` / `writeonly`: mode flags (`MULLE_BUFFER_IS_READONLY = 0x40`, `MULLE_BUFFER_IS_WRITEONLY = 0x80`) enforced by debug assertions.

### Design layers

- `struct mulle__buffer` (in `mulle--buffer.h`) is the internal base: storage pointers + `_size` + `_type`, **no allocator**, and none of its functions check for `NULL`.
- `struct mulle_buffer` (in `mulle-buffer.h`) is the user-facing type: base fields plus a `struct mulle_allocator *_allocator`. It is a possibly-growing memory block (the C analog of `NSData`/`NSMutableData`).
- Since version 5.2.0 the read-only accessors take `const struct mulle_buffer *` and delegate to a const-qualified inline layer (`_mulle__buffer_*`) in `mulle--buffer.h`.
- `struct mulle_flushablebuffer` extends `struct mulle_buffer` with a flusher callback, userinfo, and a `_flushed` byte counter. It is initialized as write-only.

Sizes on 64-bit: `sizeof(struct mulle__buffer) = 48`, `sizeof(struct mulle_buffer) = 56`, `sizeof(struct mulle_flushablebuffer) = 80` (24/28/40 on 32-bit).

**NOT THREAD-SAFE**: All instances must be used from a single thread.

The umbrella header `<mulle-buffer/mulle-buffer.h>` includes everything, including `mulle-flushablebuffer.h`.

## 3. Core API & Data Structures

### 3.1. `mulle--buffer.h` (internal base)

#### `struct mulle__buffer`

- **Purpose**: The allocator-free base buffer implementation. Generally do not touch directly, but its `_type` flags and seek-modes enum are shared with the public layer.
- **Key Fields** (`MULLE__BUFFER_BASE`):
  - `unsigned char *_storage`: the allocated memory block.
  - `unsigned char *_curr`: single read/write/seek cursor.
  - `unsigned char *_sentinel`: one past the end of the allocation.
  - `unsigned char *_initial_storage`: original (stack/static) storage for flexible buffers.
  - `size_t _size`: initial capacity / storage size; on overflow it keeps the pre-overflow logical length.
  - `unsigned int _type`: mode flags (see enums below).

- **Type flags** (enum):
  - `MULLE_BUFFER_IS_FLEXIBLE = 0`, `MULLE_BUFFER_IS_INFLEXIBLE = 1`, `MULLE_BUFFER_IS_FLUSHABLE = 3`, `MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE = 5`, `MULLE_BUFFER_IS_OVERFLOWN = 8`.

- **Seek modes** (enum):
  - `MULLE_BUFFER_SEEK_SET = 0`, `MULLE_BUFFER_SEEK_CUR = 1`, `MULLE_BUFFER_SEEK_END = 2`.

- **Key functions** (all prefixed `_mulle__buffer_*`; those the public layer delegates to):
  - Creation/destruction: `_mulle__buffer_create`, `_mulle__buffer_destroy`.
  - Initialization: `_mulle__buffer_init`, `_mulle__buffer_init_with_allocated_bytes`, `_mulle__buffer_init_with_static_bytes`, `_mulle__buffer_init_inflexible_with_static_bytes`, `_mulle__buffer_init_with_const_bytes`, `_mulle__buffer_done`, `_mulle__buffer_reset`.
  - Resize: `_mulle__buffer_grow`, `_mulle__buffer_size_to_fit`, `_mulle__buffer_set_length`, `_mulle__buffer_guarantee`, `_mulle__buffer_advance`.
  - Modification: `_mulle__buffer_make_inflexible`, `_mulle__buffer_add_byte`, `_mulle__buffer_add_char`, `_mulle__buffer_add_bytes`, `_mulle__buffer_add_string`, `_mulle__buffer_add_c_char`, `_mulle__buffer_add_c_chars`, `_mulle__buffer_add_c_string`, `_mulle__buffer_memset`, `_mulle__buffer_remove_all`, `_mulle__buffer_remove_last_byte`, `_mulle__buffer_pop_byte`, `_mulle__buffer_remove_in_range`, `_mulle__buffer_zero_last_byte`, `_mulle__buffer_make_string`, `_mulle__buffer_set_overflown`.
  - Extraction: `_mulle__buffer_extract_data`, `_mulle__buffer_extract_string`, `_mulle__buffer_get_string`.
  - Reading (static inline, const-qualified since 5.2.0): `_mulle__buffer_get_byte`, `_mulle__buffer_get_last_byte`, `_mulle__buffer_next_byte`, `_mulle__buffer_peek_byte`, `_mulle__buffer_next_character`, `_mulle__buffer_next_bytes`, `_mulle__buffer_reference_bytes`, `_mulle__buffer_memcmp`, `_mulle__buffer_seek_byte`, `_mulle__buffer_copy_range`.
  - Query (static inline, const-qualified): `_mulle__buffer_has_overflown`, `_mulle__buffer_is_inflexible`, `_mulle__buffer_is_flushable`, `_mulle__buffer_is_full`, `_mulle__buffer_get_remaining_length`, `_mulle__buffer_is_big_enough`, `_mulle__buffer_is_empty`, `_mulle__buffer_is_void`, `_mulle__buffer_get_length`, `_mulle__buffer_get_capacity`, `_mulle__buffer_get_bytes`, `_mulle__buffer_get_data`, `_mulle__buffer_get_staticlength`, `_mulle__buffer_intersects_bytes`, `_mulle__buffer_self_referencing`.
  - Seek (global): `_mulle__buffer_get_seek`, `_mulle__buffer_set_seek`, `_mulle__buffer_get_lseek`, `_mulle__buffer_lseek`.
  - Helpers: `_mulle_char_strnlen` (strnlen replacement; strnlen is not C), `_mulle__buffer_flush`.
  - Deprecated spellings `_mulle__buffer_is_inflexable`, `mulle__buffer_init_inflexable_with_static_bytes`, `_mulle__buffer_make_inflexable` remain for backwards compatibility.

### 3.2. `mulle-buffer.h`

#### `struct mulle_buffer`

- **Purpose**: The public, user-facing growable byte buffer.
- **Key Fields** (`MULLE_BUFFER_BASE`): all `mulle__buffer` fields plus `struct mulle_allocator *_allocator`.
- **Related**: `MULLE_BUFFER_DEFAULT_CAPACITY` is `128` (default capacity for heap-init and the guaranteed minimum stack backing for `mulle_buffer_do`). `MULLE_BUFFER_DATA`, `MULLE_BUFFER_FLEXIBLE_DATA`, `MULLE_BUFFER_FLEXIBLE_FILLED_DATA`, `MULLE_BUFFER_INFLEXIBLE_DATA`, `MULLE_BUFFER_INFLEXIBLE_FILLED_DATA` are compound-literal initializers for static/global data-section use (side-effect-free expressions only); in code use the single-evaluation inline wrappers `_mulle_buffer_data`, `_mulle_buffer_flexible_data`, `_mulle_buffer_flexible_filled_data`, `_mulle_buffer_inflexible_data`, `_mulle_buffer_inflexible_filled_data`.

#### Allocation / initialization / destruction

- `struct mulle_buffer *mulle_buffer_alloc(struct mulle_allocator *allocator)` — malloc the struct only (inline).
- `#define mulle_buffer_alloc_default()` — `mulle_buffer_alloc(NULL)`.
- `struct mulle_buffer *mulle_buffer_create(struct mulle_allocator *allocator)` — allocate + init with default capacity (global).
- `#define mulle_buffer_create_default()` — `mulle_buffer_create(NULL)`.
- `void mulle_buffer_destroy(struct mulle_buffer *buffer)` — free storage + struct (inline; only for `create`d buffers).
- `void mulle_buffer_done(struct mulle_buffer *buffer)` — finalize an embedded/stack buffer; safe on zero-initialized storage (inline).
- `void mulle_buffer_set_allocator(struct mulle_buffer *buffer, struct mulle_allocator *allocator)` (inline; `NULL` → `&mulle_default_allocator`).
- `void mulle_buffer_init(struct mulle_buffer *buffer, size_t capacity, struct mulle_allocator *allocator)` (inline).
- `#define mulle_buffer_init_default(buffer)` — `mulle_buffer_init(buffer, MULLE_BUFFER_DEFAULT_CAPACITY, NULL)`.
- `void mulle_buffer_init_with_capacity(struct mulle_buffer *buffer, size_t capacity, struct mulle_allocator *allocator)` — deprecated alias.
- `void mulle_buffer_init_with_allocated_bytes(struct mulle_buffer *buffer, void *storage, size_t length, struct mulle_allocator *allocator)` — growable buffer over caller-allocated, freeable storage (inline; `assert(storage || !length)`).
- `void mulle_buffer_init_with_static_bytes(struct mulle_buffer *buffer, void *storage, size_t length, struct mulle_allocator *allocator)` — growable buffer over caller stack/static storage (inline).
- `void mulle_buffer_init_inflexible_with_static_bytes(struct mulle_buffer *buffer, void *storage, size_t length)` — fixed-size, writable, storage belongs to caller, no allocator (inline).
- `void mulle_buffer_init_with_const_bytes(struct mulle_buffer *buffer, const void *storage, size_t length)` — read-only view over `const`/compiled-in data (global).
- `void mulle_buffer_reset(struct mulle_buffer *buffer)` — reinitialize to default state keeping allocator; discards `_initial_storage` backing (global).

#### Sizing / growth / length

- `int mulle_buffer_grow(struct mulle_buffer *buffer, size_t min_amount)` — guarantee capacity ≥ min_amount; 0 on success, negative on failure (inline).
- `void mulle_buffer_size_to_fit(struct mulle_buffer *buffer)` — shrink storage to fit contents exactly (inline).
- `void mulle_buffer_make_inflexible(struct mulle_buffer *buffer, void *storage, size_t length)` — replace contents, convert to fixed size over caller storage (inline).
- `int mulle_buffer_set_length(struct mulle_buffer *buffer, size_t length, unsigned int options)` — sets logical length; options: `MULLE_BUFFER_SHRINK_OR_ZEROFILL=0`, `MULLE_BUFFER_NO_SHRINK=1`, `MULLE_BUFFER_NO_ZEROFILL=2`, `MULLE_BUFFER_NO_SHRINK_OR_ZEROFILL=3`; `-1` on failure (inline).

#### Extraction (ownership transfer)

All extractors consume the buffer: it is left empty with no backing storage and must be re-initialized (`mulle_buffer_reset` / `mulle_buffer_init_*`) before reuse. The caller owns the returned memory and must free it with the same allocator the buffer used (`mulle_buffer_get_allocator`).

- `struct mulle_data mulle_buffer_extract_data(struct mulle_buffer *buffer)` (inline) — empty buffer → `{bytes == NULL, length == 0}`; NULL buffer → invalid `mulle_data`.
- `void *mulle_buffer_extract_string(struct mulle_buffer *buffer)` (inline) — zero-terminates first; empty buffer → allocated `""` (still caller-owned); NULL buffer → NULL.
- `void *mulle_buffer_extract_bytes(struct mulle_buffer *buffer)` (inline) — bytes pointer only; empty buffer → NULL; NULL buffer → NULL.

#### Removal

- `void mulle_buffer_remove_all(struct mulle_buffer *buffer)` (inline) — resets cursor to storage; **no-op when overflown** (sticky flag).
- `void mulle_buffer_remove_in_range(struct mulle_buffer *buffer, struct mulle_range range)` (inline).

#### Accessors

- `static inline struct mulle_allocator *mulle_buffer_get_allocator(const struct mulle_buffer *buffer)` — returns `_allocator`; NULL for static/const-storage buffers (they own no allocator and must not be freed); NULL arg → `&mulle_default_allocator`.
- `static inline struct mulle_data mulle__buffer_get_data(const struct mulle_buffer *buffer)` — (note double-underscore name) bytes+length as `mulle_data`; NULL arg → invalid data.
- `void *mulle_buffer_get_bytes(const struct mulle_buffer *buffer)` (inline) — pointer to storage; not null-terminated.
- `char *mulle_buffer_get_string(struct mulle_buffer *buffer)` (inline) — **mutating**: appends a NUL terminator (may grow storage), only valid on writeable buffers.
- `size_t mulle_buffer_get_length(const struct mulle_buffer *buffer)` (inline) — follows the cursor; overflown → pre-overflow length kept in `_size`.
- `size_t mulle_buffer_get_capacity(const struct mulle_buffer *buffer)` (inline).
- `size_t mulle_buffer_get_staticlength(const struct mulle_buffer *buffer)` (inline) — current length if still backed by `_initial_storage`, else 0.

#### Seeking & positioning

- `int mulle_buffer_set_seek(struct mulle_buffer *buffer, long seek, int mode)` (inline) — `long` seek is 32-bit on LLP64 (Windows); positions ≥ 2 GB truncate there. `MULLE_BUFFER_SEEK_END` is relative to the **end of the allocation** (new position = capacity + offset). Returns 0 on success, -1 on failure (invalid/flushable/out-of-range).
- `long mulle_buffer_get_seek(const struct mulle_buffer *buffer)` (inline).
- `off_t mulle_buffer_lseek(struct mulle_buffer *buffer, off_t offset, int mode)` (inline) — `off_t` variant mirroring POSIX `lseek(2)`; returns the new position or -1.
- `off_t mulle_buffer_get_lseek(const struct mulle_buffer *buffer)` (inline) — `off_t` position; -1 if buffer invalid.
- `void *mulle_buffer_advance(struct mulle_buffer *buffer, size_t length)` (inline) — reserves `length` bytes, advances cursor, returns pointer to the newly reserved area.

#### Read/write-only mode

- `int mulle_buffer_is_readonly(const struct mulle_buffer *buffer)` (inline).
- `int mulle_buffer_is_writeonly(const struct mulle_buffer *buffer)` (inline).
- `void mulle_buffer_set_readonly(struct mulle_buffer *buffer)` (inline).
- `void mulle_buffer_set_writeonly(struct mulle_buffer *buffer)` (inline).

#### Copy out

- `void mulle_buffer_copy_range(struct mulle_buffer *buffer, struct mulle_range range, void *dst)` (inline).

#### Query / state

All of the following are `static inline` and take `const struct mulle_buffer *buffer`:

- `int mulle_buffer_is_inflexible(const struct mulle_buffer *buffer)` — 1 for NULL.
- `int mulle_buffer_is_flushable(const struct mulle_buffer *buffer)`.
- `int mulle_buffer_is_full(const struct mulle_buffer *buffer)`.
- `size_t mulle_buffer_remaining_length(const struct mulle_buffer *buffer)` — capacity minus cursor; 0 when overflown.
- `int mulle_buffer_is_big_enough(const struct mulle_buffer *buffer, size_t len)`.
- `int mulle_buffer_is_empty(const struct mulle_buffer *buffer)`.
- `int mulle_buffer_is_void(const struct mulle_buffer *buffer)` — backing storage cannot hold content.
- `int mulle_buffer_has_overflown(const struct mulle_buffer *buffer)` — the official write-contract error channel; 1 for NULL, sticky.
- `int mulle_buffer_intersects_bytes(const struct mulle_buffer *buffer, const void *bytes, size_t length)` — range-overlap test.

Convenience macro for the official write contract:
- `#define mulle_buffer_return_if_overflown(buffer, rval)` — `return(rval)` as soon as the buffer has overflown.

#### Write operations (in `mulle-buffer.h`)

Single bytes / chars:
- `void mulle_buffer_add_byte(struct mulle_buffer *buffer, unsigned char c)` (inline).
- `void mulle_buffer_add_char(struct mulle_buffer *buffer, int c)` (inline; asserts `CHAR_MIN..CHAR_MAX`).
- `void mulle_buffer_remove_last_byte(struct mulle_buffer *buffer)` (inline).
- `int _mulle_buffer_pop_byte(struct mulle_buffer *buffer)` (inline; no NULL check) — pop last byte or -1.
- `int mulle_buffer_pop_byte(struct mulle_buffer *buffer)` (inline; NULL-safe).

Bulk data:
- `void mulle_buffer_add_bytes(struct mulle_buffer *buffer, const void *bytes, size_t length)` (inline) — assert on self-aliasing in debug builds.
- `void mulle_buffer_add_chars(struct mulle_buffer *buffer, const char *s, size_t length)` (inline) — typed alias of `add_bytes`.
- `void mulle_buffer_add_bytes_callback(void *buffer, void *bytes, size_t length)` (global) — callback-compatible form (matches `mulle_utf_add_bytes_function_t`).
- `void *mulle_buffer_guarantee(struct mulle_buffer *buffer, size_t length)` — returns pointer to reserved unused area (inline); use `mulle_buffer_advance` to forward the cursor.
- `void mulle_buffer_memset(struct mulle_buffer *buffer, int c, size_t length)` (inline).

Strings:
- `void mulle_buffer_add_string(struct mulle_buffer *buffer, const char *s)` (inline).
- `void mulle_buffer_append_string(struct mulle_buffer *buffer, const char *s)` (inline alias for `add_string`).
- `size_t mulle_buffer_add_string_with_maxlength(struct mulle_buffer *buffer, const char *bytes, size_t length)` (inline) — returns bytes actually appended.
- `void mulle_buffer_strcat(struct mulle_buffer *buffer, const char *bytes)` (inline synonym for `add_string`).
- `void mulle_buffer_strcpy(struct mulle_buffer *buffer, const char *bytes)` (inline) — clears then adds.
- `void mulle_buffer_add_string_if_empty(struct mulle_buffer *buffer, const char *bytes)` (inline).
- `void mulle_buffer_add_string_if_not_empty(struct mulle_buffer *buffer, const char *bytes)` (inline).
- `void mulle_buffer_add_buffer(struct mulle_buffer *buffer, struct mulle_buffer *other)` (inline) — append another buffer's contents.
- `void mulle_buffer_add_buffer_range(struct mulle_buffer *buffer, struct mulle_buffer *other, struct mulle_range range)` (inline).

C-string escaping:
- `void mulle_buffer_add_c_char(struct mulle_buffer *buffer, char c)` (inline) — adds C escape sequence, no quotes.
- `void mulle_buffer_add_c_chars(struct mulle_buffer *buffer, const char *s, size_t length)` (inline) — escaped, no quotes.
- `void mulle_buffer_add_c_chars_callback(void *buffer, void *bytes, size_t length)` (global).
- `void mulle_buffer_add_c_string(struct mulle_buffer *buffer, const char *s)` (inline) — escapes and wraps in `""`.

Termination helpers:
- `int mulle_buffer_zero_last_byte(struct mulle_buffer *buffer)` (inline) — returns -1 (NULL buffer), 0 (zeroed, no loss), 1 (lossy truncate, may corrupt UTF-8), 2 (void buffer).
- `int mulle_buffer_make_string(struct mulle_buffer *buffer)` (inline) — ensure trailing NUL; returns 0 (no truncation), 1 (truncated), 2 (void buffer). Does not advance length.

#### Read operations

All `static inline` in `mulle-buffer.h`:

- `int mulle_buffer_get_byte(const struct mulle_buffer *buffer, size_t index)` — byte at index or -1.
- `int mulle_buffer_get_last_byte(const struct mulle_buffer *buffer)` — last byte or -1.
- `int mulle_buffer_next_byte(struct mulle_buffer *buffer)` — consumes + returns next byte, or -1. (`-1 == no more bytes`)
- `int mulle_buffer_peek_byte(const struct mulle_buffer *buffer)` — next byte without consuming, or -1.
- `int mulle_buffer_next_character(struct mulle_buffer *buffer)` — unsigned `next_byte`, or -1.
- `int mulle_buffer_next_bytes(struct mulle_buffer *buffer, void *buf, size_t len)` — copy out up to `len` bytes (advances cursor); 0 on success, -1 if not enough.
- `void *mulle_buffer_reference_bytes(struct mulle_buffer *buffer, size_t len)` — borrowed pointer to next `len` bytes (advances cursor); NULL if not big enough.
- `long mulle_buffer_seek_byte(struct mulle_buffer *buffer, unsigned char byte)` — seeks cursor forward to first occurrence; 0 found, -1 not found/invalid/overflown.
- `int mulle_buffer_memcmp(const struct mulle_buffer *buffer, const void *bytes, size_t length)` — comparison, +1 if NULL buffer.

#### Hexdump (16 bytes per line)

- `enum mulle_buffer_hexdump_options { mulle_buffer_hexdump_default = 0x0, mulle_buffer_hexdump_no_offset = 0x1, mulle_buffer_hexdump_no_hex = 0x2, mulle_buffer_hexdump_no_ascii = 0x4 }` — bits ORable.
- `void mulle_buffer_hexdump_line(struct mulle_buffer *buffer, const void *bytes, unsigned int n, size_t counter, unsigned int options)` (global) — dumps only for `n >= 1 && n <= 16`.
- `void mulle_buffer_hexdump(struct mulle_buffer *buffer, const void *bytes, size_t length, size_t counter, unsigned int options)` (global) — dumps all; does not append `\0`.

#### Deprecated backwards-compatibility spellings

- `mulle_buffer_is_inflexable`, `mulle_buffer_init_inflexable_with_static_bytes`, `mulle_buffer_make_inflexable` — deprecated aliases of the `inflexible` variants.

#### Convenience `do` macros

All create a stack-allocated buffer whose storage self-destructs when the block exits. `break` inside the block is safe; a `return` leaks — use `mulle_buffer_return(name, value)` instead (requires `typeof`-capable compiler). `mulle_buffer_do_string` additionally assigns the extracted allocated string to `s`.

- `mulle_buffer_do(name)` — default allocator, guaranteed ≥ `MULLE_BUFFER_DEFAULT_CAPACITY` (128) bytes of stack backing.
- `mulle_buffer_do_allocator(name, allocator)` — same with a custom allocator.
- `mulle_buffer_do_flexible(name, data, len)` — growable buffer over caller storage; mallocs when exhausted.
- `mulle_buffer_do_flexible_filled(name, data, len)` — growable buffer pre-filled with caller data.
- `mulle_buffer_do_inflexible(name, data, len)` — fixed-size; overflow → overflown state, no malloc.
- `mulle_buffer_do_inflexible_filled(name, data, len)` — fixed-size, pre-filled, writable.
- `mulle_buffer_do_string(name, allocator, s)` — block-scoped buffer; on exit assigns allocated C string to `s` (free with the same allocator).
- `mulle_buffer_return(name, value)` — compute value, `done` the buffer, return (no leak).

Mode assertion macros: `mulle_buffer_assert_readable(buffer)`, `mulle_buffer_assert_writeable(buffer)` (assert on the opposing `_type` flag).

### 3.3. `mulle-flushablebuffer.h`

#### `struct mulle_flushablebuffer`

- **Purpose**: A write-only, inflexible, stream-like output buffer that flushes contents to a sink when full, keeping memory bounded. It is the public API for "dump and other longer output".
- **Key Fields** (`MULLE_FLUSHABLEBUFFER_BASE`): all `mulle_buffer` fields plus:
  - `mulle_flushablebuffer_flusher_t *_flusher`: the flush callback.
  - `void *_userinfo`: opaque data passed to the flusher.
  - `size_t _flushed`: counts bytes delivered to the flusher (informational; seek follows it).
- **Related macros**: `MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY` (`sizeof(void*) * 32`), `MULLE_FLUSHABLEBUFFER_MIN_CAPACITY` (`sizeof(double)` — for `mulle_buffer` to guarantee ≥ 8 bytes), `MULLE_FLUSHABLEBUFFER_TYPE` (= `MULLE_BUFFER_IS_FLUSHABLE | MULLE_BUFFER_IS_WRITEONLY`).
- `mulle_flushablebuffer_as_buffer(struct mulle_flushablebuffer *buffer)` — cast to `struct mulle_buffer *` (inline); write via the `mulle_buffer_*` functions.

#### Flusher type and contract

```
typedef size_t   mulle_flushablebuffer_flusher_t( void *buf,
                                                  size_t one,
                                                  size_t len,
                                                  void *userinfo);
```

Like `fwrite`. The flusher is responsible for retrying partial writes internally: it must keep writing until all `len` bytes are delivered, then return `len`. A **short return is a genuine delivery failure**: the buffer marks itself overflown and retains the undelivered bytes only for inspection. Do not retry a flusher that reported a short return (the delivered prefix would be duplicated).

#### Initialization

- Compound-literal initializers for static/global data sections (side-effect-free expressions only): `MULLE_FLUSHABLEBUFFER_STATIC_DATA(xstorage, xlength, xflusher, xuserinfo)` and `MULLE_FLUSHABLEBUFFER_ALLOCATED_DATA(xstorage, xlength, xflusher, xuserinfo, xallocator)`. Single-evaluation inline wrappers for use in code: `_mulle_flushablebuffer_static_data(...)`, `_mulle_flushablebuffer_allocated_data(...)`.
- `void mulle_flushablebuffer_init(struct mulle_flushablebuffer *buffer, void *storage, size_t length, mulle_flushablebuffer_flusher_t *flusher, void *userinfo)` — backwards compatibility, uses `&mulle_default_allocator`.
- `void mulle_flushablebuffer_init_with_static_bytes(struct mulle_flushablebuffer *buffer, void *storage, size_t length, mulle_flushablebuffer_flusher_t flusher, void *userinfo, struct mulle_allocator *allocator)` — storage is caller-owned (asserts `storage && length && flusher`, `length >= MULLE_FLUSHABLEBUFFER_MIN_CAPACITY`).
- `void mulle_flushablebuffer_init_with_allocated_bytes(struct mulle_flushablebuffer *buffer, void *storage, size_t length, mulle_flushablebuffer_flusher_t flusher, void *userinfo, struct mulle_allocator *allocator)` — storage will be freed on done.

#### Flush lifecycle

- `int mulle_flushablebuffer_flush(struct mulle_flushablebuffer *buffer)` (inline) — manually flush buffered data; -1 if buffer NULL.
- `int mulle_flushablebuffer_done(struct mulle_flushablebuffer *buffer)` (inline) — flush remaining data, then finalize; returns 0 on success, non-zero if the flush failed (buffer still alive).
- `struct mulle_flushablebuffer *mulle_flushablebuffer_create(size_t length, mulle_flushablebuffer_flusher_t *flusher, void *userinfo, struct mulle_allocator *allocator)` (global) — heap instance.
- `int mulle_flushablebuffer_destroy(struct mulle_flushablebuffer *buffer)` (global) — flush + free storage + free struct; on flush failure still frees (call `mulle_flushablebuffer_flush` first if delivery matters).

## 4. Performance Characteristics

- **Append (`add_byte`, `add_bytes`, `add_string`, etc.)**: O(1) amortized per byte due to geometric overallocation/growth (grow via allocator realloc); the copy on realloc is O(capacity). `guarantee`/`advance` give O(1) reservation for batch writes (e.g., `fread` loops).
- **Growth (`mulle_buffer_grow`)**: O(n) amortized; grows to at least the requested `min_amount`. `size_to_fit` may shrink the allocation (O(n) copy when shrinking).
- **Seek (`set_seek`/`lseek`/`get_seek`/`get_lseek`)**: O(1). POSIX-like `off_t` variants (`lseek`/`get_lseek`) avoid 2 GB truncation on LLP64 platforms.
- **Accessors (`get_length`, `get_capacity`, `is_*`, `has_overflown`, `remaining_length`)**: O(1) inline.
- **Extract (`extract_data`/`extract_bytes`/`extract_string`)**: O(1) on the data path (ownership transfer of the malloc block); `extract_string` may add O(n) for zero-termination.
- **Reading**: `get_byte`/`next_byte`/`peek_byte`/`next_character` O(1); `memcmp`/`memset`/`hexdump`/`copy_range` O(n); `seek_byte` O(n) worst case (memchr scan).
- **Flushablebuffer**: fixed capacity; writes are O(1) until full then flush O(n); memory stays bounded regardless of total output length.
- **Memory**: struct is 56 bytes (64-bit); small workloads stay entirely on the stack with zero mallocs (`mulle_buffer_do` guarantees 128 bytes).
- **Thread-safety**: **None.** All instances must be used from a single thread; external locking is required for sharing.

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Use the `mulle_buffer_do` family for scoped buffers.** They own the stack storage and clean up automatically.
   ```c
   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "hello");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
   ```
2. **Choose the right mode.**
   - Dynamic output: `mulle_buffer_do` (flexible, defaults to 128 B stack) or `mulle_buffer_do_flexible(buf, data, len)` for a custom stack chunk.
   - Fixed-size / overflow-protected: `mulle_buffer_do_inflexible` or `mulle_buffer_init_inflexible_with_static_bytes`.
   - Read-only view of compiled-in data: `mulle_buffer_init_with_const_bytes`.
   - Long/bounded output: `mulle_flushablebuffer`.
3. **Always free extracted memory** with the same allocator the buffer used (`mulle_buffer_get_allocator`). `extract_*` consumes the buffer — re-initialize before reuse.
4. **Check `mulle_buffer_has_overflown` after writing to fixed-size or flushable buffers** whose capacity you did not size explicitly; use `mulle_buffer_return_if_overflown(buffer, rval)` for the one-line pattern.
5. **Use `mulle_buffer_get_string` only on writeable buffers** — it is a mutating operation that appends a NUL (and may grow). `mulle_buffer_get_bytes` does not guarantee termination.
6. **For large-scale reads**, reserve with `mulle_buffer_guarantee` + write + `mulle_buffer_advance`, then `mulle_buffer_size_to_fit` and `mulle_buffer_extract_data`.
7. **Never append a slice of the buffer's own storage** (self-aliasing). Copy out first via `mulle_buffer_copy_range` if needed.

### Common Pitfalls

- **`_`-prefixed fields and functions are internal.** Do not manipulate `_storage`, `_curr`, `_sentinel`, `_type` directly.
- **`return` inside a `mulle_buffer_do` block leaks.** Use `mulle_buffer_return(name, value)` (or manually call `mulle_buffer_done` first). `break` is safe.
- **Overflow is sticky.** `mulle_buffer_remove_all` will not clear it; `mulle_buffer_reset` (or `done` + init) is required to reuse.
- **`MULLE_BUFFER_SEEK_END` is relative to capacity, not written length** — and a flushable buffer's seek counts flushed bytes. Positions ≥ 2 GB need the `off_t` `lseek`/`get_lseek` family on LLP64.
- **A NULL buffer is reported as overflown** by `mulle_buffer_has_overflown` (returns 1), and `mulle_buffer_is_inflexible(NULL)` returns 1.
- **Infexible is spelled `inflexible`** (correct); the `_inflexable` spelling is deprecated.
- **`do_string` result must be freed** (`mulle_free`) and must not escape the semantics of "one allocated string".

### Idiomatic usage

Use the block macros for temporary strings; manual `mulle_buffer_init_*`/`mulle_buffer_done` for buffers that must outlive the function; `mulle_flushablebuffer` for streaming output; `hexdump` for diagnostics.

## 6. Integration Examples

Examples follow the mulle coding style: 3-space indent, Allman braces, one variable per line with column-aligned types, `return( expr);`.

### Example 1: Simple string building with `mulle_buffer_do`

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

void  test( void)
{
   mulle_buffer_do( buffer)
   {
      unsigned int  i;

      for( i = 0; i < 10; i++)
         mulle_buffer_add_byte( buffer, 'a' + i % 26);

      mulle_buffer_memset( buffer, 'z', 10);
      mulle_buffer_add_string( buffer, "hello");

      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
}

int  main( void)
{
   test();
   return( 0);
}
```

### Example 2: Allocated string outliving the block (`mulle_buffer_do_string`)

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

void  test( void)
{
   char  *s;

   // "s" is a malloced string, useable after the block
   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle_buffer_add_string( buffer, "VfL Bochum 1848");
      break;
   }

   printf( "%s\n", s);
   mulle_free( s);   // free with the same allocator (the default here)
}

int  main( void)
{
   test();
   return( 0);
}
```

### Example 3: Fixed-size buffer with overflow check (the official write contract)

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

void  test( void)
{
   char  tmp[ 8];   // space for seven characters plus a trailing zero

   mulle_buffer_do_inflexible( buffer, tmp, sizeof( tmp))
   {
      // writes beyond the capacity silently truncate and set the sticky flag
      // the pre-overflow content is preserved
      mulle_buffer_add_string( buffer, "VfL_");
      mulle_buffer_add_string( buffer, "Bochum");

      if( mulle_buffer_has_overflown( buffer))
         fprintf( stderr, "buffer overflowed, content truncated\n");

      // get_string zero-terminates the last byte ("VfL_Boc")
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
   // the single-line official helper is mulle_buffer_return_if_overflown( buffer, rval);
}

int  main( void)
{
   test();
   return( 0);
}
```

### Example 4: Streaming output with `mulle_flushablebuffer`

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

// like fwrite; must return `len` on success
static size_t   fwrite_stdout( void *buf, size_t one, size_t len, void *userinfo)
{
   return( fwrite( buf, one, len, (FILE *) userinfo));
}

void  dump( void *bytes, size_t length)
{
   struct mulle_flushablebuffer   output;
   struct mulle_buffer            *buffer;
   char                           storage[ 256];

   mulle_flushablebuffer_init_with_static_bytes( &output,
                                                 storage,
                                                 sizeof( storage),
                                                 fwrite_stdout,
                                                 stdout,
                                                 NULL);
   buffer = mulle_flushablebuffer_as_buffer( &output);

   mulle_buffer_hexdump( buffer, bytes, length, 0, mulle_buffer_hexdump_default);

   if( mulle_flushablebuffer_done( &output))   // flush remaining, or final write failed
      fprintf( stderr, "flush failed\n");
}

int  main( void)
{
   char   data[ 45];
   int    i;

   for( i = 0; i < 45; i++)
      data[ i] = (char) i;

   dump( data, sizeof( data));
   return( 0);
}
```

### Example 5: Reading a file into a malloced buffer (guarantee/advance/size_to_fit)

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

struct mulle_data   read_file( FILE *fp)
{
   struct mulle_buffer   buffer;
   struct mulle_data     data;
   void                  *ptr;
   size_t                length;
   size_t                size;

   mulle_buffer_init_default( &buffer);
   while( ! feof( fp))
   {
      ptr  = mulle_buffer_guarantee( &buffer, 0x1000);
      size = mulle_buffer_remaining_length( &buffer);
      assert( size >= 0x1000);   // could also be larger, use it
      length = fread( ptr, 1, size, fp);

      mulle_buffer_advance( &buffer, length);
   }
   mulle_buffer_size_to_fit( &buffer);

   data = mulle_buffer_extract_data( &buffer);   // consumes the buffer
   mulle_buffer_done( &buffer);

   return( data);   // caller owns `data.bytes`
}
```

### Example 6: Seeking and sequential reading with an inflexible buffer

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int  main( void)
{
   struct mulle_buffer   buffer;
   static char           string[] = "VfL Bochum 1848\n";
   int                   c;

   // fixed-size, writable view over `string`; no allocator involved
   mulle_buffer_init_inflexible_with_static_bytes( &buffer, string, sizeof( string));

   mulle_buffer_set_seek( &buffer, 4, MULLE_BUFFER_SEEK_SET);
   while( (c = mulle_buffer_next_character( &buffer)) != -1)
      putchar( c);

   mulle_buffer_done( &buffer);
   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` library dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-allocator` — memory allocation / freeing (`mulle_allocator`, the no-fail allocator contract, `mulle_allocator_malloc`, `mulle_allocator_free`, `mulle_default_allocator`, `mulle_stdlib_allocator`).
- `mulle-data` — the `struct mulle_data` type used by `mulle_buffer_extract_data` / `mulle__buffer_get_data` / `mulle_buffer_get_data`.

(`mulle-c11` helpers such as `MULLE_C_DEPRECATED`, `MULLE_C_CONFINED_LOOP`, `MULLE_C_NONNULL_FIRST` are available transitively.)

To embed `mulle-buffer` in your own project, include the umbrella header:

```c
#include <mulle-buffer/mulle-buffer.h>
```