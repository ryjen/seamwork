# QART: ownership at a byte parsing boundary

## Question

For an untrusted binary frame, what interface makes ownership, lifetime, bounds, and failure easiest to reason about?

## Historical evidence

Archived `libcoda-net` included APIs such as:

```cpp
buffered_socket& write(void* buffer, size_t size);
secure_layer::send(const void* data, size_t size);
secure_layer::read(void* data, size_t size);
```

and exposed internal input/output buffers by reference.

Those interfaces are normal native/API shapes, but they leave more relationships to convention:

- whether null is meaningful;
- whether pointer and length agree;
- whether memory is borrowed;
- how long returned/referenced storage remains valid;
- where size limits are enforced.

The retained network threat model also calls for pure parser boundaries and bounded handling of malformed/truncated/oversized peer data.

## Alternatives

### A. Raw pointer + size

```cpp
parse(const std::byte* data, std::size_t size)
```

**Advantages**

- interoperates naturally with C/native APIs;
- minimal representation.

**Costs**

- pointer and size are independent arguments;
- null + nonzero length is an additional invalid state;
- borrowing exists only by convention;
- pointer arithmetic is easy to spread through parser code.

The study's `parse_raw` adapter needs an explicit `null_input` failure before it can safely construct the selected view.

### B. Owning input/result

An owning `std::vector<std::byte>` makes lifetime unambiguous.

**Advantages**

- easy retention;
- result can outlive source independently.

**Costs**

- accepting/returning ownership can force or hide copies when callers only need to inspect a frame;
- ownership transfer is stronger semantics than a parser requires.

For this parse-now consumer, ownership is unnecessary by default.

### C. Borrowed `std::span<const std::byte>`

Selected.

```cpp
parse(std::span<const std::byte> input)
    -> expected<frame_view, parse_error>
```

The input contract is one range rather than two independently meaningful arguments. Constness prevents parser mutation. Subspans keep bounds calculations local.

The result's `frame_view` name and payload span explicitly communicate borrowing.

**Trade-off:** the type system does not prove that the source outlives the returned view. The caller still owns that responsibility. The API therefore documents the view lifetime and demonstrates explicit copying for retention.

### D. Return offsets instead of a span

A parser could return payload offset + length.

That avoids returning a borrowing object but pushes slicing back onto every caller and recreates paired integer invariants. It is useful when the input buffer may move/reallocate between parsing and use; that is not the current consumer.

## Size-limit policy

The study accepts `std::size_t max_payload` explicitly.

A strong `frame_limit` type was considered but adds little while there is only one obvious unit: bytes. A compile-time policy would unnecessarily couple the parser to one deployment limit.

The value parameter keeps the resource bound visible at the trust boundary.

## Error policy

`std::expected<frame_view, parse_error>` is appropriate because malformed peer data is ordinary recoverable input, not an exceptional program failure.

Errors distinguish:

- truncated header;
- over-limit declared payload;
- truncated payload.

The raw-pointer baseline also needs `null_input`.

Downstream use of a successfully parsed frame has no error channel added merely for uniformity.

## Bounds reasoning

For a 16-bit payload length:

1. require at least two header bytes;
2. decode the length from those valid bytes;
3. reject values above the caller's explicit limit;
4. calculate available payload as `input.size() - header_size` only after step 1;
5. require declared length <= available payload;
6. create a bounded subspan.

The parser never needs unchecked pointer arithmetic and the fixed-width length cannot overflow `header_size + payload_length` on supported `size_t` targets.

## Recommendation

Use a borrowed `std::span<const std::byte>` for pure parse-and-use boundaries.

Copy only when retention is a real caller requirement.

Do not generalize this result into a networking or protocol framework. The useful lesson is narrower:

> make borrowing the default when ownership is not required, and make the copy visible when lifetime must extend.
