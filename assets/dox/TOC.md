# mulle-buffer Library Documentation for AI

## 1. Introduction & Purpose

**mulle-buffer** provides a growable, resizable byte buffer for building arbitrary-length binary or text data dynamically or in static storage. It solves the problem of not knowing buffer size in advance, offering:

- **Dynamic growth**: Automatically expands from stack to heap as needed (starts with stack allocation)
- **Flexible and inflexible modes**: Fixed-size or growable buffers
- **Stream-like interface**: Seek position, advance pointer, remaining length tracking
- **Convenience operations**: String concatenation, hexdumps, C-string escaping, quoted output
- **Macro-based lifecycle management**: `mulle_buffer_do` macro ensures proper initialization/cleanup

This library is a foundational component of mulle-core and is used by NSMutableData in the Objective-C runtime, as well as other data structure libraries that need dynamic buffering.

## 2. Key Concepts & Design Philosophy

- **Flex point**: Buffers start on the stack (via `mulle_buffer_do` macro) with default capacity of 96 bytes; they automatically grow to the heap when exceeded
- **Dual modes**: 
  - **Flexible**: Can grow dynamically (most common)
  - **Inflexible**: Fixed-size, won't grow (for static or preallocated storage)
- **Allocator abstraction**: Uses `mulle_allocator` for all memory management, supporting custom allocators
- **Stream-oriented**: Maintains current seek position (_curr), allowing sequential writes and seeking (like stdio)
- **Embedded structure**: Can be embedded on stack or allocated on heap; struct size is approximately 2x the size of a pointer on 64-bit systems
- **Byte-stream model**: Linear sequence of bytes accessed by position; no type information beyond raw bytes

## 3. Core API & Data Structures

### 3.1. `mulle--buffer.h` (Internal Base)

#### `struct mulle__buffer` (Internal Base Structure)
- **Purpose**: Core buffer implementation (underscore prefix indicates internal, but useful to understand)
- **Key Fields**:
  - `unsigned char *_storage`: Pointer to the allocated memory block
  - `unsigned char *_curr`: Current position for reading/writing (like file pointer)
  - `unsigned char *_sentinel`: One past the last valid byte (end marker)
  - `unsigned char *_initial_storage`: For inflexible buffers, the original storage pointer
  - `size_t _size`: Total capacity in bytes (set to -1 for non-growing)
  - `unsigned int _type`: Type flags (FLEXIBLE, INFLEXIBLE, FLUSHABLE, READONLY, WRITEONLY, etc.)

### 3.2. `mulle-buffer.h`

#### `struct mulle_buffer`
- **Purpose**: User-facing buffer struct; extends `mulle__buffer` by adding allocator pointer
- **Key Fields**:
  - All base fields from `mulle__buffer`
  - `struct mulle_allocator *_allocator`: Allocator for dynamic growth

#### Lifecycle Functions

**Creation/Destruction:**
- `mulle_buffer_create(allocator)`: Allocate and initialize on heap with default capacity
- `mulle_buffer_alloc(allocator)`: Allocate structure only (inline)
- `mulle_buffer_destroy(buffer)`: Deallocate buffer and its storage
- `mulle_buffer_done(buffer)`: Finalize buffer (for stack-allocated buffers)

**Initialization (for stack allocation):**
- `mulle_buffer_init(buffer, allocator)`: Initialize stack-allocated buffer with default capacity
- `mulle_buffer_init_with_capacity(buffer, allocator, capacity)`: Initialize with specific capacity
- `mulle_buffer_init_inflexible_with_static_bytes(buffer, bytes, length)`: Initialize as read-only view of existing memory
- `mulle_buffer_init_with_const_bytes(buffer, bytes, length)`: Initialize from const data
- `mulle_buffer_do { ... }` macro: Stack-allocated buffer with automatic cleanup (most common pattern)

**Buffer Mode Management:**
- `mulle_buffer_make_inflexible(buffer, size)`: Convert flexible buffer to fixed-size
- `mulle_buffer_set_readonly(buffer)`: Prevent write operations
- `mulle_buffer_set_writeonly(buffer)`: Prevent read operations

#### Growth & Capacity Management
- `mulle_buffer_grow(buffer, minimum_length)`: Force buffer to grow to at least `minimum_length` bytes
- `mulle_buffer_size_to_fit(buffer)`: Trim capacity to exact current length (shrink allocation)
- `mulle_buffer_set_length(buffer, length)`: Change logical length (can truncate or extend)
- `mulle_buffer_guarantee(buffer, length)`: Ensure at least `length` bytes available after current position
- `mulle_buffer_get_capacity(buffer)`: Current total capacity
- `mulle_buffer_get_length(buffer)`: Current logical length (bytes used)
- `mulle_buffer_remaining_length(buffer)`: Bytes available from current position to end
- `mulle_buffer_is_full(buffer)`: Check if buffer at capacity
- `mulle_buffer_is_empty(buffer)`: Check if length is 0
- `mulle_buffer_is_big_enough(buffer, length)`: Check if remaining space >= length
- `mulle_buffer_has_overflown(buffer)`: Check if inflexible buffer exceeded

#### Seeking & Positioning
- `mulle_buffer_set_seek(buffer, offset, mode)`: Set position (like fseek; SEEK_SET, SEEK_CUR, SEEK_END)
- `mulle_buffer_get_seek(buffer)`: Get current position
- `mulle_buffer_advance(buffer, delta)`: Move position forward by delta bytes

#### Data Access
- `mulle_buffer_get_bytes(buffer)`: Get pointer to buffer storage (not null-terminated)
- `mulle_buffer_get_string(buffer)`: Get pointer to buffer as string (requires null termination)
- `mulle_buffer_extract_bytes(buffer)`: Extract and steal ownership of bytes (clears buffer)
- `mulle_buffer_extract_string(buffer)`: Extract as null-terminated C string (steals ownership)
- `mulle_buffer_extract_data(buffer)`: Extract as `mulle_data` struct (steals ownership)

#### Writing Operations

**Single bytes/characters:**
- `mulle_buffer_add_byte(buffer, byte)`: Add one byte
- `mulle_buffer_add_char(buffer, c)`: Add one char
- `mulle_buffer_remove_last_byte(buffer)`: Remove last byte

**Bulk data:**
- `mulle_buffer_add_bytes(buffer, bytes, length)`: Add raw bytes
- `mulle_buffer_add_string(buffer, s)`: Add null-terminated string (minus null terminator)
- `mulle_buffer_append_string(buffer, s)`: Alias for add_string
- `mulle_buffer_add_buffer(buffer, other)`: Append contents of another buffer

**C-string escaping:**
- `mulle_buffer_add_c_char(buffer, c)`: Add character with C escape sequences (e.g., '\n' -> "\\n")
- `mulle_buffer_add_c_chars(buffer, s, length)`: Add multiple chars with escaping
- `mulle_buffer_add_c_string(buffer, s)`: Add escaped C string
- `mulle_buffer_add_c_chars_callback(buffer, s, length)`: Callback version for fprintf integration

**Conditional operations:**
- `mulle_buffer_add_string_if_empty(buffer, s)`: Add only if buffer currently empty
- `mulle_buffer_add_string_if_not_empty(buffer, s)`: Add only if buffer not empty

**String operations:**
- `mulle_buffer_strcpy(buffer, s)`: Copy string (replacing buffer contents)
- `mulle_buffer_strcat(buffer, s)`: Concatenate string (alias for add_string)

**Bulk fill:**
- `mulle_buffer_memset(buffer, value, count)`: Fill buffer with repeated byte

#### Text Processing
- `mulle_buffer_add_c_char(buffer, c)`: Add char with C escape sequences
- `mulle_buffer_make_string(buffer)`: Add null terminator (doesn't increment length)
- `mulle_buffer_zero_last_byte(buffer)`: Explicitly add null terminator

#### Clearing/Resetting
- `mulle_buffer_remove_all(buffer)`: Clear buffer (length = 0, position = 0)
- `mulle_buffer_remove_in_range(buffer, range)`: Remove specific range
- `mulle_buffer_reset(buffer)`: Full reset to initial state

#### Inspection/Debugging
- `mulle_buffer_is_inflexible(buffer)`: Check if fixed-size
- `mulle_buffer_is_flushable(buffer)`: Check if buffer is flushable type
- `mulle_buffer_is_readonly(buffer)`: Check read-only flag
- `mulle_buffer_is_writeonly(buffer)`: Check write-only flag
- `mulle_buffer_is_void(buffer)`: Check if uninitialized (invalid)
- `mulle_buffer_intersects_bytes(buffer, bytes, length)`: Check if range overlaps buffer
- `mulle_buffer_copy_range(buffer, range, dst)`: Copy subrange to destination
- `mulle_buffer_get_staticlength(buffer)`: Get initial/static length

#### Byte Operations
- `mulle_buffer_pop_byte(buffer)`: Remove and return last byte (or -1 if empty)
- `_mulle_buffer_pop_byte(buffer)`: Unsafe version (doesn't check bounds)

#### Macros
- `mulle_buffer_do(var_name) { ... }`: Create stack-allocated buffer with automatic cleanup (default capacity 96 bytes)
- `mulle_buffer_do_flexible(var_name, capacity) { ... }`: Stack buffer with specified capacity
- `mulle_buffer_do_inflexible(var_name, bytes, length) { ... }`: Stack buffer from existing memory (read-only view)
- `mulle_buffer_do_string(var_name, string) { ... }`: Stack buffer from string
- `MULLE_BUFFER_DEFAULT_CAPACITY`: 96 bytes (2x pointer size on 64-bit)

### 3.3. `mulle-flushablebuffer.h`

#### `struct mulle_flushablebuffer`
- **Purpose**: Buffer with flush callback for writing output (like buffered stdio)
- Extends `mulle_buffer` with a flush function pointer
- Useful for generating output that needs to be written in chunks (files, network, logging)

#### Key Functions
- `mulle_flushablebuffer_create(allocator, flusher, flusher_context)`: Create with callback
- `mulle_flushablebuffer_done(buffer)`: Cleanup (flushes any remaining data)
- `mulle_flushablebuffer_flush(buffer)`: Manually flush buffered data
- Uses `mulle_buffer_add_bytes_callback` and `mulle_buffer_add_c_chars_callback` for integration

## 4. Performance Characteristics

- **Memory model**: 
  - Default 96 bytes on stack (MULLE_BUFFER_DEFAULT_CAPACITY)
  - Growth: Typically uses doubling strategy or allocator-determined strategy
  - No separate allocation overhead for small data

- **Operation complexity**:
  - Add/write operations: O(1) amortized for append (due to growth strategy)
  - Seek: O(1)
  - Copy: O(n) for n bytes
  - Extract: O(1) (transfers ownership)
  - Grow: O(n) for n bytes (involves reallocation and copy)

- **Inflexible buffers**: 
  - No growth possible; writes beyond capacity set overflow flag
  - Useful for fixed-size buffers or read-only data views

- **Thread-safety**: None; external locking required if shared across threads

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Use `mulle_buffer_do` macro** for most cases: Provides stack allocation with automatic cleanup
   ```c
   mulle_buffer_do(buffer) {
       mulle_buffer_add_string(buffer, "hello");
       // automatic cleanup
   }
   ```

2. **Choose initialization based on use case**:
   - Dynamic: Use `mulle_buffer_do` (default flex mode)
   - Fixed size from start: Use `mulle_buffer_do_inflexible`
   - Large preallocated: Use `mulle_buffer_do_flexible(buf, capacity)`

3. **Prefer string functions over raw bytes**: `mulle_buffer_add_string` is safer than `mulle_buffer_add_bytes` with strlen

4. **Use extract functions for ownership transfer**: `mulle_buffer_extract_string` and `mulle_buffer_extract_data` transfer ownership (caller must free)

5. **Make strings with `mulle_buffer_make_string`**: Adds null terminator and prepares for `mulle_buffer_get_string`

### Common Pitfalls

1. **Forgetting null termination**: `mulle_buffer_get_bytes` does NOT null-terminate; use `mulle_buffer_make_string` first if needed

2. **Extracted memory requires freeing**: After `mulle_buffer_extract_bytes`, caller owns the memory

3. **Position tracking**: `mulle_buffer_add_*` writes at `_curr` position; use `mulle_buffer_get_seek` if you need to know where you are

4. **Readonly/Writeonly flags**: Once set, operations in the other direction will assert; use carefully

5. **Inflexible buffer overflow**: Writes beyond capacity in inflexible mode set `MULLE_BUFFER_HAS_OVERFLOWN` flag but don't necessarily fail; check with `mulle_buffer_has_overflown`

6. **Allocator lifecycle**: If custom allocator is used, it must outlive the buffer or be carefully managed

## 6. Integration Examples

### Example 1: Simple string building with mulle_buffer_do

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int main() {
    mulle_buffer_do(buffer) {
        mulle_buffer_add_string(buffer, "Hello");
        mulle_buffer_add_string(buffer, ", ");
        mulle_buffer_add_string(buffer, "World!");
        mulle_buffer_make_string(buffer);
        
        printf("%s\n", (char *)mulle_buffer_get_bytes(buffer));
    }
    return 0;
}
```

### Example 2: Building binary data with growth

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int main() {
    struct mulle_buffer *buffer = mulle_buffer_create(NULL);
    
    for (int i = 0; i < 10000; i++) {
        mulle_buffer_add_byte(buffer, 'A' + (i % 26));
    }
    
    printf("Buffer size: %zu bytes\n", mulle_buffer_get_length(buffer));
    
    mulle_buffer_destroy(buffer);
    return 0;
}
```

### Example 3: Fixed-size buffer from existing memory

```c
#include <mulle-buffer/mulle-buffer.h>
#include <string.h>
#include <stdio.h>

int main() {
    char static_data[20] = "hello";
    struct mulle_buffer buffer;
    
    // Create read-only view of existing data
    mulle_buffer_init_inflexible_with_static_bytes(&buffer, static_data, 20);
    
    printf("Length: %zu\n", mulle_buffer_get_length(&buffer));
    printf("Data: %s\n", (char *)mulle_buffer_get_bytes(&buffer));
    
    mulle_buffer_done(&buffer);
    return 0;
}
```

### Example 4: Extract ownership and process

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    mulle_buffer_do(buffer) {
        mulle_buffer_add_string(buffer, "extracted data");
        mulle_buffer_make_string(buffer);
        
        // Transfer ownership to caller
        char *result = (char *)mulle_buffer_extract_string(buffer);
        
        printf("Result: %s\n", result);
        
        // Caller must free
        free(result);
    }
    return 0;
}
```

### Example 5: C-string escaping

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int main() {
    mulle_buffer_do(buffer) {
        mulle_buffer_add_c_string(buffer, "hello\nworld\t!");
        mulle_buffer_make_string(buffer);
        
        printf("%s\n", (char *)mulle_buffer_get_bytes(buffer));
        // Output: hello\nworld\t!
    }
    return 0;
}
```

### Example 6: Seeking and conditional writes

```c
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int main() {
    mulle_buffer_do(buffer) {
        mulle_buffer_add_string_if_empty(buffer, "first");
        mulle_buffer_add_string_if_not_empty(buffer, " ");
        mulle_buffer_add_string_if_not_empty(buffer, "second");
        
        printf("Position: %ld\n", mulle_buffer_get_seek(buffer));
        
        mulle_buffer_set_seek(buffer, 0, SEEK_SET);
        printf("After seek: %ld\n", mulle_buffer_get_seek(buffer));
        
        mulle_buffer_make_string(buffer);
        printf("Result: %s\n", (char *)mulle_buffer_get_bytes(buffer));
    }
    return 0;
}
```

## 7. Dependencies

- `mulle-allocator`: For memory allocation and freeing
- `mulle-data`: For `mulle_data` struct in extract operations
- `mulle-c11`: Cross-platform C compiler glue

