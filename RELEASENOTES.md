## 5.1.0







feature: add typed 'chars' add APIs and confine buffer macros for safer usage

* new `mulle_buffer_add_chars()` (and internal `_mulle__buffer_add_chars())` — convenience inline helpers to append char arrays to buffers
* buffer/flushablebuffer macros updated to use `MULLE_C_CONFINED_LOOP,` reducing macro-scope break/loop hazards and improving safety
* `_mulle__buffer_remove_in_range` exported with `MULLE__BUFFER_GLOBAL` and include guard accepts `MULLE__CORE_BUILD` to ensure correct linkage in core builds
