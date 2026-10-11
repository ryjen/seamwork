# Native size-boundary study

Issue #63 extracts one narrow robustness problem from archived `libcoda-net`: C++ `std::size_t` buffer lengths crossing a native API whose count type is `int`.

No OpenSSL or socket code is imported.

## Selected design

The C++ boundary receives a borrowed byte range:

```cpp
write_once(std::span<const std::byte>, native_sink)
    -> std::expected<std::size_t, write_error>
```

Before calling the native API, it validates:

```cpp
std::in_range<int>(bytes.size())
```

Only a representable count is narrowed.

## Zero length

A zero-length write returns `0` without entering the native API.

That avoids depending on native-library-specific behavior for null/empty pointers when there is no work to perform.

## Partial writes

A positive native return smaller than the requested size is returned as a successful count.

The adapter does **not** retry automatically. Retry/chunking policy depends on the actual transport/TLS contract and belongs at a higher boundary.

## Oversized buffers

The conversion itself is testable without allocating an `INT_MAX + 1` byte buffer:

```cpp
checked_native_count(static_cast<size_t>(INT_MAX) + 1)
    -> unexpected(size_error)
```

The native call occurs only after that validation.

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_native_size_boundary
./build/dev/studies/native-size-boundary/seamwork_native_size_boundary_example
```

## Promotion status

**Study only.**

This does not create a TLS/socket wrapper or a generic numeric-conversion component.
