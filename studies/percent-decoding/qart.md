# QART: bounded percent decoding

## Question

How should a percent-decoding boundary handle truncation, invalid hex, byte semantics, and caller policy in modern C++?

## Historical baseline

Archived `libcoda-net::uri::decode` reacts to `%` by looking at the next two positions before establishing that those positions are within the input range.

Its helper also computes a value for arbitrary non-digit characters instead of restricting input to hexadecimal digits.

The retained network threat model requires malformed URI/protocol input to fail in bounded, deterministic ways.

## Alternatives

### A. Unchecked iterator/index lookahead

Historical shape.

Rejected.

The correctness of `i[1]` / `i[2]` depends on a precondition that is not checked before access. Invalid hex also has no explicit failure state.

### B. Bounds-first parser + `std::expected`

Selected.

```cpp
decode_percent(string_view)
    -> expected<string, decode_error>
```

Malformed external data is recoverable input, so returning the error keeps the control flow explicit at the trust boundary.

The parser validates range length before lookahead and validates each nibble before conversion.

### C. Exception reporting

The study provides `alternatives::decode_or_throw`.

Exceptions can be reasonable when malformed input is exceptional for an application. At this parser/trust boundary, invalid external data is expected enough that a typed return is clearer.

### D. Lenient malformed-escape pass-through

The study provides `alternatives::decode_lenient`.

It decodes valid escapes and copies malformed sequences literally.

This can be useful for compatibility but is not selected as the secure default because it hides malformed encoding rather than forcing the caller to decide what to do with it.

## Plus-to-space semantics

The legacy decoder maps `+` to space unconditionally.

That behavior belongs to particular form/query encodings, not percent decoding in general.

The selected API therefore uses an explicit policy:

```cpp
plus_policy::literal // default
plus_policy::space
```

## Byte and text boundaries

The decoder treats escapes as bytes.

For example, `%00` creates an embedded NUL in the owned output string.

UTF-8 validation, Unicode normalization, URI component rules, and hostname processing are deliberately outside this study. Combining them with percent decoding would blur separate trust decisions.

## Ownership

Input is borrowed via `std::string_view`.

Output is owned because decoding constructs a new byte sequence and because callers commonly need the result independent of the encoded input.

## Recommendation

Use bounds-first ASCII hex parsing and explicit malformed-input results.

Keep `+` interpretation explicit and keep text/URI semantics outside the decoder.

The important refinement is small:

> prove the bytes exist before interpreting them, and reject bytes outside the declared grammar.
