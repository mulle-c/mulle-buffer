### 5.2.1



**BREAKING** ``mulle_buffer`` and ``mulle_flushablebuffer`` now embed their base buffer as a real nested `base` member instead of a layout-compatible prefix. Inherited fields move accordingly `(`buffer->_curr`` becomes ``buffer->base._curr`,` and for flushable buffers ``buffer->base.base._curr`),` and the public ``MULLE_BUFFER_BASE`` / ``MULLE_FLUSHABLEBUFFER_BASE`` macros no longer expand to a field list. Code that accessed inherited fields directly or extended these structs via those macros must be updated.

* fix a -O2 miscompilation where the `(struct `mulle__buffer` *) buffer` prefix cast let the optimizer break aliasing assumptions, causing `buffer/intersect.c` and wrappers such as ``mulle_buffer_intersects_bytes`` to compute wrongly
* accessors now hand out `&buffer->base` instead of type-punning, making the layout well-defined under strict aliasing
* add regression coverage for the nested-member layout and for the previously never-compiled ``mulle_buffer_do_allocator`,` ``mulle_buffer_do_flexible_filled`` and ``mulle_buffer_do_inflexible_filled`` macros

## 5.2.0






feature: make `extract_string` always return a valid C string

* ``mulle_buffer_extract_string`` now returns an allocated `""` for an empty buffer instead of `NULL`; only a `NULL` buffer argument yields `NULL` (behavior change for callers that checked for `NULL`)
* add single-evaluation ``_mulle_flushablebuffer_static_data`` and ``_mulle_flushablebuffer_allocated_data`` helpers that wrap the existing macros with side-effect-safe arguments
* updated extract tests for the new empty-string semantics




* added missing BSD license headers to generic include files
* fixed incorrect filename in mulle-flushablebuffer.h header comment
* restructured API documentation assets


### 5.1.1

Various small improvements
