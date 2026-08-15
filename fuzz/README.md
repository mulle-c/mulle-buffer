# Fuzzing mulle-buffer

This directory contains fuzzing targets for the `mulle-buffer` library using
`libFuzzer`.

## Prerequisites

- `clang` with `libFuzzer` support (usually included in modern clang/LLVM).
- `mulle-sde` (to provide dependency paths and the environment).

## Usage

Use the `mulle-buffer-fuzz` script to build and run fuzzers.

### Build all targets

```bash
./fuzz/mulle-buffer-fuzz build
```

### Run a target

```bash
./fuzz/mulle-buffer-fuzz run -t 60 fuzz-add-extract
```

If no target is specified, all targets are run sequentially for the default
time (60s).

### Replay a crash

If a crash is found, the input file will be saved in
`fuzz/crashes/<target>/`. You can replay it with:

```bash
./fuzz/mulle-buffer-fuzz replay <target> <input_file>
```

### Minimize corpus

```bash
./fuzz/mulle-buffer-fuzz minimize <target>
```

## Targets

- `fuzz-add-extract`: Add random bytes, verify get_byte/get_bytes/get_last_byte,
  extract and verify extracted data matches.
- `fuzz-flushable`: Push random bytes through a flushable buffer, verify total
  flushed matches input size.
- `fuzz-modify`: Random mutation operations on a single buffer instance: add,
  remove, seek, set_length, memset, pop_byte. Asserts no overflow.
- `fuzz-inflexible`: Push bytes into flexible buffer backed by stack storage,
  verify contents up to overflow.
- `fuzz-seek`: Add data, randomly seek to all modes, verify positions and
  boundary failures.
- `fuzz-string`: Build a C string from fuzz data, verify get_string and
  extracted string matches input.

## CI

The `mulle-buffer-fuzz` script is intended to be run in a GitHub Actions
job similar to:

```yaml
run: fuzz/mulle-buffer-fuzz run -t 120
```