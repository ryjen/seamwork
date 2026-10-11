# Percent-decoding study

Issue #65 extracts one malformed-input boundary from archived `libcoda-net::uri::decode`.

The historical decoder performs lookahead after `%` without first proving two following characters remain, and its hex helper does not reject arbitrary non-hex input.

This study does not port the URI class. It isolates percent decoding as a pure string/byte boundary.

## Selected interface

```cpp
decode_percent(
    std::string_view input,
    plus_policy plus = plus_policy::literal
) -> std::expected<std::string, decode_error>
```

Malformed external input is represented as an ordinary typed result.

## Bounds-first parsing

When the current byte is `%`, the decoder:

1. checks that three input bytes remain including the percent sign;
2. validates the high hex digit;
3. validates the low hex digit;
4. decodes exactly one byte;
5. advances exactly three input positions.

No lookahead occurs before the bounds check.

## Character classification

The hex helper uses explicit ASCII ranges:

- `0..9`;
- `A..F`;
- `a..f`.

It does not call locale-sensitive character classification functions or pass arbitrary signed `char` values into them.

## Plus policy

Percent decoding and form/query `+` handling are separate concerns.

The default preserves `+` literally. Callers that specifically want form-style semantics request `plus_policy::space`.

## Byte semantics

`%00` decodes to a NUL byte inside the returned `std::string`.

The decoder does not validate UTF-8. Percent decoding produces bytes; text-encoding policy belongs at a higher boundary.

## Alternatives

The study includes two concrete alternatives:

- exception reporting for malformed input;
- lenient pass-through of malformed escapes.

Both are less suitable for the selected trust boundary than explicit fail-closed `std::expected` results.

See [QART](qart.md).

## Promotion status

**Study only.**

No URI, parser, encoding, or networking component is promoted.
