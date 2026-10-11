# QART: does Seamwork need a formatter abstraction?

## Question

Does the useful behavior of archived `libcoda-format` still justify a project-owned formatting API in C++23?

## Historical value

The old library solved a real problem at the time: positional typed formatting with width, alignment, precision, and hexadecimal/scientific rendering.

Representative historical forms included:

```text
{0}
{0,-8}
{0:f2}
{1,4:X}
```

It also supported incremental stateful binding:

```cpp
format f("{2} saw {0} {1}!");
f.args(20, "eagles");
f.args("A bear");
```

That feature required the formatter object to own parsed specifiers, a binding cursor, replacements, reset behavior, and malformed-binding errors.

Seamwork does not require source compatibility with that grammar.

## Alternatives

### A. Direct `std::format`

Use standard compile-time format strings directly.

The current consumer needs alignment, precision, and hexadecimal output, all of which are directly expressible.

**Advantages**

- no project parser or grammar;
- compile-time checking for ordinary literal format strings;
- standard formatting vocabulary;
- no project-owned binding state;
- no formatter component lifecycle.

**Costs**

- call sites depend directly on standard formatting semantics;
- historical `coda::format` output is not guaranteed byte-for-byte compatible;
- dynamic format strings need a different API/failure path.

### B. Thin Seamwork wrapper

A wrapper can forward a checked format string and arguments to `std::format`.

The study compiles such a wrapper as an alternative.

**Result:** it adds no demonstrated semantics. It does not change ownership, error behavior, localization, allocation policy, or diagnostics. Naming a pass-through function is not enough reason to own an abstraction.

### C. `std::format_to` with caller-owned output

When output ownership/allocation matters, formatting directly into caller-owned storage makes that resource decision visible:

```cpp
std::format_to(std::back_inserter(output), "...", args...);
```

This is a useful alternative, but it is still a standard-library call rather than a Seamwork abstraction.

### D. Rebuild the custom parser

This could preserve the old grammar and incremental `.args(...)` behavior.

Rejected for current consumers.

It would reintroduce parser correctness, fuzzing surface, state, error policy, and maintenance cost solely for compatibility that Seamwork does not require.

## Runtime format strings

Compile-time format strings and runtime-supplied strings have different trust/failure characteristics.

The study uses `std::vformat` for the runtime case and verifies that malformed input produces `std::format_error`.

That distinction is clearer than making every format operation runtime-parseable by default.

If a future product requires untrusted runtime templates, it should define that trust/error/resource policy at the product boundary rather than hide it in a generic formatter wrapper.

## Historical behavior mapping

| Historical need | C++23 direction |
| --- | --- |
| positional values | standard replacement fields |
| right/left width | standard alignment/width specifiers |
| floating precision | standard precision specifiers |
| hexadecimal | standard integer presentation types |
| stream/string rendering | `std::format` or `std::format_to` |
| malformed runtime grammar | `std::vformat` + `std::format_error` |
| incremental `.args(...)` state | no current requirement; remove |
| reset/copy formatter state | disappears with stateless calls |
| project parser/fuzzer | disappears unless a custom runtime grammar returns |

Exact legacy formatting quirks are deliberately not compatibility requirements.

## Recommendation

Use **direct `std::format`** for ordinary checked formatting and **`std::format_to`** where caller-owned output matters.

Do not promote a Seamwork formatting component.

The old abstraction's most important modern lesson is that a standard facility can remove an entire responsibility set:

```text
custom grammar + parser + mutable binder + renderer + reset state
                          |
                          v
                 direct standard call
```

## Trade-off accepted

Seamwork gives up compatibility with the historical custom grammar and any unusual historical rendering quirks.

In return it stops owning parser correctness, custom formatting semantics, and stateful binding behavior.

No benchmark is included because the recommendation is based on contract reduction and maintenance surface, not a performance claim.
