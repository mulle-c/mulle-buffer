# mulle-buffer design decisions

This document records the current design contracts of `mulle_buffer` that
are easy to misread from the code or the API docs alone. It is a companion
to `API_BUFFER.md` / `API_FLUSHABLEBUFFER.md`.

## The buffer is a write-first builder

`mulle_buffer` is designed as an append-oriented byte builder: writing is
the primary, well-supported operation, and reading is a secondary
convenience. The single-cursor model below is a consequence of that bias,
not a general-purpose file-like read/write abstraction. Full read/write
support (independent written-length and read-position) is intentionally not
part of the design.

## Default capacity

`MULLE_BUFFER_DEFAULT_CAPACITY` is `128` (see `src/mulle-buffer.h`).

A lazily-allocated flexible buffer stores the requested capacity in `_size`,
reports it from `mulle_buffer_get_capacity()`, and uses it as the size of the
first heap allocation. So "capacity" is a single value: reported before and
after the buffer is materialized, and honored by the first allocation.

| Use | Effect |
|-----|--------|
| `mulle_buffer_init_default` | capacity of 128; heap storage allocated lazily, and the first allocation is 128 |
| `mulle_buffer_create_default` | same, through `mulle_buffer_create( NULL)` |
| `mulle_buffer_do` / `mulle_buffer_do_allocator` | size of the stack `alloca` backing |
| `mulle_buffer_do_string` | capacity of `128 / 2` (its `MULLE_BUFFER_DATA` sets `_size` to `MULLE_BUFFER_DEFAULT_CAPACITY / 2`), so first allocation is 64 |

128 is a power of two and large enough to hold a terminal line of text
without hitting the heap.

### The stack backing is rounded up, not exact

`mulle_buffer_do` does not give you a literal 128 bytes. The `alloca` is
an array of whole `struct mulle_buffer`s:

``` c
#define _mulle_buffer_chars_to_struct( len) \
   ((len + sizeof( struct mulle_buffer) - 1) / sizeof( struct mulle_buffer))
```

So the usable stack backing is `chars_to_struct(128) * sizeof(struct)`,
plus one more `struct mulle_buffer` for the storage header on the stack:

| platform | struct size | backing | total stack |
|----------|-------------|---------|-------------|
| 64-bit   | 56          | 168     | 224         |
| 32-bit   | 32          | 128     | 160         |

The portable promise is therefore: **`mulle_buffer_do` provides at least
`MULLE_BUFFER_DEFAULT_CAPACITY` (128) bytes of usable stack backing.**

## Read/write mode checks are debug preconditions

`mulle_buffer` is primarily a write-throughput-oriented byte builder. The
`MULLE_BUFFER_IS_READONLY` and `MULLE_BUFFER_IS_WRITEONLY` flags describe
programmer-selected modes, and the public operations use
`mulle_buffer_assert_readable` and `mulle_buffer_assert_writeable` to check
those modes in debug builds.

These checks are intentionally assertions rather than unconditional runtime
branches in every read or write operation. Adding a mode branch to every put
would penalize the library's primary hot path in release builds. The intended
contract is therefore:

- In a debug build, an attempt to write a read-only buffer or read a write-only
  buffer trips an assertion at the violating call site. This makes mode
  violations visible during development without changing the release fast
  path.
- In a release build, mode correctness is a programmer precondition. The
  caller must not pass a read-only buffer to a write operation or a write-only
  buffer to a read operation. Release builds do not promise to diagnose that
  misuse.
- `mulle_buffer_init_with_const_bytes` creates a read-only view. Its storage
  must never be passed to a write operation.
- This is the same contract style used by the no-fail allocator: the library
  relies on a precondition and uses debug checks to catch violations early;
  it does not add a per-operation recovery branch to the normal path.

This is deliberate and should not be interpreted as missing release-mode
validation. Consumers that require runtime mode validation can provide an
assertion override or build an opt-in checked configuration, but the default
library configuration keeps the write path branch-free.

## Allocator contract

### `_allocator == NULL` means "the buffer does not own its storage"

A `mulle_buffer` over static or `const` storage has a `NULL` allocator.
`mulle_buffer_get_allocator` then returns `NULL`; that is not an error (see
`src/mulle-buffer.h`, `mulle_buffer_get_allocator`).

- `mulle_buffer_done` must not free storage when `_allocator` is `NULL`.
- A *flexible* buffer (one that grows and owns storage) always has a
  non-`NULL` allocator.

### No-allocator is set directly on the field

Every inflexible-over-static initializer assigns `buffer->_allocator = NULL`
directly. `mulle_buffer_set_allocator( buffer, NULL)` must not be used for
this, because it resolves `NULL` to the default allocator.

### Allocation never returns NULL

`mulle_buffer` relies on the `mulle-allocator` failure contract: an
allocation or reallocation never returns `NULL` to the caller. The
`mulle_allocator_realloc`/`malloc`/`calloc` helpers call the allocator's
`fail` handler when the underlying allocation returns `NULL`, and the
default handler `mulle_allocation_fail` is `MULLE_C_NO_RETURN` (it aborts).
The grow path in `_mulle__buffer_grow` therefore does not check the result
of `mulle_allocator_realloc`; it assumes a valid, non-`NULL` block. A
custom allocator whose `realloc` can return `NULL` without calling `fail`
would corrupt the buffer state and must not be used.

## Single-cursor model

`mulle_buffer` has one cursor `_curr` that is used as the write position,
the read/seek position, and the source of `get_length()` (`_curr - _storage`).
Because the buffer is write-first, this is intentional:

- **Seeking rewinds the cursor**, so `get_length()` reflects the cursor
  position, not a separate record of written content. Rewinding then reading
  or appending works on the storage from that point on.
- **Reads are bounded by the allocation** `[curr, sentinel)`, not by a
  remembered written length, because no such length is kept once the cursor
  moves.
- **`SEEK_END` is relative to the allocation capacity**, mirroring POSIX
  `lseek`, not to the end of written data.

Callers that need "read only the N bytes I wrote" must track `N` themselves
or keep the cursor at the write position.

After a rewind, the byte read functions (`_mulle__buffer_next_byte`,
`_mulle__buffer_peek_byte`) scan `[curr, sentinel)` — the full allocation,
including bytes that were never written. A parser that rewinds to zero and
then reads will therefore consume uninitialized capacity, and how many bytes
it sees changes with the allocator's chosen capacity. This is deliberate: the
shared cursor cannot distinguish "written" from "merely allocated", so a
parser must stop at a length it tracks itself. A separate written-length/read
cursor is intentionally not provided (see the write-first model above).

## Terminology

All API documentation uses these terms with one precise meaning, tied to the
single-cursor model above:

- **append** — adding bytes at the cursor. Every write (`add_byte`,
  `add_bytes`, `add_string`, `memset`, ...) is an append: it copies at
  `_curr` and advances `_curr`. The buffer has no random-access write API;
  there is no way to write in the middle without first seeking.
- **end** — the position where the next append will land, i.e. the cursor
  `_curr`. "The end of the buffer" always means this position, never a byte
  value or the allocation end.
- **length** — the value of `mulle_buffer_get_length()`, which is
  `_curr - _storage`. It follows the cursor, so it is a *logical* length:
  after a seek it reflects the cursor position, not a count of bytes written
  (there is no written-bytes counter).
- **capacity** — the allocation size, `_sentinel - _storage`. For a flexible
  buffer it is the current backing size (grows on demand); for an inflexible
  buffer it is the fixed backing size; for a flushable buffer it is the
  per-flush chunk size. `length` is always `<= capacity`.
- **seek** — moving the cursor within `[_storage, _sentinel]`. Offsets are
  byte counts relative to the start (`MULLE_BUFFER_SEEK_SET`), the cursor
  (`MULLE_BUFFER_SEEK_CUR`), or the capacity end (`MULLE_BUFFER_SEEK_END`).
  `SEEK_END` is therefore relative to *capacity*, not written content.

In short: writes and reads happen at the cursor, `length` is where the cursor
is, `capacity` is how far the cursor may go before growth (flexible), failure
(inflexible), or flush (flushable).

## State diagram

A `mulle_buffer` is always in one of the states below. "Capacity" is
`_sentinel - _storage`; "length" is `_curr - _storage`.

```
                init / create / reset / done+init
                │  (storage allocated lazily on first write)
                ▼
             ┌────────┐
   append ──▶│ EMPTY  │  length == 0
             └────────┘
                │ append
                ▼
             ┌────────┐      append ────────────────┐
             │ ACTIVE │  length < capacity           ▼
             └────────┘                          ┌────────┐
                │ append (length == capacity)     │  FULL  │  length == capacity
                ▼                                └────────┘
             growth (flexible)  ─────▶  ACTIVE  (capacity grows)
             or                   ─────▶  OVERFLOWN (inflexible, sticky)
             or flush (flushable) ─────▶  EMPTY       (on success)
                                        or OVERFLOWN (flusher failure)
```

Transition table (cursor = `_curr`):

| Operation                    | State change                                      | Cursor / length effect                |
|------------------------------|---------------------------------------------------|---------------------------------------|
| `init_*` / `create_*`        | any ──▶ EMPTY                                     | cursor = start, length 0              |
| `reset`                      | any (incl. OVERFLOWN) ──▶ EMPTY                   | cursor = start, length 0, clears overflow |
| `add_*`, `memset`            | append                                            | advances cursor, length grows         |
| flexible buffer, cursor == cap | ACTIVE ──▶ ACTIVE (grows capacity first)        | capacity grows, then append as above  |
| inflexible buffer, cursor == cap | ACTIVE ──▶ OVERFLOWN                           | content truncated, overflow sticky    |
| flushable buffer, cursor == cap | ACTIVE ──▶ EMPTY (flush OK) or OVERFLOWN (flush failed) | flushed bytes removed, cursor resets |
| `remove_last_byte`           | ACTIVE ──▶ ACTIVE / EMPTY                         | cursor back 1, length shrinks         |
| `remove_all`                 | ACTIVE / FULL ──▶ EMPTY                           | cursor = start, length 0; **does not clear overflow** |
| `set_length` / `set_seek`    | no state change                                   | moves cursor, changes reported length |
| `extract_data` / `extract_string` / `extract_bytes` | any ──▶ EMPTY (no backing)            | buffer consumed; caller owns result   |
| `done`                       | any ──▶ destroyed                                 | frees storage, object invalid         |

- **OVERFLOWN is sticky**: `remove_all` and appends are no-ops in OVERFLOWN.
  Only `reset` or a full `done` + re-init returns to EMPTY.
- **Reads** (`get_byte`, `next_byte`, `peek_byte`, `get_string`) move the
  cursor too; `get_length` reports the cursor position, so reading changes
  the reported length (see Terminology).

## `seek_byte`

`mulle_buffer_seek_byte` searches forward from the current cursor and
returns `0` and moves the cursor on a match, or `-1` on no match, overflow,
or an invalid buffer.

On a buffer with no storage, or a cursor already at the sentinel, it returns
`-1` without forming an invalid pointer or calling `memchr` on a `NULL`
cursor.

The search region is the capacity `[curr, sentinel)`, not the logical
content (see the single-cursor model above). Seeking on a length-zero buffer
that still has backing searches the stale or uninitialized backing.

## `memcmp`

`mulle_buffer_memcmp` returns the result of `memcmp` verbatim, so only the
*sign* is guaranteed, not the magnitude (glibc's vectorized `memcmp`
returns the loaded-word difference). The doc contract "negative if the
buffer is less" is therefore authoritative; callers and tests must
normalize to the sign rather than expect `-1`.

When the buffer is a strict prefix of the compared bytes, the library
returns exactly `-1` from its own `length > get_length` guard.

## Extraction and aliasing

### `mulle_buffer_extract_data`

`mulle_buffer_extract_data` transfers ownership of the buffer's content to the
caller ("you only do this once"): the caller owns the returned bytes and must
free them. The buffer itself is consumed by the operation; it is not a
promise that the buffer remains reusable afterwards.

- Heap-backed buffer: the heap storage is handed to the caller directly and
  the buffer is left with no storage (all pointers `NULL`).
- Stack/static-backed buffer: the content is *copied* to a new heap
  allocation owned by the caller; the stack backing may be retained in the
  non-empty case, but the buffer must still be considered consumed.
- Empty buffer: the returned `mulle_data` has `bytes == NULL`, `length == 0`,
  and the buffer is left with no storage.

### Self-aliasing

Self-append (source pointing into the buffer's own storage) is **not**
supported by any add operation.

- `mulle_buffer_add_bytes` rejects self-append: growth may reallocate and
  move the storage, or a flush may deliver the bytes to the sink, before
  the copy happens, leaving the source pointer stale. Debug builds trip an
  assertion in `_mulle__buffer_add_bytes`; release builds are not safe and
  callers must avoid passing a slice of the buffer's own storage.
- `mulle_buffer_add_buffer` and `mulle_buffer_add_buffer_range` forward the
  source bytes through `_mulle__buffer_add_bytes`, so `buffer == other` is
  subject to the same restriction.
- The string family (`mulle_buffer_add_string`, `add_string_with_maxlength`,
  `add_c_chars`, `add_c_string`) also does not support self-reference; when
  the source points into the buffer's own storage it marks the buffer
  overflown via `_mulle__buffer_self_referencing` and appends nothing,
  giving a defined runtime result.