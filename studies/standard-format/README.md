# Standard formatting study

Issue #48 is Seamwork's first non-Yahtsee design study.

It revisits the archived `libcoda-format` problem without porting the old formatter. The question is whether C++23 standard formatting now makes the project-specific abstraction unnecessary.

## Historical baseline

The archived `coda::format` class owned:

- a custom positional grammar;
- parser/specifier state;
- incremental `.args(...)` binding;
- width/type/precision rendering;
- stream conversion;
- reset/copy/move state;
- malformed-input rejection.

That was useful when the standard library did not provide comparable formatting facilities. It also meant the project owned parser correctness, fuzzing, formatting semantics, state management, and compatibility policy.

## Practical consumer

The study renders a diagnostic record with three ordinary requirements:

- left-aligned component name;
- floating-point precision;
- hexadecimal status code.

The direct C++23 call is intentionally unsurprising:

```cpp
std::format(
    "component={:<8} latency={:.2f}ms code={:04X}",
    component,
    latency_ms,
    code
);
```

The study also writes the same output through `std::format_to` into caller-owned storage and exercises `std::vformat` for the separate runtime-format-string case.

## Selected direction

Prefer the standard library directly.

There is currently no demonstrated Seamwork policy that justifies a formatting wrapper. A wrapper that simply forwards arguments to `std::format` adds a named dependency without changing ownership, error semantics, validation, or resource policy.

For caller-owned output, use `std::format_to` directly at the point where output ownership matters.

For genuinely runtime format strings, `std::vformat` keeps the runtime failure surface explicit through `std::format_error`.

See [QART](qart.md) for the alternatives and historical mapping.

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_standard_format
./build/dev/studies/standard-format/seamwork_standard_format_example
```

## Promotion status

**Promote nothing.**

The successful outcome is deletion of a project-specific abstraction from the modern design vocabulary unless a future consumer demonstrates a real formatting policy boundary.
