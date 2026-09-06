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
