# Borrowed frame parsing study

Issue #54 revisits a narrow ownership/trust seam from archived `libcoda-net` without rebuilding the networking library.

The historical code crossed several byte boundaries with raw pointer + length pairs, exposed references to internal buffers, and mixed protocol behavior with live transport. The retained threat model explicitly recommends pure parsers, bounded malformed-input handling, and clearer ownership.

## Practical boundary

The study uses a tiny length-prefixed frame:

```text
+------------+------------------+
| u16 length | payload bytes... |
+------------+------------------+
```

The parser performs no I/O. The caller owns the input bytes.

## Selected interface

The first version accepted `std::span<const std::byte>` directly. That clearly expressed borrowing, but an implicit conversion from a temporary owning container could still create a span whose storage died immediately after the call.

The refined public boundary accepts only a contiguous, sized **borrowed range** of bytes:

```cpp
template <borrowed_byte_range Range>
parse(Range&& input, std::size_t max_payload)
    -> std::expected<frame_view, parse_error>
```

This keeps caller-owned lvalue arrays/vectors ergonomic, accepts spans, and rejects common rvalue-owning calls such as `parse(std::vector<std::byte>{...})`.

`frame_view::payload` still **borrows from the caller's input**. C++ cannot prove arbitrary span lifetime; the constraint removes a common accidental dangling path rather than claiming complete lifetime safety.

The parser checks the header before reading it, validates the declared payload against an explicit maximum, verifies availability before slicing, and returns structured failures.

## Retention

Immediate processing stays zero-copy.

If a caller needs the payload to outlive the source buffer, it opts into ownership explicitly:

```cpp
auto view = parse(input);
auto owned = retain(*view); // copy happens here
```

The study intentionally does not hide that copy behind the parser.

## Baseline comparison

A `parse_raw(const std::byte*, size_t)` adapter is included only as a baseline. It needs an extra null-pointer/length mismatch state that `std::span` removes from the selected parser contract.

See [QART](qart.md).

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_borrowed_frame
./build/dev/studies/borrowed-frame/seamwork_borrowed_frame_example
```

## Promotion status

**Study only.**

This does not introduce a networking, protocol, buffer, or parser framework.
