# QART: checked narrowing at a native byte boundary

## Question

How should a C++23 byte-oriented API cross into a native function that accepts an `int` byte count?

## Historical evidence

Archived `libcoda-net::secure_layer` exposes `size_t` lengths and forwards them into native TLS calls.

The retained threat model explicitly requires validation before narrowing and consistent native error translation.

## Alternatives

### A. Direct `static_cast<int>`

```cpp
native.write(bytes.data(), static_cast<int>(bytes.size()));
```

Rejected.

When the source value is not representable as `int`, the conversion does not express the intended safety contract. The native function can receive a value unrelated to the caller's requested byte count.

### B. Checked narrowing with `std::in_range<int>`

Selected.

```cpp
if (!std::in_range<int>(size)) {
    return unexpected(size_error{...});
}
auto count = static_cast<int>(size);
```

The cast remains present, but only after a standard-library predicate proves representability.

The study keeps `checked_native_count` local to this study. One boundary is not evidence for a generic numeric-conversion library.

### C. Automatically chunk oversized input

Rejected at this low-level boundary.

Chunking changes one logical call into multiple native calls and immediately creates additional policy:

- partial-write handling;
- retry timing;
- ordering after failures;
- library-specific WANT_READ/WANT_WRITE behavior;
- TLS record/API semantics.

A higher-level transport operation can add chunking when its real dependency contract justifies it.

### D. Prefer a wider native API

If the actual dependency/version offers a `size_t`-accepting API with equivalent semantics, prefer that interface.

This study models the narrower contract because supported native APIs are not universally identical.

## Zero-length behavior

The selected adapter completes an empty write as zero without calling the native sink.

This keeps empty-range semantics within the C++ boundary and avoids relying on native behavior for an operation with no payload.

## Native result translation

A negative native return becomes typed failure.

A non-negative return remains a byte count, including a partial positive write.

The wrapper deliberately does not convert "partial" into an error or automatically retry: both choices require higher-level protocol knowledge.

## Recommendation

Validate before narrowing, preserve partial counts, and keep retry/chunking policy out of the low-level adapter.

The broader lesson is:

> an explicit modern C++ size type does not remove the need to model the concrete integer contract at a native boundary.
