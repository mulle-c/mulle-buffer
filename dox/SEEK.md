# Seek semantics

This document explains the relationship between POSIX `fseek`/`lseek`, GNU
`fmemopen`, and `mulle_buffer_set_seek`.

## The short version

For POSIX-style `SEEK_END`, the offset is signed relative to the logical end
of the stream:

| Call | Meaning |
|---|---|
| `seek( 0, SEEK_END)` | Seek exactly to the end. |
| `seek( -N, SEEK_END)` | Seek `N` bytes before the end. |
| `seek( +N, SEEK_END)` | Seek `N` bytes after the end, when the stream permits it. |

Therefore, an offset from `SEEK_END` is **not required to be negative**. It is
negative when moving backwards from the end. Zero means the end, and positive
values move forwards from the end.

The old `mulle_buffer_set_seek` convention was different: a positive offset
meant “that many bytes backwards from the allocation end.” That was a legacy
mulle-buffer convention, not POSIX behavior.

## POSIX and `fmemopen` examples

A regular seekable file uses its logical file length as the `SEEK_END` base.
For example, if a file contains `VfL Bochum 1848\n`, its length is 16 bytes:

```c
fseek( fp, 0,  SEEK_END);  // position 16
fseek( fp, -6, SEEK_END);  // position 10, points at " 1848\n"
fseek( fp, 1,  SEEK_END);  // position 17, past EOF
```

Reading from position 10 prints:

```text
 1848
```

The GNU/Linux `fmemopen` probe in `/tmp/mulle-buffer-seek-probe` uses the
15-byte string `VfL Bochum 1848` in a 32-byte backing array and reports:

```text
fmemopen backing capacity: 32
fmemopen logical length: 15
fseek(0, 2) -> 15
fseek(-6, 2) -> 9
from six before end: "m 1848"
fseek(1, 2) -> 16
from one past logical end: ""
```

This demonstrates that `fmemopen` uses the logical stream length (15), not the
backing capacity (32), as the `SEEK_END` base. The positive seek succeeds here
because the resulting position remains within the backing allocation.

The companion regular-file probe is:

```text
/tmp/mulle-buffer-seek-probe/file-seek
```

Run both examples with:

```sh
cd /tmp/mulle-buffer-seek-probe
make run
```

## Current `mulle_buffer` behavior

The public functions are:

```c
mulle_buffer_set_seek( buffer, offset, mode); // returns 0 or -1
mulle_buffer_lseek( buffer, offset, mode);    // returns position or -1
```

`mulle_buffer_set_seek` delegates to `mulle_buffer_lseek`. Both use the
`MULLE_BUFFER_SEEK_SET`, `MULLE_BUFFER_SEEK_CUR`, and
`MULLE_BUFFER_SEEK_END` modes.

The current implementation in `src/mulle--buffer.c` has these rules:

- `SEEK_SET` is relative to the beginning of the allocation.
- `SEEK_CUR` is relative to the current cursor.
- `SEEK_END` is relative to the **allocation capacity**, not the logical
  written length.
- `SEEK_END` accepts zero and negative offsets.
- `SEEK_END` rejects positive offsets.
- All resulting positions must remain within the allocation.
- Seeking a flushable buffer fails.

Consequently, on a buffer with capacity 32:

```c
mulle_buffer_set_seek( buffer,  0, MULLE_BUFFER_SEEK_END); // position 32
mulle_buffer_set_seek( buffer, -1, MULLE_BUFFER_SEEK_END); // position 31
mulle_buffer_set_seek( buffer,  1, MULLE_BUFFER_SEEK_END); // failure
```

This is only partially POSIX-like. The sign convention now matches POSIX for
backward seeks, but the end base and the handling of positive offsets do not.
The capacity-based behavior is connected to `mulle_buffer`'s single-cursor,
write-first design: it does not retain a separate written-length cursor after
seeks and rewinds.

## Historical compatibility change

Before the `SEEK_END` implementation was changed, the code effectively did:

```c
plan = &buffer->_sentinel[ -seek];
```

Thus the old behavior was:

```c
mulle_buffer_set_seek( buffer, 0, MULLE_BUFFER_SEEK_END); // capacity end
mulle_buffer_set_seek( buffer, 1, MULLE_BUFFER_SEEK_END); // capacity - 1
mulle_buffer_set_seek( buffer, -1, MULLE_BUFFER_SEEK_END); // failure
```

The implementation was changed when `mulle_buffer_lseek` was introduced in
commit `9e798ad`. The test in `test/buffer/seek.c` consequently changed its
`SEEK_END` example from `1` to `-1`.

## Migrating callers

For a caller that used the old convention to seek backwards from the
allocation end, translate the offset as follows:

```c
// Old convention: N bytes before the allocation end
mulle_buffer_set_seek( buffer, N, MULLE_BUFFER_SEEK_END);

// New sign convention
mulle_buffer_set_seek( buffer, -N, MULLE_BUFFER_SEEK_END);
```

Zero remains zero. `SEEK_SET` and `SEEK_CUR` offsets do not need sign
translation.

Take care with unsigned values. Do not negate an unsigned value and expect a
negative offset:

```c
mulle_buffer_lseek( buffer, -(off_t) distance, MULLE_BUFFER_SEEK_END);
```

If the caller actually means “the end of the written archive” rather than “the
end of the allocation,” sign translation alone is not enough. That caller must
use an explicitly tracked logical length, or the buffer API must first gain a
separate written-length/seek model.

### Caller inventory

The local `mulle-buffer` repository has no production caller of
`mulle_buffer_set_seek`; its callers are tests and fuzz targets. The historical
`test/buffer/seek.c` call used `+1` with `SEEK_END`, and the current test uses
`-1` after the sign change. Its `SEEK_SET` and `SEEK_CUR` calls are unrelated
to this migration.

The relevant downstream caller is in `MulleObjCArchiverFoundation`:

```c
// MulleObjCUnarchiver.m, _startDecode
mulle_buffer_set_seek( &_buffer,
                       sizeof( long long) * 5 + 8,
                       MULLE_BUFFER_SEEK_END);
```

This is the footer lookup. The expression is 48 bytes: five 64-bit table
offsets plus the eight-byte `"**off**"` marker. The buffer is initialized over
the complete archive data, so its allocation capacity is the archive length.
The old positive offset intentionally means “48 bytes before the allocation
end”; it does not mean “48 bytes past the end.” Under the new sign convention,
this one call must become:

```c
mulle_buffer_set_seek( &_buffer,
                       -(long) (sizeof( long long) * 5 + 8),
                       MULLE_BUFFER_SEEK_END);
```

The canonical source is normally
`src/MulleObjCArchiverFoundation/MulleObjCUnarchiver.m`; an amalgamated checkout
may expose it under a path such as
`srcO/MulleFoundation/MulleObjCArchiverFoundation`. Update the canonical source
and regenerate the amalgamated source rather than editing only the generated
copy.

The other archiver calls found in `MulleObjCUnarchiver.m` and
`NSKeyedUnarchiver.m` use positive absolute offsets with `SEEK_SET` or positive
forward offsets with `SEEK_CUR`. They must **not** be negated. Do not make
`mulle_buffer_set_seek` accept both `SEEK_END` signs; that would make the public
API ambiguous and hide stale callers.

If more downstream code is available, the safe audit rule is: only negate a
positive offset when the mode is exactly `MULLE_BUFFER_SEEK_END` and the old
intent was “N bytes before the end.” Never perform a blanket sign replacement
on all `mulle_buffer_set_seek` arguments.

The current checkout does not contain the MulleFoundation source tree, so this
repository cannot apply that downstream call-site edit directly.

## Open design question

There are two possible future contracts:

1. **Keep the current mulle-buffer contract:** `SEEK_END` means allocation
   capacity, positive offsets fail, and the single cursor remains the source of
   `get_length()`.
2. **Implement true file-like behavior:** `SEEK_END` means logical written
   length, positive offsets are allowed according to the storage policy, and a
   separate written length is retained independently from the cursor.

Changing from the first contract to the second is substantially larger than
changing the sign of a `SEEK_END` offset. It affects reads after rewinds,
`get_length()`, writes after seeks, flexible-buffer growth, and archive/parser
callers. Until that design decision is made, callers should use the current
capacity-based contract explicitly and should use negative offsets for backward
movement from `SEEK_END`.
